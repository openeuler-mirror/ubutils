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
#include <string.h>
#include <unistd.h>
#include <ubutils.h>
#include "lsub.h"

typedef FILE * ub_ids_file;
#define ub_ids_gets(file, line, size) fgets(line, size, file)
#define ub_ids_eof(file) feof(file)
#define ub_ids_open(uacc) fopen((uacc)->id_file_name, "r")
#define ub_ids_close(file) fclose(file)
#define UB_ERROR_MSG(file, err) \
    do { if (!(err) && ferror(file)) (err) = "I/O error"; } while (0)

#define MAX_LINE 1024
#define SUB_CONFIG 0xff

enum id_entry_src {
    SRC_UNKNOWN,
    SRC_LOCAL,
};

enum id_entry_type {
    ID_UNKNOWN,
    ID_VENDOR,
    ID_DEVICE,
    ID_SUBSYSTEM,
    ID_GEN_SUBSYSTEM,
    ID_CLASS,
    ID_SUBCLASS,
    ID_PROGIF
};

struct ub_ids_id {
    uint32_t cat;
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsys_vendor_id;
    uint32_t subsys_id;
    uint32_t base_id;
    uint32_t sub_id;
};

struct ub_ids_name {
    char *vendor_name;
    char *device_name;
    char *numeric_name;
    char *classnum_name;
    char *base_name;
    char *sub_name;
    char *unknown;
};

static inline unsigned int id_hash(uint32_t cat, uint32_t id12, uint32_t id34, uint32_t id56)
{
    unsigned int h;

    /* id34 move left by 3 digits, id56 move left by 5 digits, cat move left by 7 digits for hash count */
    h = id12 ^ (id34 << 3) ^ (id56 << 5) ^ (cat << 7);
    return h % HASH_SIZE;
}

static inline uint32_t id_pair(uint32_t x, uint32_t y)
{
    /* move left by 16 digits for device id bit width */
    return ((x << 16) | y);
}

static char *ub_id_lookup(struct ub_access *uacc, struct ub_ids_id *ids_id)
{
    struct id_entry *n, *best;
    uint32_t cat = ids_id->cat;
    uint32_t id1 = ids_id->vendor_id;
    uint32_t id2 = ids_id->device_id;
    uint32_t id3 = ids_id->subsys_vendor_id;
    uint32_t id4 = ids_id->subsys_id;
    uint32_t id5 = ids_id->base_id;
    uint32_t id6 = ids_id->sub_id;
    uint32_t id12 = id_pair(id1, id2);
    uint32_t id34 = id_pair(id3, id4);
    uint32_t id56 = id_pair(id5, id6);

    if (uacc->id_hash) {
        n = uacc->id_hash[id_hash(cat, id12, id34, id56)];
        best = NULL;
        for (; n; n = n->next) {
            if (n->id12 != id12 || n->id34 != id34 || n->id56 != id56 ||  n->cat != cat)
                continue;
            if (!best || best->src < n->src)
                best = n;
        }
        if (best)
            return best->name;
    }

    return NULL;
}

static char *id_lookup(struct ub_access *uacc, struct ub_ids_id *ids_id)
{
    char *name;

    name = ub_id_lookup(uacc, ids_id);
    if (!name)
        return 0; /* Can not find IDs name */

    return (name[0] ? name : NULL); /* IDs name of the entry is empty */
}

