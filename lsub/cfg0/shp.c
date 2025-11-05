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

static void cfg0_shp_cap_capability(uint16_t slot_num, uint8_t *data, char *display_buf)
{
    uint8_t pp_sup, wlp_sup, plp_sup, pds_sup, pwcs_sup;
    uint16_t start_port, end_port;
    uint8_t *slot_data;

    slot_data = data;
    pp_sup = to_1bit(slot_data, CFG_BIT0);
    wlp_sup = to_1bit(slot_data, CFG_BIT1);
    plp_sup = to_1bit(slot_data, CFG_BIT2);
    pds_sup = to_1bit(slot_data, CFG_BIT3);
    pwcs_sup = to_1bit(slot_data, CFG_BIT4);
    slot_data += CFG_DWORD_LEN;
    start_port = to_uint16(slot_data);
    slot_data += CFG_WORD_LEN;
    end_port = to_uint16(slot_data);

    sprintf(display_buf,
        "\n\t\t\t\tSlot%u Cap: PPS%s WLPS%s PLPS%s PDSS%s PWCS%s"
        "\n\t\t\t\tStart-Port:%u End-Port:%u",
        slot_num, bit_parser(pp_sup), bit_parser(wlp_sup), bit_parser(plp_sup),
        bit_parser(pds_sup), bit_parser(pwcs_sup), start_port, end_port);

    printf("%s", cfg0_info->display_buf);
}

static void cfg_shp_cap_control(uint16_t slot_num, uint8_t *data, char *display_buf)
{
    uint8_t pp_ctrl, wl_ctrl, pl_ctrl, ms_ctrl, pd_ctrl, pds_ctrl, pw_ctrl;
    uint8_t *slot_data;

    slot_data = data;
    pp_ctrl = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    wl_ctrl = (uint8_t)to_chunkbits(slot_data, CFG_BIT0, CFG_BIT1);
    slot_data += CFG_DWORD_LEN;
    pl_ctrl = (uint8_t)to_chunkbits(slot_data, CFG_BIT0, CFG_BIT1);
    slot_data += CFG_DWORD_LEN;
    ms_ctrl = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    pd_ctrl = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    pds_ctrl = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    pw_ctrl = to_1bit(slot_data, CFG_BIT0);

    sprintf(display_buf,
        "\n\t\t\t\tSlot%u Ctl: PP%s WL:%s PL:%s MS%s PD%s PDS%s PW%s",
        slot_num, bit_parser(pp_ctrl), led_parse(wl_ctrl), led_parse(pl_ctrl),
        bit_parser(ms_ctrl), bit_parser(pd_ctrl), bit_parser(pds_ctrl), bit_parser(pw_ctrl));

    printf("%s", cfg0_info->display_buf);
}

static void cfg_shp_cap_status(uint16_t slot_num, uint8_t *data, char *display_buf)
{
    uint8_t pp_sta, pd_sta, pdsc_sta;
    uint8_t *slot_data;

    slot_data = data;
    pp_sta = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    pd_sta = to_1bit(slot_data, CFG_BIT0);
    slot_data += CFG_DWORD_LEN;
    pdsc_sta = to_1bit(slot_data, CFG_BIT0);

    sprintf(display_buf,
        "\n\t\t\t\tSlot%u Sta: PP%s PD%s PDSC%s",
        slot_num, bit_parser(pp_sta), bit_parser(pd_sta), bit_parser(pdsc_sta));

    printf("%s", cfg0_info->display_buf);
}

void cfg0_shp_cap(uint8_t *data, uint32_t data_len)
{
    uint32_t slice_size;
    uint8_t *slot_data;
    uint16_t slot_num;
    uint8_t slice_ver;
    uint16_t i;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    slot_num = to_uint16(data + SHP_SLOT_NUM);
    sprintf(cfg0_info->display_buf,
        "\n\t\tCFG0_SHP_CAP: slice[0x%x, 0x%x] id[%d] Slot Number:%u",
        slice_ver, slice_size, CFG0_SHP_CAP_ID, slot_num);

    printf("%s", cfg0_info->display_buf);

    if ((slot_num == 0) || (data_len < SHP_SLOT_LEN(slot_num - 1))) {
        return;
    }

    for (i = 0; i < slot_num; i++) {
        slot_data = data + SHP_SLOT_DATA_OFF(i);
        /* Capability */
        cfg0_shp_cap_capability(i, slot_data, cfg0_info->display_buf);
        slot_data += SHP_SLOT_DATA_CAP_LEN;
        /* Control */
        cfg_shp_cap_control(i, slot_data, cfg0_info->display_buf);
        slot_data += SHP_SLOT_DATA_CTRL_LEN;
        /* Status */
        cfg_shp_cap_status(i, slot_data, cfg0_info->display_buf);
    }
}