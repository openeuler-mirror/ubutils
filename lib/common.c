// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <ubutils.h>

static int g_opt_debug;
struct device *g_first_dev;
static struct ub_methods *g_ub_methods[UB_ACCESS_MAX] = {
    &linux_sysfs,
};

static void UB_PRINTF(1, 2)
ub_err_print(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    (void)fputs("lsub error: ", stderr);
    (void)vfprintf(stderr, msg, args);
    (void)fputc('\n', stderr); /* Prints the entire line */
    va_end(args);
}

static void UB_PRINTF(1, 2)
ub_warning_print(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    (void)vfprintf(stdout, msg, args); /* Branch print without \n */
    va_end(args);
}

static void UB_PRINTF(1, 2)
ub_debug_print(const char *msg, ...)
{
    va_list args;

    if (g_opt_debug) {
        va_start(args, msg);
        (void)vfprintf(stdout, msg, args); /* Branch print without \n */
        va_end(args);
    }
}

int parse_x64(char *c, unsigned long long int *resp)
{
    unsigned long long int value;
    char *stop;

    if (c == NULL || *c == '\0') {
        return -1;
    }
    errno = 0;
    value = strtoull(c, &stop, HEX);
    if (errno) {
        return -1;
    }
    if ((value & ~0ULL) != value) {
        return -1;
    }
    *resp = value;
    if (*stop) {
        return 0;
    } else {
        return 1;
    }
}

int parse_x32(char *c, unsigned int *resp)
{
    char *stop;
    unsigned long int value;

    if (c == NULL || *c == '\0') {
        return -1;
    }
    errno = 0;
    value = strtoul(c, &stop, HEX);
    if (errno) {
        return -1;
    }
    if ((value & ~0U) != value) {
        return -1;
    }
    *resp = (unsigned int)value;
    if (*stop) {
        return 0;
    } else {
        return 1;
    }
}

int parse_x16(char *c, unsigned short *resp)
{
    char *stop;
    unsigned long value;

    if (c == NULL || *c == '\0') {
        return -1;
    }
    errno = 0;
    value = strtoul(c, &stop, HEX);
    if (errno != 0) {
        return -1;
    }
    if (value > UINT16_MAX) {
        return -1;
    }
    *resp = (unsigned short)value;
    if (*stop != '\0') {
        return 0;
    } else {
        return 1;
    }
}

struct ub_entity *ub_get_uent_by_uent_num(struct ub_access *uacc, uint32_t uent_num)
{
    struct ub_entity *uent = uacc->uents;

    while (uent) {
        if (uent->uent_num == uent_num) {
            return uent;
        }
        uent = uent->next;
    }

    return NULL;
}

struct ub_access *ub_alloc_acc(void)
{
    struct ub_access *uacc;
    int i;

    uacc = (struct ub_access *)calloc(1, sizeof(struct ub_access));
    if (!uacc) {
        return NULL;
    }
    uacc->error = ub_err_print;
    uacc->warning = ub_warning_print;
    uacc->debug = ub_debug_print;

    for (i = 0; i < UB_ACCESS_MAX; i++) {
        if (g_ub_methods[i] && g_ub_methods[i]->config) {
            g_ub_methods[i]->config(uacc);
        }
    }

    return uacc;
}

struct ub_entity *ub_alloc_uent(struct ub_access *uacc)
{
    struct ub_entity *uent;
    struct ub_route_tb *route_tb;

    uent = (struct ub_entity *)calloc(1, sizeof(struct ub_entity));
    if (!uent) {
        return NULL;
    }

    uent->access = uacc;
    uent->methods = uacc->methods;
    if (uent->methods->init_dev) {
        uent->methods->init_dev(uent);
    }

    return uent;
}

int ub_fill_uent_info(struct ub_entity *uent)
{
    return uent->methods->fill_info(uent);
}

int ub_scan_uent(struct ub_access *uacc)
{
    return uacc->methods->scan(uacc);
}

int ub_read_block(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len)
{
    return uent->methods->read(uent, pos, buf, len);
}

