// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <ubutils.h>
#include "lsub.h"

/**
  * read one slice
  */
int slice_read(struct ub_entity *uent, uint32_t slice_addr, uint8_t *data_buf, uint32_t *data_len)
{
    uint8_t slice_header[CFG_DWORD_LEN] = { 0 };
    uint32_t slice_used_size;
    int ret_code;

    /* read slice header to get slice size */
    ret_code = ub_read_block(uent, slice_addr, slice_header, CFG_DWORD_LEN);
    if (ret_code) {
        return ret_code;
    }

    /* read slice body */
    slice_used_size = (uint32_t)to_chunkbits(slice_header, CFG_BIT4, CFG_BIT31);
    *data_len = MIN(slice_used_size * CFG_DWORD_LEN, *data_len);

    ret_code = ub_read_block(uent, slice_addr, data_buf, (int)(*data_len));
    if (ret_code) {
        return ret_code;
    }

    return 0;
}

/**
  * get slice version
  */
uint8_t slice_get_version(uint8_t *slice_data)
{
    return (uint8_t)to_chunkbits(slice_data, CFG_BIT0, CFG_BIT3);
}

/**
  * get slice size
  */
uint32_t slice_get_size(uint8_t *slice_data)
{
    return (uint32_t)to_chunkbits(slice_data, CFG_BIT4, CFG_BIT31);
}