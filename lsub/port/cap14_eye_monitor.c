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

void port_cap14_eye_monitor(uint8_t *data, uint32_t data_len)
{
    uint32_t slice_size;
    uint8_t slice_ver;
    uint32_t lane_num;
    uint8_t *cur_data;
    uint64_t tmp0, tmp1;
    int off;

    static struct val_desc eye_test_desc[] = { { 0x0, "IDLE" },    { 0x1, "Abort" },        { 0x2, "In progress" },
                                               { 0x3, "Timeout" }, { 0x4, "Success Done" }, { INVALID_DESC, "" } };

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tPORT_CAP14_EYE_MONITOR: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size,
            port_cap_id);
    printf("%s", port_info->display_buf);

    if (data_len < PEM_LEN || slice_size < PEM_LEN) {
        return;
    }

    lane_num = slice_size - PEM_LEN;

    cur_data = data + PEM_CTRL;
    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT7);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tEYE MONITOR Control 1: Eye Monitor Test:0x%x", (uint8_t)tmp0);

    cur_data = data + PEM_STA;

    for (uint32_t i = 0; i < lane_num; i++) {
        tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT2);
        tmp1 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT31);
        off += sprintf(port_info->display_buf + off,
                       "\n\t\t\tLane%u State:"
                       "\n\t\t\t\tEye Monitor Test Execution Status:%s"
                       "\n\t\t\t\tEye Monitor Test Result:0x%x",
                       i, get_description(tmp0, eye_test_desc),
                       (uint16_t)tmp1);

        cur_data += CFG_DWORD_LEN;
    }

    printf("%s", port_info->display_buf);
}