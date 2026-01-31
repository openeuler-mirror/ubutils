// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <string.h>
#include <lsub-regs.h>
#include "../lsub.h"
#include "port.h"

static struct val_desc eq_desc[] = { { 0x0, "Full_EQ" },
                                     { 0x1, "Only_Highest_Data_Rate_EQ" },
                                     { 0x2, "Skip_EQ" },
                                     { INVALID_DESC, "" } };
static struct val_desc link_speed_desc[] = { { 0x0, "Data_Rate0"}, { 0x1, "Data_Rate1"}, { 0x2, "Data_Rate2"},
                                             { 0x3, "Data_Rate3"}, { 0x4, "Data_Rate4"}, { 0x5, "Data_Rate5"},
                                             { 0x6, "Data_Rate6"}, { 0x7, "Data_Rate7"}, { 0x8, "Data_Rate8"},
                                             { 0x9, "Data_Rate9"}, { INVALID_DESC, "" } };
static struct val_desc width_desc[] = { { 0x0, "Reserved"}, { 0x1, "x1" }, { 0x2, "x2" }, { 0x4, "x4" },
                                        { 0x8, "x8" }, { INVALID_DESC, "" } };
static struct val_desc retry_ack_desc[] = { { 0x0, "NORMAL" }, { 0x1 , "ACK" },
                                            { INVALID_DESC, "" } };
static struct val_desc retry_req_desc[] = { { 0x0, "NORMAL"}, { 0x1, "REQ" }, { 0x2, "WAIT" }, { 0x3, "RETRAIN" },
                                            { 0x4, "ERROR" }, { INVALID_DESC, "" } };
static struct val_desc link_desc[] = { { 0x0, "DLL_DISABLE"}, { 0x1, "DLL_PARA_INIT" }, { 0x2, "DLL_CRD_INIT" },
                                       { 0x3, "DLL_NORMAL" }, { INVALID_DESC, "" } };

#define PORT_VECTOR_SIZE 10
static const char *bit_num[CFG_1BYTE_BITS] = { "1", "2", "4", "8", "16", "32", "64", "128" };
static const char *mult_num[] = { "Rate0", "Rate1", "Rate2", "Rate3",
                                  "Rate4", "Rate5", "Rate6", "Rate7",
                                  "Rate8", "Rate9" };
static const char *fec_mode[CFG_1BYTE_BITS] = { "X1", "X2", "X4", "X8", "RSVD", "RSVD", "RSVD", "RSVD" };

/*
 * This function returns the address of a static character array. You cannot call this function multiple times
 * within a sprintf or printf, as the result returned will be overwritten by the last call.
 */
static char *get_bit_mult_desc(uint8_t *value, uint8_t bit_len, const char *desc[])
{
#define DESP_SIZE 200
    static char disable[] = "all rate disable";
    static char reserved[] = "reserved";
    static char ret[DESP_SIZE];
    uint8_t i;

    memset(ret, 0, DESP_SIZE);

    for (i = 0; i < bit_len; i++) {
        if (to_1bit(value, i)) {
            strcat(ret, " ");
            strcat(ret, desc[i]);
        }
    }

    if (strlen(ret) == 0) {
        if (bit_len == PORT_VECTOR_SIZE) {
            return disable;
        }
        return reserved;
    }

    return ret;
}

static void pl_data_cap1(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;
    uint64_t tmp0, tmp1, tmp2;
    int off, i;

    cur_data = data + PL_DATA_CAP1;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    btmp0 = to_1bit(cur_data, CFG_BIT16);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Capability 1:"
                  "\n\t\t\t\tFEATURE_ID:0x%x\n\t\t\t\tRXBUF_VL_SHARE_pro%s", (uint32_t)tmp0,
                  bit_parser(btmp0));
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT7);
    tmp1 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT15);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tDATA_ACK_GRAIN_SIZE_pro:%s",
                   get_bit_mult_desc((uint8_t *)&tmp0, CFG_1BYTE_BITS, bit_num));
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_ACK_GRAIN_SIZE_pro:%s",
                   get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num));
    cur_data += CFG_DWORD_LEN;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_CREDIT_GRAIN_SIZE_pro VL0~15:");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tDATA_CREDIT_GRAIN_SIZE_pro VL0~15:");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    tmp1 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT23);
    tmp2 = to_chunkbits(cur_data, CFG_BIT24, CFG_BIT31);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tVL_ENABLE_pro:0x%x\n\t\t\t\tFLOW_CTRL_SIZE_pro:%s"
                   "\n\t\t\t\tPACKET_MIN_INTERVAL:0x%x",
                   (uint16_t)tmp0, get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num),
                   (uint16_t)tmp2);
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT19);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tRETRY_BUF_DEPTH:0x%x", (uint32_t)tmp0);

    printf("%s", port_info->display_buf);
}

