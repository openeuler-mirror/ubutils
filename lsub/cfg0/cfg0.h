/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef LSUB_CFG0_H
#define LSUB_CFG0_H

#include "../lsub.h"

extern struct ub_entity_cfg_info *cfg0_info;

void cfg0_shp_cap(uint8_t *data, uint32_t data_len);
void cfg0_err_record_cap(uint8_t *data, uint32_t data_len);
void cfg0_err_info_cap(uint8_t *data, uint32_t data_len);
void cfg0_emq_cap(uint8_t *data, uint32_t data_len);

#endif