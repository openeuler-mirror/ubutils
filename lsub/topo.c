// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "lsub.h"

static struct ub_bus_controller g_ubc = { NULL, NULL };

void ub_free_ubc(void)
{
    struct ub_bus_controller *ubc, *next_ubc;

    for (ubc = g_ubc.chain; ubc; ubc = next_ubc) {
        next_ubc = ubc->chain;
        free(ubc);
    }
}

static uint8_t preset_sub_uent(struct ub_entity *uent)
{
    struct ub_port *port, *peer_port;
    int ret = 0;

    if (!uent || uent->par_uent_num == UB_INIT_PARENT ||
        uent->scaned) {
        return 0;
    }

    /* Traverse all ports in this uent, mark par_uent_num */
    port = uent->route_tb->port_ls;
    while (port) {
        peer_port = port->link_port;
        if (peer_port &&
            peer_port->uent->par_uent_num == UB_INIT_PARENT &&
            peer_port->uent->ubc_uent_num == uent->ubc_uent_num) {
            peer_port->uent->par_uent_num = uent->uent_num;
            ret = 1; /* means some uents attributes are modified */
        }
        port = port->next;
    }
    uent->scaned = 1;

    return (uint8_t)ret;
}

static void preset_all_uent_par(void)
{
    struct device *dev;
    uint8_t keep_scan_flag = 1;

    while (keep_scan_flag) {
        keep_scan_flag = 0;
        for (dev = g_first_dev; dev; dev = dev->next) {
            keep_scan_flag |= preset_sub_uent(dev->uent);
        }
    }
}

static int grow_topo(void)
{
    struct device *dev;
    struct ub_bus_controller **last_ubc, *ubc;
    struct ub_port;
    int ret;

    /* Build list of UB Bus Controller */
    last_ubc = &g_ubc.chain;
    for (dev = g_first_dev; dev; dev = dev->next) {
        if (ub_is_ibus_controller(dev->uent)) {
            ubc = (struct ub_bus_controller *)malloc(sizeof(struct ub_bus_controller));
            if (!ubc) {
                *last_ubc = NULL;
                dev->uent->access->error("ubc alloc failed.\n");
                return -ENOMEM;
            }
            ubc->ubc_dev = dev;
            *last_ubc = ubc;
            last_ubc = &ubc->chain;
            dev->uent->par_uent_num = UB_HOST_DEV_PARENT;
        }
        /* create route_tb and port for each ub dev, and link them */
        ret = sysfs_get_direct_link(dev->uent);
        if (ret) {
            *last_ubc = NULL;
            dev->uent->access->error("get direct link failed, ret = %d.\n", ret);
            return -EINVAL;
        }
    }
    *last_ubc = NULL;

    preset_all_uent_par();

    return 0;
}

static void print_line(char *line, char *p)
{
    *p++ = '\n';
    *p = 0;
    (void)fputs(line, stdout);
    for (p = line; *p; p++) {
        if (*p == '+' || *p == '|') {
            *p = '|';
        } else {
            *p = ' ';
        }
    }
}

static void show_src_port(struct ub_entity *uent, char *line, char *p);

static void show_one_uent(struct ub_entity *uent, struct ub_entity *scan_uent,
    char *line, char *p)
{
    if (!uent || !scan_uent) {
        print_line(line, p);
        return;
    }

    if (uent->par_uent_num != scan_uent->uent_num ||
        uent->ubc_uent_num != scan_uent->ubc_uent_num
        || uent->printed) {
        p += sprintf(p, "-\"%05x\"", uent->uent_num);
        print_line(line, p);
        return;
    }
    p += sprintf(p, "-[%05x]-", uent->uent_num);
    uent->printed = 1;
    show_src_port(uent, line, p);
}

static void show_dst_port(struct ub_port *port, struct ub_port *peer_port,
    uint8_t subdivision, char *line, char *p)
{
    if (!port) {
        print_line(line, p);
        return;
    }
    p += sprintf(p, "%03x-", port->port_num);
    switch (subdivision) {
        case UB_TOPO_PORT_ONLY_ONE:
            *p++ = '-';
            break;
        case UB_TOPO_PORT_MULTI:
            *p++ = '-';
            break;
        case UB_TOPO_PORT_LAST:
            *p++ = '-';
            break;
        default:
            break;
    }
    show_one_uent(port->uent, peer_port->uent, line, p);
}

static void show_src_port(struct ub_entity *uent, char *line, char *p)
{
    struct ub_port *port;
    int len;

    if (!uent->route_tb || !uent->route_tb->port_ls) {
        print_line(line, p);
        return;
    }
    port = uent->route_tb->port_ls;

    /* Only one port in uent */
    if (!port->next) {
        *p++ = '-';
        p += sprintf(p, "-%03x-", port->port_num);
        show_dst_port(port->link_port, port, UB_TOPO_PORT_ONLY_ONE, line, p);
        return;
    }

    /* Multi ports in uent */
    while (port->next) {
        p[0] = '+';
        len = sprintf(p + 1, "-%03x-", port->port_num);
        show_dst_port(port->link_port, port, UB_TOPO_PORT_MULTI, line, p + len + 1);
        port = port->next;
    }
    *p++ = '\\';
    p += sprintf(p, "-%03x-", port->port_num);
    show_dst_port(port->link_port, port, UB_TOPO_PORT_LAST, line, p);
}

static void show_topo_ubc(struct ub_bus_controller *ub_bus_controller, char *line, char *p)
{
    struct ub_bus_controller *ubc = ub_bus_controller->chain;
    struct ub_entity *uent;
    struct ub_entity scan_uent;

    if (!ubc) {
        return;
    }

    memset(&scan_uent, 0, sizeof(scan_uent));
    scan_uent.uent_num = UB_HOST_DEV_PARENT;

    *p++ = '-';
    /* Only one UB Bus Controller */
    if (!ubc->chain) {
        *p++ = '-';
        if (!ubc->ubc_dev || !ubc->ubc_dev->uent) {
            return;
        }
        uent = ubc->ubc_dev->uent;
        scan_uent.ubc_uent_num = uent->ubc_uent_num;
        show_one_uent(uent, &scan_uent, line, p);
        return;
    }

    /* Multi UB Bus Controller */
    while (ubc->chain) {
        p[0] = '+';
        if (!ubc->ubc_dev || !ubc->ubc_dev->uent) {
            ubc = ubc->chain;
            continue;
        }
        uent = ubc->ubc_dev->uent;
        scan_uent.ubc_uent_num = uent->ubc_uent_num;
        show_one_uent(uent, &scan_uent, line, p + 1);
        ubc = ubc->chain;
    }
    *p++ = '\\';
    if (!ubc->ubc_dev || !ubc->ubc_dev->uent) {
        return;
    }
    uent = ubc->ubc_dev->uent;
    scan_uent.ubc_uent_num = uent->ubc_uent_num;
    show_one_uent(uent, &scan_uent, line, p);
    return;
}

void show_topo(void)
{
    char line[2048];
    int ret;

    ret = grow_topo();
    if (ret) {
        return;
    }
    show_topo_ubc(&g_ubc, line, line);
}