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
#define CFG_QWORD_LEN           0x8
#define CFG_DWORD_LEN           0x4
#define CFG_WORD_LEN            0x2
#define CFG_BYTE_LEN            0x1

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
#define CFG1_SLICE_ADDR(i)      ((0x10000 + 0x100 * (i)) * CFG_DWORD_LEN)
#define PORT_SLICE_ADDR(i, j)   ((0x10000 * ((i) + 2) + 0x100 * (j)) * CFG_DWORD_LEN)
#define ROUTE_TBL_SLICE         (0xF0000000ULL * CFG_DWORD_LEN)

/* cfg0 */
#define CFG0_PORT_NUMBER        (0x1 * CFG_DWORD_LEN)
#define CFG0_CAP_BITMAP         (0x2 * CFG_DWORD_LEN)
#define CFG0_SUPPORT_FEATURE    (0xA * CFG_DWORD_LEN)
#define CFG0_GUID               (0xE * CFG_DWORD_LEN)
#define CFG0_EID                (0x12 * CFG_DWORD_LEN)
#define CFG0_UBFM_EID           (0x16 * CFG_DWORD_LEN)
#define CFG0_NET_ADDR           (0x1A * CFG_DWORD_LEN)
#define CFG0_UPI                (0x1F * CFG_DWORD_LEN)
#define CFG0_MODULE             (0x20 * CFG_DWORD_LEN)
#define CFG0_DEV_RST            (0x21 * CFG_DWORD_LEN)
#define CFG0_MTU_CFG            (0x23 * CFG_DWORD_LEN)
#define CFG0_CC_EN              (0x24 * CFG_DWORD_LEN)
#define CFG0_TRUST_HOST_EN      (0x25 * CFG_DWORD_LEN)
#define CFG0_UBFM_CNA           (0x26 * CFG_DWORD_LEN)
#define CFG0_USER_EID           (0x27 * CFG_DWORD_LEN)
#define CFG0_USER_CNA           (0x2B * CFG_DWORD_LEN)
#define CFG0_SHP_CAP_ID         0x2
#define CFG0_ERR_RECORD_CAP_ID  0x3
#define CFG0_ERR_INFO_CAP_ID    0x4
#define CFG0_EMQ_CAP_ID         0x5

/* cfg1 */
#define CFG1_CAP_BITMAP         (0x1 * CFG_DWORD_LEN)
#define CFG1_SUPPORT_FEATURE    (0x9 * CFG_DWORD_LEN)
#define CFG1_ERS0_SS            (0xD * CFG_DWORD_LEN)
#define CFG1_ERS1_SS            (0xE * CFG_DWORD_LEN)
#define CFG1_ERS2_SS            (0xF * CFG_DWORD_LEN)
#define CFG1_ERS0_SA            (0x10 * CFG_DWORD_LEN)
#define CFG1_ERS1_SA            (0x12 * CFG_DWORD_LEN)
#define CFG1_ERS2_SA            (0x14 * CFG_DWORD_LEN)
#define CFG1_ERS0_UBA           (0x16 * CFG_DWORD_LEN)
#define CFG1_ERS1_UBA           (0x18 * CFG_DWORD_LEN)
#define CFG1_ERS2_UBA           (0x1A * CFG_DWORD_LEN)
#define CFG1_ELR                (0x1C * CFG_DWORD_LEN)
#define CFG1_ELR_DONE           (0x1D * CFG_DWORD_LEN)
#define CFG1_SYS_PGS            (0x21 * CFG_DWORD_LEN)
#define CFG1_EID_UPI_TBA        (0x22 * CFG_DWORD_LEN)
#define CFG1_EID_UPI_TEN        (0x24 * CFG_DWORD_LEN)
#define CFG1_CLASS_CODE         (0x29 * CFG_DWORD_LEN)
#define CFG1_DEV_TOKEN_ID       (0x2D * CFG_DWORD_LEN)
#define CFG1_BUS_EN             (0x2E * CFG_DWORD_LEN)
#define CFG1_DEV_RS_ACESS_EN    (0x2F * CFG_DWORD_LEN)
#define CFG1_DECODER_CAP_ID     0x1
#define CFG1_JETTY_CAP_ID       0x2
#define CFG1_INT_TYPE1_CAP_ID   0x3
#define CFG1_INT_TYPE2_CAP_ID   0x4
#define CFG1_UB_MEM_CAP_ID      0x6

/* port */
#define PORT_BASIC_CAP_BITMAP   (0x1 * CFG_DWORD_LEN)
#define PORT_BASIC_INFO         (0x9 * CFG_DWORD_LEN)
#define PORT_BASIC_NB_INFO      (0xA * CFG_DWORD_LEN)
#define PORT_BASIC_CNA          (0xF * CFG_DWORD_LEN)
#define PORT_BASIC_RST          (0x10 * CFG_DWORD_LEN) /* Rort Reset Attribute is WC */

