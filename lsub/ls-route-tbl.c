// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ubutils.h>
#include "lsub.h"

static void show_default_route_tbl(struct ub_entity *uent, uint16_t port_nums)
{
    uint32_t *default_tlb;
    uint32_t ret;
    int ebw;
    int i;

    ebw = (int)ROUTE_TBL_ROUTE_TBL_EBW(port_nums);
    default_tlb = (uint32_t *)calloc((size_t)ebw, (size_t)sizeof(uint32_t));
    if (!default_tlb) {
        printf("RouteTable Default table alloc failed.\n");
        return;
    }

    ret = (uint32_t)ub_read_block(uent, ROUTE_TBL_SLICE + ROUTE_TBL_DEFAULT_ROUTE_TBL,
                                  (uint8_t *)default_tlb, (int)sizeof(uint32_t) * ebw);
    if (ret) {
        printf("RouteTable default table read failed.\n");
        free(default_tlb);
        return;
    }

    printf("Default RouteTable:\n");
    for (i = 0; i < ebw; i++) {
        printf("\t\t[%04x] %08x\n", (uint32_t)(i * (int)sizeof(uint32_t)), default_tlb[i]);
    }
    free(default_tlb);
}

static void show_exact_route_tbl(struct ub_entity *uent, uint16_t port_nums,
                                 uint32_t cna)
{
    uint32_t *exact_tlb;
    uint32_t j;
    uint32_t ebw;
    uint32_t ret;
    uint32_t ers;

    ret = (uint32_t)ub_read_block(uent, (uint64_t)(ROUTE_TBL_SLICE +
                                  ROUTE_TBL_EXACT_ROUTE_SUP),
                                  (uint8_t *)&ers, sizeof(ers));
    if (ret) {
        printf("RouteTable exact table info read failed.\n");
        return;
    }

    if (to_1bit((uint8_t *)&ers, CFG_BIT0) == 0 ||
        to_1bit((uint8_t *)&ers, CFG_BIT16) == 0) {
        return;
    }

    ebw = (uint32_t)ROUTE_TBL_ROUTE_TBL_EBW(port_nums);
    exact_tlb = (uint32_t *)calloc(ebw, sizeof(uint32_t));
    if (!exact_tlb) {
        printf("RouteTable Exact table alloc failed.\n");
        return;
    }
    ret = (uint32_t)ub_read_block(uent, ROUTE_TBL_SLICE +
                                  ROUTE_TBL_ROUTE_TBL_ENTRY(port_nums, cna),
                                  (uint8_t *)exact_tlb, (int)(sizeof(uint32_t) * ebw));
    if (ret) {
        printf("RouteTable exact table entry read failed.\n");
        free(exact_tlb);
        return;
    }

    printf("Exact RouteTable:\n");
    for (j = 0; j < ebw; j++) {
        printf("\tEntry%04x\t[%04lx] %08x\n", cna, j * sizeof(uint32_t), exact_tlb[j]);
    }
    free(exact_tlb);
}

void show_route_tbl(struct ub_access *uacc, uint32_t uent_num, uint32_t cna)
{
    struct ub_entity *uent;
    uint16_t port_nums, entry_num;
    uint32_t route_tbl_sup;
    uint32_t ret;

    if (uent_num == 0) {
        return;
    }

    uent = ub_get_uent_by_uent_num(uacc, uent_num);
    if (!uent) {
        return;
    }

    ret = (uint32_t)ub_read_block(uent, CFG0_SLICE_ADDR(0) + CFG0_SUPPORT_FEATURE,
                                  (uint8_t *)&route_tbl_sup, sizeof(route_tbl_sup));
    route_tbl_sup = to_1bit((uint8_t *)&route_tbl_sup, CFG_BIT4);
    if (ret || !route_tbl_sup) {
        printf("RouteTable not support in this UB entity %05x.\n", uent_num);
        return;
    }

    ret = (uint32_t)ub_read_block(uent, CFG0_SLICE_ADDR(0) + CFG0_PORT_NUMBER,
                                  (uint8_t *)&port_nums, sizeof(port_nums));
    if (ret) {
        printf("port num read failed.\n");
        return;
    }

    ret = (uint32_t)ub_read_block(uent, ROUTE_TBL_SLICE + ROUTE_TBL_NUM_OF_TLB_ENTRY,
                                  (uint8_t *)&entry_num, sizeof(entry_num));
    if (ret) {
        printf("RouteTable basic info read failed.\n");
        return;
    }

    if (cna >= entry_num) {
        printf("Expected value error, cna(%04x) >= entry_num(%04x).\n", cna, entry_num);
        return;
    }

    show_default_route_tbl(uent, port_nums);
    show_exact_route_tbl(uent, port_nums, cna);
}