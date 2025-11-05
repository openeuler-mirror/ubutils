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

void port_cap2_link_log(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver, btmp0, btmp1, tmp;
    uint8_t *cur_data;
    uint32_t slice_size;
    uint32_t ltb_log;
    int off, i;

    static struct val_desc err_log_desc[] = { { 0x0, "Flit Err" }, { 0x1, "DLTB Err" }, { 0x2, "CLTB Err" },
                                              { 0x3, "ELTB Err" }, { 0x4, "RLTB Err" }, { INVALID_DESC, "" } };

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tport cap2 link log: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size,
            port_cap_id);
    printf("%s", port_info->display_buf);

    if (data_len < PLL_LEN) {
        return;
    }

    btmp0 = to_1bit(data + PLL_ERR_LOG, CFG_BIT0);
    btmp1 = to_1bit(data + PLL_ERR_COUNT_CTRL, CFG_BIT0);
    off = sprintf(port_info->display_buf, "\n\t\t\tFlit Err Log Valid%s Flit Err Count Enable%s", bit_parser(btmp0),
            bit_parser(btmp1));

    cur_data = data + PLL_ERR_COUNT_STA;

    for (i = 0; i <= LTB_LOG_COUNT; i++) {
        ltb_log = to_uint32(cur_data);
        off += sprintf(port_info->display_buf + off, "\n\t\t\t%s Count:%u",
                       get_description((uint64_t)i, err_log_desc), (uint32_t)ltb_log);
        cur_data += CFG_DWORD_LEN;
    }

    for (i = 0; i <= LTB_LOG_COUNT; i++) {
        tmp = to_1bit(data + PLL_LTB_ERR_CTRL, (uint8_t)i);
        off += sprintf(port_info->display_buf + off, "\n\t\t\tLTB Error Log %d Count enable%s", i, bit_parser(tmp));
    }

    printf("%s", port_info->display_buf);
}