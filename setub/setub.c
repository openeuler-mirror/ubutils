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

static void usage(void)
{
    fprintf(stderr,
        "Usage: setub [<options>] (<uent> <reg>[=<value>]*)*\n"
        "-h      help information\n");
}

static int get_setub_opt(int argc, char **argv, struct ub_bi_para *bi_para,
                         struct ub_access *uacc)
{
    int ret = 0;
    int opt;

    while ((opt = getopt(argc, argv, SETUB_OPTIONS)) != -1) {
        switch (opt) {
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

    ret = get_setub_opt(argc, argv, &bi_para, uacc);
    if (ret != 0) {
        uacc->error("get_setub_opt failed, ret = %d.\n", ret);
        goto err_ub_cleanup;
    }

err_ub_cleanup:
    ub_cleanup(uacc);

    return ret;
}