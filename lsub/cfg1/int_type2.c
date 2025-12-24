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

void cfg1_int_type2_cap(uint8_t *data, uint32_t data_len)
{
    uint64_t interrupt_pending_tbl_start_addr;
    uint64_t interrupt_addr_tbl_start_addr;
    uint64_t interrupt_vec_tbl_start_addr;
    uint16_t interrupt_addr_tbl_num;
    uint16_t interrupt_vec_tbl_num;
    uint8_t interrupt_mask;
    uint32_t interrupt_id;
    uint8_t interrupt_en;
    uint32_t slice_size;
    uint8_t slice_ver;
    uint8_t *pos;
    int off;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg1_info->display_buf,
        "\n\t\tCFG1_CAP4_INT_TYPE2: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG1_INT_TYPE2_CAP_ID);

    if (slice_size < CFG1_INT_TYPE2_LEN || data_len < CFG1_INT_TYPE2_LEN) {
        printf("%s", cfg1_info->display_buf);
        return;
    }

    pos = data + CFG1_INT_TYPE2_INTERRUPT_VEC_TBL_NUM;
    interrupt_vec_tbl_num = to_uint16(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_ADDR_TBL_NUM;
    interrupt_addr_tbl_num = to_uint16(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_VEC_TBL_START_ADDR;
    interrupt_vec_tbl_start_addr = to_uint64(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_ADDR_TBL_START_ADDR;
    interrupt_addr_tbl_start_addr = to_uint64(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_PENDING_TBL_START_ADDR;
    interrupt_pending_tbl_start_addr = to_uint64(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_ID;
    interrupt_id = to_uint32(pos);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_MASK;
    interrupt_mask = to_1bit(pos, CFG_BIT0);

    pos = data + CFG1_INT_TYPE2_INTERRUPT_EN;
    interrupt_en = to_1bit(pos, CFG_BIT0);

    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\tMax Index of Interrupt Vector: %u"
        "\n\t\t\tMax Index of Interrupt Address: 0x%lx"
        "\n\t\t\tInterrupt Vector Table Start Address: 0x%lx"
        "\n\t\t\tInterrupt Address Table Start Address: 0x%lx"
        "\n\t\t\tInterrupt Pending Table Start Address: 0x%lx"
        "\n\t\t\tInterrupt ID: %u\n\t\t\tInterrupt Mask%s"
        "\n\t\t\tInterrupt Enable%s",
        interrupt_vec_tbl_num, interrupt_addr_tbl_num,
        interrupt_vec_tbl_start_addr, interrupt_addr_tbl_start_addr,
        interrupt_pending_tbl_start_addr, interrupt_id,
        bit_parser(interrupt_mask), bit_parser(interrupt_en));
    printf("%s", cfg1_info->display_buf);
}