static void pl_phy_cap1(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t tmp0, tmp1, tmp2, tmp3, tmp4;
    uint8_t btmp0, btmp1;
    int off;

    static const char *phy_mult_desc[] = { "PHY-Mode1", "PHY-Mode2" };
    static const char *fec_mult_desc[] = { "RS(128,120,T=2)", "RS(128,120,T=4)"};
    static const char *eq_mult_desc[] = { "Full-EQ", "Only_Highest_Data_Rate_EQ",
                                          "Skip_EQ" };

    cur_data = data + PL_PHY_CAP1;

    tmp0 = (uint8_t)to_chunkbits(cur_data, CFG_BIT0, CFG_BIT3);
    tmp1 = (uint8_t)to_chunkbits(cur_data, CFG_BIT4, CFG_BIT9);
    tmp2 = (uint8_t)to_chunkbits(cur_data, CFG_BIT10, CFG_BIT15);
    tmp3 = (uint8_t)to_chunkbits(cur_data, CFG_BIT16, CFG_BIT23);
    tmp4 = (uint8_t)to_chunkbits(cur_data, CFG_BIT24, CFG_BIT27);
    btmp0 = to_1bit(cur_data, CFG_BIT28);
    btmp1 = to_1bit(cur_data, CFG_BIT29);

    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Capability 1:"
                  "\n\t\t\t\tMax Link Speed:%s\n\t\t\t\tMaximum Link Width:%s",
                  get_description(tmp0, link_speed_desc), get_description(tmp1, width_desc));
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tPHY Mode support:%s",
                   get_bit_mult_desc(&tmp2, CFG_BIT2, phy_mult_desc));
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tFEC Mode support:%s",
                   get_bit_mult_desc(&tmp3, CFG_BIT2, fec_mult_desc));
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tEQ Mode support:%s",
                   get_bit_mult_desc(&tmp4, CFG_BIT3, eq_mult_desc));
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tOver fibre support%s\n\t\t\t\tFix data_Rate mode support%s",
                   bit_parser(btmp0), bit_parser(btmp1));

    printf("%s", port_info->display_buf);
}

static void pl_phy_cap2(uint8_t *data)
{
    uint8_t *cur_data;
    uint16_t tmp0;

    cur_data = data + PL_PHY_CAP2;

    tmp0 = (uint16_t)to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Capability 2:"
                  "\n\t\t\t\tSupported Link Speeds Vector(Data Rate):%s",
                  get_bit_mult_desc((uint8_t *)&tmp0, PORT_VECTOR_SIZE, mult_num));

    printf("%s", port_info->display_buf);
}

static void pl_phy_cap3(uint8_t *data)
{
    uint8_t btmp0, btmp1, btmp2, btmp3;
    uint8_t *cur_data;

    cur_data = data + PL_PHY_CAP3;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    btmp1 = to_1bit(cur_data, CFG_BIT2);
    btmp2 = to_1bit(cur_data, CFG_BIT3);
    btmp3 = (uint8_t)to_chunkbits(cur_data, CFG_BIT8, CFG_BIT15);
    sprintf(port_info->display_buf,
        "\n\t\t\tPHY Link Capability 3:"
        "\n\t\t\t\tAsymmetry Link support%s\n\t\t\t\tPrecode Support NRZ%s"
        "\n\t\t\t\tPrecode Support PAM4%s\n\t\t\t\tFEC Interleave Mode:%s",
        bit_parser(btmp0), bit_parser(btmp1),
        bit_parser(btmp2), get_bit_mult_desc((uint8_t *)&btmp3, CFG_1BYTE_BITS, fec_mode));

    printf("%s", port_info->display_buf);
}

static void pl_cap(uint8_t *data)
{
    pl_data_cap1(data);
    pl_phy_cap1(data);
    pl_phy_cap2(data);
    pl_phy_cap3(data);
    /* PHY Link Capactiy 4/5 is Resvered, Skip */
}

