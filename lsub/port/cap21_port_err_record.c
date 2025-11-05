// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <lsub-regs.h>
#include "../lsub.h"
#include "port.h"

static void port_dev_uc_err_st_mask(uint8_t *data)
{
    uint8_t *sup_data;
    uint8_t *sev_data;
    int off;

    sup_data = data + CFG_QWORD_LEN;
    sev_data = data + PORT_C_ERR_SEV - PORT_DEV_UC_ERR_ST;

    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPort Uncorrectable Error Status and Mask:");

    if (!to_1bit(sev_data, CFG_BIT20)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tDL Protocol Error                    Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT20)), bit_parser(to_1bit(sup_data, CFG_BIT20)));
    }

    if (!to_1bit(sev_data, CFG_BIT21)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tDL Replay Abort Error                Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT21)), bit_parser(to_1bit(sup_data, CFG_BIT21)));
    }

    if (!to_1bit(sev_data, CFG_BIT23)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tPacket Length Error                  Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT23)), bit_parser(to_1bit(sup_data, CFG_BIT23)));
    }

    if (!to_1bit(sev_data, CFG_BIT27)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tICRC Check Error                     Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT27)), bit_parser(to_1bit(sup_data, CFG_BIT27)));
    }

    off += sprintf(port_info->display_buf + off,
                   "\n\t\t\t\tReceive Buffer Overflow              Status%s Mask%s"
                   "\n\t\t\t\tFlow Control Overflow                Status%s Mask%s",
                   bit_parser(to_1bit(data, CFG_BIT28)), bit_parser(to_1bit(sup_data, CFG_BIT28)),
                   bit_parser(to_1bit(data, CFG_BIT29)), bit_parser(to_1bit(sup_data, CFG_BIT29)));

    printf("%s", port_info->display_buf);
}

static void port_dev_uc_err_sev(uint8_t *data)
{
    uint8_t btmp0, btmp1;

    btmp0 = to_1bit(data, CFG_BIT28);
    btmp1 = to_1bit(data, CFG_BIT29);
    sprintf(port_info->display_buf,
        "\n\t\t\tPort Uncorrectable Error Severity:"
        "\n\t\t\t\tReceive Buffer Overflow              Severity: %s"
        "\n\t\t\t\tFlow Control Overflow                Severity: %s",
        btmp0 ? "Fatal" : "Non-Fatal",
        btmp1 ? "Fatal" : "Non-Fatal");

    printf("%s", port_info->display_buf);
}

