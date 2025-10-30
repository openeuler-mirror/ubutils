/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef LSUB_PORT_H
#define LSUB_PORT_H

#include "../lsub.h"

#define PORT_VL_NUM 16
#define PORT_RATE_SIZE 9
#define FEC_DESC_NUM 4
#define BCRC_DESC_NUM 3

struct val_desc {
#define INVALID_DESC 0xffff
    uint32_t val;
    const char *desc;
};

extern struct ub_entity_cfg_info *port_info;
extern unsigned int port_cap_id;
extern struct val_desc fec_desc[FEC_DESC_NUM];
extern struct val_desc bcrc_desc[BCRC_DESC_NUM];

const char *get_description(uint64_t value, struct val_desc *table);
void port_cap1_link(uint8_t *data, uint32_t data_len);
#endif