static void pl_data_cfg(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;
    uint64_t tmp0, tmp1, tmp2;
    int off, i;

    cur_data = data + PL_DATA_CFG;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT7);
    tmp1 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT15);
    btmp0 = to_1bit(cur_data, CFG_BIT16);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Configuration:"
                  "\n\t\t\t\tDATA_ACK_GRAIN_SIZE_cfg:%s",
                  get_bit_mult_desc((uint8_t *)&tmp0, CFG_1BYTE_BITS, bit_num));
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_ACK_GRAIN_SIZE_cfg:%s",
                   get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num));
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tRXBUF_VL_SHARE_cfg%s", bit_parser(btmp0));
    cur_data += CFG_DWORD_LEN;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_CREDIT_GRAIN_SIZE_cfg VL0~15:");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tDATA_CREDIT_GRAIN_SIZE_cfg VL0~15:");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    tmp1 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT23);
    tmp2 = to_chunkbits(cur_data, CFG_BIT24, CFG_BIT31);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tVL_ENABLE_cfg:0x%x\n\t\t\t\tFLOW_CTRL_SIZE_cfg:%s"
                   "\n\t\t\t\tPACKET_MIN_INTERVAL:0x%x",
                   (uint32_t)tmp0, get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num),
                   (uint32_t)tmp2);
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT19);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tRETRY_BUF_DEPTH: 0x%x", (uint32_t)tmp0);
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tFEATURE_ID_cfg: 0x%x", (uint32_t)tmp0);

    printf("%s", port_info->display_buf);
}

static void pl_data_remote_init_credit(uint8_t *data)
{
    uint8_t *cur_data;
    int off, i;

    cur_data = data + PL_DRIC;
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Remote Init Credit Cfg:"
                  "\n\t\t\t\tcfg remote credit VL0~15(hex):"
                  "\n\t\t\t\t\t");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, " %x", *(uint16_t *)cur_data);
        cur_data += CFG_WORD_LEN;
    }

    printf("%s", port_info->display_buf);
}

static void pl_data_retry(uint8_t *data)
{
    uint8_t *cur_data;
    uint32_t tmp0;

    cur_data = data + PL_DATA_RETRY;
    tmp0 = (uint32_t)to_chunkbits(cur_data, CFG_BIT0, CFG_BIT31);
    (void)sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Retry Cfg:\n\t\t\t\tWait Retry Ack Timeout_L:0x%xus", tmp0);

    printf("%s", port_info->display_buf);
}

static void pl_data_perf_optim(uint8_t *data)
{
    uint8_t *cur_data;
    uint16_t tmp0, tmp1;
    int off, i;

    cur_data = data + PL_DATA_PERF_OPTIM;
    tmp0 = (uint16_t)to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    tmp1 = (uint16_t)to_chunkbits(cur_data, CFG_BIT16, CFG_BIT31);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Performance Optimization Cfg:"
                  "\n\t\t\t\tCfg Ack Return:0x%x\n\t\t\t\tCfg Ack Llcrd:0x%x", tmp0, tmp1);

    cur_data = data + PL_DATA_CRD_VN;
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCfg Crd Vn Llcrd VL0~15(hex):"
                                                 "\n\t\t\t\t\t");
    for (i = 0; i < PORT_VN_NUM; i++) {
        off += sprintf(port_info->display_buf + off, " %x", *(uint16_t *)cur_data);
        cur_data += CFG_WORD_LEN;
    }

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCfg Crd Vn Return VL0~15(hex):"
                                                 "\n\t\t\t\t\t");
    for (i = 0; i < PORT_VN_NUM; i++) {
        off += sprintf(port_info->display_buf + off, " %x", *(uint16_t *)cur_data);
        cur_data += CFG_WORD_LEN;
    }

    printf("%s", port_info->display_buf);
}

