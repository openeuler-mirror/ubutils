// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <ubutils.h>

static void ub_define_param(struct ub_access *uacc, const char *param,
                            const char *value, const char *help)
{
    struct ub_param *p;

    p = (struct ub_param *)malloc(sizeof(*p));
    if (!p) {
        return;
    }
    *p = (struct ub_param) {
        .next = uacc->params,
        .param = (char *)param,
        .value = (char *)value,
        .value_malloced = 0,
        .help = (char *)help,
    };

    uacc->params = p;
}

static char *ub_get_param(struct ub_access *uacc, const char *param)
{
    struct ub_param *p;

    for (p = uacc->params; p; p = p->next) {
        if (!strcmp(p->param, param)) {
            return p->value;
        }
    }

    return NULL;
}

static inline char *sysfs_name(struct ub_access *uacc)
{
    return ub_get_param(uacc, "sysfs.path");
}

static int sysfs_ub_detect(struct ub_access *uacc)
{
    if (access(sysfs_name(uacc), R_OK)) {
        uacc->debug("...cannot open %s", sysfs_name(uacc));
        return 0;
    }
    uacc->debug("...using %s", sysfs_name(uacc));

    return 1;
}

static void sysfs_ub_config(struct ub_access *uacc)
{
    ub_define_param(uacc, "sysfs.path", UB_PATH_SYS_BUS_UB, "Path to the sysfs device");
}

static void sysfs_ub_init(struct ub_access *uacc)
{
    uacc->fd = -1;
}

static void sysfs_ub_cleanup(struct ub_access *uacc)
{
    if (uacc->fd >= 0) {
        close(uacc->fd);
        uacc->fd = -1;
    }

    if (uacc->free_id_name) {
        free(uacc->id_file_name);
        uacc->free_id_name = 0;
    }
}

struct ub_methods linux_sysfs = {
    "linux-sysfs",
    "The sys filesystem on Linux",
    sysfs_ub_config,
    sysfs_ub_detect,
    sysfs_ub_init,
    sysfs_ub_cleanup,
};