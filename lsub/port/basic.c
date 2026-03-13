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
#include "port.h"

#define PORT_CAP_NUM 22
static const char *port_cap_list[PORT_CAP_NUM] = {"CAP0_NULL", "CAP1_LINK", "CAP2_LINK_LOG",
                                                  "CAP3_RSVD", "CAP4_DATA_RATE1", "CAP5_DATA_RATE2",
                                                  "CAP6_DATA_RATE3", "CAP7_DATA_RATE4", "CAP8_DATA_RATE5",
                                                  "CAP9_DATA_RATE6", "CAP10_DATA_RATE7", "CAP11_DATA_RATE8",
                                                  "CAP12_DATA_RATE9", "CAP13_RSVD", "CAP14_EYE_MONITOR",
                                                  "CAP15_QDLWS", "CAP16_RSVD", "CAP17_RSVD",
                                                  "CAP18_RSVD", "CAP19_RSVD",
                                                  "CAP20_LMSM_ST", "CAP21_PORT_ERR_RECORD"};

int port_check_id(struct ub_entity *uent, uint32_t port_id)
{
    uint16_t port_num = 0;
    int port_sup;
    int ret;

    ret = ub_read_block(uent, CFG0_PORT_NUMBER, (uint8_t *)&port_num, CFG_WORD_LEN);
    if (ret) {
        fprintf(stderr, "cfg0 port number read failed.\n");
        return 0;
    }

    port_sup = port_id < port_num;
    if (!port_sup) {
        fprintf(stderr, "The port ID is greater than the total number of ports(%u)\n", port_num);
    }

    return port_sup;
}

static void port_basic_cap_bitmap(uint8_t *data)
{
#define LINE_PARSE 4
    int cnt = 0;
    int i, j;
    int off;

    /* save CAP bitmap */
    memcpy(port_info->port_cap_bits, data, CFG_CAP_BITMAP_LEN);

    off = sprintf(port_info->display_buf, "%s", "\n\t\t\t\tPORT_CAP Bitmap: ");
    for (i = CFG_CAP_BITMAP_LEN - 1, j = 1; i >= 0; i--, j++) {
        off += sprintf(port_info->display_buf + off, "%02x", port_info->port_cap_bits[i]);
        if (j == (CFG_CAP_BITMAP_LEN >> 1)) {
            off += sprintf(port_info->display_buf + off, "%s", "\n\t\t\t\t                 ");
        } else if ((j != CFG_CAP_BITMAP_LEN) && ((j % CFG_DWORD_LEN) == 0)) {
            off += sprintf(port_info->display_buf + off, "%s", "-");
        }
    }

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tEnabled Port Cap: ");

    for (uint8_t k = 0; k < PORT_CAP_NUM; k++) {
        if (to_1bit(data, k)) {
            off += sprintf(port_info->display_buf + off, "%s ", port_cap_list[k]);
            cnt++;
            if (cnt % LINE_PARSE == 0) {
                off += sprintf(port_info->display_buf + off, "\n\t\t\t\t                  ");
            }
        }
    }

    printf("%s", port_info->display_buf);
    /**
     * In the UB protocol, bit0 of the bitmap is invalid. It is used
     * here to mark whether the bitmap has been initialized.
     */
    port_info->port_cap_bits[0] |= 0x1;
}

static void port_basic_info(uint8_t *data)
{
    uint16_t port_index;
    uint8_t port_type, domain_boundary;

    port_index = (uint16_t)to_chunkbits(data, CFG_BIT0, CFG_BIT15);
    port_type = to_1bit(data, CFG_BIT16);
    domain_boundary = to_1bit(data, CFG_BIT17);

    sprintf(port_info->display_buf, "\n\t\t\t\tPort info:\n\t\t\t\t\tPort index:%u"
            "\n\t\t\t\t\tPort type:%s\n\t\t\t\t\tEnumeration Boundary%s",
            port_index, port_type ? "Virtual" : "Physical", bit_parser(domain_boundary));

    printf("%s", port_info->display_buf);
}