static void pl_data_markerr(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;

    cur_data = data + PL_DATA_MKERR;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    sprintf(port_info->display_buf,
            "\n\t\t\tData Link MarkErr Cfg:\n\t\t\t\tCfg MarkErr En%s", bit_parser(btmp0));

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl1(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;
    uint64_t tmp0, tmp1;
    int off;

    static struct val_desc phy_mode_desc[] = { { 0x0, "PHY-mode1" }, { 0x1, "PHY-mode2" }, { INVALID_DESC, "" } };

    cur_data = data + PL_PHY_CTRL1;
    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT3);

    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Control 1:\n\t\t\t\tTarget Link Speed:%s",
                  get_description(tmp0, link_speed_desc));

    tmp0 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT13);
    tmp1 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT21);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tTarget TX Link Width:%s\n\t\t\t\tTarget RX Link Width:%s",
                   get_description(tmp0, width_desc), get_description(tmp1, width_desc));

    btmp0 = to_1bit(cur_data, CFG_BIT22);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tChange Link width not Retrain Link%s", bit_parser(btmp0));

    tmp0 = to_chunkbits(cur_data, CFG_BIT24, CFG_BIT27);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tPHY Mode Control:%s", get_description(tmp0, phy_mode_desc));

    btmp0 = to_1bit(cur_data, CFG_BIT31);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tRetrain Link%s", bit_parser(btmp0));

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl2(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0, btmp1, btmp2;
    uint64_t tmp0;
    int off;

    cur_data = data + PL_PHY_CTRL2;

    tmp0 = to_chunkbits(cur_data, CFG_BIT4, CFG_BIT5);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Control 2:\n\t\t\t\tEQ Mode control:%s",
                  get_description(tmp0, eq_desc));

    btmp0 = to_1bit(cur_data, CFG_BIT6);
    btmp1 = to_1bit(cur_data, CFG_BIT7);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tDe-emphasis value:%s\n\t\t\t\tPerform Equalization%s",
                   btmp0 ? "-6dB" : "-3.5dB", bit_parser(btmp1));

    btmp0 = to_1bit(cur_data, CFG_BIT16);
    btmp1 = to_1bit(cur_data, CFG_BIT17);
    btmp2 = to_1bit(cur_data, CFG_BIT18);
    off += sprintf(port_info->display_buf + off,
        "\n\t\t\t\tOver fibre mode Enable%s\n\t\t\t\tFix data_Rate mode Enable%s"
        "\n\t\t\t\tBypass Probe Enable%s", bit_parser(btmp0),
        bit_parser(btmp1), bit_parser(btmp2));

    btmp0 = to_1bit(cur_data, CFG_BIT19);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tPort Type Nego Enable%s", bit_parser(btmp0));

    btmp0 = to_1bit(cur_data, CFG_BIT21);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tLink Disable%s", bit_parser(btmp0));

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl5(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0;

    cur_data = data + PL_PHY_CTRL5;

    tmp0 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT31);
    sprintf(port_info->display_buf,
                   "\n\t\t\tPHY Link Control 5:\n\t\t\t\tPrecode Enable Vector:%s",
                   get_bit_mult_desc((uint8_t *)&tmp0, PORT_VECTOR_SIZE, mult_num));

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl6_7(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0;
    int off = 0, i;

    cur_data = data + PL_PHY_CTRL6;
    off = sprintf(port_info->display_buf, "\n\t\t\tPHY Link Control 6 and 7:");
    for (i = 0; i <= PORT_RATE_SIZE; i++) {
        tmp0 = to_chunkbits(cur_data, (uint8_t)(CFG_BIT0 + i * CFG_BIT4), (uint8_t)(CFG_BIT3 + i * CFG_BIT4));
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\tFEC Mode for Data_Rate%d:%s", i,
                       get_description(tmp0, fec_desc));
    }

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl8_9(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0;
    int off = 0;
    uint8_t i;

    cur_data = data + PL_PHY_CTRL8;
    off = sprintf(port_info->display_buf, "\n\t\t\tPHY Link Control 8 and 9:");
    for (i = 0; i <= PORT_RATE_SIZE; i++) {
        tmp0 = to_chunkbits(cur_data, (uint8_t)(CFG_BIT0 + i * CFG_BIT4), (uint8_t)(CFG_BIT3 + i * CFG_BIT4));
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\tBCRC Mode for Data_Rate%d:%s", i,
                       get_description(tmp0, bcrc_desc));
    }

    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl10(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0;

    cur_data = data + PL_PHY_CTRL10;
    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);

    sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Control 10:\n\t\t\t\tFEC Interleave Enable Vector:%s",
                  get_bit_mult_desc((uint8_t *)&tmp0, PORT_VECTOR_SIZE, mult_num));
    printf("%s", port_info->display_buf);
}

