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

void port_cap20_lmsm_st(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;
    uint8_t *cur;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tport LMSM state: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size,
            port_cap_id);
    printf("%s", port_info->display_buf);

    if (data_len < PLMSM_ST_LEN) {
        return;
    }

    cur = data + PLMSM_ST_CTRL;
    (void)sprintf(port_info->display_buf, "\n\t\t\tLMSM ST Control 1:"
                                          "\n\t\t\t\tProbe.Wait%s\n\t\t\t\tProbe.Confirm%s"
                                          "\n\t\t\t\tRXEQ%s\n\t\t\t\tDiscovery.Active%s"
                                          "\n\t\t\t\tDiscovery.Confirm%s\n\t\t\t\tConfig.Active%s"
                                          "\n\t\t\t\tConfig.Check%s\n\t\t\t\tConfig.Confirm%s"
                                          "\n\t\t\t\tSent.NullPacket%s\n\t\t\t\tRetrain.Active%s"
                                          "\n\t\t\t\tRetrain.Confirm%s\n\t\t\t\tChange.Speed%s"
                                          "\n\t\t\t\tEQ.Coarse.Active%s\n\t\t\t\tEQ.Active%s"
                                          "\n\t\t\t\tEQ.Passive%s\n\t\t\t\tEQ.Coarse.Confirm%s",
                                          bit_parser(to_1bit(cur, CFG_BIT0)), bit_parser(to_1bit(cur, CFG_BIT1)),
                                          bit_parser(to_1bit(cur, CFG_BIT2)), bit_parser(to_1bit(cur, CFG_BIT3)),
                                          bit_parser(to_1bit(cur, CFG_BIT4)), bit_parser(to_1bit(cur, CFG_BIT5)),
                                          bit_parser(to_1bit(cur, CFG_BIT6)), bit_parser(to_1bit(cur, CFG_BIT7)),
                                          bit_parser(to_1bit(cur, CFG_BIT8)), bit_parser(to_1bit(cur, CFG_BIT10)),
                                          bit_parser(to_1bit(cur, CFG_BIT11)), bit_parser(to_1bit(cur, CFG_BIT13)),
                                          bit_parser(to_1bit(cur, CFG_BIT14)), bit_parser(to_1bit(cur, CFG_BIT15)),
                                          bit_parser(to_1bit(cur, CFG_BIT16)), bit_parser(to_1bit(cur, CFG_BIT17)));

    printf("%s", port_info->display_buf);
}