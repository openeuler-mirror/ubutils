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

void cfg1_int_type1_cap(uint8_t *data, uint32_t data_len)
{
    const char *num_desp[] = {"1", "2", "4", "8", "16", "32"};
    uint8_t supported_interrupt_num;
    uint32_t interrupt_pending;
    uint8_t interrupt_en_num;
    uint32_t interrupt_data;
    uint64_t interrupt_addr;
    uint32_t interrupt_mask;
    uint32_t interrupt_id;
    uint8_t interrupt_en;
    uint32_t slice_size;
    uint8_t slice_ver;
    int off;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg1_info->display_buf,
        "\n\t\tCFG1_CAP3_INT_TYPE1: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG1_INT_TYPE1_CAP_ID);

    if (slice_size < CFG1_INT_TYPE1_LEN || data_len < CFG1_INT_TYPE1_LEN) {
        printf("%s", cfg1_info->display_buf);
        return;
    }

    interrupt_en = to_1bit(data + CFG1_INT_TYPE1_INTERRUPT_EN, CFG_BIT0);

    supported_interrupt_num = (uint8_t)to_chunkbits(data + CFG1_INT_TYPE1_SUPPORTED_INTERRUPT_NUM,
                                                    CFG_BIT0, CFG_BIT2);

    interrupt_en_num = (uint8_t)to_chunkbits(data + CFG1_INT_TYPE1_INTERRUPT_EN_NUM,
                                             CFG_BIT0, CFG_BIT2);

    interrupt_data = to_uint32(data + CFG1_INT_TYPE1_INTERRUPT_DATA);

    interrupt_addr = to_uint64(data + CFG1_INT_TYPE1_INTERRUPT_ADDR);

    interrupt_id = to_uint32(data + CFG1_INT_TYPE1_INTERRUPT_ID);

    interrupt_mask = to_uint32(data + CFG1_INT_TYPE1_INTERRUPT_MASK);

    interrupt_pending = to_uint32(data + CFG1_INT_TYPE1_INTERRUPT_PENDING);

    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\tInterrupt Enable%s"
        "\n\t\t\tSupported Interrupt Number: %s"
        "\n\t\t\tInterrupt Enable Number: %s"
        "\n\t\t\tInterrupt Data: 0x%x"
        "\n\t\t\tInterrupt Addr: 0x%lx"
        "\n\t\t\tInterrupt ID: %u"
        "\n\t\t\tInterrupt Mask: 0x%x"
        "\n\t\t\tInterrupt Pending: 0x%x",
        bit_parser(interrupt_en),
        (supported_interrupt_num < CFG_ARRAY_SIZE(num_desp, char*)) ? num_desp[supported_interrupt_num] : CFG_RESERVED,
        (interrupt_en_num < CFG_ARRAY_SIZE(num_desp, char*)) ? num_desp[interrupt_en_num] : CFG_RESERVED,
        interrupt_data, interrupt_addr, interrupt_id, interrupt_mask, interrupt_pending);
    printf("%s", cfg1_info->display_buf);
}