static void pl_phy_ctrl11(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0;

    cur_data = data + PL_PHY_CTRL11;
    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);

    sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Control 11:\n\t\t\t\tSkip EQ Coarsetune Enable:%s",
                  get_bit_mult_desc((uint8_t *)&tmp0, PORT_VECTOR_SIZE, mult_num));
    printf("%s", port_info->display_buf);
}

static void pl_config(uint8_t *data)
{
    pl_data_cfg(data);
    pl_data_remote_init_credit(data);
    pl_data_retry(data);
    pl_data_perf_optim(data);
    pl_data_markerr(data);
    /* Skip Data Link Control Reserve 1 */
    pl_phy_ctrl1(data);
    pl_phy_ctrl2(data);
    /* PHY Link Control 3 4 is Resvered, Skip */
    pl_phy_ctrl5(data);
    pl_phy_ctrl6_7(data);
    pl_phy_ctrl8_9(data);
    pl_phy_ctrl10(data);
    pl_phy_ctrl11(data);
}

static void pl_data_status(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;
    uint64_t tmp0, tmp1, tmp2;
    int off, i;

    cur_data = data + PL_DATA_STATUS;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tData Link Status:"
                  "\n\t\t\t\tRXBUF_VL_SHARE_pro_sta%s",
                  bit_parser(btmp0));
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT7);
    tmp1 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT15);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tDATA_ACK_GRAIN_SIZE_pro_sta:%s",
                   get_bit_mult_desc((uint8_t *)&tmp0, CFG_1BYTE_BITS, bit_num));
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_ACK_GRAIN_SIZE_pro_sta:%s",
                   get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num));
    cur_data += CFG_DWORD_LEN;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tCTRL_CREDIT_GRAIN_SIZE_pro_sta VL0~15(hex):");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tDATA_CREDIT_GRAIN_SIZE_pro_sta VL0~15(hex):");
    for (i = 0; i < PORT_VL_NUM; i++) {
        off += sprintf(port_info->display_buf + off, "\n\t\t\t\t\tVL[%d]:%s", i,
                       get_bit_mult_desc(cur_data + i, CFG_1BYTE_BITS, bit_num));
    }
    cur_data += PORT_VL_NUM;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT15);
    tmp1 = to_chunkbits(cur_data, CFG_BIT16, CFG_BIT23);
    tmp2 = to_chunkbits(cur_data, CFG_BIT24, CFG_BIT31);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tVL_ENABLE_pro_sta:0x%x\n\t\t\t\tFLOW_CTRL_SIZE_pro_sta:%s"
                   "\n\t\t\t\tPACKET_MIN_INTERVAL_sta:0x%x",
                   (uint32_t)tmp0, get_bit_mult_desc((uint8_t *)&tmp1, CFG_1BYTE_BITS, bit_num),
                   (uint32_t)tmp2);
    cur_data += CFG_DWORD_LEN;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT19);
    off += sprintf(port_info->display_buf + off, "\n\t\t\t\tRETRY_BUF_DEPTH_sta:0x%x", (uint32_t)tmp0);

    printf("%s", port_info->display_buf);
}

static void pl_dlsms(uint8_t *data)
{
    uint8_t *cur_data;
    uint64_t tmp0, tmp1, tmp2;

    cur_data = data + PL_DLSMS;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT3);
    tmp1 = to_chunkbits(cur_data, CFG_BIT4, CFG_BIT7);
    tmp2 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT11);
    sprintf(port_info->display_buf,
            "\n\t\t\tData Link State Machine Status:"
            "\n\t\t\t\tLink Status:%s\n\t\t\t\tRetry Req Status:%s"
            "\n\t\t\t\tRetry Ack Status:%s", get_description(tmp0, link_desc),
            get_description(tmp1, retry_req_desc), get_description(tmp2, retry_ack_desc));

    printf("%s", port_info->display_buf);
}

static void pl_port_status(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0, btmp1;

    cur_data = data + PL_PORT_LINK_STATUS;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    btmp1 = to_1bit(cur_data, CFG_BIT1);
    sprintf(port_info->display_buf,
            "\n\t\t\tPort Link Status 0:"
            "\n\t\t\t\tPHY Link state:%s\n\t\t\t\tData Link Layer Link State:%s",
            btmp0 ? "Up" : "Down", btmp1 ? "DLL_Normal" : "Others");

    printf("%s", port_info->display_buf);
}

