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

#define LS_MAX_SLICE            8
#define INVLID_EID_MSG          "Invalid EID"
#define INVLID_CNA_MSG          "Invalid CNA"
#define INVALID_VERBOSE_MSG     "Invalid verbose"
#define TOO_MANY_SLICE_MSG      "Too many slices, only 8 of them"
#define DECIMAL_NUM             10
#define LS_VERBOSE              1
#define LS_HEX                  2

struct cmd_option {
    char character;
    int (*execute_func)(struct ub_access *uacc);
};

static int g_opt_topo;
static struct lsub_cmd_param ls_cmd;
static struct ub_entity_cfg_info cfg_info;

static char help_info[] =
"Usage: lsub [<switches>]\n"
"-t\t\tShow UB entity topo\n"
"-l\t\tShow UB entity list\n"
"-n\t\tShow numeric ID: <entity_num> Class <class_code>: Device <vendor_id>:<device_id>\n"
"-i <file>\tUse specified ID database instead of /usr/share/hwdata/ub.ids\n"
"-k\t\tShow kernel drivers handling each entity\n"
"-b <-E <eid>>\tShow specific UB bus instance by EID, print bus instance list by default\n"
"\n"
"Selection of entities and so on:\n"
"-e <entity_num>\tDisplay the entity with the specified entity number,\n"
"\t\tentity number is simplified numbering of ub entity in the ubus driver\n"
"-E <eid>\tDisplay the bus instance with the specified EID\n"
"-r <cna>\tShow UB entity route table in specified CNA\n"
"-s [[<cfg0|cfg1|port>:]<slice>][,[<slice>]]\tDisplay informations about specified slices\n"
"\n"
"Display options:\n"
"-v\t\tBe verbose\n";

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

static const char *parse_entity(char *str)
{
    uint64_t uent_num;

    uent_num = strtoul(str, NULL, HEX);
    if ((uent_num == 0) || (uent_num > MAX_UENT_NUM)) {
        return INVLID_EID_MSG;
    }

    ls_cmd.uent_num = (uint32_t)uent_num;
    return NULL;
}

static const char *parse_cna(char *str)
{
    uint64_t cna;

    cna = strtoul(str, NULL, HEX);
    if (cna > MAX_CNA) {
        return INVLID_CNA_MSG;
    }

    ls_cmd.cna = (uint32_t)cna;
    return NULL;
}

static uint8_t *parse_slice_prefix(struct lsub_cmd_param *cmd, char *str)
{
#define CFG0_PREFIX_STRING      "cfg0"
    if (strcasecmp(str, CFG0_PREFIX_STRING) == 0) {
        return cmd->cfg0_slice;
    } else {
        return NULL;
    }
}

static int is_slice_id(char *str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }

    return 1;
}

static int parse_slice_context(struct lsub_cmd_param *cmd, char *cfg_str)
{
    uint8_t *cfg_id;
    char *tmp_str;
    char *save_str;
    unsigned long slice_id;

    /* syntax: (cfg0|cfg1|port)[:slice[,slice]{0-3}] */
    tmp_str = strtok_r(cfg_str, ":", &save_str);
    if (tmp_str == NULL) {
        return -1;
    }

    cfg_id = parse_slice_prefix(cmd, tmp_str);
    if (cfg_id == NULL) {
        return 0;
    }

    /* If no slice is selected, the default slice is CFG0\CFG1\PORT BASIC. */
    if (save_str[0] == '\0') {
        cfg_id[0] = 1;
        return 0;
    }

    while ((tmp_str = strtok_r(NULL, ",", &save_str)) != NULL) {
        if (!is_slice_id(tmp_str)) {
            return -1;
        }
        slice_id = strtoul(tmp_str, NULL, DECIMAL_NUM);
        if (slice_id >= MAX_SLICE) {
            return -1;
        }
        cfg_id[slice_id] = 1;
    }

    return 0;
}

static int calc_slice_cnt(struct lsub_cmd_param *cmd)
{
    int slice_num;
    int i;

    for (i = 0; i < MAX_SLICE; i++) {
        if (cmd->cfg0_slice[i] == 1) {
            cmd->cfg0_slice_count++;
        }
    }

    slice_num = cmd->cfg0_slice_count;

    return slice_num;
}

static const char *parse_slice(char *str)
{
    const char *delim = "#";
    char *tmp_str;
    char *save_str;
    int ret_code;
    int slice_num;

    /* syntax: (cfg0|cfg1|port)[:slice[,slice]{0-7}][#(cfg0|cfg1|port)[:slice[,slice]{0-7}]] */
    for (tmp_str = strtok_r(str, delim, &save_str);
        tmp_str != NULL;
        tmp_str = strtok_r(NULL, delim, &save_str)) {
        ret_code = parse_slice_context(&ls_cmd, tmp_str);
        if (ret_code != 0) {
            return INVALID_VERBOSE_MSG;
        }
    }

    slice_num = calc_slice_cnt(&ls_cmd);
    if (slice_num > LS_MAX_SLICE) {
        return TOO_MANY_SLICE_MSG;
    }

    return NULL;
}

