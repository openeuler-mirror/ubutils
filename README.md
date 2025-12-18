# 📌 Introduction

Ubutils is the basic DFX tool of ubus, including setub, lsub and ub.ids.
The setub module is normally used to configure and query the configuration space of the ub protocol.
The lsub module is normally used to query ub entity information and entity topology.
The ub.ids identify the entity on the UB bus through a combination of a vendor ID, class code and device
ID. Which can translate these to a human-readable string.


# 📦 Architecture and Components
## 1. core component(See manual pages for more details)

lsub:

	provide a command-line interface to query ub entity information and entity topology by calling the
	/sys/bus/ub/devices sysfs file.

setub:

	provide a command-line interface configure and query the configuration space of the ub protocol by calling the
	/sys/bus/ub/devices sysfs file.

ub.ids:

	provide the translation to a human-readable string for entities on the UB bus.


## 2. Driver dependency

The ubutils depends on ubfi.ko ubus.ko hisi_ubus.ko


# 🛠 Installation and Preparation
## 1. Prerequisites
Ensure that ubfi.ko ubus.ko have been loaded before using ubutils:

	insmod ubfi.ko
	insmod ubus.ko
	insmod hisi_ubus.ko

## 2. Compiling and (Un)Installing
If you want to installing the package, you can use build.sh to install:

	sudo ./build.sh install

When you are bored of using ubutils, you can uninstall:

	sudo ./build.sh uninstall

# 🧩 code structure
## ubutils code

    ├─ README.md                  # current document
    ├─ CMakeLists.txt             # build file
    ├─ ub.ids                     # ids file
    ├─ lsub.man                   # lsub manuals document
    ├─ setub.man                  # setub manuals document
    ├─ build.sh                   # shell for (un)installing package
    ├─ include/
    │   ├─ lsub-regs.h            # regs header file
    │   └── ubutils.h             # common header file
    ├─ lib/
    │   ├─ common.c               # common source file
    │   └── sysfs.c               # common source file
    ├─ lsub/
    │   ├─ CMakeLists.txt         # build file for lsub
    │   ├─ lsub.c                 # lsub source file
    │   ├─ topo.c                 # lsub source file
    │   └── ...                   # other source files
    ├─ setub/
    │   ├─ kernel_headers
    │   │   └── ubus.h            # kernel header file
    │   ├─ setub.c                # setub source file
    │   └── CMakeLists.txt        # build file for setub
    └── ubutils.spec              # spec file


# 🚀 Function and Usage

- Support configuring and querying the configuration space of the ub protocol.
- Support binding or unbinding the entity GUID and BusInstance GUID/entity EID and BusInstance.
- Support querying ub entity information and entity topology.
- Support displaying the vendor ID, class code and device ID of the UB entity.
- Support viewing route information.
- Support displaying kernel drivers handling each entity.
- Support dispalying specific UB bus instance by EID and bus instance list.
- Support dispalying specified ID database of entity.


# 📦 Version and compatibility

Kernel version: openeuler OLK-6.6


# 📄 Connection and contribution

Issue feedback: Submit an issue in the openeuler community.

# 📌 Copyright Notice

Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
This software is provided on an "as is" basis, without warranties of any kind, either express or implied, including but not limited to non-infringement, merchantability or fit for a particular purpose.
