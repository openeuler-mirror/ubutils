// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ubutils.h>

#define BYTE_SZ 1
#define WORD_SZ 2
#define DWORD_SZ 4

static void usage(void)
{
    fprintf(stderr,
        "Usage: setub [<options>] (<uent> <reg>[=<value>]*)*\n"
        "-h      help information\n"
        "-s      Read/Write configuration space\n"
        "        <uent>: <uent_num>\n"
        "        <reg>:    <position>[.(B|W|L)]\n");
}

static void UB_PRINTF(1, 2)
parse_err(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stderr, "setub: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, ".\nTry 'setub -h' for more information.\n");
    va_end(args);
}

static int parse_uent_num(char *base, struct option *op)
{
    uint32_t num;

    if (!base) {
        parse_err("Missing uent_num");
        return -EINVAL;
    }

    if (parse_x32(base, &num) <= 0) {
        parse_err("Invalid uent_num \"%s\"", base);
        return -EINVAL;
    }

    if (num > MAX_UENT_NUM) {
        parse_err("uent_num \"%s\" out of range", base);
        return -EINVAL;
    }

    op->uent_num = num;

    return 0;
}

static int parse_position(char *base, struct option *op)
{
    unsigned long long int num;

    if (!base) {
        parse_err("Missing position");
        return -EINVAL;
    }

    if (parse_x64(base, &num) <= 0) {
        parse_err("Invalid position \"%s\"", base);
        return -EINVAL;
    }

    if (num > MAX_POSITION) {
        parse_err("position \"%s\" out of range", base);
        return -EINVAL;
    }

    op->position = num;

    return 0;
}

static int parse_value(char *base, struct option *op)
{
#define RANGE_CNT 3
    uint32_t num;
    uint32_t num_range[RANGE_CNT] = {0xFF, 0xFFFF, 0xFFFFFFFF};

    if (!base) {
        parse_err("Missing value");
        return -EINVAL;
    }

    if (parse_x32(base, &num) <= 0) {
        parse_err("Invalid value \"%s\"", base);
        return -EINVAL;
    }

    if (num > num_range[op->width >> 1]) {
        parse_err("value \"%s\" out of range", base);
        return -EINVAL;
    }

    return 0;
}

static int parse_width(char *width, struct option *op)
{
#define WIDTH_MASK 0xdf
    if (!width) {
        parse_err("Missing width");
        return -EINVAL;
    }

    if (width[1]) {
        parse_err("Invalid width \"%s\"", width);
        return -EINVAL;
    }
    switch (*(uint8_t *)width & (uint8_t)WIDTH_MASK) {
        case 'B':
            op->width = BYTE_SZ;
            break;
        case 'W':
            op->width = WORD_SZ;
            break;
        case 'L':
            op->width = DWORD_SZ;
            break;
        default:
            parse_err("Invalid width \"%c\"", *width);
            return -EINVAL;
    }

    if (op->position & (op->width - 1)) {
        parse_err("Unaligned register address");
        return -EINVAL;
    }

    return 0;
}

static int parse_op(char *base, struct option *op)
{
    char *position, *width, *value;

    /* Split the argument */
    position = base;
    if ((value = strrchr(base, '='))) {
        *value++ = 0;
    }
    if ((width = strrchr(base, '.'))) {
        *width++ = 0;
    }

    op->writeable = value ? 1 : 0;
    op->value = 0;

    if (parse_position(position, op)) {
        return -EINVAL;
    }

    if (parse_width(width, op)) {
        return -EINVAL;
    }

    if (op->writeable && parse_value(value, op)) {
        return -EINVAL;
    }

    return 0;
}

static int execute(struct ub_access *uacc, struct option *op)
{
    struct ub_entity *uent;

    for (uent = uacc->uents; uent; uent = uent->next) {
        if (uent->uent_num == op->uent_num) {
            break;
        }
    }
    if (!uent) {
        printf("Can not find uent: %05x\n", op->uent_num);
        return -EINVAL;
    }

    if (op->writeable) {
        ub_write_block(uent, op->position, (uint8_t *)&op->value, (int)op->width);
    } else {
        ub_read_block(uent, op->position, (uint8_t *)&op->value, (int)op->width);
        switch (op->width) {
            case BYTE_SZ:
                printf("0x%02x\n", op->value);
                break;
            case WORD_SZ:
                printf("0x%04x\n", op->value);
                break;
            case DWORD_SZ:
                printf("0x%08x\n", op->value);
                break;
            default:
                break;
        }
    }

    return 0;
}

static int ub_option_slot(struct ub_access *uacc, char *arg, char *ind)
{
    struct option op;
    int ret;

    ret = parse_uent_num(arg, &op);
    if (ret) {
        return ret;
    }

    ret = parse_op(ind, &op);
    if (ret) {
        return ret;
    }

    return execute(uacc, &op);
}

static int get_setub_opt(int argc, char **argv, struct ub_bi_para *bi_para,
                         struct ub_access *uacc)
{
    int ret = 0;
    int opt;

    while ((opt = getopt(argc, argv, SETUB_OPTIONS)) != -1) {
        switch (opt) {
            case 's':
                if (optind >= argc) {
                    parse_err("Missing reg, %d", optind);
                    return -EINVAL;
                }
                ret = ub_option_slot(uacc, optarg, argv[optind]);
                break;
            case 'h':
                usage();
                break;

	    default:
                break;
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    struct ub_bi_para bi_para = {0};
    struct ub_access *uacc;
    int ret;

    uacc = ub_alloc_acc();
    if (!uacc) {
        return -ENOMEM;
    }

    ret = ub_init(uacc);
    if (ret) {
        uacc->error("uacc init failed, ret = %d.\n", ret);
        goto err_ub_cleanup;
    }

    ret = ub_scan_devices(uacc);
    if (ret) {
        printf("scan devices failed, ret = %d.\n", ret);
        goto err_ub_cleanup;
    }

    ret = get_setub_opt(argc, argv, &bi_para, uacc);
    if (ret != 0) {
        uacc->error("get_setub_opt failed, ret = %d.\n", ret);
        goto err_ub_cleanup;
    }

err_ub_cleanup:
    ub_cleanup(uacc);

    return ret;
}