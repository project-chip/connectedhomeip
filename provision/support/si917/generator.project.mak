####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH = /home/thiru/matter/stash_slc/matter/third_party/silabs/gecko_sdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.4.0

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
 '-DconfigUSE_POSIX_ERRNO=1' \
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
 '-DconfigUSE_POSIX_ERRNO=1' \
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
 -I$(SDK_PATH)/../wifi_sdk/components/service/bsd_socket/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/CMSIS/Driver/Include \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/config \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver \
 -I$(SDK_PATH)/../wifi_sdk/components/service/network_manager/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/rom_driver/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/board/silabs/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/config \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/errno/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/icmp \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/inc \
 -I$(SDK_PATH)/../wifi_sdk/components/protocol/wifi/inc \
 -I$(SDK_PATH)/../wifi_sdk/resources/certificates \
 -I$(SDK_PATH)/../wifi_sdk/resources/other \
 -I$(SDK_PATH)/../wifi_sdk/components/common/inc \
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
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c++11 \
 -fno-rtti \
 -fno-exceptions \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -std=gnu++17 \
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
$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/board/silabs/src/rsi_board.o: $(SDK_PATH)/../wifi_sdk/components/board/silabs/src/rsi_board.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/board/silabs/src/rsi_board.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/board/silabs/src/rsi_board.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/board/silabs/src/rsi_board.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/board/silabs/src/rsi_board.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/common/src/sl_utility.o: $(SDK_PATH)/../wifi_sdk/components/common/src/sl_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/common/src/sl_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/common/src/sl_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/common/src/sl_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/common/src/sl_utility.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_timers.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/icmp/sl_net_ping.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/memory/malloc_buffers.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_credentials.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_rsi_utility.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_callback_framework.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/src/sl_si91x_driver.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.o: $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/device/silabs/si91x/wireless/threading/sli_si91x_multithreaded.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/si91x/sl_wifi.o: $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/si91x/sl_wifi.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/si91x/sl_wifi.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/si91x/sl_wifi.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/si91x/sl_wifi.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/si91x/sl_wifi.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.o: $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/protocol/wifi/src/sl_wifi_callback_framework.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.o: $(SDK_PATH)/../wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/si91x/sl_net_si91x.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net.o: $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.o: $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_certificate_store.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.o: $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_credentials.o

$(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.o: $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/../wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.c
CDEPS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.d
OBJS += $(OUTPUT_DIR)/sdk/_/wifi_sdk/components/service/network_manager/src/sl_net_basic_profiles.o

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