static char *format_name_pair(char *buf, size_t size, struct ub_ids_name *n)
{
    int res;
    #define MAX_NAME_LEN 0x80
    char class_part[MAX_NAME_LEN] = {};
    char vendor_part[MAX_NAME_LEN] = {};
    char device_part[MAX_NAME_LEN] = {};

    if (n->base_name && n->sub_name) {
        res = snprintf(class_part, sizeof(class_part), "%s %s ", n->sub_name, n->classnum_name);
    } else if (n->base_name) {
        res = snprintf(class_part, sizeof(class_part), "%s %s ", n->base_name, n->classnum_name);
    } else{
        res = snprintf(class_part, sizeof(class_part), "Class %s ", n->classnum_name);
    }

    if (n->vendor_name) {
        res = snprintf(vendor_part, sizeof(vendor_part), "%s ", n->vendor_name);
    }

    if (n->device_name) {
        res = snprintf(device_part, sizeof(device_part), "%s ", n->device_name);
    } else {
        res = snprintf(device_part, sizeof(device_part), "%s ", n->unknown);
    }

    res = snprintf(buf, size, "%s%s%s%s", class_part, vendor_part, device_part, n->numeric_name);

    if (res < 0 || (size_t)res >= size)
        return (char*)"<ub ids parse: buffer too small>";

    return buf;
}

static struct id_entry *ub_id_alloc(struct ub_access *uacc, unsigned int size)
{
    struct id_entry *entry;

    if (!uacc->id_hash) {
        uacc->id_hash = (struct id_entry **)calloc(HASH_SIZE, sizeof(struct id_entry *));
        if (!uacc->id_hash) {
            uacc->error("id hash list allocation failed");
            return NULL;
        }
    }

    if (size == 0 || size >= MAX_LINE) {
        uacc->error("size of id hash entry invalid, size: %u", size);
        return NULL;
    }

    entry = (struct id_entry *)calloc(1, size);
    if (!entry)
        uacc->error("id hash entry allocation of %u bytes failed", size);

    return entry;
}

static int ub_id_insert(struct ub_access *uacc, struct ub_ids_id *ids_id,
                 char *text, enum id_entry_src src)
{
    uint32_t id12 = id_pair(ids_id->vendor_id, ids_id->device_id);
    uint32_t id34 = id_pair(ids_id->subsys_vendor_id, ids_id->subsys_id);
    uint32_t id56 = id_pair(ids_id->base_id, ids_id->sub_id);
    unsigned int h = id_hash(ids_id->cat, id12, id34, id56);
    struct id_entry *n = uacc->id_hash ? uacc->id_hash[h] : NULL;
    uint32_t len = strlen(text) + 1;

    while (n && (n->id12 != id12 || n->id34 != id34 || n->id56 || n->cat != ids_id->cat))
        n = n->next;
    if (n) /* Matching location found */
        return -1;

    n = ub_id_alloc(uacc, sizeof(struct id_entry) + len);
    if (!n)
        return -1;

    n->id12 = id12;
    n->id34 = id34;
    n->id56 = id56;
    n->cat = (uint8_t)ids_id->cat;
    n->src = src;
    memcpy(n->name, text, len);
    n->next = uacc->id_hash[h];
    uacc->id_hash[h] = n;
    return 0;
}

static int id_hex(char *ptr, int cnt)
{
    uint32_t x = 0;
    int count = cnt;
    char *p = ptr;

    while (count--) {
        x <<= 4; /* 4 bit for 1 hex number */
        if (*p >= '0' && *p <= '9')
            x += (uint32_t)(*p - '0');
        else if (*p >= 'a' && *p <= 'f')
            x += (uint32_t)(*p - 'a' + 10); /* number 10 */
        else if (*p >= 'A' && *p <= 'F')
            x += (uint32_t)(*p - 'A' + 10); /* number 10 */
        else
            return -1;
        p++;
    }
    return (int)x;
}

static inline int id_white_p(int c)
{
    return (c == ' ') || (c == '\t');
}

