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
#include <ubus.h>

#define BYTE_SZ 1
#define WORD_SZ 2
#define DWORD_SZ 4
#define SIZE_8 8

static void usage(void)
{
    fprintf(stderr,
        "Usage: setub [<options>] (<uent> <reg>[=<value>]*)*\n"
        "-h      help information\n"
        "-s      Read/Write configuration space\n"
        "        <uent>: <uent_num>\n"
        "        <reg>:    <position>[.(B|W|L)]\n"
                "-b      Bus Instance operation:\n"
        "        create: create bus instance\n"
        "        destroy: destroy bus instance\n"
        "        bind: bind entity to bus instance\n"
        "        unbind: unbind entity from bus instance\n"
        "\n"
        "Bus instance options:\n"
        "-g      Bus instance guid: vendor-device-version-type-rsvd-seq_num\n"
        "-d      Entity guid: vendor-device-version-type-rsvd-seq_num\n"
        "-e      Bus Instance eid, used in cluster mode\n"
        "-u      Bus Instance upi, used in cluster mode\n");
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

#define UBUS_DEFAULT_TYPE 2

static void ub_create_bi_init(struct ubus_ioctl_bus_instance *bi, struct ub_bi_para *bi_para)
{
    bi->argsz = (uint32_t)sizeof(struct ubus_cmd_bi_create);
    bi->sub_cmd = UBUS_CMD_BI_CREATE;
    bi->create.type = UBUS_DEFAULT_TYPE;
    *((uint64_t *)bi->create.guid) = bi_para->guid.datas.guid_l;
    *((uint64_t *)bi->create.guid + 1) = bi_para->guid.datas.guid_h;
    bi->create.eid = bi_para->eid;
    bi->create.upi = bi_para->upi;
}

static void ub_destroy_bi_init(struct ubus_ioctl_bus_instance *bi, struct ub_bi_para *bi_para)
{
    bi->argsz = (uint32_t)sizeof(struct ubus_cmd_bi_destroy);
    bi->sub_cmd = UBUS_CMD_BI_DESTROY;
    *((uint64_t *)bi->destroy.guid) = bi_para->guid.datas.guid_l;
    *((uint64_t *)bi->destroy.guid + 1) = bi_para->guid.datas.guid_h;
}

static void ub_bind_bi_init(struct ubus_ioctl_bus_instance *bi, struct ub_bi_para *bi_para)
{
    bi->argsz = (uint32_t)sizeof(struct ubus_cmd_bi_bind);
    bi->sub_cmd = UBUS_CMD_BI_BIND;
    *((uint64_t *)bi->bind.instance_guid) = bi_para->guid.datas.guid_l;
    *((uint64_t *)bi->bind.instance_guid + 1) = bi_para->guid.datas.guid_h;
    *((uint64_t *)bi->bind.dev_guid) = bi_para->d_guid.datas.guid_l;
    *((uint64_t *)bi->bind.dev_guid + 1) = bi_para->d_guid.datas.guid_h;
}

static void ub_unbind_bi_init(struct ubus_ioctl_bus_instance *bi, struct ub_bi_para *bi_para)
{
    bi->argsz = (uint32_t)sizeof(struct ubus_cmd_bi_unbind);
    bi->sub_cmd = UBUS_CMD_BI_UNBIND;
    *((uint64_t *)bi->unbind.instance_guid) = bi_para->guid.datas.guid_l;
    *((uint64_t *)bi->unbind.instance_guid + 1) = bi_para->guid.datas.guid_h;
    *((uint64_t *)bi->unbind.dev_guid) = bi_para->d_guid.datas.guid_l;
    *((uint64_t *)bi->unbind.dev_guid + 1) = bi_para->d_guid.datas.guid_h;
}

static void ub_opt_bi(struct ub_bi_para *bi_para)
{
    const char *arg[UBUS_CMD_BI_NUM] = {"create", "destroy", "bind", "unbind"};
    struct ubus_ioctl_bus_instance bi = {0};
    uint32_t opt = bi_para->opt;
    int ret;
    int fd;

    switch (opt) {
        case UBUS_CMD_BI_CREATE:
            ub_create_bi_init(&bi, bi_para);
            break;
        case UBUS_CMD_BI_DESTROY:
            ub_destroy_bi_init(&bi, bi_para);
            break;
        case UBUS_CMD_BI_BIND:
            ub_bind_bi_init(&bi, bi_para);
            break;
        case UBUS_CMD_BI_UNBIND:
            ub_unbind_bi_init(&bi, bi_para);
            break;
        default:
            (void)printf("Invalid opt: %u\n", opt);
            return;
    }

    fd = open(UB_UNIFIED_BUS, O_RDWR);
    if (fd < 0) {
        (void)printf("sysfs_instance_setup failed!\n");
        return;
    }

    ret = ioctl(fd, UBUS_IOCTL_BUS_INSTANCE, &bi);
    if (ret != 0) {
        (void)printf("ioctl failed, ret is %d\n", ret);
        (void)close(fd);
        return;
    }

    if (opt == UBUS_CMD_BI_CREATE) {
        (void)printf("create BI success, guid:0x%llx%016llx eid:0x%x\n",
                      *((uint64_t *)&bi.create.guid[SIZE_8]),
                      *((uint64_t *)&bi.create.guid[0]),
                      bi.create.eid);
    } else {
        (void)printf("%s BI success\n", arg[opt]);
    }

    (void)close(fd);
}

#define GUID_PARSE_CNT 6
#define GUID_LINE_CNT 5
#define GUID_FORMAT_LEN 37
#define LINE_OFFSET_1 4
#define LINE_OFFSET_2 9
#define LINE_OFFSET_3 11
#define LINE_OFFSET_4 13
#define LINE_OFFSET_5 20

static bool check_guid_format(char *c)
{
    size_t len = strlen(c);
    char *tmp = c;
    int cnt = 0;

    if (len != GUID_FORMAT_LEN) {
        return false;
    }

    if (*(c + LINE_OFFSET_1) != '-' || *(c + LINE_OFFSET_2) != '-' ||
        *(c + LINE_OFFSET_3) != '-' || *(c + LINE_OFFSET_4) != '-' ||
        *(c + LINE_OFFSET_5) != '-') {
        return false;
    }

    while(*tmp != '\0') {
        if (*tmp == '-') {
            cnt++;
        }
        tmp++;
    }

    if (cnt != GUID_LINE_CNT) {
        return false;
    }

    return true;
}

static int parse_guid(char *c, struct ub_guid *guid)
{
    uint32_t type, vendor, rsv0, version, device;
    long long unsigned int seq_num;
    int ret;

    if (!check_guid_format(c)) {
        return -EINVAL;
    }

    ret = sscanf(c, "%x-%x-%x-%x-%x-%llx",
                 &vendor, &device, &version, &type,
                 &rsv0, &seq_num);
    if (ret != GUID_PARSE_CNT) {
        return -EINVAL;
    }

#define TYPE_MASK 0xf
#define VERSION_MASK 0xf
    guid->bits.vendor = (uint16_t)vendor;
    guid->bits.device = (uint16_t)device;
    guid->bits.type = type & TYPE_MASK;
    guid->bits.version = version & VERSION_MASK;
    guid->bits.rsvd0 = (uint32_t)rsv0;
    guid->bits.seq_num = seq_num;

    (void)printf("guid_h: %016llx guid_l: %016llx\n", guid->datas.guid_h, guid->datas.guid_l);

    return 0;
}

static int parse_eid(char *c, uint32_t *eid)
{
    if (parse_x32(c, eid) <= 0) {
        parse_err("Invalid eid value \"%s\"", c);
        return -EINVAL;
    }

    (void)printf("parse eid: 0x%x\n", *eid);
    return 0;
}

static int parse_upi(char *c, uint16_t *upi)
{
    if (parse_x16(c, upi) <= 0) {
            parse_err("Invalid upi value \"%s\"", c);
            return -EINVAL;
    }

    (void)printf("parse upi: 0x%x\n", *upi);
    return 0;
}

static int parse_cmd(char *cmd, uint32_t *opt)
{
    if (strcmp(cmd, "create") == 0) {
        *opt = UBUS_CMD_BI_CREATE;
    } else if (strcmp(cmd, "destroy") == 0) {
        *opt = UBUS_CMD_BI_DESTROY;
    } else if (strcmp(cmd, "bind") == 0) {
        *opt = UBUS_CMD_BI_BIND;
    } else if (strcmp(cmd, "unbind") == 0) {
        *opt = UBUS_CMD_BI_UNBIND;
    } else {
        return -1;
    }

    return 0;
}

static int ub_check_cluster(void)
{
    char buf[SIZE_8];
    int fd;

    if ((fd = open(UB_CLUSTER, O_RDONLY)) < 0) {
        return 0;
    }

    if (read(fd, buf, SIZE_8) <= 0) {
        (void)close(fd);
        return 0;
    }

    (void)close(fd);
    return (int)strtol(buf, NULL, 0);
}

static int get_setub_opt_d_guid(struct ub_bi_para *bi_para)
{
    if (parse_guid(optarg, &bi_para->d_guid) != 0) {
        parse_err("Invalid d_guid, %s", optarg);
        return -EINVAL;
    }
    bi_para->dguid_flag = 1;

    return 0;
}

static int get_setub_opt_eid(struct ub_bi_para *bi_para)
{
    if (parse_eid(optarg, &bi_para->eid) != 0) {
        parse_err("Invalid eid, %s", optarg);
        return -EINVAL;
    }

    return 0;
}

static int get_setub_opt_upi(struct ub_bi_para *bi_para)
{
    if (parse_upi(optarg, &bi_para->upi) != 0) {
        parse_err("Invalid upi, %s", optarg);
        return -EINVAL;
    }

    return 0;
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
            case 'b':
                if (parse_cmd(optarg, &bi_para->opt) != 0) {
                    parse_err("Invalid bus instance ops, %s", optarg);
                    return -EINVAL;
                }
                bi_para->opt_en = 1;
                break;
            case 'g':
                if (parse_guid(optarg, &bi_para->guid) != 0) {
                    parse_err("Invalid guid, %s", optarg);
                    return -EINVAL;
                }
                bi_para->guid_flag = 1;
                break;
            case 'd':
                if (get_setub_opt_d_guid(bi_para) != 0) {
                    return -EINVAL;
                }
                break;

            case 'e':
                if (get_setub_opt_eid(bi_para) != 0) {
                    return -EINVAL;
                }
                break;

            case 'u':
                if (get_setub_opt_upi(bi_para) != 0) {
                    return -EINVAL;
                }
                break;

        default:
                break;
        }
    }

    return ret;
}

