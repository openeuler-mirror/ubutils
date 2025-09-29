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

static int g_opt_topo;
static struct lsub_cmd_param ls_cmd;

static char help_info[] =
"Usage: lsub [<switches>]\n"
"-t\t\tShow UB entity topo\n"
"-l\t\tShow UB entity list\n"
"-n\t\tShow numeric ID: <entity_num> Class <class_code>: Device <vendor_id>:<device_id>\n"
"-i <file>\tUse specified ID database instead of /usr/share/hwdata/ub.ids\n"
"-k\t\tShow kernel drivers handling each entity\n";

static void show_list(struct ub_access *uacc, uint32_t uent_num)
{
    struct ub_entity *uent;
    char buf[1024], numeric_name[64], *str_name;

    for (uent = uacc->uents; uent; uent = uent->next) {
        if (uent_num != 0 && uent->uent_num != uent_num) {
            continue;
        }

        if (uacc->numeric_ids) {
            sprintf(numeric_name, "Class <%04x>: Device <%04x>:<%04x>",
                    uent->class_code, uent->vendor_id, uent->device_id);
            str_name = numeric_name;
        } else {
            str_name = ub_lookup_name(uacc, buf, sizeof(buf), uent->vendor_id, uent->device_id, uent->class_code);
        }
        printf("<%05x> %s\n", uent->uent_num, str_name);
        if (uacc->kernel_driver) {
            printf("\tKernel driver in use: %s\n", uent->driver_name);
        }
    }
}

static void show_mue_ue_list(struct ub_access *uacc, uint32_t uent_num)
{
    struct ub_entity *uent;

    for (uent = uacc->uents; uent; uent = uent->next) {
        /* The specified uent needs to be displayed, but the uent is not matched */
        if (uent_num != 0 && uent->uent_num != uent_num) {
            continue;
        }

        /* All uents need to be listed, but the uent is not UBE0. */
        if (uent_num == 0 && uent->entity_idx != 0) {
            continue;
        }

        if (uent->entity_idx == 0) {
            printf("primary/mue[UBE0] <%05x>\n", uent->uent_num);
            sysfs_get_mue_list(uent);
            sysfs_get_ue_list(uent, 0);
        } else if (uent == ub_get_uent_by_uent_num(uacc, uent->primary_entity)) {
            printf("pool entity[UBE%u] <%05x>\n", uent->entity_idx, uent->uent_num);
        } else if (uent->is_mue) {
            printf("mue[UBE%u] <%05x>, primary is <%05x>\n",
                   uent->entity_idx, uent->uent_num, uent->primary_entity);
        } else {
            printf("ue[UBE%u] <%05x>, primary is <%05x>\n",
                   uent->entity_idx, uent->uent_num, uent->primary_entity);
        }
    }
}

static int list_show_flag;
static int mue_ue_flag;

static int cmd_option_help(struct ub_access *uacc)
{
    uacc->debug("cmd_option_help\n");
    (void)printf("%s", help_info);
    list_show_flag = 0;
    return 0;
}

static int cmd_option_topo(struct ub_access *uacc)
{
    uacc->debug("g_opt_topo enable\n");
    g_opt_topo = 1;
    return 0;
}

static int cmd_option_numeric(struct ub_access *uacc)
{
    uacc->debug("numeric_ids enable\n");
    uacc->numeric_ids = 1;
    return 0;
}

static int cmd_option_kernel(struct ub_access *uacc)
{
    uacc->debug("kernel_driver enable\n");
    uacc->kernel_driver = 1;
    return 0;
}

static int cmd_option_ids(struct ub_access *uacc)
{
    ub_set_ids_file_path(uacc, optarg, 0);
    return 0;
}

static int cmd_option_mue_ue_list(struct ub_access *uacc)
{
    uacc->debug("mue_ue_flag enable\n");
    mue_ue_flag = 1;
    return 0;
}

static struct cmd_option cmd_options[] = {
    { 'h', cmd_option_help },
    { 't', cmd_option_topo },
    { 'l', cmd_option_mue_ue_list },
    { 'n', cmd_option_numeric },
    { 'i', cmd_option_ids },
    { 'k', cmd_option_kernel },
};

static void cmd_option_further_proc(struct ub_access *uacc)
{
    if (g_opt_topo) {
        show_topo();
    } else if (mue_ue_flag) {
        show_mue_ue_list(uacc, ls_cmd.uent_num);
    } else if (list_show_flag) {
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

    /* Free all UB Bus Controller */
    ub_free_ubc();

    return ret;
}
