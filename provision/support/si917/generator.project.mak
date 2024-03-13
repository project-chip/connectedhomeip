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
 '-DFLASH_PAGE_SIZE=1' \
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
 '-DFLASH_PAGE_SIZE=1' \
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

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.o: ../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.o

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

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol.o: ../../../examples/platform/silabs/provision/ProvisionProtocol.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionProtocol.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionProtocol.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol2.o: ../../../examples/platform/silabs/provision/ProvisionProtocol2.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionProtocol2.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionProtocol2.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol2.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionProtocol2.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o: ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorage.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.o: ../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o: ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o

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