static int check_ls_cmd(struct ub_access *uacc, int ls_type)
{
    struct ub_entity *uent;
    int slice_num;

    if (ls_cmd.uent_num == 0) {
        return -EINVAL;
    }

    uent = ub_get_uent_by_uent_num(uacc, ls_cmd.uent_num);
    if (!uent) {
        return -EINVAL;
    }
    cfg_info.uent = uent;
    cfg_info.port_num = CFG_INVALID_PORT_NUM;

    slice_num = ls_cmd.cfg0_slice_count;
    if (ls_type == LS_VERBOSE) {
        /* If no slice is selected, the default slice is CFG0 and CFG1 BASIC. */
        if (slice_num == 0) {
            ls_cmd.cfg0_slice[0] = 1;
        }
    } else if (ls_type == LS_HEX) {
        if (slice_num != 1) {
            fprintf(stderr, "You need to select one slice.\n");
            return -EINVAL;
        }
    }

    return 0;
}

static void show_slice(struct lsub_cmd_param *cmd, struct ub_entity_cfg_info *info, uint8_t type)
{
    uint8_t *ls_data;
    int (*ls_basic)(struct ub_entity_cfg_info *info);
    int (*ls_cap)(struct ub_entity_cfg_info *info, uint32_t cap_id);
    uint32_t i;

    if (type == CFG0_SLICE_TYPE) {
        ls_data = cmd->cfg0_slice;
        ls_basic = lsub_cfg0_basic;
        ls_cap = lsub_cfg0_cap;
    } else {
        return;
    }

    for (i = 0; i < MAX_SLICE; i++) {
        if (!ls_data[i]) {
            continue;
        }
        if (i == 0) {
            ls_basic(info);
        } else {
            ls_cap(info, i);
        }
    }
}

static void show_verbose(struct ub_access *uacc)
{
    char uent_info[64];
    struct ub_entity *uent;

    if (check_ls_cmd(uacc, LS_VERBOSE)) {
        return;
    }
    uent = cfg_info.uent;

    /* show ub entity information */
    sprintf(uent_info, "<%05x> Class <%04x>: Device <%04x>:<%04x>",
            uent->uent_num, uent->class_code, uent->vendor_id, uent->device_id);
    printf("%s", uent_info);

    show_slice(&ls_cmd, &cfg_info, CFG0_SLICE_TYPE);
    printf("\n");
}

static int list_show_flag;
static int verbose_flag;
static int route_tbl_flag;
static int mue_ue_flag;
static int bi_flag;

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

static int cmd_option_entity_name(struct ub_access *uacc)
{
    const char *err_msg;

    uacc->debug("cmd_option_entity_name\n");
    if ((err_msg = parse_entity(optarg))) {
        (void)printf("%s\n", err_msg);
        return -EINVAL;
    }
    return 0;
}

static int cmd_option_slice(struct ub_access *uacc)
{
    const char *err_msg;

    uacc->debug("cmd_option_slice\n");
    if ((err_msg = parse_slice(optarg))) {
        (void)printf("%s\n", err_msg);
        return -EINVAL;
    }
    return 0;
}

static int cmd_option_verbose(struct ub_access *uacc)
{
    uacc->debug("verbose_flag enable\n");
    verbose_flag = 1;
    return 0;
}

static int cmd_option_routetbl(struct ub_access *uacc)
{
    const char *err_msg;

    if ((err_msg = parse_cna(optarg))) {
        (void)printf("%s\n", err_msg);
        return -EINVAL;
    }
    uacc->debug("route_tbl_flag enable\n");
    route_tbl_flag = 1;
    return 0;
}

static int cmd_option_mue_ue_list(struct ub_access *uacc)
{
    uacc->debug("mue_ue_flag enable\n");
    mue_ue_flag = 1;
    return 0;
}

static const char *parse_businstance(char *str)
{
    uint64_t bi_eid;

    bi_eid = strtoul(str, NULL, HEX);
    if ((bi_eid == 0) || (bi_eid > MAX_UENT_NUM)) {
        return INVLID_EID_MSG;
    }

    ls_cmd.bi_eid = (uint32_t)bi_eid;
    return NULL;
}

static int cmd_option_bi(struct ub_access *uacc)
{
    uacc->debug("bi_flag enable\n");
    bi_flag = 1;
    return 0;
}

static int cmd_option_bi_eid(struct ub_access *uacc)
{
    const char *err_msg;

    uacc->debug("cmd_option_bi_eid\n");
    if (optarg != NULL) {
        if ((err_msg = parse_businstance(optarg))) {
            (void)printf("%s\n", err_msg);
            return -EINVAL;
        }
    }

    return 0;
}

static struct cmd_option cmd_options[] = {
    { 'h', cmd_option_help },
    { 't', cmd_option_topo },
    { 'l', cmd_option_mue_ue_list },
    { 'n', cmd_option_numeric },
    { 'i', cmd_option_ids },
    { 'e', cmd_option_entity_name },
    { 's', cmd_option_slice },
    { 'v', cmd_option_verbose },
    { 'r', cmd_option_routetbl },
    { 'k', cmd_option_kernel },
    { 'b', cmd_option_bi },
    { 'E', cmd_option_bi_eid },
};

static void cmd_option_further_proc(struct ub_access *uacc)
{
    if (verbose_flag) {
        show_verbose(uacc);
    } else if (g_opt_topo) {
        show_topo();
    } else if (route_tbl_flag) {
        show_route_tbl(uacc, ls_cmd.uent_num, ls_cmd.cna);
    } else if (mue_ue_flag) {
        show_mue_ue_list(uacc, ls_cmd.uent_num);
    } else if (bi_flag) {
        show_bi_info(uacc, &ls_cmd);
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
