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

/* a word is 2 bytes (16 bits), a doubleword is 4 bytes (32 bits), and a quadword is 8 bytes (64 bits). */
#define CFG_DWORD_LEN           0x4

/**
  * +------------+---------------+-----------------+
  * | ID         | start address | end address     |
  * +------------+---------------+-----------------+
  * | CFG0_BASIC | 0x0000_0000   | 0x0000_00FF     |
  * +------------+---------------+-----------------+
  * | CFG0_CAP1  | 0x0000_0100   | 0x0000_01FF     |
  * +------------+---------------+-----------------+
  * | CFG0_CAP2  | 0x0000_0200   | 0x0000_02FF     |
  * +------------+---------------+-----------------+
  * ....
  * +------------+---------------+-----------------+
  * | CFG1_BASIC | 0x0001_0000   | 0x0001_00FF     |
  * +------------+---------------+-----------------+
  * | CFG1_CAP1  | 0x0001_0100   | 0x0001_01FF     |
  * +------------+---------------+-----------------+
  * | CFG1_CAP2  | 0x0001_0200   | 0x0001_02FF     |
  * +------------+---------------+-----------------+
  * ....
  * +------------+---------------+-----------------+
  * | Port0_BASIC| 0x0002_0000   | 0x0002_00FF     |
  * +------------+---------------+-----------------+
  * | Port0_CAP1 | 0x0002_0100   | 0x0002_01FF     |
  * +------------+---------------+-----------------+
  * | Port0_CAP2 | 0x0002_0200   | 0x0002_02FF     |
  * +------------+---------------+-----------------+
  * ....
  * +------------+---------------+-----------------+
  * | ROUTE_TABLE| 0xF000_0000   | 0xFFFF_FFFF     |
  * +------------+---------------+-----------------+
  */
#define CFG0_SLICE_ADDR(i)      ((0x100 * (i)) * CFG_DWORD_LEN)
#define ROUTE_TBL_SLICE         (0xF0000000ULL * CFG_DWORD_LEN)

/* cfg0 */
#define CFG0_PORT_NUMBER        (0x1 * CFG_DWORD_LEN)
#define CFG0_SUPPORT_FEATURE    (0xA * CFG_DWORD_LEN)

/* route table */
#define ROUTE_TBL_NUM_OF_TLB_ENTRY          (0x1 * CFG_DWORD_LEN)
#define ROUTE_TBL_EXACT_ROUTE_SUP           (0x1 * CFG_DWORD_LEN + 2)
#define ROUTE_TBL_DEFAULT_ROUTE_TBL         (0x10 * CFG_DWORD_LEN)
#define ROUTE_TBL_ROUTE_TBL_EBW(port_nums)  ((((port_nums) - 1) >> 5) + 1)
#define ROUTE_TBL_ROUTE_TBL_ENTRY(port_nums, cna) \
    ((0x10 + (((cna) + 1) * ROUTE_TBL_ROUTE_TBL_EBW(port_nums))) * CFG_DWORD_LEN)

enum {
    CFG_BIT0 = 0,
    CFG_BIT4 = 4,
    CFG_BIT16 = 16,
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

#define CFG_1BYTE_MASK  0x7
#define CFG_1BYTE_DISP  0x3

static inline uint8_t to_1bit(uint8_t *data, uint8_t bit)
{
    uint8_t pos, off;

    pos = bit >> CFG_1BYTE_DISP;
    off = bit & CFG_1BYTE_MASK;

    return (data[pos] >> off) & 0x1;
}

void ub_set_ids_file_path(struct ub_access *uacc, char *name, int to_be_freed);
char *ub_lookup_name(struct ub_access *uacc, char *buf, size_t size,
                     uint32_t vendor_id, uint32_t device_id , uint32_t class_id);
void show_route_tbl(struct ub_access *uacc, uint32_t uent_num, uint32_t cna);

#endif /* LSUB_H */