static void pl_phy_status1(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0;
    uint64_t tmp0, tmp1, tmp2;
    int off;

    cur_data = data + PL_PHY_STATUS1;

    tmp0 = to_chunkbits(cur_data, CFG_BIT0, CFG_BIT3);
    tmp1 = to_chunkbits(cur_data, CFG_BIT4, CFG_BIT8);
    tmp2 = to_chunkbits(cur_data, CFG_BIT12, CFG_BIT16);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Status 1:"
                  "\n\t\t\t\tCurrent Link Speed:%s\n\t\t\t\tCurrent TX Link Width:%s"
                  "\n\t\t\t\tCurrent RX Link Width:%s", get_description(tmp0, link_speed_desc),
                  get_description(tmp1, width_desc), get_description(tmp2, width_desc));

    tmp0 = to_chunkbits(cur_data, CFG_BIT20, CFG_BIT25);
    btmp0 = to_1bit(cur_data, CFG_BIT28);
    off += sprintf(port_info->display_buf + off,
        "\n\t\t\t\tCurrent LMSM State:0x%x\n\t\t\t\tCurrent Port Type:%s",
        (uint32_t)tmp0, btmp0 ? "Primary port" : "Secondary port");

    printf("%s", port_info->display_buf);
}

static void pl_phy_status2(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0, btmp1;
    uint64_t tmp0, tmp1, tmp2;
    int off;

    cur_data = data + PL_PHY_STATUS2;

    btmp0 = to_1bit(cur_data, CFG_BIT2);
    btmp1 = to_1bit(cur_data, CFG_BIT3);
    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPHY Link Status 2:"
                  "\n\t\t\t\tCurrent Precode State: TX%s RX%s",
                  bit_parser(btmp0), bit_parser(btmp1));

    tmp0 = to_chunkbits(cur_data, CFG_BIT4, CFG_BIT7);
    tmp1 = to_chunkbits(cur_data, CFG_BIT8, CFG_BIT11);
    tmp2 = to_chunkbits(cur_data, CFG_BIT12, CFG_BIT15);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tCurrent FEC Mode:%s"
                   "\n\t\t\t\tCurrent EQ Mode:%s"
                   "\n\t\t\t\tCurrent BCRC Mode:%s",
                   get_description(tmp0, fec_desc), get_description(tmp1, eq_desc),
                   get_description(tmp2, bcrc_desc));

    btmp0 = to_1bit(cur_data, CFG_BIT16);
    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tCurrent FEC Interleave state%s",
                   bit_parser(btmp0));

    printf("%s", port_info->display_buf);
}

static void pl_phy_status3(uint8_t *data)
{
    uint8_t *cur_data;

    cur_data = data + PL_PHY_STATUS3;

    sprintf(port_info->display_buf, "\n\t\t\tPHY Link Status 3:0x%x", *((uint32_t *)cur_data));

    printf("%s", port_info->display_buf);
}

static void pl_phy_err_status(uint8_t *data)
{
    uint8_t *cur_data;
    uint8_t btmp0, btmp1;

    cur_data = data + PL_PHY_ERR_STATUS;

    btmp0 = to_1bit(cur_data, CFG_BIT0);
    btmp1 = to_1bit(cur_data, CFG_BIT1);
    sprintf(port_info->display_buf,
            "\n\t\t\tPHY Link Error status:"
            "\n\t\t\t\tDeskew_buf_overflow%s Ebch decode err%s",
            bit_parser(btmp0), bit_parser(btmp1));

    printf("%s", port_info->display_buf);
}

static void pl_status(uint8_t *data)
{
    pl_data_status(data);
    pl_dlsms(data);
    /* Skip Data Link status Reserve 1/2/3/4 */
    pl_port_status(data);
    pl_phy_status1(data);
    pl_phy_status2(data);
    pl_phy_status3(data);
    /* Skip PHY Link status 4 */
    pl_phy_err_status(data);
}

void port_cap1_link(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tPORT_CAP1_LINK: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size,
            port_cap_id);
    printf("%s", port_info->display_buf);

    if (data_len < PORT_CAP0_LINK_LEN) {
        return;
    }

    pl_cap(data);
    pl_config(data);
    pl_status(data);
}