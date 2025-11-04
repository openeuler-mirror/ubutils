// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <string.h>
#include <errno.h>
#include "../lsub.h"
#include "cfg1.h"

#define CFG1_ERS0_ID           0
#define CFG1_ERS1_ID           1
#define CFG1_ERS2_ID           2
#define CFG1_SS_4K            4096
#define CFG1_SS_64K           65536

#define SIZE_1G_MASK            0x3FFFFFFF
#define SIZE_1G_BITS            30
#define SIZE_1M_MASK            0xFFFFF
#define SIZE_1M_BITS            20
#define SIZE_1K_MASK            0x3FF
#define SIZE_1K_BITS            10
#define CFG1_CAP_NUM 7

static const char *cfg1_cap_list[CFG1_CAP_NUM] = {"CAP0_NULL", "CAP1_DECODER",
                                                  "CAP2_JETTY", "CAP3_INT_TYPE1",
                                                  "CAP4_INT_TYPE2", "CAP5_RSVD",
                                                  "CAP6_UB_MEM "};
struct ub_entity_cfg_info *cfg1_info;

static void cfg1_cap_bitmap(uint8_t *data)
{
    int i, j;
    int off;

    /* save CAP bitmap */
    memcpy(cfg1_info->cfg1_cap_bits, data, CFG_CAP_BITMAP_LEN);

    off = sprintf(cfg1_info->display_buf, "%s", "\n\t\t\t\tCAP Bitmap: ");
    for (i = CFG_CAP_BITMAP_LEN - 1, j = 1; i >= 0; i--, j++) {
        off += sprintf(cfg1_info->display_buf + off, "%02x", cfg1_info->cfg1_cap_bits[i]);
        if (j == (CFG_CAP_BITMAP_LEN >> 1)) {
            off += sprintf(cfg1_info->display_buf + off, "%s", "\n\t\t\t\t            ");
        } else if ((j != CFG_CAP_BITMAP_LEN) && ((j % CFG_DWORD_LEN) == 0)) {
            off += sprintf(cfg1_info->display_buf + off, "%s", "-");
        }
    }

    off += sprintf(cfg1_info->display_buf + off, "\n\t\t\t\tEnabled Cfg1 Cap:\n\t\t\t\t\t");

    for (uint8_t k = 0; k < CFG1_CAP_NUM; k++) {
        if (to_1bit(data, k)) {
            off += sprintf(cfg1_info->display_buf + off, "%s ", cfg1_cap_list[k]);
        }
    }

    printf("%s", cfg1_info->display_buf);
    /**
      * In the UB protocol, bit0 of the bitmap is invalid. It is used
      * here to mark whether the bitmap has been initialized.
      */
    cfg1_info->cfg1_cap_bits[0] |= 0x1;
}