static const char *ids_parse_top_level(struct ub_access *uacc,
                                       struct ub_ids_id *ids_id,
                                       char **ptr,
                                       uint8_t *continue_flag)
{
    static const char parse_error[] = "Parse error";
    int vendor_id = 0;
    int base_id = 0;

    *continue_flag = 0;
    if ((*ptr)[0] == 'C' && (*ptr)[1] == ' ') { /* Class block */
        /* move 2 byte to find base id and 2 for it's len, 4 for white space character check */
        if ((base_id = id_hex((*ptr) + 2, 2)) < 0 || !id_white_p((*ptr)[4]))
            return parse_error;
        ids_id->cat = ID_CLASS;
        ids_id->base_id = (uint32_t)base_id;
        ids_id->vendor_id = (uint32_t)vendor_id;
        *ptr += 5; /* move 5 byte for used info */
    } else if ((*ptr)[0] == 'S' && (*ptr)[1] == ' ') { /* Generic subsystem block */
        /* move 2 byte to find vendor id and 4 for it's len, 6 for white space character check */
        if ((vendor_id = id_hex((*ptr) + 2, 4)) < 0 || (*ptr)[6])
            return parse_error;
        ids_id->cat = ID_VENDOR;
        ids_id->vendor_id = (uint32_t)vendor_id;
        ids_id->device_id = ids_id->subsys_vendor_id = ids_id->subsys_id = ids_id->base_id = 0;
        if (!ub_id_lookup(uacc, ids_id))
            return "Vendor does not exist";
        ids_id->cat = ID_GEN_SUBSYSTEM;
        *continue_flag = 1;
        return NULL;
    } else if ((*ptr)[0] >= 'A' && (*ptr)[0] <= 'Z' && (*ptr)[1] == ' ') { /* Unrecognized block (RFU) */
        ids_id->cat = ID_UNKNOWN;
        ids_id->base_id= 0;
        *continue_flag = 1;
        return NULL;
    } else { /* Vendor ID */
        /* only vendor id used, 4 for it's len, 4 for white space character check */
        if ((vendor_id = id_hex(*ptr, 4)) < 0 || !id_white_p((*ptr)[4]))
            return parse_error;
        ids_id->cat = ID_VENDOR;
        ids_id->base_id= 0;
        ids_id->vendor_id = (uint32_t)vendor_id;
        *ptr += 5; /* move 5 byte for used info */
    }
    ids_id->device_id = ids_id->subsys_vendor_id = ids_id->subsys_id = 0;
    ids_id->sub_id = SUB_CONFIG;

    return NULL; /* Without error message */
}

static const char *ids_parse_nest_level1(struct ub_ids_id *ids_id,
                                         char **ptr)
{
    static const char parse_error[] = "Parse error";
    int device_id;
    int sub_id;

    switch (ids_id->cat) {
        case ID_VENDOR:
        case ID_DEVICE:
        case ID_SUBSYSTEM:
            /* 4 for device id len, 4 for white space character check */
            if ((device_id = id_hex(*ptr, 4)) < 0 || !id_white_p((*ptr)[4]))
                return parse_error;
            ids_id->device_id = (uint32_t)device_id;
            *ptr += 5; /* move 5 byte for used info */
            ids_id->cat = ID_DEVICE;
            ids_id->sub_id = SUB_CONFIG;
            ids_id->subsys_vendor_id = ids_id->subsys_id = ids_id->base_id = 0;
            break;
        case ID_GEN_SUBSYSTEM:
            /* 4 for device id len, 4 for white space character check */
            if ((device_id = id_hex(*ptr, 4)) < 0 || !id_white_p((*ptr)[4]))
                return parse_error;
            ids_id->device_id = (uint32_t)device_id;
            *ptr += 5; /* move 5 byte for used info */
            ids_id->sub_id = SUB_CONFIG;
            ids_id->subsys_vendor_id = ids_id->subsys_id = ids_id->base_id = 0;
            break;
        case ID_CLASS:
            /* 2 for sub id len, 2 for white space character check */
            if ((sub_id = id_hex(*ptr, 2)) < 0 || !id_white_p((*ptr)[2]))
                return parse_error;
            ids_id->sub_id = (uint32_t)sub_id;
            *ptr += 3; /* move 3 byte for used info */
            ids_id->subsys_vendor_id = ids_id->subsys_id = 0;
            break;
        case ID_SUBCLASS:
        case ID_PROGIF:
            /* 2 for device id len, 2 for white space character check */
            if ((device_id = id_hex(*ptr, 2)) < 0 || !id_white_p((*ptr)[2]))
                return parse_error;
            ids_id->device_id = (uint32_t)device_id;
            *ptr += 3; /* move 3 byte for used info */
            ids_id->cat = ID_SUBCLASS;
            ids_id->subsys_vendor_id = ids_id->subsys_id = 0;
            break;
        default:
            return parse_error;
    }

    return NULL;
}

