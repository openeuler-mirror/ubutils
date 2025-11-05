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

void port_cap15_qdlws(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;
    uint8_t *cur_data;
    uint8_t btmp0, btmp1, btmp2;
    uint64_t tmp0;
    int off;

    static struct val_desc qdlws_desc[] = {
        { 0x0, "IDLE" },     { 0x1, "NAK" }, { 0x2, "In progress" }, { 0x3, "Timeout" }, { 0x4, "Successful Done" },
        { INVALID_DESC, "" }
    };

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tport QDLWS: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size, port_cap_id);
    printf("%s", port_info->display_buf);

    if (slice_size < PQDLWS_LEN || data_len < PQDLWS_LEN) {
        return;
    }

    cur_data = data + PQDLWS_CAP;
    btmp0 = to_1bit(cur_data, CFG_BIT0);
    off = sprintf(port_info->display_buf, "\n\t\t\t\tAsymmetry Link width change support%s", bit_parser(btmp0));

    cur_data = data + PQDLWS_CTRL;
    btmp0 = to_1bit(cur_data, CFG_BIT0);
    btmp1 = to_1bit(cur_data, CFG_BIT1);
    btmp2 = to_1bit(cur_data, CFG_BIT2);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tGlobal Enable for QDLWS%s TX QDLWS%s RX QDLWS%s",
                   bit_parser(btmp0), bit_parser(btmp1), bit_parser(btmp2));

    cur_data = data + PQDLWS_STA;
    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT2);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tQDLWS Execution status:%s",
                   get_description(tmp0, qdlws_desc));

    printf("%s", port_info->display_buf);
}