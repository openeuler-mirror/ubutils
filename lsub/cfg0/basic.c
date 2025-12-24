// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../lsub.h"
#include "cfg0.h"

/* ipv4 has 32 bits */
#define IPV4_BYTE_LEN       4
/* ipv6 has 128 bits */
#define IPV6_BYTE_LEN       16

#define CFG0_CAP_NUM 6
static const char *cfg0_cap_list[CFG0_CAP_NUM] = {"CAP0_NULL", "CAP1_RSVD",
                                                  "CAP2_SHP", "CAP3_DEVICE_ERR_RECORD",
                                                  "CAP4_DEVICE_ERR_INFO", "CAP5_EMQ"};
struct ub_entity_cfg_info *cfg0_info;

static void cfg0_port_num_and_entity_num(uint8_t *data)
{
    uint16_t port_num, entity_num;

    port_num = to_uint16(data);
    cfg0_info->port_num = port_num;

    if (ub_is_primary(cfg0_info->uent)) {
        entity_num = to_uint16(data + CFG_WORD_LEN);
        cfg0_info->entity_num = entity_num;
        sprintf(cfg0_info->display_buf,
            "\n\t\t\t\tTotal Number of Ports: %u\n\t\t\t\tTotal Number of Entities: %u", port_num, entity_num);
    } else {
        sprintf(cfg0_info->display_buf,
            "\n\t\t\t\tTotal Number of Ports: %u", port_num);
    }

    printf("%s", cfg0_info->display_buf);
}

static void cfg0_cap_bitmap(uint8_t *data)
{
    int i, j;
    int off;

    /* save CAP bitmap */
    memcpy(cfg0_info->cfg0_cap_bits, data, CFG_CAP_BITMAP_LEN);

    off = sprintf(cfg0_info->display_buf, "%s", "\n\t\t\t\tCFG0_CAP Bitmap: ");
    for (i = CFG_CAP_BITMAP_LEN - 1, j = 1; i >= 0; i--, j++) {
        off += sprintf(cfg0_info->display_buf + off, "%02x", cfg0_info->cfg0_cap_bits[i]);
        if (j == (CFG_CAP_BITMAP_LEN >> 1)) {
            off += sprintf(cfg0_info->display_buf + off, "%s", "\n\t\t\t\t                 ");
        } else if ((j != CFG_CAP_BITMAP_LEN) && ((j % CFG_DWORD_LEN) == 0)) {
            off += sprintf(cfg0_info->display_buf + off, "%s", "-");
        }
    }

    off += sprintf(cfg0_info->display_buf + off, "\n\t\t\t\tEnabled Cfg0 Cap: ");

    for (uint8_t k = 0; k < CFG0_CAP_NUM; k++) {
        if (to_1bit(data, k)) {
            off += sprintf(cfg0_info->display_buf + off, "%s ", cfg0_cap_list[k]);
        }
    }

    printf("%s", cfg0_info->display_buf);
    /**
      * According to the UB protocol specifications, bit0 of the bitmap is invalid. It is used
      * here to mark whether the bitmap has been initialized.
      */
    cfg0_info->cfg0_cap_bits[0] |= 0x1;
}

static void cfg0_supported_feature(uint8_t *data)
{
    uint8_t entity_avail;
    uint8_t mtu_sup, rt_sup, upi_sup;
    uint8_t switch_sup, cc_sup;

    /* save feature bits */
    memcpy(cfg0_info->cfg0_sup_feat, data, CFG_SUP_FEATURE_LEN);

    entity_avail = to_1bit(data, CFG_BIT0);
    mtu_sup = (uint8_t)to_chunkbits(data, CFG_BIT1, CFG_BIT3);
    rt_sup = to_1bit(data, CFG_BIT4);
    upi_sup = to_1bit(data, CFG_BIT5);
    switch_sup = to_1bit(data, CFG_BIT7);
    cc_sup = to_1bit(data, CFG_BIT9);

    sprintf(cfg0_info->display_buf,
        "\n\t\t\t\tSupported Feature: EntityAvail%s MTU:%s RouteTable%s UPI%s Switch%s CC%s",
        bit_parser(entity_avail), mtu_parser(mtu_sup), bit_parser(rt_sup), bit_parser(upi_sup),
        bit_parser(switch_sup), bit_parser(cc_sup));

    printf("%s", cfg0_info->display_buf);
}

