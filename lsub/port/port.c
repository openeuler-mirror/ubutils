// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include "port.h"

struct ub_entity_cfg_info *port_info;
unsigned int port_cap_id;
struct val_desc fec_desc[FEC_DESC_NUM] = { { 0x0, "FEC Bypass" },
                                       { 0x1, "Mode0 RS(128, 120,T=2)" },
                                       { 0x2, "Mode1 RS(128, 120,T=4)" },
                                       { INVALID_DESC, "" } };
struct val_desc bcrc_desc[BCRC_DESC_NUM] = { { 0x0, "No BCRC" }, { 0x1, "CRC30" },
                                        { INVALID_DESC, "" } };


const char *get_description(uint64_t value, struct val_desc *table)
{
    int i;

    for (i = 0; table[i].val != INVALID_DESC; i++) {
        if (value == table[i].val) {
            return table[i].desc;
        }
    }

    return CFG_RESERVED;
}