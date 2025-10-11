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

static int err_msg_que_ctl(uint8_t *data, char *display_buf)
{
    uint8_t tmp0, tmp1, tmp2;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = to_1bit(data, CFG_BIT1);
    tmp2 = to_1bit(data, CFG_BIT2);

    return sprintf(display_buf,
        "\n\t\t\tError Message Queue Control:"
        "\n\t\t\t\tCorrectable Error Report Enable%s"
        "\n\t\t\t\tNon-Fatal UnCorrectable Error Report Enable%s"
        "\n\t\t\t\tFatal UnCorrectable Error Report Enable%s",
        bit_parser(tmp0), bit_parser(tmp1), bit_parser(tmp2));
}

static int emq_cor_err_ctl(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint8_t tmp0;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT16, CFG_BIT31);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Correctable Error Control:"
        "\n\t\t\t\tCorrectable Error Mask%s"
        "\n\t\t\t\tCorrectable Error Queue Depth:0x%x",
        bit_parser(tmp0), tmp1);
}

static int emq_cor_err_sta(uint8_t *data, char *display_buf)
{
    uint8_t tmp0, tmp1;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = to_1bit(data, CFG_BIT8);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Correctable Error Status:"
        "\n\t\t\t\tCorrectable Error detected%s"
        "\n\t\t\t\tCorrectable Error Overflow%s",
        bit_parser(tmp0), bit_parser(tmp1));
}

static int emq_uncor_nf_err_ctl(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint8_t tmp0;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT16, CFG_BIT31);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Non-Fatal Uncorrectable Error Control:"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Mask%s"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Queue Depth:0x%x",
        bit_parser(tmp0), tmp1);
}

static int emq_uncor_nf_err_sta(uint8_t *data, char *display_buf)
{
    uint8_t tmp0, tmp1;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = to_1bit(data, CFG_BIT8);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Non-Fatal Uncorrectable Error Status:"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error detected%s"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Overflow%s",
        bit_parser(tmp0), bit_parser(tmp1));
}

static int emq_uncor_f_err_ctl(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint8_t tmp0;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT16, CFG_BIT31);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Fatal Uncorrectable Error Control:"
        "\n\t\t\t\tFatal Uncorrectable Error Mask%s"
        "\n\t\t\t\tFatal Uncorrectable Error Queue Depth:0x%x",
        bit_parser(tmp0), tmp1);
}

static int emq_uncor_f_err_sta(uint8_t *data, char *display_buf)
{
    uint8_t tmp0, tmp1;

    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    tmp0 = to_1bit(data, CFG_BIT0);
    tmp1 = to_1bit(data, CFG_BIT8);

    return sprintf(display_buf,
        "\n\t\t\tEMQ Fatal Uncorrectable Error Status:"
        "\n\t\t\t\tFatal Uncorrectable Error detected%s"
        "\n\t\t\t\tFatal Uncorrectable Error Overflow%s",
        bit_parser(tmp0), bit_parser(tmp1));
}

static inline int cor_err_con_point(uint8_t *data, char *display_buf)
{
    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    return sprintf(display_buf,
        "\n\t\t\t\tCorrectable Error Consumer Pointer:0x%x",
        to_uint16(data));
}

static inline int cor_err_pro_point(uint8_t *data, char *display_buf)
{
    return sprintf(display_buf,
        "\n\t\t\t\tCorrectable Error Producer Pointer:0x%x",
        to_uint16(data));
}

static int cor_err_info(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint32_t tmp;
    uint8_t tmp2;

    tmp = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT32, CFG_BIT43);
    tmp2 = to_1bit(data, CFG_BIT63);

    return sprintf(display_buf,
        "\n\t\t\tCorrectable Error Information:"
        "\n\t\t\t\tCorrectable Error Source Primary CNA: 0x%06x"
        "\n\t\t\t\tCorrectable Error Source Port Index: 0x%x"
        "\n\t\t\t\tCorrectable Error Level: %s",
        tmp, tmp1, tmp2 ? "Device" : "Port");
}

static inline int uncor_nf_err_con_point(uint8_t *data, char *display_buf)
{
    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    return sprintf(display_buf,
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Consumer Pointer:0x%x",
        to_uint16(data));
}

static inline int uncor_nf_err_pro_point(uint8_t *data, char *display_buf)
{
    return sprintf(display_buf,
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Producer Pointer:0x%x",
        to_uint16(data));
}

