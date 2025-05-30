#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/bluetooth/mesh)
zephyr_library_include_directories(${ZEPHYR_NRF_MODULE_DIR}/subsys/bluetooth/mesh)

# # Include source files
# target_sources(app PRIVATE
#   src/main.c
#   src/model_handler.c
#   src/vnd_cli.c
#   src/vnd_srv.c
# )

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})


# Include directories
target_include_directories(app PRIVATE include)
