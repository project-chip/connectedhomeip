####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.1.1

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DUC_BUILD=1' \
 '-DEZSP_ASH=1' \
 '-DEZSP_UART=1' \
 '-DUSE_ZAP_CONFIG=1' \
 '-DCONFIGURATION_HEADER="app/framework/util/config.h"'

ASM_DEFS += \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DEZSP_HOST=1' \
 '-DGATEWAY_APP=1' \
 '-DUC_BUILD=1' \
 '-DEZSP_ASH=1' \
 '-DEZSP_UART=1' \
 '-DUSE_ZAP_CONFIG=1' \
 '-DCONFIGURATION_HEADER="app/framework/util/config.h"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I../zap \
 -I../../common \
 -I$(SDK_PATH)/platform/service/cli/inc \
 -I$(SDK_PATH)/platform/service/cli/src \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/emlib/host/inc \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/service/legacy_common_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal/inc \
 -I$(SDK_PATH)/util/third_party/printf \
 -I$(SDK_PATH)/util/third_party/printf/inc \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/event_queue \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/util/serial \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/service-function \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print \
 -I$(SDK_PATH)/. \
 -I$(SDK_PATH)/protocol/zigbee \
 -I$(SDK_PATH)/protocol/zigbee/app/ezsp-host \
 -I$(SDK_PATH)/protocol/zigbee/app/util/gateway \
 -I$(SDK_PATH)/protocol/zigbee/app/util/ezsp \
 -I$(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework \
 -I$(SDK_PATH)/protocol/zigbee/stack \
 -I$(SDK_PATH)/protocol/zigbee/stack/platform/host \
 -I$(SDK_PATH)/protocol/zigbee/stack/include \
 -I$(SDK_PATH)/platform/radio/mac \
 -I$(SDK_PATH)/platform/service/token_manager/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/common \
 -I$(SDK_PATH)/platform/service/legacy_host/inc \
 -I$(SDK_PATH)/protocol/zigbee/stack/config \
 -I$(SDK_PATH)/protocol/zigbee/stack/zll \
 -I$(SDK_PATH)/protocol/zigbee/stack/core \
 -I$(SDK_PATH)/platform/service/legacy_printf/inc \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/signature-decode \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/include \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/util \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/security \
 -I$(SDK_PATH)/protocol/zigbee/app/util/counters \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/cli \
 -I$(SDK_PATH)/protocol/zigbee/app/util/common \
 -I$(SDK_PATH)/protocol/zigbee/app/util/security \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/ota-storage-common \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core

GROUP_START =
GROUP_END =

PROJECT_LIBS = \
 -lpthread \
 -lc \
 -lm

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format

CXX_FLAGS += \
 -std=c++11 \
 -Wall \
 -Wextra \
 -Os \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format

ASM_FLAGS += \

LD_FLAGS += \


####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o: $(SDK_PATH)/platform/common/src/sl_slist.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_slist.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_slist.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o: $(SDK_PATH)/platform/common/src/sl_string.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_string.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_string.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o: $(SDK_PATH)/platform/service/cli/src/sl_cli.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_arguments.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_arguments.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_command.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_command.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_input.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_input.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_io.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_io.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_threaded_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_threaded_host.o

$(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o: $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
	@echo 'Building $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/cli/src/sl_cli_tokenize.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/cli/src/sl_cli_tokenize.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_stdio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_stdio.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o: $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o: $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/crc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o: $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/ember-printf-convert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-printf-convert.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o: $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/micro_host.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/micro_host.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o: $(SDK_PATH)/platform/service/legacy_hal/src/random.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/random.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/random.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o: $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/system-timer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/system-timer.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o: $(SDK_PATH)/platform/service/legacy_host/src/token.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_host/src/token.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_host/src/token.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_host/src/token.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o: $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
	@echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o

$(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o: $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
	@echo 'Building $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/token_manager/src/sl_token_def.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_def.o

# $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
# 	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c'
# 	@mkdir -p $(@D)
# 	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.c
# CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.d
# OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ash/ash-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ash/ash-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-io.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-io.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-queues.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-queues.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o: $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/ezsp-host/ezsp-host-ui.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/ezsp-host/ezsp-host-ui.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/core-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/core-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/network-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/network-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/option-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/option-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/security-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/security-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zcl-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zcl-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/cli/zdo-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/cli/zdo-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_host_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/ezsp/ezsp-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/backchannel-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin-host/gateway/gateway-support.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl-framework-core/zcl-framework-core-cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-custom-cluster-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/zcl_cli/zigbee-zcl-global-cli.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-node.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-node.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-security-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-security-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/af-trust-center.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/af-trust-center.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o: $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/security/crypto-state.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/security/crypto-state.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o: $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/service-function/sl_service_function.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/service-function/sl_service_function.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o: $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/signature-decode/sl_signature_decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-event.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-event.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/af-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/af-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-size.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-size.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-storage.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-storage.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/attribute-table.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/attribute-table.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/client-api.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/client-api.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/global-other-callback.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/global-other-callback.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/multi-network.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/multi-network.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print-formatter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print-formatter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-cluster-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-cluster-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/process-global-message.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/process-global-message.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/service-discovery-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/service-discovery-host.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/time-util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/time-util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o: $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/util/util.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/util/util.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o: $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/common/library.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/common/library.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/common/library.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-callbacks.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-callbacks.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-enum-decode.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp-frame-utilities.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/ezsp.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/ezsp.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/serial-interface-uart.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/serial-interface-uart.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o: $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/linux-serial.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/linux-serial.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o: $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/serial/sl_zigbee_command_interpreter.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o: $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/zigbee-framework/zigbee-device-host.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o: $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/event_queue/event-queue.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/event_queue/event-queue.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o: $(SDK_PATH)/util/third_party/printf/printf.c
	@echo 'Building $(SDK_PATH)/util/third_party/printf/printf.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/printf.o

$(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o: $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
	@echo 'Building $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/printf/src/iostream_printf.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/printf/src/iostream_printf.o

$(OUTPUT_DIR)/project/_/_/common/creds.o: ../../common/creds.c
	@echo 'Building ../../common/creds.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../common/creds.c
CDEPS += $(OUTPUT_DIR)/project/_/_/common/creds.d
OBJS += $(OUTPUT_DIR)/project/_/_/common/creds.o

$(OUTPUT_DIR)/project/_/host_creds.o: ../host_creds.c
	@echo 'Building ../host_creds.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../host_creds.c
CDEPS += $(OUTPUT_DIR)/project/_/host_creds.d
OBJS += $(OUTPUT_DIR)/project/_/host_creds.o

$(OUTPUT_DIR)/project/_/host_ui.o: ../host_ui.c
	@echo 'Building ../host_ui.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../host_ui.c
CDEPS += $(OUTPUT_DIR)/project/_/host_ui.d
OBJS += $(OUTPUT_DIR)/project/_/host_ui.o

$(OUTPUT_DIR)/project/_/main.o: ../main.c
	@echo 'Building ../main.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../main.c
CDEPS += $(OUTPUT_DIR)/project/_/main.d
OBJS += $(OUTPUT_DIR)/project/_/main.o

$(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o: autogen/sl_cli_command_table.c
	@echo 'Building autogen/sl_cli_command_table.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_command_table.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_command_table.o

$(OUTPUT_DIR)/project/autogen/sl_cli_instances.o: autogen/sl_cli_instances.c
	@echo 'Building autogen/sl_cli_instances.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cli_instances.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cli_instances.o

$(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o: autogen/sl_cluster_service_gen.c
	@echo 'Building autogen/sl_cluster_service_gen.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_cluster_service_gen.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_cluster_service_gen.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@echo 'Building autogen/sl_event_handler.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o: autogen/sl_iostream_handles.c
	@echo 'Building autogen/sl_iostream_handles.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_handles.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o

$(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o: autogen/sli_cli_hooks.c
	@echo 'Building autogen/sli_cli_hooks.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sli_cli_hooks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sli_cli_hooks.o

$(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o: autogen/zigbee_common_callback_dispatcher.c
	@echo 'Building autogen/zigbee_common_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_common_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o: autogen/zigbee_host_callback_dispatcher.c
	@echo 'Building autogen/zigbee_host_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_host_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_host_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o: autogen/zigbee_stack_callback_dispatcher.c
	@echo 'Building autogen/zigbee_stack_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_stack_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o: autogen/zigbee_zcl_callback_dispatcher.c
	@echo 'Building autogen/zigbee_zcl_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_zcl_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_zcl_callback_dispatcher.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA