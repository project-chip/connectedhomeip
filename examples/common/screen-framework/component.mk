#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
#    Description:
#      Component makefile for the screen framework component used by the CHIP
#      ESP32 demo applications.
#

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_SRCDIRS := .
COMPONENT_OBJS := Display.o ScreenManager.o Screen.o ListScreen.o