static const char *ids_parse_nest_level2(struct ub_ids_id *ids_id,
                                         char **ptr)
{
    static const char parse_error[] = "Parse error";
    int subsys_vendor_id = 0;

    switch (ids_id->cat) {
        case ID_DEVICE:
        case ID_SUBSYSTEM:
            /* 4 for subsys vendor id, 4 for white space character check */
            if ((subsys_vendor_id = id_hex(*ptr, 4)) < 0 || !id_white_p((*ptr)[4]))
                return parse_error;
            ids_id->subsys_vendor_id = (uint32_t)subsys_vendor_id;
            /* move 5 to find subsys id and 4 for it's len, 9 for white space character check */
            if ((subsys_vendor_id = id_hex((*ptr) + 5, 4)) < 0 || !id_white_p((*ptr)[9]))
                return parse_error;
            ids_id->subsys_vendor_id = (uint32_t)subsys_vendor_id;
            *ptr += 10; /* move 10 byte for used info */
            ids_id->cat = ID_SUBSYSTEM;
            break;
        case ID_CLASS:
        case ID_SUBCLASS:
        case ID_PROGIF:
            /* 2 for subsys vendor id, 2 for white space character check */
            if ((subsys_vendor_id = id_hex(*ptr, 2)) < 0 || !id_white_p((*ptr)[2]))
                return parse_error;
            ids_id->subsys_vendor_id = (uint32_t)subsys_vendor_id;
            *ptr += 3; /* move 3 byte for used info */
            ids_id->cat = ID_PROGIF;
            ids_id->subsys_id = 0;
            break;
        default:
            return parse_error;
    }

    return NULL;
}

static const char *ids_line_preprocess(char **ptr, char *line,
                                       ub_ids_file ids_file, uint8_t *continue_flag)
{
    *continue_flag = 0;
    *ptr = line;

    while (**ptr && **ptr != '\n' && **ptr != '\r')
        (*ptr)++;

    if (!(**ptr) && !ub_ids_eof(ids_file))
        return "Line too long";
    **ptr = 0;
    if (*ptr > line && ((*ptr)[-1] == ' ' || (*ptr)[-1] == '\t'))
        *--(*ptr) = 0;

    *ptr = line;
    while (id_white_p(**ptr))
        (*ptr)++;
    if (!**ptr || **ptr == '#') {
        *continue_flag = 1;
        return NULL;
    }

    *ptr = line;
    while (**ptr == '\t')
        (*ptr)++;

    return NULL;
}

static const char *id_parse_list(struct ub_access *uacc,
                                 ub_ids_file ids_file, int *line_no)
{
    static const char parse_error[] = "Parse error";
    struct ub_ids_id ids_id = {};
    char line[MAX_LINE];
    const char *err;
    uint8_t continue_flag;
    int nest;
    char *p;

    *line_no = 0;
    ids_id.cat = 0xffffffff;
    ids_id.sub_id = SUB_CONFIG;
    ids_id.vendor_id = ids_id.device_id = ids_id.subsys_vendor_id = ids_id.subsys_id = ids_id.base_id= 0;
    while (ub_ids_gets(ids_file, line, sizeof(line))) {
        (*line_no)++;
        err = ids_line_preprocess(&p, line, ids_file, &continue_flag);
        if (err)
            return err;
        if (continue_flag)
            continue;

        nest = p - line;
        if (!nest) { /* Top-level entries */
            err = ids_parse_top_level(uacc, &ids_id, &p, &continue_flag);
            if (err)
                return err;
            if (continue_flag)
                continue;
        } else if (ids_id.cat == ID_UNKNOWN) { /* Nested entries in RFU blocks are skipped */
            continue;
        } else if (nest == 1) { /* Nesting level 1 */
            err = ids_parse_nest_level1(&ids_id, &p);
            if (err)
                return err;
        } else if (nest == 2) { /* Nesting level 2 */
            err = ids_parse_nest_level2(&ids_id, &p);
            if (err)
                return err;
        } else { /* Nesting level 3 or more */
            return parse_error;
        }

        while (id_white_p(*p))
            p++;
        if (!*p)
            return parse_error;
        if (ub_id_insert(uacc, &ids_id, p, SRC_LOCAL))
            return "Duplicate item or memory application failed";
    }

    return NULL;
}

