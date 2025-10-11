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

#define UBUTILS_VERSION "1.0.0"
#define UB_SYS_DIR_LEN 1024
#define OBJBUFSIZE 1024
#define OBJNAMELEN 1024
#define UB_CONFIG_SIZE 1024
#define UB_UNIFIED_BUS "/dev/unified_bus"
#define UB_CLUSTER "/sys/bus/ub/cluster"
#define UB_PATH_SYS_BUS_UB "/sys/bus/ub"
#define UB_PATH_DIRECT_LINK "direct_link"
#define UB_PATH_MUE_LIST "mue_list"
#define UB_PATH_UE_LIST "ue_list"
#define UB_INIT_PARENT 0xFFFFFFFF
#define UB_HOST_DEV_PARENT 0xFFFFFFFE
#define MAX_UENT_NUM 0xFFFFF
#define MAX_POSITION 0x3FFFFFFFFULL
#define UB_GUID_MAXLEN 128
#define UB_INSTANCE_MAXLEN UB_GUID_MAXLEN
#define UB_INSTANCE_PARA_NUM 4
#define MAX_DRIVER_NAME_LEN 256

#define UB_PRINTF(x, y) __attribute__((format(printf, x, y)))
#define LSUB_OPTIONS "htlni:kbE:"
#define SETUB_OPTIONS "hs:b:g:d:e:u:"
#define HASH_SIZE 4099
#define HEX 16

struct id_entry {
    struct id_entry *next;
    uint32_t id12;
    uint32_t id34;
    uint32_t id56;
    uint8_t cat;
    uint8_t src;
    char name[1];
};

struct option {
    uint32_t uent_num;
    uint64_t position;
    uint32_t width;
    bool writeable; /* 1=write; 0=read */
    uint32_t value;
};

struct device {
    struct device *next;
    struct ub_entity *uent;
    uint8_t *config;
};

struct ub_entity {
    struct ub_entity *next; /* Next uent in the chain */

    /* These fields are define by specification */
    uint32_t vendor_id;
    uint16_t device_id;
    uint32_t class_code; /* UB entity class code */
    uint8_t entity_type; /* UB entity type */
    uint32_t ubc_uent_num;
    uint32_t entity_idx;
    uint32_t uent_num;
    uint32_t primary_entity;
    uint32_t bi_eid;
    uint8_t is_mue;

    /* Fields for tool usage */
    struct ub_access *access;
    struct ub_methods *methods;
    struct ub_route_tb *route_tb;
    uint32_t par_uent_num; /* parent dev's uent number */
    uint32_t scaned; /* topo show scaned */
    uint32_t printed; /* topo show printed */
    char driver_name[MAX_DRIVER_NAME_LEN];
};

struct ub_bi {
    struct ub_bi *next;
    char str[UB_GUID_MAXLEN];
    uint8_t type;
    uint32_t eid;
    uint16_t upi;
};

/* Options you can change */
struct ub_access {
    unsigned int method; /* Access method */
    int numeric_ids; /* numeric ids show in list */
    int kernel_driver; /* kernel driver */

    struct ub_entity *uents; /* uents found on this bus */
    struct ub_bi *bi; /* busInstance list found on ub system */
    struct ub_bi *tail; /* store the last bi */

    /* Fields used internally */
    struct ub_methods *methods;
    struct ub_param *params;
    int fd; /* sys: fd for config space */
    struct id_entry **id_hash;
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
    int (*scan)(struct ub_access *);
    int (*fill_info)(struct ub_entity *);
    int (*read)(struct ub_entity *, uint64_t, uint8_t *, int);
    int (*write)(struct ub_entity *, uint64_t, uint8_t *, int);
    void (*init_dev)(struct ub_entity *);
    void (*cleanup_dev)(struct ub_entity *);
};

struct ub_param {
    struct ub_param *next;
    char *param; /* Name of the parameter */
    char *value; /* Value of the parameter */
    int value_malloced; /* Used internally */
    char *help; /* Explanation of the parameter */
};

struct ub_route_tb_entity {
    struct ub_route_tb_entity *next;
};

struct ub_port {
    uint32_t port_num;
    struct ub_entity *uent;
    struct ub_port *next;
    struct ub_port *link_port;
};

struct ub_route_tb {
    struct ub_port *port_ls;
    struct ub_route_tb_entity *entity;
};

enum ub_access_type {
    /* Known access methods, remember to update access.c as well */
    UB_ACCESS_SYS_BUS_UB, /* Linux /sys/bus/ub */
    UB_ACCESS_MAX
};

enum {
    UB_HEADER_TYPE_BUS_INSTANCE = 0,
    UB_HEADER_TYPE_INDEPENDENT_UB_CONTROLLER = 1,
    UB_HEADER_TYPE_INTEGRATED_UB_CONTROLLER = 2,
    UB_HEADER_TYPE_INDEPENDENT_UB_SWITCH = 3,
    UB_HEADER_TYPE_INTEGRATED_UB_SWITCH = 4,
};

struct lsub_cmd_param {
    /* selected uent number */
    uint32_t uent_num;
    /* busInstance eid */
    uint32_t bi_eid;
};

struct ub_guid {
    union {
        struct {
            uint64_t seq_num;
            uint32_t rsvd0 : 24;
            uint32_t type : 4;
            uint32_t version : 4;
            uint32_t device : 16;
            uint32_t vendor : 16;
        } bits;

        struct {
            uint64_t guid_l;
            uint64_t guid_h;
        } datas;
    };
};

struct ub_bi_para {
    struct ub_guid guid;
    struct ub_guid d_guid;
    uint32_t opt;
    uint32_t eid;
    uint16_t upi;
    uint32_t type;
    uint32_t opt_en;
    uint8_t guid_flag;
    uint8_t dguid_flag;
};

struct ub_access *ub_alloc_acc(void);
int ub_alloc_port(struct ub_entity *uent, uint32_t loc_port_num,
                  uint32_t link_name, uint32_t link_port_num);
struct ub_entity *ub_alloc_uent(struct ub_access *uacc);
struct ub_port *ub_get_port_by_port_num(struct ub_entity *uent, uint32_t port_num);
struct ub_entity *ub_get_uent_by_uent_num(struct ub_access *uacc, uint32_t uent_num);
int sysfs_get_direct_link(struct ub_entity *uent);
void show_topo(void);
void ub_free_ubc(void);
int ub_sel_access_methods(struct ub_access *uacc);
int ub_init(struct ub_access *uacc);
struct device *ub_scan_one_device(struct ub_entity *uent);
int ub_scan_uent(struct ub_access *uacc);
void ub_free_uent(struct ub_entity *uent);
void ub_cleanup(struct ub_access *uacc);
int ub_read_block(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len);
int ub_write_block(struct ub_entity *uent, uint64_t pos, uint8_t *buf, int len);
int ub_fill_uent_info(struct ub_entity *uent);
int parse_x64(char *c, unsigned long long int *resp);
int parse_x32(char *c, unsigned int *resp);
int parse_x16(char *c, unsigned short *resp);
void sysfs_get_if_mue(struct ub_entity *uent);
void sysfs_get_mue_list(struct ub_entity *uent);
void sysfs_get_ue_list(struct ub_entity *uent, uint8_t level);
int ub_add_bi(struct ub_access *uacc, struct ub_bi *bi);
void show_bi_info(struct ub_access *uacc, struct lsub_cmd_param *ls_cmd);
int sysfs_get_bi(struct ub_access *uacc);


extern struct ub_methods linux_sysfs;

#endif /* UBUTILS_H */
