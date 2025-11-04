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

void ub_cleanup(struct ub_access *uacc)
{
    if (uacc->methods && uacc->methods->cleanup) {
        uacc->methods->cleanup(uacc);
    }
    ub_free_params(uacc);
    free(uacc);
}