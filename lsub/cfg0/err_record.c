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
#include "cfg0.h"

static void unctb_err_sts_and_mask(uint8_t *data, char *display_buf)
{
    uint8_t *sup_data;
    uint8_t *sev_data;
    int off;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    sup_data = data + CFG_QWORD_LEN;
    sev_data = data + CTB_ERR_SEV - UNCTB_ERR_STS;

    off = sprintf(display_buf,
                  "\n\t\t\tDevice Uncorrectable Error Status and Mask:");

    if (!to_1bit(sev_data, CFG_BIT20)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tDL Protocol Error                    Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT20)), bit_parser(to_1bit(sup_data, CFG_BIT20)));
    }

    if (!to_1bit(sev_data, CFG_BIT21)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tDL Replay Abort Error                Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT21)), bit_parser(to_1bit(sup_data, CFG_BIT21)));
    }

    if (!to_1bit(sev_data, CFG_BIT23)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tPacket Length Error                  Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT23)), bit_parser(to_1bit(sup_data, CFG_BIT23)));
    }

    if (!to_1bit(sev_data, CFG_BIT27)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tICRC Check Error                     Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT27)), bit_parser(to_1bit(sup_data, CFG_BIT27)));
    }

    off += sprintf(display_buf + off,
                   "\n\t\t\t\tReceiver Overflow                    Status%s Mask%s"
                   "\n\t\t\t\tFlow Control Overflow                Status%s Mask%s",
                   bit_parser(to_1bit(data, CFG_BIT28)), bit_parser(to_1bit(sup_data, CFG_BIT28)),
                   bit_parser(to_1bit(data, CFG_BIT29)), bit_parser(to_1bit(sup_data, CFG_BIT29)));

    printf("%s", display_buf);
}

static void unctb_err_sev(uint8_t *data, char *display_buf)
{
    uint8_t btmp0, btmp1;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    btmp0 = to_1bit(data, CFG_BIT28);
    btmp1 = to_1bit(data, CFG_BIT29);

    sprintf(display_buf,
            "\n\t\t\tDevice Uncorrectable Error Severity:"
            "\n\t\t\t\tReceiver Overflow                    Severity: %s"
            "\n\t\t\t\tFlow Control Overflow                Severity: %s",
            btmp0 ? "Fatal" : "Non-Fatal",
            btmp1 ? "Fatal" : "Non-Fatal");

    printf("%s", display_buf);
}