static void port_c_err_st_mask(uint8_t *data)
{
#define PORT_C_ERR_STA_NUM 32
    uint8_t btmp_mask[PORT_C_ERR_STA_NUM];
    uint8_t btmp_st[PORT_C_ERR_STA_NUM];
    uint8_t *err_data;
    uint8_t *sev_data;
    uint8_t i;
    int off;

    for (i = 0; i < PORT_C_ERR_STA_NUM; i++) {
        btmp_st[i] = to_1bit(data, i);
    }

    err_data = data + CFG_QWORD_LEN;
    sev_data = data + PORT_C_ERR_SEV - PORT_C_ERR_ST;

    for (i = 0; i < PORT_C_ERR_STA_NUM; i++) {
        btmp_mask[i] = to_1bit(err_data, i);
    }

    off = sprintf(port_info->display_buf,
                  "\n\t\t\tPort Correctable Error Status and Mask:"
                  "\n\t\t\t\tBlock Align Unlock                   Status%s Mask%s"
                  "\n\t\t\t\tElasticity Buffer Overflow/Underflow Status%s Mask%s"
                  "\n\t\t\t\tLoss of Lane-to-Lane Deskw           Status%s Mask%s"
                  "\n\t\t\t\tAMCTL decode Error                   Status%s Mask%s"
                  "\n\t\t\t\tLTB CRC Erro                         Status%s Mask%s"
                  "\n\t\t\t\tLink speed reduce Error              Status%s Mask%s"
                  "\n\t\t\t\tLink width reduce Error              Status%s Mask%s"
                  "\n\t\t\t\tEQ Coarsetune Timeout                Status%s Mask%s"
                  "\n\t\t\t\tEQ Finetune Timeout                  Status%s Mask%s"
                  "\n\t\t\t\tLink width negotiation Timeout       Status%s Mask%s"
                  "\n\t\t\t\tLMSM Discovery Timeout               Status%s Mask%s"
                  "\n\t\t\t\tQDLWS timeout                        Status%s Mask%s"
                  "\n\t\t\t\tUnsupport LW Switch REQ              Status%s Mask%s"
                  "\n\t\t\t\tLMSM Retrain state Timeout           Status%s Mask%s"
                  "\n\t\t\t\tDL Retry ACK Timeout                 Status%s Mask%s"
                  "\n\t\t\t\tDL Retry Rollover                    Status%s Mask%s",
                  bit_parser(btmp_st[CFG_BIT1]), bit_parser(btmp_mask[CFG_BIT1]),
                  bit_parser(btmp_st[CFG_BIT2]), bit_parser(btmp_mask[CFG_BIT2]),
                  bit_parser(btmp_st[CFG_BIT3]), bit_parser(btmp_mask[CFG_BIT3]),
                  bit_parser(btmp_st[CFG_BIT4]), bit_parser(btmp_mask[CFG_BIT4]),
                  bit_parser(btmp_st[CFG_BIT5]), bit_parser(btmp_mask[CFG_BIT5]),
                  bit_parser(btmp_st[CFG_BIT6]), bit_parser(btmp_mask[CFG_BIT6]),
                  bit_parser(btmp_st[CFG_BIT7]), bit_parser(btmp_mask[CFG_BIT7]),
                  bit_parser(btmp_st[CFG_BIT8]), bit_parser(btmp_mask[CFG_BIT8]),
                  bit_parser(btmp_st[CFG_BIT9]), bit_parser(btmp_mask[CFG_BIT9]),
                  bit_parser(btmp_st[CFG_BIT10]), bit_parser(btmp_mask[CFG_BIT10]),
                  bit_parser(btmp_st[CFG_BIT11]), bit_parser(btmp_mask[CFG_BIT11]),
                  bit_parser(btmp_st[CFG_BIT14]), bit_parser(btmp_mask[CFG_BIT14]),
                  bit_parser(btmp_st[CFG_BIT15]), bit_parser(btmp_mask[CFG_BIT15]),
                  bit_parser(btmp_st[CFG_BIT17]), bit_parser(btmp_mask[CFG_BIT17]),
                  bit_parser(btmp_st[CFG_BIT18]), bit_parser(btmp_mask[CFG_BIT18]),
                  bit_parser(btmp_st[CFG_BIT19]), bit_parser(btmp_mask[CFG_BIT19]));

    if (to_1bit(sev_data, CFG_BIT20)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tDL Protocol Error                    Status%s Mask%s",
                           bit_parser(btmp_st[CFG_BIT20]), bit_parser(btmp_mask[CFG_BIT20]));
    }

    if (to_1bit(sev_data, CFG_BIT21)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tDL Retry Error                       Status%s Mask%s",
                           bit_parser(btmp_st[CFG_BIT21]), bit_parser(btmp_mask[CFG_BIT21]));
    }

    if (to_1bit(sev_data, CFG_BIT23)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tPacket Length Error                  Status%s Mask%s",
                           bit_parser(btmp_st[CFG_BIT23]), bit_parser(btmp_mask[CFG_BIT23]));
    }

    if (to_1bit(sev_data, CFG_BIT27)) {
            off += sprintf(port_info->display_buf + off,
                           "\n\t\t\t\tICRC Check Error                     Status%s Mask%s",
                           bit_parser(btmp_st[CFG_BIT27]), bit_parser(btmp_mask[CFG_BIT27]));
    }

    printf("%s", port_info->display_buf);
}

static void port_c_err_sev(uint8_t *data)
{
    uint8_t btmp0, btmp1, btmp2, btmp3;

    btmp0 = to_1bit(data, CFG_BIT20);
    btmp1 = to_1bit(data, CFG_BIT21);
    btmp2 = to_1bit(data, CFG_BIT23);
    btmp3 = to_1bit(data, CFG_BIT27);
    sprintf(port_info->display_buf,
        "\n\t\t\tPort Correctable Error Severity:"
        "\n\t\t\t\tDL Protocol Error                    Severity: %s"
        "\n\t\t\t\tDL Retry Error                       Severity: %s"
        "\n\t\t\t\tPacket Length Error                  Severity: %s"
        "\n\t\t\t\tICRC Check Error                     Severity: %s",
        btmp0 ? "Correctable" : "Non-Fatal",
        btmp1 ? "Correctable" : "Non-Fatal",
        btmp2 ? "Correctable" : "Non-Fatal",
        btmp3 ? "Correctable" : "Non-Fatal");

    printf("%s", port_info->display_buf);
}

void port_cap21_err_record(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;
    uint8_t *cur_data;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(port_info->display_buf, "\n\t\tport error record: slice[0x%x, 0x%x] id[%u]", slice_ver, slice_size,
            port_cap_id);
    printf("%s", port_info->display_buf);

     if (data_len < PORT_ERR_RECORD_LEN) {
        return;
    }

    cur_data = data + PORT_DEV_UC_ERR_ST;
    port_dev_uc_err_st_mask(cur_data);

    cur_data = data + PORT_DEV_UC_ERR_SEV;
    port_dev_uc_err_sev(cur_data);

    cur_data = data + PORT_C_ERR_ST;
    port_c_err_st_mask(cur_data);

    cur_data = data + PORT_C_ERR_SEV;
    port_c_err_sev(cur_data);
}