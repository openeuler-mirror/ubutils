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

extern struct device *g_first_dev;

enum {
    UB_TOPO_PORT_ONLY_ONE,
    UB_TOPO_PORT_MULTI,
    UB_TOPO_PORT_LAST
};

struct ub_bus_controller {
    struct ub_bus_controller *chain;
    struct device *ubc_dev;
};

#define CLASS_CODE_MASK 0xff

/**
  * Check whether the entity is UB Bus Controller.
  */
static inline bool ub_is_ibus_controller(struct ub_entity *uent)
{
    if (uent->entity_type == UB_HEADER_TYPE_INTEGRATED_UB_CONTROLLER &&
        (uent->class_code & CLASS_CODE_MASK) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void ub_set_ids_file_path(struct ub_access *uacc, char *name, int to_be_freed);
char *ub_lookup_name(struct ub_access *uacc, char *buf, size_t size,
                     uint32_t vendor_id, uint32_t device_id , uint32_t class_id);

#endif /* LSUB_H */
