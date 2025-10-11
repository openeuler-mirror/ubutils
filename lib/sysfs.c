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

void sysfs_get_if_mue(struct ub_entity *uent)
{
    char namebuf[OBJNAMELEN];

    sysfs_obj_name(uent, UB_PATH_UE_LIST, namebuf);
    if (access(namebuf, R_OK)) {
        uent->is_mue = 0;
    } else {
        uent->is_mue = 1;
    }
}

void sysfs_get_mue_list(struct ub_entity *uent)
{
    char namebuf[OBJNAMELEN], buf[256]; /* 256 for len of list line */
    struct ub_access *uacc = uent->access;
    struct ub_entity *uent_i;
    uint32_t mue_uent_num;
    FILE *file;

    sysfs_obj_name(uent, UB_PATH_MUE_LIST, namebuf);
    file = fopen(namebuf, "r");
    if (file == NULL) {
        return;
    }

    while (fgets(buf, (int)sizeof(buf), file)) {
        if (sscanf(buf, "%05x", &mue_uent_num) != 1) { /* 1 fields */
            uacc->error("Syntax error in %s", namebuf);
            continue;
        }

        uent_i = ub_get_uent_by_uent_num(uacc, mue_uent_num);
        if (!uent_i) {
            (void)fclose(file);
            return;
        }

        printf("\tUBE%u's mue[UBE%u]: <%05x>\n",
               uent->entity_idx, uent_i->entity_idx, mue_uent_num);
        sysfs_get_ue_list(uent_i, 1);
    }
    (void)fclose(file);
}

void sysfs_get_ue_list(struct ub_entity *uent, uint8_t level)
{
    char namebuf[OBJNAMELEN], buf[256]; /* 256 for len of list line */
    struct ub_access *uacc = uent->access;
    char tab_str[] = "\t\t";
    struct ub_entity *uent_i;
    uint32_t ue_uent_num;
    FILE *file;

    sysfs_obj_name(uent, UB_PATH_UE_LIST, namebuf);
    file = fopen(namebuf, "r");
    if (file == NULL) {
        return;
    }

    if (level == 0) {
        tab_str[1] = '\0';
    }

    while (fgets(buf, (int)sizeof(buf), file)) {
        if (sscanf(buf, "%05x", &ue_uent_num) != 1) { /* 1 fields */
            uacc->error("Syntax error in %s", namebuf);
            continue;
        }

        uent_i = ub_get_uent_by_uent_num(uacc, ue_uent_num);
        if (!uent_i) {
            (void)fclose(file);
            return;
        }

        printf("%sUBE%u's ue[UBE%u]: <%05x>\n",
               tab_str, uent->entity_idx, uent_i->entity_idx, ue_uent_num);
    }
    (void)fclose(file);
}

