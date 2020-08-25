#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2020 Google LLC.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#   @file
#         Component makefile for incorporating FreeRTOS into an EFR32
#         application.
#

#
#   This makefile is intended to work in conjunction with the efr32-app.mk
#   makefile to build the OpenWeave example applications on Silicon Labs platforms.
#   EFR32 applications should include this file in their top level Makefile
#   along with the other makefiles in this directory.  E.g.:
#
#       PROJECT_ROOT = $(realpath .)
#
#       BUILD_SUPPORT_DIR = $(PROJECT_ROOT)/third_party/openweave-core/config/efr32
#
#       include $(BUILD_SUPPORT_DIR)/efr32-app.mk
#       include $(BUILD_SUPPORT_DIR)/efr32-openweave.mk
#       include $(BUILD_SUPPORT_DIR)/efr32-openthread.mk
#       include $(BUILD_SUPPORT_DIR)/efr32-freertos.mk
#
#       PROJECT_ROOT := $(realpath .)
#
#       APP := openweave-efr32-bringup
#
#       SRCS = \
#           $(PROJECT_ROOT)/main.cpp \
#           ...
#
#       $(call GenerateBuildRules)
#

ifeq ($(EFR32FAMILY), efr32mg12)
  FREERTOS_TARGET = ARM_CM4F
else
ifeq ($(EFR32FAMILY), efr32mg21)
  FREERTOS_TARGET = ARM_CM3
endif
endif

FREERTOS_OUTPUT_DIR = $(OUTPUT_DIR)/freertos
FREERTOS_LIB_DIR = $(FREERTOS_OUTPUT_DIR)

STD_LDFLAGS += \
    -L$(FREERTOS_LIB_DIR)

STD_INC_DIRS += \
    $(FREERTOS_ROOT)/Source/include/

$(FREERTOS_OUTPUT_DIR)/croutine.c.o : $(FREERTOS_ROOT)/Source/croutine.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/croutine.c -o $(FREERTOS_OUTPUT_DIR)/croutine.c.o

$(FREERTOS_OUTPUT_DIR)/list.c.o     : $(FREERTOS_ROOT)/Source/list.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/list.c -o $(FREERTOS_OUTPUT_DIR)/list.c.o

$(FREERTOS_OUTPUT_DIR)/queue.c.o    : $(FREERTOS_ROOT)/Source/queue.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/queue.c -o $(FREERTOS_OUTPUT_DIR)/queue.c.o

$(FREERTOS_OUTPUT_DIR)/event_groups.c.o    : $(FREERTOS_ROOT)/Source/event_groups.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/event_groups.c -o $(FREERTOS_OUTPUT_DIR)/event_groups.c.o

$(FREERTOS_OUTPUT_DIR)/tasks.c.o    : $(FREERTOS_ROOT)/Source/tasks.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/tasks.c -o $(FREERTOS_OUTPUT_DIR)/tasks.c.o

$(FREERTOS_OUTPUT_DIR)/timers.c.o   : $(FREERTOS_ROOT)/Source/timers.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/timers.c -o $(FREERTOS_OUTPUT_DIR)/timers.c.o

$(FREERTOS_OUTPUT_DIR)/port.c.o     : $(FREERTOS_ROOT)/Source/portable/GCC/$(FREERTOS_TARGET)/port.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/portable/GCC/$(FREERTOS_TARGET)/port.c -o $(FREERTOS_OUTPUT_DIR)/port.c.o

$(FREERTOS_OUTPUT_DIR)/heap_3.c.o     : $(FREERTOS_ROOT)/Source/portable/MemMang/heap_3.c $(FREERTOSCONFIG_DIR)/FreeRTOSConfig.h
	$(NO_ECHO) $(CCACHE) $(CC) -c  $(STD_CFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(DEFINE_FLAGS) $(INC_FLAGS) $(FREERTOS_ROOT)/Source/portable/MemMang/heap_3.c -o $(FREERTOS_OUTPUT_DIR)/heap_3.c.o

FREERTOS_OBJECTS := \
    $(FREERTOS_OUTPUT_DIR)/croutine.c.o \
    $(FREERTOS_OUTPUT_DIR)/list.c.o \
    $(FREERTOS_OUTPUT_DIR)/queue.c.o \
    $(FREERTOS_OUTPUT_DIR)/event_groups.c.o \
    $(FREERTOS_OUTPUT_DIR)/tasks.c.o \
    $(FREERTOS_OUTPUT_DIR)/timers.c.o \
    $(FREERTOS_OUTPUT_DIR)/port.c.o \
    $(FREERTOS_OUTPUT_DIR)/heap_3.c.o


# Add FreeRTOSBuildRules to the list of late-bound build rules that
# will be evaluated when GenerateBuildRules is called.
LATE_BOUND_RULES += FreeRTOSBuildRules

# Rules for configuring, building and installing FreeRTOS from source.
define FreeRTOSBuildRules

$(FREERTOS_LIB_DIR)/libfreertos.a : $(FREERTOS_OUTPUT_DIR) $(FREERTOS_LIB_DIR) $(FREERTOS_OBJECTS)
	@echo "$(HDR_PREFIX)AR $(FREERTOS_LIB_DIR)/libfreertos.a"
	$(NO_ECHO)$(AR) rcs $(FREERTOS_LIB_DIR)/libfreertos.a $(FREERTOS_OBJECTS)

.phony: $(FREERTOS_OUTPUT_DIR)
$(FREERTOS_OUTPUT_DIR) :
	@echo "$(HDR_PREFIX)MKDIR $@"
	$(NO_ECHO)mkdir -p $(FREERTOS_OUTPUT_DIR)

.phony: build-freertos
build-freertos : $(FREERTOS_LIB_DIR)/libfreertos.a

.phony: install-freertos
install-freertos : $(FREERTOS_OUTPUT_DIR)
	@echo "$(HDR_PREFIX)CP $(FREERTOS_OUTPUT_DIR)"
	$(NO_ECHO)cp -r $(FREERTOS_ROOT)/Source/include $(FREERTOS_OUTPUT_DIR)/

.phony: clean-freertos
clean-freertos :
	@echo "$(HDR_PREFIX)RM $(FREERTOS_OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(FREERTOS_OUTPUT_DIR)

endef


# ==================================================
# FreeRTOS-specific help definitions
# ==================================================

define TargetHelp +=

  build-freertos        Build the FreeRTOS library.

  install-freertos      Install FreeRTOS library and headers in
                        build output directory for use by application.

  clean-freertos        Clean all build outputs produced by the FreeRTOS
                        build process.
endef




