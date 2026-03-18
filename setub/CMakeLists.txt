# SPDX-License-Identifier: MIT
# Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.

file(GLOB SETUB_SRC ./*.c ../lib/*.c)
add_executable(setub ${SETUB_SRC})

if(ENABLE_ASAN)
    message("setub asan enable")
    set(CMAKE_C_FLAGS "-lasan -lm")
    set(CMAKE_LD_FLAGS "-fsanitize=adress -fsanitize=undefined")
endif()

target_include_directories(setub PRIVATE
    ./
    ../include
    ./kernel_headers)
target_link_directories(setub PRIVATE ${CMAKE_INSTALL_PREFIX}/lib)
target_link_options(setub PRIVATE
    -pie
)
install(TARGETS setub RUNTIME OPTIONAL)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_target_properties(setub PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wuninitialized -Wno-error=format -Wundef -Wunused -Wdate-time -Wfloat-equal -Wshadow -Wvla \
        -Wdisabled-optimization -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wtype-limits -Wshift-negative-value -Wswitch-default \
        -Wframe-larger-than=8192 -Wshift-overflow=2 -Wwrite-strings -Wmissing-format-attribute -Wformat-nonliteral -Wduplicated-cond -Wtrampolines \
        -Wlogical-op -Wsuggest-attribute=format -Wduplicated-branches -Wmissing-include-dirs -Wformat-signedness -Wmissing-declarations \
        -Wreturn-local-addr -Wredundant-decls -Wfloat-conversion -Wmissing-prototypes -Wstrict-prototypes \
        -std=gnu99 -fno-common -Wextra \
        -fstack-protector-strong -fstack-protector -fstack-protector-all -D_FORTIFY_SOURCE=2 -O2 -Wformat -Wformat-security -ftrapv \
        -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--as-needed -Wl,--strip-all -fPIE")