static int ub_load_name_list(struct ub_access *uacc)
{
    ub_ids_file ids_file;
    int line_no;
    const char *err;
    char *real_file_name;

    if (!uacc->id_file_name) {
        uacc->id_file_name = (char *)DEFAULT_IDS_PATH;
    } else {
        real_file_name = realpath(uacc->id_file_name, NULL);
        if (!real_file_name)
            return -1;
        if (uacc->free_id_name)
            free(uacc->id_file_name);
        uacc->id_file_name = real_file_name;
        uacc->free_id_name = 1;
    }
    if (!(ids_file = ub_ids_open(uacc)))
        return -1;

    err = id_parse_list(uacc, ids_file, &line_no);
    UB_ERROR_MSG(ids_file, err);
    ub_ids_close(ids_file);
    if (err) {
        uacc->error("%s at %s, line %d\n", err, uacc->id_file_name, line_no);
        return -1;
    }

    return 0;
}

char *ub_lookup_name(struct ub_access *uacc, char *buf, size_t size,
                     uint32_t vendor_id, uint32_t device_id, uint32_t class_id)
{
    char *vendor_name, *device_name, *sub_name, *base_name;
    struct ub_ids_name ids_name = {};
    struct ub_ids_id ids_id = {};
    char classbuf[16] = {};
    char numbuf[16] = {};
    #define OFFSET_LEN 8

    if (!uacc->id_hash)
        (void)ub_load_name_list(uacc);

    (void)sprintf(numbuf, "<%04x>:<%04x>", vendor_id, device_id);
    (void)sprintf(classbuf, "<%04x>:", class_id);

    ids_id.cat = ID_CLASS;
    ids_id.base_id = class_id & 0xFF;
    ids_id.sub_id = (class_id >> OFFSET_LEN) & 0xFF;
    ids_id.vendor_id = 0;
    ids_id.device_id = 0;
    ids_id.subsys_vendor_id = 0;
    ids_id.subsys_id = 0;
    sub_name = id_lookup(uacc, &ids_id);

    ids_id.sub_id = SUB_CONFIG;
    base_name = id_lookup(uacc, &ids_id);

    ids_id.cat = ID_DEVICE;
    ids_id.base_id = 0;
    ids_id.sub_id = SUB_CONFIG;
    ids_id.vendor_id = vendor_id;
    ids_id.device_id = device_id;
    ids_id.subsys_vendor_id = 0;
    ids_id.subsys_id = 0;
    device_name = id_lookup(uacc, &ids_id);

    ids_id.cat = ID_VENDOR;
    ids_id.base_id = 0;
    ids_id.sub_id = SUB_CONFIG;
    ids_id.vendor_id = vendor_id;
    ids_id.device_id = 0;
    ids_id.subsys_vendor_id = 0;
    ids_id.subsys_id = 0;
    vendor_name = id_lookup(uacc, &ids_id);

    ids_name.vendor_name = vendor_name;
    ids_name.device_name = device_name;
    ids_name.numeric_name = numbuf;
    ids_name.classnum_name = classbuf;
    ids_name.sub_name = sub_name;
    ids_name.base_name = base_name;
    ids_name.unknown = (char *)"Device";
    return format_name_pair(buf, size, &ids_name);
}
