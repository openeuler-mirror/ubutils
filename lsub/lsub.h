/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef LSUB_H
#define LSUB_H

#include <ubutils.h>

#define DEFAULT_IDS_PATH "/usr/share/hwdata/ub.ids"

char *ub_lookup_name(struct ub_access *uacc, char *buf, size_t size,
                     uint32_t vendor_id, uint32_t device_id , uint32_t class_id);

#endif /* LSUB_H */
