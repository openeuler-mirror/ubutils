/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef LSUB_CFG1_H
#define LSUB_CFG1_H

#include "../lsub.h"

extern struct ub_entity_cfg_info *cfg1_info;

void cfg1_decoder_cap(uint8_t *data, uint32_t data_len);
void cfg1_jetty_cap(uint8_t *data, uint32_t data_len);
void cfg1_int_type1_cap(uint8_t *data, uint32_t data_len);
void cfg1_int_type2_cap(uint8_t *data, uint32_t data_len);
void cfg1_ub_mem_cap(uint8_t *data, uint32_t data_len);

#endif