static int uncor_nf_err_info(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint32_t tmp;
    uint8_t tmp2;

    tmp = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT32, CFG_BIT43);
    tmp2 = to_1bit(data, CFG_BIT63);

    return sprintf(display_buf,
        "\n\t\t\tNon-Fatal Uncorrectable Error Information:"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Source Primary CNA: 0x%06x"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Source Port Index: 0x%x"
        "\n\t\t\t\tNon-Fatal Uncorrectable Error Level: %s",
        tmp, tmp1, tmp2 ? "Device" : "Port");
}

static inline int uncor_f_err_con_point(uint8_t *data, char *display_buf)
{
    if (!ub_is_primary(cfg0_info->uent)) {
        return 0;
    }

    return sprintf(display_buf,
        "\n\t\t\t\tFatal Uncorrectable Error Consumer Pointer:0x%x",
        to_uint16(data));
}

static inline int uncor_f_err_pro_point(uint8_t *data, char *display_buf)
{
    return sprintf(display_buf,
        "\n\t\t\t\tFatal Uncorrectable Error Producer Pointer:0x%x",
        to_uint16(data));
}

static int uncor_f_err_info(uint8_t *data, char *display_buf)
{
    uint16_t tmp1;
    uint32_t tmp;
    uint8_t tmp2;

    tmp = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);
    tmp1 = (uint16_t)to_chunkbits(data, CFG_BIT32, CFG_BIT43);
    tmp2 = to_1bit(data, CFG_BIT63);

    return sprintf(display_buf,
        "\n\t\t\tFatal Uncorrectable Error Information:"
        "\n\t\t\t\tFatal Uncorrectable Error Source Primary CNA: 0x%06x"
        "\n\t\t\t\tFatal Uncorrectable Error Source Port Index: 0x%x"
        "\n\t\t\t\tFatal Uncorrectable Error Level: %s",
        tmp, tmp1, tmp2 ? "Device" : "Port");
}

void cfg0_emq_cap(uint8_t *data, uint32_t data_len)
{
    uint32_t slice_size;
    uint8_t slice_ver;
    int off;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg0_info->display_buf,
        "\n\t\tCFG0_EMQ_CAP: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG0_EMQ_CAP_ID);

    if (data_len < EMQ_LEN) {
        (void)printf("%s", cfg0_info->display_buf);
        return;
    }

    off += err_msg_que_ctl(data + ERR_MSG_QUE_CTL, cfg0_info->display_buf + off);
    off += emq_cor_err_ctl(data + EMQ_COR_ERR_CTL, cfg0_info->display_buf + off);
    off += emq_cor_err_sta(data + EMQ_COR_ERR_STA, cfg0_info->display_buf + off);
    off += emq_uncor_nf_err_ctl(data + EMQ_UNCOR_NF_ERR_CTL, cfg0_info->display_buf + off);
    off += emq_uncor_nf_err_sta(data + EMQ_UNCOR_NF_ERR_STA, cfg0_info->display_buf + off);
    off += emq_uncor_f_err_ctl(data + EMQ_UNCOR_F_ERR_CTL, cfg0_info->display_buf + off);
    off += emq_uncor_f_err_sta(data + EMQ_UNCOR_F_ERR_STA, cfg0_info->display_buf + off);
    off += cor_err_con_point(data + COR_ERR_CON_POINT, cfg0_info->display_buf + off);
    off += cor_err_pro_point(data + COR_ERR_PRO_POINT, cfg0_info->display_buf + off);
    off += cor_err_info(data + COR_ERR_INFO, cfg0_info->display_buf + off);
    off += uncor_nf_err_con_point(data + UNCOR_NF_ERR_CON_POINT, cfg0_info->display_buf + off);
    off += uncor_nf_err_pro_point(data + UNCOR_NF_ERR_PRO_POINT, cfg0_info->display_buf + off);
    off += uncor_nf_err_info(data + UNCOR_NF_ERR_INFO, cfg0_info->display_buf + off);
    off += uncor_f_err_con_point(data + UNCOR_F_ERR_CON_POINT, cfg0_info->display_buf + off);
    off += uncor_f_err_pro_point(data + UNCOR_F_ERR_PRO_POINT, cfg0_info->display_buf + off);
    off += uncor_f_err_info(data + UNCOR_F_ERR_INFO, cfg0_info->display_buf + off);

    (void)printf("%s", cfg0_info->display_buf);
}