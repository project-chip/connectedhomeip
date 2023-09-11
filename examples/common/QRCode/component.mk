#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
# SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
#
# SPDX-License-Identifier: Apache-2.0
#
#    Description:
#      Component makefile for the QRCode component used by the CHIP
#      ESP32 demo applications.
#

CFLAGS += -Wno-unknown-pragmas

COMPONENT_ADD_INCLUDEDIRS := repo/c
COMPONENT_SRCDIRS := repo/c
COMPONENT_OBJS := repo/c/qrcodegen.o