static void port_basic_nb_info(uint8_t *data)
{
#define GUID_SIZE 16
    uint16_t vendor_id, device_id, port_index;
    uint8_t *guid_data, *port_data;
    uint8_t type, version;
    uint64_t seq_num;
    uint32_t rsvd;

    port_data = data;
    port_index = (uint16_t)to_chunkbits(port_data, CFG_BIT0, CFG_BIT15);

    sprintf(port_info->display_buf,
            "\n\t\t\t\tNeighbor Port Info:\n\t\t\t\t\tNeighbor_port_idx:%u", port_index);
    printf("%s", port_info->display_buf);

    guid_data = data + CFG_DWORD_LEN;
    seq_num = to_uint64(guid_data);
    guid_data += CFG_QWORD_LEN;
    rsvd = (uint32_t)to_chunkbits(guid_data, CFG_BIT0, CFG_BIT23);
    type = (uint8_t)to_chunkbits(guid_data, CFG_BIT24, CFG_BIT27);
    version = (uint8_t)to_chunkbits(guid_data, CFG_BIT28, CFG_BIT31);
    device_id = (uint16_t)to_chunkbits(guid_data, CFG_BIT32, CFG_BIT47);
    vendor_id = (uint16_t)to_chunkbits(guid_data, CFG_BIT48, CFG_BIT63);
    sprintf(port_info->display_buf, "\n\t\t\t\t\tNeighbor_port_GUID: "
            "%04x-%04x-%01x-%01x-%06x-%016lx", vendor_id, device_id, version, type, rsvd, seq_num);
    printf("%s", port_info->display_buf);
}

static void port_basic_cna(uint8_t *data)
{
    uint32_t cna;

    cna = (uint32_t)to_chunkbits(data, CFG_BIT0, CFG_BIT23);

    sprintf(port_info->display_buf, "\n\t\t\t\tPort CNA: 0x%x", cna);

    printf("%s", port_info->display_buf);
}

static void port_basic_reset(uint8_t *data)
{
    uint8_t *port_data = &(port_info->data_buf[PORT_BASIC_INFO]);
    uint8_t port_type;
    uint8_t port_rst;

    port_type = to_1bit(port_data, CFG_BIT16);
    if (port_type == 0) {
        port_rst = to_1bit(data, CFG_BIT0);
        (void)sprintf(port_info->display_buf, "\n\t\t\t\tPort Reset: 0x%x", port_rst);
        (void)printf("%s", port_info->display_buf);
    }
}

/*
 * list port basic configuration space
 */
int lsub_port_basic(struct ub_entity_cfg_info *info)
{
    static struct ub_cfg_basic_cat port_basic[] = {
        { PORT_BASIC_CAP_BITMAP, port_basic_cap_bitmap },
        { PORT_BASIC_INFO, port_basic_info },
        { PORT_BASIC_NB_INFO, port_basic_nb_info },
        { PORT_BASIC_CNA, port_basic_cna },
        { PORT_BASIC_RST, port_basic_reset},
        { 0x0, NULL }
    };
    char desp[128];
    struct ub_cfg_basic_cat *cur;
    uint32_t data_len;
    uint8_t slice_ver;
    uint32_t slice_size;
    int i;
    int ret_code;

    port_info = info;

    /* If uent is not mue, skip it. */
    if (!ub_is_primary(port_info->uent)) {
        return 0;
    }

    memset(port_info->data_buf, 0, CFG_SLICE_LEN);
    data_len = CFG_SLICE_LEN;
    ret_code = slice_read(port_info->uent, PORT_SLICE_ADDR(port_info->port, 0), port_info->data_buf, &data_len);
    if (ret_code) {
        fprintf(stderr, "lsub error: failed to read PORT BASIC.\n");
        return -EIO;
    }

    slice_ver = slice_get_version(port_info->data_buf);
    slice_size = slice_get_size(port_info->data_buf);
    sprintf(desp, "\n\t\tPORT_BASIC: slice[0x%x, 0x%x]", slice_ver, slice_size);
    printf("%s", desp);
    for (i = 0; port_basic[i].show != NULL; i++) {
        cur = &(port_basic[i]);
        /* Do not exceed the data length */
        if (cur->pos >= data_len) {
            break;
        }
        cur->show(&(port_info->data_buf[cur->pos]));
    }

    return 0;
}
