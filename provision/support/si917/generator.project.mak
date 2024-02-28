####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH ?= /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.4.1

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DCHIP_SYSTEM_CONFIG_USE_SOCKETS=1' \
 '-DEXT_IRQ_COUNT=75' \
 '-DFLASH_PAGE_SIZE=4096' \
 '-DMBEDTLS_CONFIG_FILE="siwx917-chip-mbedtls-config.h"' \
 '-DOPTIMIZE_TINYCRYPT_ASM=1' \
 '-DPROVISION_CHANNEL_ENABLED=1' \
 '-DSIWX_917=1' \
 '-DSRAM_SIZE=0x2fc00UL' \
 '-DTINYCRYPT_PRIMITIVES=1' \
 '-DSIWG917M111MGTBA=1' \
 '-DSLI_SI917=1' \
 '-DSLI_SI917B0=1' \
 '-DSLI_SI91X_MCU_ENABLE_FLASH_BASED_EXECUTION=1' \
 '-DSLI_SI91X_MCU_COMMON_FLASH_MODE=1' \
 '-DSLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER=1' \
 '-DSLI_SI91X_MCU_CONFIG_RADIO_BOARD_VER2=1' \
 '-DSL_BOARD_NAME="BRD4338A"' \
 '-DSL_BOARD_REV="A02"' \
 '-DSL_NET_COMPONENT_INCLUDED=1' \
 '-D__STATIC_INLINE=static inline' \
 '-DSRAM_BASE=0x0cUL' \
 '-DSLI_SI91X_MCU_ENABLE_IPMU_APIS=1' \
 '-DSL_SI91X_SI917_RAM_MEM_CONFIG=1' \
 '-DSLI_SI91X_OFFLOAD_NETWORK_STACK=1' \
 '-DSI917=1' \
 '-DSLI_SI91X_ENABLE_OS=1' \
 '-DSLI_SI91X_MCU_INTERFACE=1' \
 '-DTA_DEEP_SLEEP_COMMON_FLASH=1' \
 '-DSL_WIFI_COMPONENT_INCLUDED=1' \
 '-DconfigNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS=2' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCHIP_SYSTEM_CONFIG_USE_SOCKETS=1' \
 '-DEXT_IRQ_COUNT=75' \
 '-DFLASH_PAGE_SIZE=4096' \
 '-DMBEDTLS_CONFIG_FILE="siwx917-chip-mbedtls-config.h"' \
 '-DOPTIMIZE_TINYCRYPT_ASM=1' \
 '-DPROVISION_CHANNEL_ENABLED=1' \
 '-DSIWX_917=1' \
 '-DSRAM_SIZE=0x2fc00UL' \
 '-DTINYCRYPT_PRIMITIVES=1' \
 '-DSIWG917M111MGTBA=1' \
 '-DSLI_SI917=1' \
 '-DSLI_SI917B0=1' \
 '-DSLI_SI91X_MCU_ENABLE_FLASH_BASED_EXECUTION=1' \
 '-DSLI_SI91X_MCU_COMMON_FLASH_MODE=1' \
 '-DSLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER=1' \
 '-DSLI_SI91X_MCU_CONFIG_RADIO_BOARD_VER2=1' \
 '-DSL_BOARD_NAME="BRD4338A"' \
 '-DSL_BOARD_REV="A02"' \
 '-DSL_NET_COMPONENT_INCLUDED=1' \
 '-D__STATIC_INLINE=static inline' \
 '-DSRAM_BASE=0x0cUL' \
 '-DSLI_SI91X_MCU_ENABLE_IPMU_APIS=1' \
 '-DSL_SI91X_SI917_RAM_MEM_CONFIG=1' \
 '-DSLI_SI91X_OFFLOAD_NETWORK_STACK=1' \
 '-DSI917=1' \
 '-DSLI_SI91X_ENABLE_OS=1' \
 '-DSLI_SI91X_MCU_INTERFACE=1' \
 '-DTA_DEEP_SLEEP_COMMON_FLASH=1' \
 '-DSL_WIFI_COMPONENT_INCLUDED=1' \
 '-DconfigNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS=2' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../../generator \
 -I../../../examples \
 -I../../../examples/platform/silabs \
 -I../../../src \
 -I../../../src/platform/silabs/SiWx917 \
 -I../../../src/include \
 -I../../../zzz_generated/app-common \
 -I../../../third_party/nlassert/repo/include \
 -I../../../third_party/nlio/repo/include \
 -I../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/inc \
 -I../../../third_party/mbedtls/repo/library \
 -I../../../third_party/mbedtls/repo/include \
 -I../../../src/lib/support \
 -I../../../src/platform/silabs \
 -I../../../third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/inc \
 -I$(SDK_PATH)/../wiseconnect/components/service/bsd_socket/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/CMSIS/Driver/Include \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/config \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver \
 -I$(SDK_PATH)/../wiseconnect/components/service/network_manager/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/rom_driver/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/inc \
 -I$(SDK_PATH)/../wiseconnect/components/board/silabs/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/config \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/errno/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/icmp \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/inc \
 -I$(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/inc \
 -I$(SDK_PATH)/../wiseconnect/components/protocol/wifi/inc \
 -I$(SDK_PATH)/../wiseconnect/resources/certificates \
 -I$(SDK_PATH)/../wiseconnect/resources/other \
 -I$(SDK_PATH)/../wiseconnect/components/common/inc \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/CMSIS/RTOS2/Include \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/util/third_party/freertos/cmsis/Include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/util/third_party/segger/systemview/SEGGER \
 -I$(SDK_PATH)/platform/service/system/inc

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lgcc \
 -lnosys \
 -lstdc++ \
 -lc \
 -lm \
 $(SDK_PATH)/platform/emdrv/nvm3/lib/libnvm3_CM4_gcc.a

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -Wno-unused-parameter \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c++17 \
 -fno-rtti \
 -fno-exceptions \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -Wno-unused-parameter \
 --specs=nano.specs \
 -g

ASM_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -x assembler-with-cpp

LD_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -T"autogen/linkerfile_SoC.ld" \
 --specs=nano.specs \
 -Xlinker -Map=$(OUTPUT_DIR)/$(PROJECTNAME).map \
 -u _printf_float \
 -Wl,--gc-sections \
 -Wl,--no-warn-rwx-segments


####################################################################
# Pre/Post Build Rules                                             #
####################################################################
pre-build:
	# No pre-build defined

post-build: $(OUTPUT_DIR)/$(PROJECTNAME).out
	# No post-build defined

####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/_/wiseconnect/components/board/silabs/src/rsi_board.o: $(SDK_PATH)/../wiseconnect/components/board/silabs/src/rsi_board.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/board/silabs/src/rsi_board.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/board/silabs/src/rsi_board.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/board/silabs/src/rsi_board.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/board/silabs/src/rsi_board.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/common/src/sl_utility.o: $(SDK_PATH)/../wiseconnect/components/common/src/sl_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/common/src/sl_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/common/src/sl_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/common/src/sl_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/common/src/sl_utility.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/icmp/sl_net_ping.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/memory/malloc_buffers.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_rsi_utility.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/src/sl_si91x_driver.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.o: $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/si91x/sl_wifi.o: $(SDK_PATH)/../wiseconnect/components/protocol/wifi/si91x/sl_wifi.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/protocol/wifi/si91x/sl_wifi.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/protocol/wifi/si91x/sl_wifi.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/si91x/sl_wifi.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/si91x/sl_wifi.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.o: $(SDK_PATH)/../wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/protocol/wifi/src/sl_wifi_callback_framework.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/si91x/sl_net_si91x.o: $(SDK_PATH)/../wiseconnect/components/service/network_manager/si91x/sl_net_si91x.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/service/network_manager/si91x/sl_net_si91x.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/service/network_manager/si91x/sl_net_si91x.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/si91x/sl_net_si91x.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/si91x/sl_net_si91x.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net.o: $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.o: $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_certificate_store.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.o: $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_credentials.o

$(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.o: $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.d
OBJS += $(OUTPUT_DIR)/sdk/_/wiseconnect/components/service/network_manager/src/sl_net_basic_profiles.o

$(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.o: $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.d
OBJS += $(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o: $(SDK_PATH)/platform/common/src/sl_slist.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_slist.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_slist.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_slist.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o: $(SDK_PATH)/platform/common/src/sl_string.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_string.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_string.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_string.o

$(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.o: $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.o

# $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# 	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
# 	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
# 	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
# OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o: $(SDK_PATH)/platform/emlib/src/em_core.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_core.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_core.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.o: $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.o: $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.o: $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.o: $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.o: $(SDK_PATH)/util/third_party/freertos/kernel/list.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/list.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/list.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.o: $(SDK_PATH)/util/third_party/freertos/kernel/queue.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/queue.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/queue.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.o: $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.o: $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.o: $(SDK_PATH)/util/third_party/freertos/kernel/timers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/timers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/timers.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.o

$(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o: $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyMbed.o: ../../../examples/platform/silabs/provision/AttestationKeyMbed.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/AttestationKeyMbed.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/AttestationKeyMbed.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyMbed.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyMbed.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.o: ../../../examples/platform/silabs/provision/ProvisionCommands.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionCommands.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionCommands.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o: ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.o: ../../../examples/platform/silabs/provision/ProvisionManager.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionManager.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionManager.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o: ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorage.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o: ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.o: ../../../examples/platform/silabs/provision/RttStreamChannel.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/RttStreamChannel.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/RttStreamChannel.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/syscalls_stubs.o: ../../../examples/platform/silabs/syscalls_stubs.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/syscalls_stubs.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/syscalls_stubs.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/syscalls_stubs.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/syscalls_stubs.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.o: ../../../src/lib/support/Base64.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/Base64.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/Base64.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.o: ../../../src/lib/support/BytesToHex.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/BytesToHex.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/BytesToHex.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem-Malloc.o: ../../../src/lib/support/CHIPMem-Malloc.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/CHIPMem-Malloc.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/CHIPMem-Malloc.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem-Malloc.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem-Malloc.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem.o: ../../../src/lib/support/CHIPMem.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/CHIPMem.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/CHIPMem.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPMem.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPPlatformMemory.o: ../../../src/lib/support/CHIPPlatformMemory.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/CHIPPlatformMemory.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/CHIPPlatformMemory.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPPlatformMemory.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/CHIPPlatformMemory.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.o: ../../../src/lib/support/logging/TextOnlyLogging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/logging/TextOnlyLogging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/logging/TextOnlyLogging.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.o

$(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.o: ../../../src/platform/silabs/Logging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/platform/silabs/Logging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/platform/silabs/Logging.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.o

$(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.o: ../../../src/platform/silabs/SilabsConfig.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/platform/silabs/SilabsConfig.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/platform/silabs/SilabsConfig.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.o

$(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.o: ../../../src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SiWx917/CHIPCryptoPALTinyCrypt.o

$(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.o: ../../../src/setup_payload/Base38Encode.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/setup_payload/Base38Encode.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/setup_payload/Base38Encode.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/aes.o: ../../../third_party/mbedtls/repo/library/aes.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/aes.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/aes.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/aes.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/aes.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1parse.o: ../../../third_party/mbedtls/repo/library/asn1parse.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/asn1parse.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/asn1parse.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1parse.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1parse.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1write.o: ../../../third_party/mbedtls/repo/library/asn1write.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/asn1write.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/asn1write.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1write.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/asn1write.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/base64.o: ../../../third_party/mbedtls/repo/library/base64.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/base64.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/base64.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/base64.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/base64.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/bignum.o: ../../../third_party/mbedtls/repo/library/bignum.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/bignum.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/bignum.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/bignum.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/bignum.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ccm.o: ../../../third_party/mbedtls/repo/library/ccm.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ccm.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ccm.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ccm.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ccm.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher.o: ../../../third_party/mbedtls/repo/library/cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/cipher.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher_wrap.o: ../../../third_party/mbedtls/repo/library/cipher_wrap.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/cipher_wrap.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/cipher_wrap.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher_wrap.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/cipher_wrap.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/constant_time.o: ../../../third_party/mbedtls/repo/library/constant_time.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/constant_time.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/constant_time.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/constant_time.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/constant_time.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ctr_drbg.o: ../../../third_party/mbedtls/repo/library/ctr_drbg.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ctr_drbg.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ctr_drbg.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ctr_drbg.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ctr_drbg.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdh.o: ../../../third_party/mbedtls/repo/library/ecdh.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ecdh.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ecdh.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdh.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdh.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdsa.o: ../../../third_party/mbedtls/repo/library/ecdsa.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ecdsa.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ecdsa.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdsa.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecdsa.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp.o: ../../../third_party/mbedtls/repo/library/ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ecp.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp_curves.o: ../../../third_party/mbedtls/repo/library/ecp_curves.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/ecp_curves.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/ecp_curves.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp_curves.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/ecp_curves.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/entropy.o: ../../../third_party/mbedtls/repo/library/entropy.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/entropy.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/entropy.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/entropy.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/entropy.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hkdf.o: ../../../third_party/mbedtls/repo/library/hkdf.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/hkdf.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/hkdf.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hkdf.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hkdf.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hmac_drbg.o: ../../../third_party/mbedtls/repo/library/hmac_drbg.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/hmac_drbg.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/hmac_drbg.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hmac_drbg.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/hmac_drbg.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/md.o: ../../../third_party/mbedtls/repo/library/md.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/md.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/md.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/md.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/md.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pem.o: ../../../third_party/mbedtls/repo/library/pem.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/pem.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/pem.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pem.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pem.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pkcs5.o: ../../../third_party/mbedtls/repo/library/pkcs5.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/pkcs5.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/pkcs5.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pkcs5.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/pkcs5.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/platform.o: ../../../third_party/mbedtls/repo/library/platform.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/platform.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/platform.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/platform.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/platform.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha256.o: ../../../third_party/mbedtls/repo/library/sha256.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/sha256.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/sha256.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha256.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha256.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha512.o: ../../../third_party/mbedtls/repo/library/sha512.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/sha512.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/sha512.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha512.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/sha512.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509.o: ../../../third_party/mbedtls/repo/library/x509.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/x509.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/x509.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509.o

$(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509_create.o: ../../../third_party/mbedtls/repo/library/x509_create.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/mbedtls/repo/library/x509_create.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/mbedtls/repo/library/x509_create.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509_create.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/mbedtls/repo/library/x509_create.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dh.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/ecc_dsa.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/error.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/oid.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pk_wrap.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkparse.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/pkwrite.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/platform_util.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509_crt.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.o: ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/mbedtls/tinycrypt/src/x509write_csr.o

$(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.o: ../../../third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../../third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../../third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.c
CDEPS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/third_party/silabs/matter_support/matter/si91x/siwx917/BRD4338A/support/src/startup_common_RS1xxxx.o

$(OUTPUT_DIR)/project/_/_/generator/app.o: ../../generator/app.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/app.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/app.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/app.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/app.o

$(OUTPUT_DIR)/project/_/_/generator/main.o: ../../generator/main.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/main.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/main.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/main.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/main.o

$(OUTPUT_DIR)/project/_/_/generator/nvm3_si917.o: ../../generator/nvm3_si917.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/nvm3_si917.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/nvm3_si917.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/nvm3_si917.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/nvm3_si917.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_event_handler.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o: autogen/sl_iostream_handles.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_iostream_handles.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_handles.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA