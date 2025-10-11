// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <lsub-regs.h>
#include "../lsub.h"
#include "cfg0.h"

static int err_info_ctl_reg(uint8_t *data, char *display_buf)
{
    uint16_t err_info_que_dep;
    uint8_t err_info_of;

    err_info_of = to_1bit(data, CFG_BIT0);
    err_info_que_dep = (uint16_t)to_chunkbits(data, CFG_BIT16, CFG_BIT31);

    return sprintf(display_buf,
        "\n\t\t\tErr Info Ctl Reg:"
        "\n\t\t\t\tError information overflow%s Error information queue depth: 0x%x",
        bit_parser(err_info_of), err_info_que_dep);
}

static int err_info_csm_point(uint8_t *data, char *display_buf)
{
    uint8_t err_info_csm_point;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    err_info_csm_point = to_1bit(data, CFG_BIT0);

    return sprintf(display_buf,
        "\n\t\t\t\tError Information Consumer Pointer%s",
        bit_parser(err_info_csm_point));
}

static int err_info_pdc_point(uint8_t *data, char *display_buf)
{
    uint8_t err_info_pdc_point;

    err_info_pdc_point = to_1bit(data, CFG_BIT0);

    return sprintf(display_buf,
        "\n\t\t\t\tError Information Producer Pointer%s",
        bit_parser(err_info_pdc_point));
}

static int spc_def_err_info(uint8_t *data, char *display_buf)
{
#define ERR_INFO_NUM 13
    long long unsigned int err_info[ERR_INFO_NUM];
    uint8_t spc_def_err_level;
    uint32_t spc_def_err_info;
    uint8_t *err_data;
    int off = 0;
    int i;

    err_data = data;
    spc_def_err_level = to_1bit(err_data, CFG_BIT63);
    spc_def_err_info = (uint32_t)to_chunkbits(err_data, CFG_BIT0, CFG_BIT31);
    off += sprintf(display_buf,
        "\n\t\t\t\tspc_def_err_level: %s spc_def_err_info: 0x%x",
        spc_def_err_level ? "device err" : "port err",
        spc_def_err_info);

    for (i = 0; i < ERR_INFO_NUM; i++) {
        err_data += CFG_QWORD_LEN;
        err_info[i] = to_uint64(err_data);
        off += sprintf(display_buf + off,
            "\n\t\t\t\terr_info[%d]: 0x%016llx",
            i, err_info[i]);
    }

    return off;
}

static int vdr_def_err_info(uint8_t *data, char *display_buf)
{
    long long unsigned int vendor_def_err_info;

    vendor_def_err_info = to_uint64(data);

    return sprintf(display_buf,
        "\n\t\t\t\tVendor Defined Error Information: 0x%016llx",
        vendor_def_err_info);
}

void cfg0_err_info_cap(uint8_t *data, uint32_t data_len)
{
    uint32_t slice_size;
    uint8_t slice_ver;
    int off;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg0_info->display_buf,
        "\n\t\tCFG0_ERR_INFO_CAP_ID: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG0_ERR_INFO_CAP_ID);

    if (data_len < ERR_INFO_LEN) {
        (void)printf("%s", cfg0_info->display_buf);
        return;
    }

    off += err_info_ctl_reg(data + ERR_INFO_CTL_REG, cfg0_info->display_buf + off);
    off += err_info_csm_point(data + ERR_INFO_CSM_POINT, cfg0_info->display_buf + off);
    off += err_info_pdc_point(data + ERR_INFO_PDC_POINT, cfg0_info->display_buf + off);
    off += spc_def_err_info(data + SPC_DEF_ERR_INFO, cfg0_info->display_buf + off);
    off += vdr_def_err_info(data + VDR_DEF_ERR_INFO, cfg0_info->display_buf + off);

    (void)printf("%s", cfg0_info->display_buf);
}