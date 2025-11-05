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

void cfg1_jetty_cap(uint8_t *data, uint32_t data_len)
{
    static const char* jfx_max_num_desp[] = {
        "8",    "16",   "32",   "64",   "128",  "256",  "512",  "1K",
        "2K",   "4K",   "8K",   "16K",  "32K",  "64K",  "128K", "256K"
    };
    uint8_t *jetty_data;
    uint16_t eventq_num;
    uint8_t jetty_max_num, jfs_max_num, jfc_max_num, jfr_max_num;
    uint8_t slice_ver;
    uint32_t slice_size;
    int off;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg1_info->display_buf,
        "\n\t\tCFG1_JETTY_CAP: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG1_JETTY_CAP_ID);

    if (slice_size < JETTY_LEN || data_len < JETTY_LEN) {
        printf("%s", cfg1_info->display_buf);
        return;
    }

    jetty_data = data + JETTY_DATA;
    jetty_max_num = (uint8_t)to_chunkbits(jetty_data, CFG_BIT0, CFG_BIT3);
    jfs_max_num = jetty_max_num;
    jfc_max_num = (uint8_t)to_chunkbits(jetty_data, CFG_BIT4, CFG_BIT7);
    jfr_max_num = (uint8_t)to_chunkbits(jetty_data, CFG_BIT8, CFG_BIT11);
    eventq_num = (uint16_t)to_chunkbits(jetty_data, CFG_BIT16, CFG_BIT31);

    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\t\tJetty Max:%s JFS Max:%s JFC Max:%s JFR Max:%s Eventq Num:%u",
        jfx_max_num_desp[jetty_max_num], jfx_max_num_desp[jfs_max_num], jfx_max_num_desp[jfc_max_num],
        jfx_max_num_desp[jfr_max_num], eventq_num);
    printf("%s", cfg1_info->display_buf);
}