static void cfg1_supported_feature(uint8_t *data)
{
    uint8_t mg_sup, dt_jur;
    uint8_t uba_sup, ers0_sup, ers1_sup, ers2_sup;

    /* save feature bits */
    memcpy(cfg1_info->cfg1_sup_feat, data, CFG_SUP_FEATURE_LEN);

    mg_sup = to_1bit(data, CFG_BIT2);
    uba_sup = to_1bit(data, CFG_BIT5);
    ers0_sup = to_1bit(data, CFG_BIT6);
    ers1_sup = to_1bit(data, CFG_BIT7);
    ers2_sup = to_1bit(data, CFG_BIT8);

    if (ub_is_ibus_controller(cfg1_info->uent)) {
        dt_jur = to_1bit(data, CFG_BIT10);
        (void)sprintf(cfg1_info->display_buf,
            "\n\t\t\t\tSupported Feature: MGS%s UBA_Support%s ERS0S%s ERS1S%s ERS2S%s "
            "Decoder MATT jurisdiction: %s", bit_parser(mg_sup),
            bit_parser(uba_sup), bit_parser(ers0_sup), bit_parser(ers1_sup), bit_parser(ers2_sup),
            dt_jur ? "UBFM" : "UB driver");
    } else {
        (void)sprintf(cfg1_info->display_buf,
            "\n\t\t\t\tSupported Feature: MGS%s UBA_Support%s ERS0S%s ERS1S%s ERS2S%s",
            bit_parser(mg_sup), bit_parser(uba_sup), bit_parser(ers0_sup),
            bit_parser(ers1_sup), bit_parser(ers2_sup));
    }
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_ers(uint8_t *data, uint8_t rs_idx)
{
    static const uint8_t rs_sup_idx[] = {CFG_BIT6, CFG_BIT7, CFG_BIT8};
    static const uint8_t rs_ss_pos[] = {CFG1_ERS0_SS, CFG1_ERS1_SS, CFG1_ERS2_SS};
    static const uint8_t rs_sa_pos[] = {CFG1_ERS0_SA, CFG1_ERS1_SA, CFG1_ERS2_SA};
    static const uint8_t rs_uba_pos[] = {CFG1_ERS0_UBA, CFG1_ERS1_UBA, CFG1_ERS2_UBA};
    uint8_t ers_sup, uba_sup;
    uint64_t ers_ss, ers_sa, ers_uba;
    uint32_t sys_pgs;
    uint64_t size;
    int off;

    ers_sup = to_1bit(cfg1_info->cfg1_sup_feat, rs_sup_idx[rs_idx]);
    if (!ers_sup) {
        return;
    }

    sys_pgs = (to_1bit(data + CFG1_SYS_PGS - rs_ss_pos[rs_idx], CFG_BIT0) ? CFG1_SS_64K : CFG1_SS_4K);

    /* G->M->KB */
    ers_ss = ((uint64_t)to_uint32(data)) * sys_pgs;
    if (((size = (ers_ss >> SIZE_1G_BITS)) > 0) &&
        ((ers_ss & SIZE_1G_MASK) == 0)) {
        off = sprintf(cfg1_info->display_buf, "\n\t\t\t\tERS%u: Size:%luGB", rs_idx, size);
    } else if (((size = (ers_ss >> SIZE_1M_BITS)) > 0) &&
        ((ers_ss & SIZE_1M_MASK) == 0)) {
        off = sprintf(cfg1_info->display_buf, "\n\t\t\t\tERS%u: Size:%luMB", rs_idx, size);
    } else {
        size = ers_ss >> SIZE_1K_BITS;
        off = sprintf(cfg1_info->display_buf, "\n\t\t\t\tERS%u: Size:%luKB", rs_idx, size);
    }

    ers_sa = to_uint64(data + rs_sa_pos[rs_idx] - rs_ss_pos[rs_idx]);
    off += sprintf(cfg1_info->display_buf + off, " SA:0x%016lx", ers_sa);

    uba_sup = to_1bit(cfg1_info->cfg1_sup_feat, CFG_BIT5);
    if (uba_sup) {
        ers_uba = to_uint64(data + rs_uba_pos[rs_idx] - rs_ss_pos[rs_idx]);
        off += sprintf(cfg1_info->display_buf + off, " UBA:0x%016lx", ers_uba);
    } else {
        off += sprintf(cfg1_info->display_buf + off, "%s", " UBA-");
    }

    printf("%s", cfg1_info->display_buf);
}

static void cfg1_ers0(uint8_t *data)
{
    cfg1_ers(data, CFG1_ERS0_ID);
}

static void cfg1_ers1(uint8_t *data)
{
    cfg1_ers(data, CFG1_ERS1_ID);
}

static void cfg1_ers2(uint8_t *data)
{
    cfg1_ers(data, CFG1_ERS2_ID);
}

static void cfg1_elr(uint8_t *data)
{
    uint8_t *elr_data;
    uint8_t elr_ctrl;
    uint8_t elr_status;

    elr_data = data;
    elr_ctrl = to_1bit(elr_data, CFG_BIT0);
    elr_data += CFG_DWORD_LEN;
    elr_status = to_1bit(elr_data, CFG_BIT0);
    sprintf(cfg1_info->display_buf,
        "\n\t\t\t\tELR-Ctl:%u ELR-Done:%u", elr_ctrl, elr_status);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_sys_pgs(uint8_t *data)
{
    static const char* sys_pgs_desp[] = {"4K", "64K"};
    uint8_t sys_pgs;

    sys_pgs = to_1bit(data, CFG_BIT0);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tSys Page Granule Size: %s", sys_pgs_desp[sys_pgs]);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_eid_upi_tba(uint8_t *data)
{
    uint64_t addr;

    if (!ub_is_ibus_controller(cfg1_info->uent)) {
        return;
    }

    addr = to_uint64(data);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tEID-UPI Table Base Addr: 0x%016lx", addr);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_eid_upi_ten(uint8_t *data)
{
    uint32_t num;

    if (!ub_is_ibus_controller(cfg1_info->uent)) {
        return;
    }

    num = to_uint32(data);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tEID-UPI Table Entry Num: %u", num);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_class_code(uint8_t *data)
{
    uint16_t class_code;

    class_code = to_uint16(data);

    sprintf(cfg1_info->display_buf, "\n\t\t\t\tClass Code:0x%04x", class_code);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_dev_token_id(uint8_t *data)
{
    uint32_t token_id;

    token_id = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT19);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tDev token id: 0x%05x", token_id);
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_bus_en(uint8_t *data)
{
    uint8_t en;

    en = to_1bit(data, CFG_BIT0);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tBus access En%s", bit_parser(en));
    printf("%s", cfg1_info->display_buf);
}

static void cfg1_ers_access_en(uint8_t *data)
{
    uint8_t en;

    en = to_1bit(data, CFG_BIT0);
    sprintf(cfg1_info->display_buf, "\n\t\t\t\tERS Access Enable%s", bit_parser(en));
    printf("%s", cfg1_info->display_buf);
}

/**
  * list cfg1 basic configuration space
  */
int lsub_cfg1_basic(struct ub_entity_cfg_info *info)
{
    static struct ub_cfg_basic_cat cfg1_basic[] = {
        {CFG1_CAP_BITMAP,       cfg1_cap_bitmap},
        {CFG1_SUPPORT_FEATURE,  cfg1_supported_feature},
        {CFG1_ERS0_SS,          cfg1_ers0},
        {CFG1_ERS1_SS,          cfg1_ers1},
        {CFG1_ERS2_SS,          cfg1_ers2},
        {CFG1_ELR,              cfg1_elr},
        {CFG1_SYS_PGS,          cfg1_sys_pgs},
        {CFG1_EID_UPI_TBA,      cfg1_eid_upi_tba},
        {CFG1_EID_UPI_TEN,      cfg1_eid_upi_ten},
        {CFG1_CLASS_CODE,       cfg1_class_code},
        {CFG1_DEV_TOKEN_ID,     cfg1_dev_token_id},
        {CFG1_BUS_EN,           cfg1_bus_en},
        {CFG1_DEV_RS_ACESS_EN,  cfg1_ers_access_en},
        {0x0,                   NULL}
    };
    char desp[128];
    struct ub_cfg_basic_cat *cur;
    uint32_t data_len;
    uint8_t slice_ver;
    uint32_t slice_size;
    int i;
    int ret_code;

    cfg1_info = info;
    memset(cfg1_info->data_buf, 0, CFG_SLICE_LEN);
    data_len = CFG_SLICE_LEN;
    ret_code = slice_read(cfg1_info->uent, CFG1_SLICE_ADDR(0), cfg1_info->data_buf, &data_len);
    if (ret_code) {
        fprintf(stderr, "lsub error: failed to read CFG1 BASIC.\n");
        return -EIO;
    }

    slice_ver = slice_get_version(cfg1_info->data_buf);
    slice_size = slice_get_size(cfg1_info->data_buf);
    sprintf(desp, "\n\t\tCFG1_BASIC: slice[0x%x, 0x%x]", slice_ver, slice_size);
    printf("%s", desp);
    for (i = 0; cfg1_basic[i].show != NULL; i++) {
        cur = &(cfg1_basic[i]);
        /* cfg1 show make sure pos + offset < data_len */
        if (cur->pos >= data_len) {
            break;
        }
        cur->show(&(cfg1_info->data_buf[cur->pos]));
    }

    return 0;
}