// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <string.h>
#include <errno.h>
#include "../lsub.h"
#include "port.h"

int port_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    uint32_t pos;
    uint8_t bits_already;
    uint8_t cap_sup;
    int ret_code;

    /*
     * According to the UB protocol specifications, bit0 of the bitmap is invalid. It is used
     * here to mark whether the bitmap has been initialized.
     */
    bits_already = to_1bit(info->port_cap_bits, CFG_BIT0);
    if (!bits_already) {
        /* read PORT_BASIC Bitmap */
        pos = PORT_SLICE_ADDR(info->port, 0) + PORT_BASIC_CAP_BITMAP;
        ret_code = ub_read_block(info->uent, pos, info->port_cap_bits, CFG_CAP_BITMAP_LEN);
        if (ret_code) {
            return 0;
        }
        info->port_cap_bits[0] |= 0x1;
    }

    cap_sup = to_1bit(info->port_cap_bits, (uint8_t)(cap_id));

    return cap_sup;
}

static struct ub_cfg_cap_cat port_cap_array[] = {
    { PORT_CAP1_LINK, port_cap1_link },
    { PORT_CAP2_LINK_LOG, port_cap2_link_log },
    { PORT_CAP4_DATA_RATE1, port_capx_data_rate },
    { PORT_CAP5_DATA_RATE2, port_capx_data_rate },
    { PORT_CAP6_DATA_RATE3, port_capx_data_rate },
    { PORT_CAP7_DATA_RATE4, port_capx_data_rate },
    { PORT_CAP8_DATA_RATE5, port_capx_data_rate },
    { PORT_CAP9_DATA_RATE6, port_capx_data_rate },
    { PORT_CAP10_DATA_RATE7, port_capx_data_rate },
    { PORT_CAP11_DATA_RATE8, port_capx_data_rate },
    { PORT_CAP12_DATA_RATE9, port_capx_data_rate },
    { PORT_CAP14_EYE_MONITOR, port_cap14_eye_monitor },
    { PORT_CAP15_QDLWS, port_cap15_qdlws },
    { CFG_INVALID_CAP_ID, NULL }
};

/*
 * list port capabilities
 */
int lsub_port_cap(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    struct ub_cfg_cap_cat *cur;
    uint32_t data_len;
    int i;
    int ret_code;

    if (cap_id > PORT_CAP21_PORT_ERR_RECORD) {
        return 0;
    }

    port_info = info;
    /* If a CAP is not supported, skip it. */
    if (!port_check_capid(port_info, cap_id)) {
        return 0;
    }

    /* If uent is not mue, skip it. */
    if (!ub_is_primary(port_info->uent)) {
        return 0;
    }

    for (i = 0; port_cap_array[i].show != NULL; i++) {
        cur = &(port_cap_array[i]);
        if (cur->cap_id == cap_id) {
            memset(port_info->data_buf, 0, CFG_SLICE_LEN);
            data_len = CFG_SLICE_LEN;
            ret_code = slice_read(port_info->uent, PORT_SLICE_ADDR(port_info->port, cap_id),
                                  port_info->data_buf, &data_len);
            if (ret_code) {
                fprintf(stderr, "lsub error: failed to read PORT CAP[%u].\n", cur->cap_id);
                return -EIO;
            }
            port_cap_id = cur->cap_id;
            cur->show(port_info->data_buf, data_len);
            return 0;
        }
    }

    /* If a CAP cannot be parsed, skip it. */
    return 0;
}