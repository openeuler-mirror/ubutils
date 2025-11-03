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

static void sysfs_obj_name(struct ub_entity *uent, const char *object, char *buf)
{
    int n = snprintf(buf, OBJNAMELEN, "%s/devices/%05x/%s",
        sysfs_name(uent->access), uent->uent_num, object);
    if (n < 0 || n >= OBJNAMELEN) {
        uent->access->error("File name too long");
        return;
    }
}

static void ub_link_dev(struct ub_access *uacc, struct ub_entity *uent)
{
    uent->next = uacc->uents;
    uacc->uents = uent;
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

static int ub_scan_attr_cfg(struct ub_entity *uent, uint32_t uent_num)
{
    uent->uent_num = uent_num;
    ub_link_dev(uent->access, uent); /* link uent in uacc */

    return 0;
}

static int sysfs_ub_scan(struct ub_access *uacc)
{
    char dirname[UB_SYS_DIR_LEN] = {0};
    DIR *dir;
    struct dirent *entry;
    struct ub_entity *uent;
    uint32_t uent_num;
    int n, ret = 0;

    n = snprintf(dirname, sizeof(dirname), "%s/devices", sysfs_name(uacc));
    if (n < 0 || n >= (int)sizeof(dirname)) {
        uacc->error("Directory name too long");
        return -EINVAL;
    }
    dir = opendir(dirname);
    if (!dir) {
        uacc->error("Cannot open %s", dirname);
        return -ENOENT;
    }

    while ((entry = readdir(dir))) {
        /* ".", ".." or a special non-device perhaps */
        if (entry->d_name[0] == '.') {
            continue;
        }

        uacc->debug("dev uent_num: %s\n", entry->d_name);

        if (sscanf(entry->d_name, "%05x", &uent_num) < 1) {
            uacc->error("sysfs_ub_scan: Couldn't parse uent_num %s", entry->d_name);
            continue;
        }
        uent = ub_alloc_uent(uacc);
        if (!uent) {
            ret = -ENOMEM;
            break;
        }

        ret = ub_scan_attr_cfg(uent, uent_num);
        if (ret) {
            free(uent);
            continue;
        }
    }

    (void)closedir(dir);

    return ret;
}

static int sysfs_setup(struct ub_entity *uent)
{
    struct ub_access *uacc = uent->access;
    char namebuf[OBJNAMELEN] = {0};

    if (uacc->fd < 0) {
        sysfs_obj_name(uent, "config", namebuf);
        uacc->fd = open(namebuf, O_RDWR);
        if (uacc->fd < 0)
            uacc->warning("Cannot open %s", namebuf);
    }

    return uacc->fd;
}

static int sysfs_ub_fill_info(struct ub_entity *uent)
{
    uent->access->debug("sysfs_ub_fill_info\n");
    return 0;
}

/* @buf contains error information. The actual length of @buf is twice @len. */
static void show_read_error(uint64_t pos, uint8_t *buf, int len)
{
#define BYTES_IN_LINE 8
#define ERR_SEG_NUM 2
    int i = 0, line_count = 0;
    int need_line_break = 0;

    for (; i < len; i++) {
        if (buf[(i + len)] == 0xFF) {
            printf("\nWarning: Failed to access the following address:\n");
            goto error;
        }
    }

    return;
error:
    for (; i < len; i++) {
        if (!(buf[(i + len)] == 0xFF)) {
            continue;
        }
        if ((i + 1 < len) && (buf[(i + 1 + len)] == 0xFF)) {
            printf("%016llx--", pos + i);
            while ((i + 1 < len) && (buf[(i + 1 + len)] == 0xFF)) {
                i++;
            }
            printf("%016llx  ", (pos + i));
            line_count += ERR_SEG_NUM;
        } else {
            printf("%016llx  ", (pos + i));
            line_count++;
        }
        need_line_break = 1;
        if (line_count >= BYTES_IN_LINE) {
            printf("\n");
            need_line_break = 0;
            line_count = 0;
        }
    }
    if (need_line_break) {
        printf("\n");
    }
    printf("\n");
}

static int sysfs_ub_read(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len)
{
    int fd;
    int ret;
    uint8_t *tmp;
#define TMP_LEN_MULTI 2

    if (!uent || !buf) {
        return -EINVAL;
    }

    if (len <= 0 || len > MAX_POSITION) {
        return -EINVAL;
    }

    fd = sysfs_setup(uent);
    if (fd < 0) {
        return -EINVAL;
    }

    tmp = (uint8_t *)calloc((size_t)len * (size_t)TMP_LEN_MULTI, sizeof(uint8_t));
    if (!tmp) {
        return -ENOMEM;
    }

    ret = (int)pread(fd, tmp, (size_t)len * (size_t)TMP_LEN_MULTI, pos);
    if (ret < 0) {
        uent->access->warning("uent %05x sysfs_ub_read: read failed: %s",
            uent->uent_num, strerror(errno));
        ret = -EINVAL;
        goto out;
    } else if (ret != (len * TMP_LEN_MULTI)) {
        uent->access->warning("uent %05x sysfs_ub_read: read failed in len chk: %s",
            uent->uent_num, strerror(errno));
        ret = -EINVAL;
        goto out;
    }

    show_read_error(pos, tmp, len);
    memcpy(buf, tmp, (size_t)len);
    ret = 0;
out:
    free(tmp);
    return ret;
}

static int sysfs_ub_write(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len)
{
    int fd;
    int ret;

    if (!uent || !buf) {
        return -EINVAL;
    }

    fd = sysfs_setup(uent);
    if (fd < 0) {
        return -EINVAL;
    }

    ret = (int)pwrite(fd, buf, (size_t)len, pos);
    if (ret < 0 || ret != len) {
        uent->access->warning("uent %05x sysfs_ub_write: write failed: %s",
            uent->uent_num, strerror(errno));
        return -EINVAL;
    }

    return 0;
}

static void sysfs_ub_cleanup_dev(struct ub_entity *uent)
{
    if (!uent) {
        return;
    }
}

struct ub_methods linux_sysfs = {
    "linux-sysfs",
    "The sys filesystem on Linux",
    sysfs_ub_config,
    sysfs_ub_detect,
    sysfs_ub_init,
    sysfs_ub_cleanup,
    sysfs_ub_scan,
    sysfs_ub_fill_info,
    sysfs_ub_read,
    sysfs_ub_write,
    NULL, /* init_dev */
    sysfs_ub_cleanup_dev,
};