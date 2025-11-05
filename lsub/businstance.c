// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ubutils.h>

#define BI_STATIC_SERVER 0
#define BI_STATIC_CLUSTER 1
#define BI_DYNAMIC_SERVER 2
#define BI_DYNAMIC_CLUSTER 3

static const char *ub_get_type_str(uint8_t type)
{
    if (type == BI_DYNAMIC_SERVER) {
        return "Dynamic_Server";
    } else if(type == BI_DYNAMIC_CLUSTER) {
        return "Dynamic_Cluster";
    } else if (type == BI_STATIC_SERVER) {
        return "Static_Server";
    } else if (type == BI_STATIC_CLUSTER) {
        return "Static_Cluster";
    }

    return "Unknown";
}

static void ub_free_bi_list(struct ub_access *uacc)
{
    struct ub_bi *bi, *next_bi;

    for (bi = uacc->bi; bi; bi = next_bi) {
        next_bi = bi->next;
        free(bi);
    }

    uacc->bi = NULL;
    uacc->tail = NULL;
}

static void ub_show_bi_list(struct ub_access *uacc)
{
    struct ub_bi *bi;

    printf("BusInstance show format: guid type eid upi\n");
    for (bi = uacc->bi; bi; bi = bi->next) {
        printf("\t%s %-15s %05x %04x\n", bi->str, ub_get_type_str(bi->type), bi->eid, bi->upi);
    }
}

static void ub_show_bi_uent(struct ub_access *uacc, uint32_t bi_eid)
{
    bool found_bi = false, found_uent = false;
    struct ub_entity *uent;
    struct ub_bi *bi;

    for (bi = uacc->bi; bi; bi = bi->next) {
        if (bi->eid == bi_eid) {
            printf("BusInstance details:\n");
            printf("\t%s %-15s %05x %04x\n", bi->str, ub_get_type_str(bi->type), bi->eid, bi->upi);
            found_bi = true;
            break;
        }
    }

    if (found_bi) {
        printf("Uents under this busInstance:\n");
        for (uent = uacc->uents; uent; uent = uent->next) {
            if (uent->bi_eid == bi_eid) {
                printf("\t%05x\n", uent->uent_num);
                found_uent = true;
            }
        }

        if (!found_uent) {
            printf("\tNULL\n");
        }
    } else {
        printf("No such busInstance found\n");
    }
}

static void ub_show_uent_bi_info(struct ub_access *uacc, uint32_t uent_num)
{
    bool found_bi = false, found_uent = false;
    struct ub_entity *uent;
    uint32_t bi_eid = 0;
    struct ub_bi *bi;

    uent = ub_get_uent_by_uent_num(uacc, uent_num);
    if (uent != NULL) {
        bi_eid = uent->bi_eid;
        found_uent = true;
    }

    if (found_uent) {
        for (bi = uacc->bi; bi; bi = bi->next) {
            if (bi->eid == bi_eid) {
                printf("This Entity's busInstance details:\n");
                printf("\t%s %-15s %05x %04x\n", bi->str, ub_get_type_str(bi->type), bi->eid, bi->upi);
                found_bi = true;
                break;
            }
        }
        if (!found_bi) {
            printf("Failed to find busInstance for this entity, somethings wrong\n");
        }
    } else {
        printf("No such entity found\n");
    }
}

void show_bi_info(struct ub_access *uacc, struct lsub_cmd_param *ls_cmd)
{
    int ret;

    ret = sysfs_get_bi(uacc);
    if (ret) {
        ub_free_bi_list(uacc);
        return;
    }

    if (ls_cmd->bi_eid != 0) {
        ub_show_bi_uent(uacc, ls_cmd->bi_eid);
    } else if (ls_cmd->uent_num != 0) {
        ub_show_uent_bi_info(uacc, ls_cmd->uent_num);
    } else {
        ub_show_bi_list(uacc);
    }

    ub_free_bi_list(uacc);
}