enum {
    PORT_CAP1_LINK = 1,
    PORT_CAP2_LINK_LOG,
    PORT_CAP3_RSVD,
    PORT_CAP4_DATA_RATE1,
    PORT_CAP5_DATA_RATE2,
    PORT_CAP6_DATA_RATE3,
    PORT_CAP7_DATA_RATE4,
    PORT_CAP8_DATA_RATE5,
    PORT_CAP9_DATA_RATE6,
    PORT_CAP10_DATA_RATE7,
    PORT_CAP11_DATA_RATE8,
    PORT_CAP12_DATA_RATE9,
    PORT_CAP13_RSVD,
    PORT_CAP14_EYE_MONITOR,
    PORT_CAP15_QDLWS,
    PORT_CAP16_RSVD,
    PORT_CAP17_RSVD,
    PORT_CAP18_RSVD,
    PORT_CAP19_RSVD,
    PORT_CAP20_LMSM_ST,
    PORT_CAP21_PORT_ERR_RECORD,
};

#define PORT_VN_NUM 16

/* route table */
#define ROUTE_TBL_NUM_OF_TLB_ENTRY          (0x1 * CFG_DWORD_LEN)
#define ROUTE_TBL_EXACT_ROUTE_SUP           (0x1 * CFG_DWORD_LEN + 2)
#define ROUTE_TBL_DEFAULT_ROUTE_TBL         (0x10 * CFG_DWORD_LEN)
#define ROUTE_TBL_ROUTE_TBL_EBW(port_nums)  ((((port_nums) - 1) >> 5) + 1)
#define ROUTE_TBL_ROUTE_TBL_ENTRY(port_nums, cna) \
    ((0x10 + (((cna) + 1) * ROUTE_TBL_ROUTE_TBL_EBW(port_nums))) * CFG_DWORD_LEN)

/* eid has 128 bits */
#define CFG_EID_LEN             0x10
/* In compact domain, eid consists of a 108-bit domain ID and a 20-bit node ID. */
#define CFG_EID_NODE_ID         20
/* the CAP bitmap of CFG0\CFG1\PORT BASIC have 256 bits(=32B) */
#define CFG_CAP_BITMAP_LEN      0x20
/* the Supported Feature of CFG0_BASIC and CFG1_BASIC have 128 bits(=16B) */
#define CFG_SUP_FEATURE_LEN     0x10
/* The size of SLICE is fixed to 1 KB except for RouteTable */
#define CFG_SLICE_LEN           1024
/* invalid CAP id */
#define CFG_INVALID_CAP_ID      256
/* invaild port number */
#define CFG_INVALID_PORT_NUM    0xFFFF
#define CFG_DISPLAY_BUF_LEN     4096

#define CFG_ARRAY_SIZE(a, b) (sizeof(a) / sizeof(b))

#define CFG_RESERVED            "reserved"

enum {
    CFG0_SLICE_TYPE = 0,
    CFG1_SLICE_TYPE,
    PORT_SLICE_TYPE
};

struct ub_cfg_basic_cat {
    uint32_t pos;
    void (*show)(uint8_t *data);
};

struct ub_cfg_cap_cat {
    uint32_t cap_id;
    void (*show)(uint8_t *cap_data, uint32_t data_len);
};

struct ub_entity_cfg_info {
    /* ub entity */
    struct ub_entity *uent;
    /* port number */
    uint16_t port_num;
    /* selected port */
    uint32_t port;
    /* entity number */
    uint16_t entity_num;
    uint8_t cfg0_cap_bits[CFG_CAP_BITMAP_LEN];
    uint8_t cfg0_sup_feat[CFG_SUP_FEATURE_LEN];
    uint8_t cfg1_cap_bits[CFG_CAP_BITMAP_LEN];
    uint8_t cfg1_sup_feat[CFG_SUP_FEATURE_LEN];
    uint8_t port_cap_bits[CFG_CAP_BITMAP_LEN];
    uint8_t data_buf[CFG_SLICE_LEN];
    char display_buf[CFG_DISPLAY_BUF_LEN];
};

static inline const char* bit_parser(uint8_t data)
{
    return data == 0 ? "-" : "+";
}

static inline const char* mtu_parser(uint8_t data)
{
    static const char* mtu_desp[] = {
        "1024B", "4096B", "8192B"
    };

    if (data < CFG_ARRAY_SIZE(mtu_desp, char*)) {
        return mtu_desp[data];
    } else {
        return CFG_RESERVED;
    }
}

static inline const char* led_parse(uint8_t status)
{
    static const char* led_status[] = {
        "Off", "On", "Blink"
    };

    if (status < CFG_ARRAY_SIZE(led_status, char*)) {
        return led_status[status];
    } else {
        return CFG_RESERVED;
    }
}

