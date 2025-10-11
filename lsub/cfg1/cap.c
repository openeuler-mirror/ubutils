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
#include "cfg1.h"

int cfg1_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    uint32_t pos;
    uint8_t bits_already;
    uint8_t cap_sup;
    int ret_code;

    bits_already = to_1bit(info->cfg1_cap_bits, CFG_BIT0);
    if (!bits_already) {
        /* read CFG1_BASIC Bitmap */
        pos = CFG1_SLICE_ADDR(0) + CFG1_CAP_BITMAP;
        ret_code = ub_read_block(info->uent, pos, info->cfg1_cap_bits, CFG_CAP_BITMAP_LEN);
        if (ret_code) {
            return 0;
        }
        info->cfg1_cap_bits[0] |= 0x1;
    }
    cap_sup = to_1bit(info->cfg1_cap_bits, (uint8_t)cap_id);
    return cap_sup;
}

/**
  * list cfg1 capabilities
  */
int lsub_cfg1_cap(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    static struct ub_cfg_cap_cat cfg1_cap_array[] = {
        {CFG1_DECODER_CAP_ID,       cfg1_decoder_cap},
        {CFG1_JETTY_CAP_ID,         cfg1_jetty_cap},
        {CFG1_INT_TYPE1_CAP_ID,     cfg1_int_type1_cap},
        {CFG1_INT_TYPE2_CAP_ID,     cfg1_int_type2_cap},
        {CFG1_UB_MEM_CAP_ID,        cfg1_ub_mem_cap},
        {CFG_INVALID_CAP_ID,        NULL}
    };
    struct ub_cfg_cap_cat *cur;
    uint32_t data_len;
    int i;
    int ret_code;

    if ((cap_id == 0) || (cap_id > CFG1_UB_MEM_CAP_ID)) {
        return 0;
    }

    cfg1_info = info;
    /* If a CAP is not supported, skip it. */
    if (!cfg1_check_capid(cfg1_info, cap_id)) {
        return 0;
    }

    for (i = 0; cfg1_cap_array[i].show != NULL; i++) {
        cur = &(cfg1_cap_array[i]);
        if (cur->cap_id == cap_id) {
            memset(cfg1_info->data_buf, 0, CFG_SLICE_LEN);
            data_len = CFG_SLICE_LEN;
            ret_code = slice_read(cfg1_info->uent, CFG1_SLICE_ADDR(cur->cap_id),
                cfg1_info->data_buf, &data_len);
            if (ret_code) {
                fprintf(stderr, "lsub error: failed to read CFG1 CAP[%u].\n", cur->cap_id);
                return -EIO;
            }
            cur->show(cfg1_info->data_buf, data_len);
            return 0;
        }
    }

    /* If a CAP cannot be parsed, skip it. */
    return 0;
}