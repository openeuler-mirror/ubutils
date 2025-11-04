/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#ifndef UBUTILS_H
#define UBUTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define UB_PATH_SYS_BUS_UB "/sys/bus/ub"

#define UB_PRINTF(x, y) __attribute__((format(printf, x, y)))
#define SETUB_OPTIONS "h"

/* Options you can change */
struct ub_access {
    unsigned int method; /* Access method */

    /* Fields used internally */
    struct ub_methods *methods;
    struct ub_param *params;
    int fd; /* sys: fd for config space */
    char *id_file_name;
    int free_id_name;

    /* Functions you can override */
    void (*error)(const char *msg, ...) UB_PRINTF(1, 2); /* Write error message and quit */
    void (*warning)(const char *msg, ...) UB_PRINTF(1, 2); /* Write a warning message */
    void (*debug)(const char *msg, ...) UB_PRINTF(1, 2); /* Write a debugging message */
};

struct ub_methods {
    char *name;
    char *help;
    void (*config)(struct ub_access *);
    int (*detect)(struct ub_access *);
    void (*init)(struct ub_access *);
    void (*cleanup)(struct ub_access *);
};

struct ub_param {
    struct ub_param *next;
    char *param; /* Name of the parameter */
    char *value; /* Value of the parameter */
    int value_malloced; /* Used internally */
    char *help; /* Explanation of the parameter */
};

enum ub_access_type {
    /* Known access methods, remember to update access.c as well */
    UB_ACCESS_SYS_BUS_UB, /* Linux /sys/bus/ub */
    UB_ACCESS_MAX
};

struct ub_bi_para {
};

struct ub_access *ub_alloc_acc(void);
int ub_sel_access_methods(struct ub_access *uacc);
int ub_init(struct ub_access *uacc);
void ub_cleanup(struct ub_access *uacc);

extern struct ub_methods linux_sysfs;

#endif /* UBUTILS_H */
