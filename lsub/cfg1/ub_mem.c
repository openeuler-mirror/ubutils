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
#include "cfg1.h"

void cfg1_ub_mem_cap(uint8_t *data, uint32_t data_len)
{
    uint8_t *ub_mem_data;
    uint32_t slice_size;
    uint8_t slice_ver;
    uint32_t usi_idx;
    int off;

    if (!ub_is_ibus_controller(cfg1_info->uent)) {
        return;
    }

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg1_info->display_buf,
        "\n\t\tCFG1_UB_MEM_CAP: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG1_UB_MEM_CAP_ID);

    if (slice_size < UB_MEM_LEN || data_len < UB_MEM_LEN) {
        (void)printf("%s", cfg1_info->display_buf);
        return;
    }

    ub_mem_data = data + UB_MEM_DATA;
    usi_idx = to_uint32(ub_mem_data);

    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\t\tUBMEM_USI_IDX:%u", usi_idx);
    (void)printf("%s", cfg1_info->display_buf);
}