static void ctb_err_sts_and_mask(uint8_t *data, char *display_buf)
{
    uint8_t *sup_data;
    uint8_t *sev_data;
    int off;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    sup_data = data + CFG_QWORD_LEN;
    sev_data = data + CTB_ERR_SEV - CTB_ERR_STS;

    off = sprintf(display_buf,
                  "\n\t\t\tDevice Correctable Error Status and Mask:"
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
                  bit_parser(to_1bit(data, CFG_BIT1)), bit_parser(to_1bit(sup_data, CFG_BIT1)),
                  bit_parser(to_1bit(data, CFG_BIT2)), bit_parser(to_1bit(sup_data, CFG_BIT2)),
                  bit_parser(to_1bit(data, CFG_BIT3)), bit_parser(to_1bit(sup_data, CFG_BIT3)),
                  bit_parser(to_1bit(data, CFG_BIT4)), bit_parser(to_1bit(sup_data, CFG_BIT4)),
                  bit_parser(to_1bit(data, CFG_BIT5)), bit_parser(to_1bit(sup_data, CFG_BIT5)),
                  bit_parser(to_1bit(data, CFG_BIT6)), bit_parser(to_1bit(sup_data, CFG_BIT6)),
                  bit_parser(to_1bit(data, CFG_BIT7)), bit_parser(to_1bit(sup_data, CFG_BIT7)),
                  bit_parser(to_1bit(data, CFG_BIT8)), bit_parser(to_1bit(sup_data, CFG_BIT8)),
                  bit_parser(to_1bit(data, CFG_BIT9)), bit_parser(to_1bit(sup_data, CFG_BIT9)),
                  bit_parser(to_1bit(data, CFG_BIT10)), bit_parser(to_1bit(sup_data, CFG_BIT10)),
                  bit_parser(to_1bit(data, CFG_BIT11)), bit_parser(to_1bit(sup_data, CFG_BIT11)),
                  bit_parser(to_1bit(data, CFG_BIT14)), bit_parser(to_1bit(sup_data, CFG_BIT14)),
                  bit_parser(to_1bit(data, CFG_BIT15)), bit_parser(to_1bit(sup_data, CFG_BIT15)),
                  bit_parser(to_1bit(data, CFG_BIT17)), bit_parser(to_1bit(sup_data, CFG_BIT17)),
                  bit_parser(to_1bit(data, CFG_BIT18)), bit_parser(to_1bit(sup_data, CFG_BIT18)),
                  bit_parser(to_1bit(data, CFG_BIT19)), bit_parser(to_1bit(sup_data, CFG_BIT19)));

    if (to_1bit(sev_data, CFG_BIT20)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tDL Protocol Error                    Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT20)), bit_parser(to_1bit(sup_data, CFG_BIT20)));
    }

    if (to_1bit(sev_data, CFG_BIT21)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tDL Retry Error                       Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT21)), bit_parser(to_1bit(sup_data, CFG_BIT21)));
    }

    if (to_1bit(sev_data, CFG_BIT23)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tPacket Length Error                  Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT23)), bit_parser(to_1bit(sup_data, CFG_BIT23)));
    }

    if (to_1bit(sev_data, CFG_BIT27)) {
            off += sprintf(display_buf + off,
                           "\n\t\t\t\tICRC Check Error                     Status%s Mask%s",
                           bit_parser(to_1bit(data, CFG_BIT27)), bit_parser(to_1bit(sup_data, CFG_BIT27)));
    }

    printf("%s", display_buf);
}

static void ctb_err_sev(uint8_t *data, char *display_buf)
{
    uint8_t btmp0, btmp1, btmp2, btmp3;

    if (!ub_is_primary(cfg0_info->uent)) {
        return;
    }

    btmp0 = to_1bit(data, CFG_BIT20);
    btmp1 = to_1bit(data, CFG_BIT21);
    btmp2 = to_1bit(data, CFG_BIT22);
    btmp3 = to_1bit(data, CFG_BIT23);

    sprintf(display_buf,
            "\n\t\t\tDevice Correctable Error Severity:"
            "\n\t\t\t\tDL Protocol Error                    Severity: %s"
            "\n\t\t\t\tDL Retry Error                       Severity: %s"
            "\n\t\t\t\tPacket Length Error                  Severity: %s"
            "\n\t\t\t\tICRC Check Error                     Severity: %s",
            btmp0 ? "Correctable" : "Non-Fatal",
            btmp1 ? "Correctable" : "Non-Fatal",
            btmp2 ? "Correctable" : "Non-Fatal",
            btmp3 ? "Correctable" : "Non-Fatal");

    printf("%s", display_buf);
}

void cfg0_err_record_cap(uint8_t *data, uint32_t data_len)
{
    uint8_t slice_ver;
    uint32_t slice_size;

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    sprintf(cfg0_info->display_buf,
        "\n\t\tCFG0_CAP3_DEVICE_ERR_RECORD: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG0_ERR_RECORD_CAP_ID);

    (void)printf("%s", cfg0_info->display_buf);

    if (data_len < ERR_RECORD_LEN) {
        return;
    }

    unctb_err_sts_and_mask(data + UNCTB_ERR_STS, cfg0_info->display_buf);
    unctb_err_sev(data + UNCTB_ERR_SEV, cfg0_info->display_buf);
    ctb_err_sts_and_mask(data + CTB_ERR_STS, cfg0_info->display_buf);
    ctb_err_sev(data + CTB_ERR_SEV, cfg0_info->display_buf);
}