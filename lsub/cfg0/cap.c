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
#include "cfg0.h"

int cfg0_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    uint32_t pos;
    uint8_t bits_already;
    uint8_t cap_sup;
    int ret_code;

    /**
      * According to the UB protocol specifications, bit0 of the bitmap is invalid. It is used
      * here to mark whether the bitmap has been initialized.
      */
    bits_already = to_1bit(info->cfg0_cap_bits, CFG_BIT0);
    if (!bits_already) {
        /* read CFG0_BASIC Bitmap */
        pos = CFG0_SLICE_ADDR(0) + CFG0_CAP_BITMAP;
        ret_code = ub_read_block(info->uent, pos, info->cfg0_cap_bits, CFG_CAP_BITMAP_LEN);
        if (ret_code) {
            return 0;
        }
        info->cfg0_cap_bits[0] |= 0x1;
    }
    cap_sup = to_1bit(info->cfg0_cap_bits, (uint8_t)cap_id);
    return cap_sup;
}

/**
  * list cfg0 capabilities
  */
int lsub_cfg0_cap(struct ub_entity_cfg_info *info, uint32_t cap_id)
{
    static struct ub_cfg_cap_cat cfg0_cap_array[] = {
        {CFG0_SHP_CAP_ID,        cfg0_shp_cap},
        {CFG0_ERR_RECORD_CAP_ID, cfg0_err_record_cap},
        {CFG0_ERR_INFO_CAP_ID,   cfg0_err_info_cap},
        {CFG0_EMQ_CAP_ID,        cfg0_emq_cap},
        {CFG_INVALID_CAP_ID,    NULL}
    };
    struct ub_cfg_cap_cat *cur;
    uint32_t data_len;
    int i;
    int ret_code;

    if ((cap_id == 0) || (cap_id > CFG0_EMQ_CAP_ID)) {
        return 0;
    }

    cfg0_info = info;
    /* If a CAP is not supported, skip it. */
    if (!cfg0_check_capid(cfg0_info, cap_id)) {
        return 0;
    }

    for (i = 0; cfg0_cap_array[i].show != NULL; i++) {
        cur = &(cfg0_cap_array[i]);
        if (cur->cap_id == cap_id) {
            memset(cfg0_info->data_buf, 0, CFG_SLICE_LEN);
            data_len = CFG_SLICE_LEN;
            ret_code = slice_read(cfg0_info->uent, CFG0_SLICE_ADDR(cur->cap_id),
                cfg0_info->data_buf, &data_len);
            if (ret_code) {
                fprintf(stderr, "lsub error: failed to read CFG0 CAP[%u].\n", cur->cap_id);
                return -EIO;
            }
            cur->show(cfg0_info->data_buf, data_len);
            return 0;
        }
    }

    /* If a CAP cannot be parsed, skip it. */
    return 0;
}