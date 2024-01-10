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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA