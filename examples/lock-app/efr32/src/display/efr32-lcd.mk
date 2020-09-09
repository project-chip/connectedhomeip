#
#   Copyright (c) 2020 Project CHIP Authors
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
#   @file
#   Common makefile definitions for building lcd display
#   specific code
#

#
#   This makefile is primarily intended to add rules to compile
#   the display code on the efr32 device


# ==================================================
# Sanity Checks
# ==================================================

ifndef EFR32_SDK_ROOT
$(error ENVIRONMENT ERROR: EFR32_SDK_ROOT not set)
endif

GLIB_SRCS = \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/bmp.c                       \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib.c                      \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_bitmap.c               \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_circle.c               \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_font_narrow_6x8.c      \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_font_normal_8x8.c      \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_font_number_16x20.c    \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_line.c                 \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_polygon.c              \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_rectangle.c            \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib/glib_string.c               \

LCD_SRCS = \
$(PROJECT_ROOT)/src/display/lcd.c                                      \
$(EFR32_SDK_ROOT)/hardware/kit/common/drivers/display.c                     \
$(EFR32_SDK_ROOT)/hardware/kit/common/drivers/displayls013b7dh03.c          \
$(EFR32_SDK_ROOT)/hardware/kit/common/drivers/displaypalemlib.c             \
$(EFR32_SDK_ROOT)/platform/middleware/glib/dmd/display/dmd_display.c        \
$(EFR32_SDK_ROOT)/util/silicon_labs/silabs_core/graphics/graphics.c         \
$(EFR32_SDK_ROOT)/hardware/kit/common/drivers/udelay.c                      \
$(GLIB_SRCS)                                                                \

LCD_INCLUDES = \
$(EFR32_SDK_ROOT)/platform/middleware/glib                                  \
$(EFR32_SDK_ROOT)/platform/middleware/glib/glib                             \
$(EFR32_SDK_ROOT)/platform/middleware/glib/dmd                              \
$(EFR32_SDK_ROOT)/hardware/kit/config                                       \
$(EFR32_SDK_ROOT)/util/silicon_labs/silabs_core/graphics                    \