static int check_bi_ops_para(struct ub_bi_para *bi_para)
{
    if (bi_para->opt == UBUS_CMD_BI_CREATE ||
        bi_para->opt == UBUS_CMD_BI_DESTROY) {
        if (bi_para->guid_flag == 0) {
            parse_err("Missing guid.\n");
            return -EINVAL;
        }
    }

    if (bi_para->opt == UBUS_CMD_BI_BIND ||
        bi_para->opt == UBUS_CMD_BI_UNBIND) {
        if (bi_para->guid_flag == 0 || bi_para->dguid_flag == 0) {
            parse_err("Missing guid or d_guid.\n");
            return -EINVAL;
        }
    }

#define UBUS_GUID_TYPE_IDEV 2
    if (ub_check_cluster() == 1) {
        if ((bi_para->opt == UBUS_CMD_BI_BIND ||
            bi_para->opt == UBUS_CMD_BI_UNBIND) &&
            bi_para->d_guid.bits.type != UBUS_GUID_TYPE_IDEV) {
            parse_err("Cluster only support bind/unbind IDEV.\n");
            return -EINVAL;
        }
    }

    return 0;
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

    if (bi_para.opt_en == 1) {
        if (check_bi_ops_para(&bi_para) != 0) {
            goto err_ub_cleanup;
        }
        ub_opt_bi(&bi_para);
    }

err_ub_cleanup:
    ub_cleanup(uacc);

    return ret;
}