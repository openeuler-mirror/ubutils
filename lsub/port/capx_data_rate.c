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
#include "port.h"

static void port_capx_data_rate_ctrl(uint8_t *data)
{
    uint8_t *cur_data;
    int off, i;

    off = sprintf(port_info->display_buf, "\n\t\t\tDATA_RATE%u Control 1:", port_cap_id - CFG_BIT3);

    cur_data = data + PDR_CTRL;
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tLocal_TX_Preset_Lane0~7(hex)"
                                                 "\n\t\t\t\t\t");
    for (i = 0; i < CFG_BIT8; i++) {
        off += sprintf(port_info->display_buf + off, " %x", (uint32_t)(*(cur_data + i) & 0xf));
    }

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tRemote_TX_Preset_Lane0~7(hex)"
                                                 "\n\t\t\t\t\t");
    for (i = 0; i < CFG_BIT8; i++) {
        off += sprintf(port_info->display_buf + off, " %x", (uint32_t)(*(cur_data + i) >> CFG_BIT4));
    }

    printf("%s", port_info->display_buf);
}

static void port_capx_data_rate_sta(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0, btmp1, btmp2, btmp3;
    uint64_t tmp0, tmp1;
    int off, i;

    off = sprintf(port_info->display_buf, "\n\t\t\tDATA_RATE%u State 1:", port_cap_id - CFG_BIT3);

    cur_data = data + PDR_STA;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    btmp1 = to_1bit(cur_data, CFG_BIT1);
    btmp2 = to_1bit(cur_data, CFG_BIT2);
    btmp3 = to_1bit(cur_data, CFG_BIT3);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tEqualization Complete%s\n\t\t\t\tEqualization Coarsetune phase Complete%s"
                   "\n\t\t\t\tEqualization Coarsetune Phase Successful%s"
                   "\n\t\t\t\tEqualization Passive Phase Successful%s",
                   bit_parser(btmp0), bit_parser(btmp1), bit_parser(btmp2), bit_parser(btmp3));

    btmp0 = to_1bit(cur_data, CFG_BIT4);
    btmp1 = to_1bit(cur_data, CFG_BIT5);
    btmp2 = to_1bit(cur_data, CFG_BIT6);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tEqualization Active Phase Successful%s\n\t\t\t\tLink Equalization Request%s"
                   "\n\t\t\t\tTransmitter Precoding On Request%s",
                   bit_parser(btmp0), bit_parser(btmp1), bit_parser(btmp2));

    btmp0 = to_1bit(cur_data, CFG_BIT8);
    btmp1 = to_1bit(cur_data, CFG_BIT9);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCurrent precode state: TX%s RX%s", bit_parser(btmp0),
                   bit_parser(btmp1));

    tmp0 = to_chunkbits(cur_data, CFG_BIT10, CFG_BIT13);
    tmp1 = to_chunkbits(cur_data, CFG_BIT14, CFG_BIT17);
    btmp0 = to_1bit(cur_data, CFG_BIT18);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tCurrent FEC Mode:%s\n\t\t\t\tCurrent BCRC Mode:%s"
                   "\n\t\t\t\tCurrent FEC Interleave state:%s",
                   get_description(tmp0, fec_desc), get_description(tmp1, bcrc_desc),
                   bit_parser(btmp0));

    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\tDATA_RATE%u State 2:\n\t\t\t\t"
                   "Current_Local_TX_Preset_Lane0~7(hex)\n\t\t\t\t\t",
                   port_cap_id - CFG_BIT3);
    cur_data += CFG_DWORD_LEN;
    for (i = 0; i < CFG_DWORD_LEN; i++) {
        off += sprintf(port_info->display_buf + off, " %x %x",
                       (uint32_t)(*cur_data & 0xf), (uint32_t)(*cur_data >> CFG_BIT4));
        cur_data += CFG_BYTE_LEN;
    }

    printf("%s", port_info->display_buf);
}

void port_capx_data_rate(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf,
            "\n\t\tPORT_CAP%u_DATA_RATE%u: slice[0x%x, 0x%x] id[%u]",
            port_cap_id, port_cap_id - CFG_BIT3, slice_ver,
            slice_size, port_cap_id);
    printf("%s", port_info->display_buf);

    if (data_len < PDR_LEN) {
        return;
    }

    port_capx_data_rate_ctrl(data);
    port_capx_data_rate_sta(data);
}