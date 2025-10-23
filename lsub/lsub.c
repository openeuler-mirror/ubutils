// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ubutils.h>
#include <ctype.h>
#include "lsub.h"

struct cmd_option {
    char character;
    int (*execute_func)(struct ub_access *uacc);
};

static struct lsub_cmd_param ls_cmd;

static char help_info[] =
"Usage: lsub [<switches>]\n";

static void show_list(struct ub_access *uacc, uint32_t uent_num)
{
    struct ub_entity *uent;
    char buf[1024], numeric_name[64], *str_name;

    for (uent = uacc->uents; uent; uent = uent->next) {
        if (uent_num != 0 && uent->uent_num != uent_num) {
            continue;
        }

        str_name = ub_lookup_name(uacc, buf, sizeof(buf), uent->vendor_id, uent->device_id, uent->class_code);
        printf("<%05x> %s\n", uent->uent_num, str_name);
    }
}

static int list_show_flag;

static int cmd_option_help(struct ub_access *uacc)
{
    uacc->debug("cmd_option_help\n");
    (void)printf("%s", help_info);
    list_show_flag = 0;
    return 0;
}

static struct cmd_option cmd_options[] = {
    { 'h', cmd_option_help },
};

static void cmd_option_further_proc(struct ub_access *uacc)
{
    if (list_show_flag) {
        show_list(uacc, ls_cmd.uent_num);
    }
}

int main(int argc, char **argv)
{
    struct ub_access *uacc;
    int opt, ret;
    size_t i;

    /* 2 argc means help or version cmd */
    if (argc == 2 && !strcmp(argv[1], "--version")) {
        puts("lsub version " UBUTILS_VERSION);
        return 0;
    }

    uacc = ub_alloc_acc();
    if (!uacc) {
        printf("uacc alloc failed.\n");
        return -ENOMEM;
    }
    ret = ub_init(uacc);
    if (ret) {
        printf("uacc init failed, ret = %d.\n", ret);
        goto err_ub_cleanup;
    }

    list_show_flag = 1;
    while ((opt = getopt(argc, argv, LSUB_OPTIONS)) != -1) {
        ret = -EINVAL;
        for (i = 0; i < sizeof(cmd_options) / sizeof(struct cmd_option); i++) {
            if (cmd_options[i].character != (char)opt) {
                continue;
            }
            ret = cmd_options[i].execute_func(uacc);
            if (ret) {
                goto err_ub_cleanup;
            }
            break;
        }

        if (ret) {
            (void)printf("%s", help_info);
            goto err_ub_cleanup;
        }
    }

    ret = ub_scan_devices(uacc);
    if (ret) {
        goto err_ub_cleanup;
    }

    cmd_option_further_proc(uacc);

err_ub_cleanup:
    ub_cleanup(uacc);

    return ret;
}