int ub_write_block(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len)
{
    return uent->methods->write(uent, pos, buf, len);
}

int ub_sel_access_methods(struct ub_access *uacc)
{
    uint32_t i;
    int ret = 0;

    for (i = 0; i < UB_ACCESS_MAX; i++) {
        if (g_ub_methods[i]) {
            uacc->debug("Trying method %u...\n", i);
            if (g_ub_methods[i]->detect(uacc)) {
                uacc->debug("...OK\n");
                uacc->methods = g_ub_methods[i];
                uacc->method = i;
                break;
            }
            uacc->debug("...No.\n");
        }
    }
    if (!uacc->methods) {
        uacc->error("Cannot find any working access method.\n");
        ret = -EINVAL;
    }

    return ret;
}

struct device *ub_scan_one_device(struct ub_entity *uent)
{
    struct device *dev;
    struct ub_access *uacc = uent->access;

    dev = (struct device *)calloc(1, sizeof(struct device));
    if (!dev) {
        return NULL;
    }
    dev->uent = uent;
    dev->config = (uint8_t *)calloc(1, UB_CONFIG_SIZE);
    if (!dev->config) {
        uacc->error("device config alloc failed.\n");
        free(dev);
        return NULL;
    }
    ub_fill_uent_info(uent);

    return dev;
}

int ub_scan_devices(struct ub_access *uacc)
{
    struct device *dev;
    struct ub_entity *uent;
    int ret;

    ret = ub_scan_uent(uacc);
    if (ret) {
        return ret;
    }

    for (uent = uacc->uents; uent; uent = uent->next) {
        dev = ub_scan_one_device(uent);
        if (!dev) {
            uacc->error("scan entity of %05x failed.\n",
                        uent->uent_num);
            return -ENXIO;
        }
        dev->next = g_first_dev;
        g_first_dev = dev;
    }

    return 0;
}

void ub_free_uent(struct ub_entity *uent)
{
    if (uent->methods->cleanup_dev) {
        uent->methods->cleanup_dev(uent);
    }
    free(uent);
}

int ub_init(struct ub_access *uacc)
{
    int ret;

    if (uacc->method) {
        if (uacc->method >= UB_ACCESS_MAX || !g_ub_methods[uacc->method]) {
            uacc->error((char *)"This access method is not supported.");
            return -EINVAL;
        }
        uacc->methods = g_ub_methods[uacc->method];
    } else {
        ret = ub_sel_access_methods(uacc);
        if (ret) {
            return -EINVAL;
        }
    }
    uacc->debug("Decided to use %s.\n", uacc->methods->name);
    uacc->methods->init(uacc);

    return 0;
}

static void ub_free_params(struct ub_access *uacc)
{
    struct ub_param *p;

    while (p = uacc->params) {
        uacc->params = p->next;
        if (p->value_malloced) {
            free(p->value);
        }
        free(p);
    }
}

static void ub_free_ids_hash(struct ub_access *uacc)
{
    struct id_entry *n;
    int i;

    if (!uacc->id_hash)
        return;

    for (i = 0; i < HASH_SIZE; i++) {
        while (uacc->id_hash[i]) {
            n = uacc->id_hash[i];
            uacc->id_hash[i] = n->next;
            free(n);
        }
    }
    free(uacc->id_hash);
    uacc->id_hash = NULL;
}

void ub_cleanup(struct ub_access *uacc)
{
    struct ub_entity *uent, *next_uent;
    struct device *dev, *next_dev;

    for (uent = uacc->uents; uent; uent = next_uent) {
        next_uent = uent->next;
        ub_free_uent(uent);
    }
    if (uacc->methods && uacc->methods->cleanup) {
        uacc->methods->cleanup(uacc);
    }
    ub_free_ids_hash(uacc);
    ub_free_params(uacc);
    free(uacc);

    /* Free all device */
    for (dev = g_first_dev; dev; dev = next_dev) {
        free(dev->config);
        next_dev = dev->next;
        free(dev);
    }
}