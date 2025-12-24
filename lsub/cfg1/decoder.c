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
#include "cfg1.h"

static int cfg1_decoder_cmdq(uint8_t *data, char *display_buf, int off)
{
    const char *err_reason[] = {"no error", "illegal command", "abort error"};
    uint8_t *cmdq_data;
    uint8_t cmdq_en, cmdq_size;
    uint8_t cmdq_wr_err;
    uint8_t cmdq_rd_err;
    uint8_t cmdq_rd_err_reason;
    uint16_t cmdq_wr_idx;
    uint16_t cmdq_rd_idx;
    uint64_t cmdq_addr;
    int ret_off = off;

    cmdq_data = data;
    cmdq_en = to_1bit(cmdq_data, CFG_BIT0);
    cmdq_size = (uint8_t)to_chunkbits(cmdq_data, CFG_BIT8, CFG_BIT11);
    cmdq_data += CFG_DWORD_LEN;

    cmdq_wr_idx = (uint16_t)to_chunkbits(cmdq_data, CFG_BIT0, CFG_BIT10);
    cmdq_wr_err = to_1bit(cmdq_data, CFG_BIT16);
    cmdq_data += CFG_DWORD_LEN;

    cmdq_rd_idx = (uint16_t)to_chunkbits(cmdq_data, CFG_BIT0, CFG_BIT10);
    cmdq_rd_err = to_1bit(cmdq_data, CFG_BIT16);
    cmdq_rd_err_reason = (uint8_t)to_chunkbits(cmdq_data, CFG_BIT17, CFG_BIT19);
    cmdq_data += CFG_DWORD_LEN;

    cmdq_addr = to_chunkbits(cmdq_data, CFG_BIT6, CFG_BIT47);

    ret_off += sprintf(display_buf + ret_off,
        "\n\t\t\tDECODER_CMDQ_CFG: cmdq_en%s cmdq_depth_use:%u"
        "\n\t\t\tDECODER_CMDQ_PROD: cmdq_wr_idx:%u cmdq_err_resp%s"
        "\n\t\t\tDECODER_CMDQ_CONS: cmdq_rd_idx:%u cmdq_err%s cmdq_err_reason:%s"
        "\n\t\t\tDECODER_CMDQ_BASE_ADDR: cmdq_base_addr:0x%lx",
        bit_parser(cmdq_en), cmdq_size, cmdq_wr_idx,
        bit_parser(cmdq_wr_err), cmdq_rd_idx, bit_parser(cmdq_rd_err),
        (cmdq_rd_err_reason < CFG_ARRAY_SIZE(err_reason, char*)) ?
        err_reason[cmdq_rd_err_reason] : CFG_RESERVED, cmdq_addr);

    return ret_off;
}

static int cfg1_decoder_eventq(uint8_t *data, char *display_buf, int off)
{
    uint8_t *eventq_data;
    uint8_t eventq_en, eventq_size;
    uint8_t eventq_wr_err;
    uint8_t eventq_rd_err;
    uint16_t eventq_wr_idx;
    uint16_t eventq_rd_idx;
    uint64_t eventq_addr;
    int ret_off = off;

    eventq_data = data;
    eventq_en = to_1bit(eventq_data, CFG_BIT0);
    eventq_size = (uint8_t)to_chunkbits(eventq_data, CFG_BIT8, CFG_BIT11);
    eventq_data += CFG_DWORD_LEN;

    eventq_wr_idx = (uint16_t)to_chunkbits(eventq_data, CFG_BIT0, CFG_BIT10);
    eventq_wr_err = to_1bit(eventq_data, CFG_BIT31);
    eventq_data += CFG_DWORD_LEN;

    eventq_rd_idx = (uint16_t)to_chunkbits(eventq_data, CFG_BIT0, CFG_BIT10);
    eventq_rd_err = to_1bit(eventq_data, CFG_BIT31);
    eventq_data += CFG_DWORD_LEN;

    eventq_addr = to_chunkbits(eventq_data, CFG_BIT6, CFG_BIT47);

    ret_off += sprintf(display_buf + ret_off,
        "\n\t\t\tDECODER_EVENTQ_CFG: eventq_en%s eventq_depth_use:%u"
        "\n\t\t\tDECODER_EVENTQ_PROD: eventq_wr_idx:%u eventq_ovfl_err%s"
        "\n\t\t\tDECODER_EVENTQ_CONS: eventq_rd_idx:%u eventq_ovfl_err_resp%s"
        "\n\t\t\tDECODER_EVENTQ_BASE_ADDR: eventq_base_addr:0x%lx",
        bit_parser(eventq_en), eventq_size, eventq_wr_idx,
        bit_parser(eventq_wr_err), eventq_rd_idx, bit_parser(eventq_rd_err),
        eventq_addr);

    return ret_off;
}