static void cfg0_guid(uint8_t *data)
{
    uint16_t device_id;
    uint8_t type, version;
    uint8_t *guid_data;
    uint16_t vendor_id;
    uint64_t seq_num;

    /**
      * +-----------+-----------+---------+------+----------+-----------------+
      * | vendor id | device id | version | type | reserved | sequence number |
      * | 16bit     | 16bit     | 4bit    | 4bit | 24bit    | 64bit           |
      * +-----------+-----------+---------+------+----------+-----------------+
      */
    guid_data = data;
    seq_num = to_uint64(guid_data);
    guid_data += CFG_QWORD_LEN;
    type = (uint8_t)to_chunkbits(guid_data, CFG_BIT24, CFG_BIT27);
    version = (uint8_t)to_chunkbits(guid_data, CFG_BIT28, CFG_BIT31);
    device_id = (uint16_t)to_chunkbits(guid_data, CFG_BIT32, CFG_BIT47);
    vendor_id = (uint16_t)to_chunkbits(guid_data, CFG_BIT48, CFG_BIT63);

    sprintf(cfg0_info->display_buf, "\n\t\t\t\tGUID: %04x-%04x-%01x-%01x-000000-%016lx",
        vendor_id, device_id, version, type, seq_num);

    printf("%s", cfg0_info->display_buf);
}

int parse_eid(uint8_t *data, char *display_buf)
{
    uint8_t domain_id, node_id;
    int ret_off = 0;
    int i;

    for (i = CFG_EID_LEN - 1; i >= 0; i--) {
        if (i == (CFG_EID_NODE_ID / CFG_1BYTE_BITS)) {
            node_id = (uint8_t)to_chunkbits(&data[i], CFG_BIT0, CFG_BIT3);
            domain_id = (uint8_t)to_chunkbits(&data[i], CFG_BIT4, CFG_BIT7);
            ret_off += sprintf(display_buf + ret_off, "%x-%x", domain_id, node_id);
        } else {
            ret_off += sprintf(display_buf + ret_off, "%02x", data[i]);
        }
    }
    return ret_off;
}