static int ub_parse_instance_sys(struct ub_access *uacc, FILE *file, unsigned int start, unsigned int cnt)
{
    char buf[UB_INSTANCE_MAXLEN] = {0};
    struct ub_bi bi = {0};
    char str[HEX] = {0};
    unsigned int i = 0;
    int ret = 0;

    if (cnt == 0)
        return 0;

    ret = snprintf(str, HEX, "%u", start);
    if (ret < 0 || ret >= HEX) {
        uacc->error("parse instance start point failed");
        return -EINVAL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        uacc->error("Failed to reset sys instance file pointer for writing instance start point");
        return -EIO;
     }

    if (fputs(str, file) < 0) {
        uacc->error("Failed to set instance start point[%u] to sysfs", start);
        return -EIO;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        uacc->error("Failed to reset sys instance file pointer for reading instance entry");
        return -EIO;
     }

    /* Ignore the first line */
    (void)fgets(buf, (int)sizeof(buf), file);
    while (i < cnt && fgets(buf, (int)sizeof(buf), file)) {
        i++;
        if (sscanf(buf, "guid:%s type:%01x eid:%05x upi:%04x\n", bi.str, (uint32_t *)&bi.type, &bi.eid, (uint32_t *)&bi.upi) != UB_INSTANCE_PARA_NUM) {
            uacc->error("Syntax error in %s", buf);
            continue;
        }
        ret = ub_add_bi(uacc, &bi);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

int sysfs_get_bi(struct ub_access *uacc)
{
#define MAX_CNT_PER_READ 32
    char namebuf[OBJNAMELEN] = {0}, buf[UB_INSTANCE_MAXLEN] = {0};
    unsigned int count, cycle, left, i;
    int n, ret = 0;
    FILE *file;

    n = snprintf(namebuf, OBJNAMELEN, "%s/instance", sysfs_name(uacc));
    if (n < 0 || n >= OBJNAMELEN) {
        uacc->error("Instance file name too long");
        return -EINVAL;
    }

    file = fopen(namebuf, "r+");
    if (file == NULL) {
        uacc->error("Open instance file failed");
        return -EIO;
    }

    if (fgets(buf, (int)sizeof(buf), file)) {
        /* just obtain count info, ignore from and show info */
        if (sscanf(buf, "count 0x%x", &count) != 1) {
            uacc->error("Syntax error in %s", buf);
            (void)fclose(file);
            return -EINVAL;
        }
    } else {
        uacc->error("Failed to get instance count from sysfs");
        (void)fclose(file);
        return -EIO;
    }

    cycle = count / MAX_CNT_PER_READ;
    left = count % MAX_CNT_PER_READ;

    for (i = 0; i <= cycle; i++) {
        if (i != cycle) {
            ret = ub_parse_instance_sys(uacc, file, i * MAX_CNT_PER_READ, MAX_CNT_PER_READ);
        } else {
            ret = ub_parse_instance_sys(uacc, file, i * MAX_CNT_PER_READ, left);
        }

        if (ret) {
            break;
        }
    }

    (void)fclose(file);
    return ret;
}

int sysfs_get_direct_link(struct ub_entity *uent)
{
    uint32_t loc_port_num, link_port_num, link_uent_num;
    struct ub_access *uacc = uent->access;
    char namebuf[OBJNAMELEN] = {0}, buf[256]; /* 256 for len of resource line */
    FILE *file;
    int i, ret = 0;

    sysfs_obj_name(uent, UB_PATH_DIRECT_LINK, namebuf);
    file = fopen(namebuf, "r");
    if (!file) {
        uacc->error("Cannot open %s: %s", namebuf, strerror(errno));
        return -ENOENT;
    }

    for (i = 0; 1; i++) {
        if (!fgets(buf, sizeof(buf), file)) {
            break;
        }
        if (sscanf(buf, "0x%03x : 0x%03x [0x%05x]", &loc_port_num,
            &link_port_num, &link_uent_num) != 3) { /* 3 fields */
            uacc->error("Syntax error in %s", namebuf);
            continue;
        }
        ret = ub_alloc_port(uent, loc_port_num, link_uent_num, link_port_num);
        if (ret) {
            break;
        }
    }
    (void)fclose(file);

    return ret;
}

static int sysfs_get_string(struct ub_entity *uent, const char *object, char *buf, int mandatory)
{
    struct ub_access *uacc = uent->access;
    void (*warn)(const char *msg, ...) = (mandatory ? uacc->error : uacc->warning);
    char name[OBJNAMELEN] = {0};
    long n;
    int fd;

    sysfs_obj_name(uent, object, name);
    if ((fd = open(name, O_RDONLY)) < 0) {
        if (mandatory || errno != ENOENT)
            warn("Cannot open %s: %s", name, strerror(errno));
        return 0;
    }

    if ((n = read(fd, buf, OBJBUFSIZE)) >= 0) {
        buf[n] = '\0';
        close(fd);
        return 1;
    }

    warn("Error reading %s: %s", name, strerror(errno));
    close(fd);
    return 0;
}

static unsigned int sysfs_get_value(struct ub_entity *uent, const char *object, int mandatory)
{
    char buf[OBJBUFSIZE];

    if (sysfs_get_string(uent, object, buf, mandatory)) {
        return (unsigned int)strtol(buf, NULL, 0);
    }

    return (unsigned int)~0;
}

static char *sysfs_get_link(struct ub_entity *uent, const char *link_name)
{
    char r_path[OBJNAMELEN] = {};
    char path[2 * OBJNAMELEN];

    sysfs_obj_name(uent, link_name, path);

    if (readlink(path, r_path, sizeof(r_path)) < 0)
        return NULL;

    sysfs_obj_name(uent, "", path);
    strcat(path, r_path);

    return realpath(path, NULL);
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
    char *driver_path;

    uent->uent_num = uent_num;
    uent->par_uent_num = UB_INIT_PARENT;

    strcpy(uent->driver_name, "No used driver");
    driver_path = sysfs_get_link(uent, "driver");
    if (driver_path) {
        char *driver = strrchr(driver_path, '/');
        driver = driver ? driver + 1 : driver_path;
        if (strlen(driver) < MAX_DRIVER_NAME_LEN)
            strcpy(uent->driver_name, driver);
        else
            uent->access->warning("Driver name len is too long\n");
        free(driver_path);
    }

    uent->entity_type = (uint8_t)sysfs_get_value(uent, "type", 1);
    uent->ubc_uent_num = sysfs_get_value(uent, "ubc", 1);
    uent->vendor_id = sysfs_get_value(uent, "vendor", 1);
    uent->device_id = (uint16_t)sysfs_get_value(uent, "device", 1);
    uent->class_code = sysfs_get_value(uent, "class_code", 1);
    uent->entity_idx = sysfs_get_value(uent, "entity_idx", 1);
    uent->primary_entity = sysfs_get_value(uent, "primary_entity", 1);
    uent->bi_eid = sysfs_get_value(uent, "instance", 1);
    sysfs_get_if_mue(uent);
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

static void free_route_port(struct ub_route_tb *route_tb)
{
    struct ub_port *port, *tmp_port;

    for (port = route_tb->port_ls; port; port = tmp_port) {
        tmp_port = port->next;
        free(port);
    }
}

static void free_route_entity(struct ub_route_tb *route_tb)
{
    struct ub_route_tb_entity *entity, *tmp_entity;

    for (entity = route_tb->entity; entity; entity = tmp_entity) {
        tmp_entity = entity->next;
        free(entity);
    }
}

static void sysfs_ub_cleanup_dev(struct ub_entity *uent)
{
    if (!uent) {
        return;
    }

    if (uent->route_tb) {
        free_route_port(uent->route_tb);
        free_route_entity(uent->route_tb);
        free(uent->route_tb);
        uent->route_tb = NULL;
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