static const char* mmio_size_parser(uint8_t mmio_size)
{
    static const char* mmio_size_desp[] = {
        "128GB", "256GB", "512GB", "1TB", "2TB", "4TB", "8TB", "16TB"
    };
    if (mmio_size < CFG_ARRAY_SIZE(mmio_size_desp, char*)) {
        return mmio_size_desp[mmio_size];
    } else {
        return CFG_RESERVED;
    }
}

void cfg1_decoder_cap(uint8_t *data, uint32_t data_len)
{
    uint8_t *decoder_data;
    uint8_t event_size, cmd_size, mmio_size, decoder_en;
    uint64_t matt_ba, mmio_ba;
    uint32_t usi_idx;
    uint8_t slice_ver;
    uint32_t slice_size;
    int off;

    if (!ub_is_primary(cfg1_info->uent)) {
        return;
    }

    slice_ver = slice_get_version(data);
    slice_size = slice_get_size(data);
    off = sprintf(cfg1_info->display_buf,
        "\n\t\tCFG1_CAP1_DECODER: slice[0x%x, 0x%x] id[%d]",
        slice_ver, slice_size, CFG1_DECODER_CAP_ID);

    if (slice_size < DECODER_LEN || data_len < DECODER_LEN) {
        printf("%s", cfg1_info->display_buf);
        return;
    }

    decoder_data = data + DECODER_DATA;
    event_size = (uint8_t)to_chunkbits(decoder_data, CFG_BIT4, CFG_BIT7);
    cmd_size = (uint8_t)to_chunkbits(decoder_data, CFG_BIT12, CFG_BIT15);
    mmio_size = (uint8_t)to_chunkbits(decoder_data, CFG_BIT16, CFG_BIT18);
    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\tDECODER: "
        "Eventq_depth:%u Cmdq_depth:%u mmio_size:%s",
        event_size, cmd_size, mmio_size_parser(mmio_size));
    decoder_data += CFG_DWORD_LEN;

    decoder_en = to_1bit(decoder_data, CFG_BIT0);
    off += sprintf(cfg1_info->display_buf + off,
        "\n\t\t\tDECODER_CTRL: Decoder_en%s",
        bit_parser(decoder_en));
    decoder_data += CFG_DWORD_LEN;

    if (ub_is_ibus_controller(cfg1_info->uent)) {
        matt_ba = to_uint64(decoder_data);
        decoder_data += CFG_QWORD_LEN;
        mmio_ba = to_uint64(decoder_data);
        decoder_data += CFG_QWORD_LEN;
        usi_idx = to_uint32(decoder_data);
        off += sprintf(cfg1_info->display_buf + off,
            "\n\t\t\tDECODER_MATT_BA: 0x%lx\n\t\t\tDECODER_MMIO_BA: 0x%lx"
            "\n\t\t\tDECODER_USI_IDX: %u", matt_ba, mmio_ba, usi_idx);
    }

    off = cfg1_decoder_cmdq(data + DECODER_CMDQ, cfg1_info->display_buf, off);
    (void)cfg1_decoder_eventq(data + DECODER_EVENTQ, cfg1_info->display_buf, off);
    printf("%s", cfg1_info->display_buf);
}