static void cfg0_eid(uint8_t *data)
{
    int off;

    off = sprintf(cfg0_info->display_buf, "\n\t\t\t\tEID: ");
    (void)parse_eid(data, cfg0_info->display_buf + off);
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_ubfm_eid(uint8_t *data)
{
    int off;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    off = sprintf(cfg0_info->display_buf, "\n\t\t\t\tUBFM EID: ");
    (void)parse_eid(data, cfg0_info->display_buf + off);
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_netaddr(uint8_t *data)
{
    uint32_t cna;

    cna = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);

    sprintf(cfg0_info->display_buf, "\n\t\t\t\tPrimary CNA: 0x%x",
            cna);

    printf("%s", cfg0_info->display_buf);
}

static void cfg0_upi(uint8_t *data)
{
    uint16_t upi;

    upi = (uint16_t)to_chunkbits(data, CFG_BIT0, CFG_BIT14);
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tUPI: 0x%x", upi);

    printf("%s", cfg0_info->display_buf);
}

static void cfg0_module(uint8_t *data)
{
    uint16_t module_vendor_id;
    uint16_t module_id;

    module_id = (uint16_t)to_chunkbits(data, CFG_BIT0, CFG_BIT15);
    module_vendor_id = (uint16_t)to_chunkbits(data, CFG_BIT16, CFG_BIT31);

    sprintf(cfg0_info->display_buf, "\n\t\t\t\tModule: Module Vendor ID:0x%x Module ID:0x%x",
            module_vendor_id, module_id);

    printf("%s", cfg0_info->display_buf);
}

static void cfg0_dev_rst(uint8_t *data)
{
    uint8_t dev_rst;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    dev_rst = to_1bit(data, CFG_BIT0);

    (void)sprintf(cfg0_info->display_buf, "\n\t\t\t\tDEV_RST%s", bit_parser(dev_rst));
    (void)printf("%s", cfg0_info->display_buf);
}

static void cfg0_mtu(uint8_t *data)
{
    uint8_t mtu_cfg;

    mtu_cfg = (uint8_t)to_chunkbits(data, CFG_BIT0, CFG_BIT2);
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tMTU_CFG: %s", mtu_parser(mtu_cfg));
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_cc(uint8_t *data)
{
    uint8_t cc_sup;
    uint8_t cc_en;

    cc_sup = to_1bit(cfg0_info->cfg0_sup_feat, CFG_BIT9);
    if (cc_sup) {
        cc_en = to_1bit(data, CFG_BIT0);
    } else {
        cc_en = 0;
    }
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tCC_EN%s", bit_parser(cc_en));
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_trust_host(uint8_t *data)
{
    uint8_t trust_host_en;

    if (!ub_is_ibus_controller(cfg0_info->uent)) {
        return;
    }

    trust_host_en = to_1bit(data, CFG_BIT0);
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tTrust_EN%s", bit_parser(trust_host_en));
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_ubfm_cna(uint8_t *data)
{
    uint32_t ubfm_cna;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    ubfm_cna = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tUBFM_CNA: 0x%x", ubfm_cna);
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_user_eid(uint8_t *data)
{
    int off;

    off = sprintf(cfg0_info->display_buf, "%s", "\n\t\t\t\tUEID: ");
    (void)parse_eid(data, cfg0_info->display_buf + off);
    printf("%s", cfg0_info->display_buf);
}

static void cfg0_user_cna(uint8_t *data)
{
    uint32_t user_cna;

    user_cna = to_uint32(data);
    sprintf(cfg0_info->display_buf, "\n\t\t\t\tUCNA: 0x%x", user_cna);
    printf("%s", cfg0_info->display_buf);
}

/**
  * list cfg0 basic configuration space
  */
int lsub_cfg0_basic(struct ub_entity_cfg_info *info)
{
    static struct ub_cfg_basic_cat cfg0_basic_array[] = {
        {CFG0_PORT_NUMBER,       cfg0_port_num_and_entity_num},
        {CFG0_CAP_BITMAP,        cfg0_cap_bitmap},
        {CFG0_SUPPORT_FEATURE,   cfg0_supported_feature},
        {CFG0_GUID,              cfg0_guid},
        {CFG0_EID,               cfg0_eid},
        {CFG0_UBFM_EID,          cfg0_ubfm_eid},
        {CFG0_NET_ADDR,          cfg0_netaddr},
        {CFG0_UPI,               cfg0_upi},
        {CFG0_MODULE,            cfg0_module},
        {CFG0_DEV_RST,           cfg0_dev_rst},
        {CFG0_MTU_CFG,           cfg0_mtu},
        {CFG0_CC_EN,             cfg0_cc},
        {CFG0_TRUST_HOST_EN,     cfg0_trust_host},
        {CFG0_UBFM_CNA,          cfg0_ubfm_cna},
        {CFG0_USER_EID,          cfg0_user_eid},
        {CFG0_USER_CNA,          cfg0_user_cna},
        {0x0,                    NULL}
    };
    char desp[128];
    struct ub_cfg_basic_cat *cur;
    uint32_t data_len;
    uint8_t slice_ver;
    uint32_t slice_size;
    int ret_code;
    int i;

    cfg0_info = info;
    memset(cfg0_info->data_buf, 0, CFG_SLICE_LEN);
    data_len = CFG_SLICE_LEN;
    ret_code = slice_read(cfg0_info->uent, CFG0_SLICE_ADDR(0), cfg0_info->data_buf, &data_len);
    if (ret_code) {
        fprintf(stderr, "lsub error: failed to read CFG0 BASIC.\n");
        return -EIO;
    }

    slice_ver = slice_get_version(cfg0_info->data_buf);
    slice_size = slice_get_size(cfg0_info->data_buf);
    sprintf(desp, "\n\t\tCFG0_BASIC: slice[0x%x, 0x%x]", slice_ver, slice_size);
    printf("%s", desp);
    for (i = 0; cfg0_basic_array[i].show != NULL; i++) {
        cur = &(cfg0_basic_array[i]);
        /* cfg0 show make sure pos + offset < data_len */
        if (cur->pos >= data_len) {
            break;
        }
        cur->show(&(cfg0_info->data_buf[cur->pos]));
    }

    return 0;
}