enum {
    CFG_BIT0 = 0,
    CFG_BIT1 = 1,
    CFG_BIT2 = 2,
    CFG_BIT3 = 3,
    CFG_BIT4 = 4,
    CFG_BIT5 = 5,
    CFG_BIT6 = 6,
    CFG_BIT7 = 7,
    CFG_BIT8 = 8,
    CFG_BIT9 = 9,
    CFG_BIT10 = 10,
    CFG_BIT11 = 11,
    CFG_BIT12 = 12,
    CFG_BIT13 = 13,
    CFG_BIT14 = 14,
    CFG_BIT15 = 15,
    CFG_BIT16 = 16,
    CFG_BIT17 = 17,
    CFG_BIT18 = 18,
    CFG_BIT19 = 19,
    CFG_BIT20 = 20,
    CFG_BIT21 = 21,
    CFG_BIT22 = 22,
    CFG_BIT23 = 23,
    CFG_BIT24 = 24,
    CFG_BIT25 = 25,
    CFG_BIT26 = 26,
    CFG_BIT27 = 27,
    CFG_BIT28 = 28,
    CFG_BIT29 = 29,
    CFG_BIT30 = 30,
    CFG_BIT31 = 31,
    CFG_BIT32 = 32,
    CFG_BIT35 = 35,
    CFG_BIT36 = 36,
    CFG_BIT39 = 39,
    CFG_BIT40 = 40,
    CFG_BIT43 = 43,
    CFG_BIT47 = 47,
    CFG_BIT48 = 48,
    CFG_BIT56 = 56,
    CFG_BIT63 = 63,
    CFG_BIT64 = 64,
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

#define CFG_1BYTE_BITS  0x8
#define CFG_1BYTE_MASK  0x7
#define CFG_1BYTE_DISP  0x3

static inline uint8_t to_1bit(uint8_t *data, uint8_t bit)
{
    uint8_t pos, off;

    pos = bit >> CFG_1BYTE_DISP;
    off = bit & CFG_1BYTE_MASK;

    return (data[pos] >> off) & 0x1;
}

static uint64_t to_chunkbits(uint8_t *data, uint8_t start_bit, uint8_t end_bit)
{
    static uint8_t valid_mask[] = {0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF};
    uint8_t start_pos, start_off;
    uint8_t end_pos, end_off;
    int i, j;
    uint64_t val;

    start_pos = start_bit >> CFG_1BYTE_DISP;
    start_off = start_bit & CFG_1BYTE_MASK;
    end_pos = end_bit >> CFG_1BYTE_DISP;
    end_off = end_bit & CFG_1BYTE_MASK;

    if (start_pos == end_pos) {
        val = (data[start_pos] & valid_mask[end_off]) >> start_off;
        return val;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    val = data[start_pos] >> start_off;
    i = start_pos + 1;
    j = CFG_1BYTE_BITS - start_off;
    while (i < end_pos) {
        val |= ((uint64_t)data[i]) << j;
        i++;
        j += CFG_1BYTE_BITS;
    }
    val |= ((uint64_t)(data[end_pos] & valid_mask[end_off])) << j;
#else
    val = (uint64_t)(data[end_pos] & valid_mask[end_off]);
    i = end_pos - 1;
    j = end_off;
    while (i > start_pos) {
        val |= ((uint64_t)data[i]) << j;
        i--;
        j += CFG_1BYTE_BITS;
    }
    val |= ((uint64_t)data[start_pos] >> start_off) << j;
#endif

    return val;
}

static inline uint16_t to_uint16(uint8_t *data)
{
    return (uint16_t)to_chunkbits(data, CFG_BIT0, CFG_BIT15);
}

static inline uint32_t to_uint32(uint8_t *data)
{
    return (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT31);
}

static inline uint64_t to_uint64(uint8_t *data)
{
    return to_chunkbits(data, CFG_BIT0, CFG_BIT63);
}

/**
  * Check whether the entity is UBE0.
  */
static inline bool ub_is_primary(struct ub_entity *uent)
{
    if (uent->entity_idx == 0) {
        return 1;
    } else {
        return 0;
    }
}

int slice_read(struct ub_entity *uent, uint32_t slice_addr, uint8_t *data_buf, uint32_t *data_len);
uint8_t slice_get_version(uint8_t *slice_data);
uint32_t slice_get_size(uint8_t *slice_data);
int lsub_cfg0_basic(struct ub_entity_cfg_info *info);
int lsub_cfg0_cap(struct ub_entity_cfg_info *info, uint32_t cap_id);
int cfg0_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id);
int lsub_cfg1_basic(struct ub_entity_cfg_info *info);
int lsub_cfg1_cap(struct ub_entity_cfg_info *info, uint32_t cap_id);
int cfg1_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id);
int port_check_id(struct ub_entity *uent, uint32_t port_id);
int lsub_port_basic(struct ub_entity_cfg_info *info);
int lsub_port_cap(struct ub_entity_cfg_info *info, uint32_t cap_id);
int port_check_capid(struct ub_entity_cfg_info *info, uint32_t cap_id);
void ub_set_ids_file_path(struct ub_access *uacc, char *name, int to_be_freed);
char *ub_lookup_name(struct ub_access *uacc, char *buf, size_t size,
                     uint32_t vendor_id, uint32_t device_id , uint32_t class_id);
void show_route_tbl(struct ub_access *uacc, uint32_t uent_num, uint32_t cna);

/* common interfaces */
int parse_eid(uint8_t *data, char *display_buf);

#endif /* LSUB_H */
