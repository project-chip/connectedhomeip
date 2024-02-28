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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztfQlz3DiW5l+pUExszOykkpbtOuyt6g5Zkl2atiytMl3dtaMJBpNEZnLEq3joqI7+74uDYPIASYDEI1UdUzNtO0nifd97uIGHh78f3dxe/8fF2dq8vb5eH70/+vvd0e3F59P15S8XZvnV3dH7u6O7o38cLYoUq+uvt2cXK5zoxz8/+d43DyhO3DD46e7oZPnq7ugbFNih4wY7/ODr+uPxD3dHf/7TXfBjFIf/jez0G5wkSN77oYM8/MU+TaP3hvH4+LhMXM/aJEs79I0kMVZp5rjhEtlhjLBQnDpCcfq8svHfOF0u7e4Ii/7mmx+3oeeg+JvA8slLOwy27i5/R966HuLvVhefPl3cmrfrtXmGP1vusfAsdvEb8tV745fQy3yUGLcoCbPYxv/auanhW2mKYiNGHrISZGD0B5cobSRZFIVxaiTuu5PvDQZsCDAMEZfEMwOUmg7aWpmXmg+Wl6EEhFErkpDXxxih2/X16owmBiHUhBAyuV1fmOfowbWRiRODMGlCtOXVJrRix2TJsthKsUiozGqB6mWWxqEHzomDCNlEbpDzBuFRFS9kgHzP3Zik3cDlfZPtIOl0YDFuPxqsaRI1U1aWhjsUiNspzw3uUUyemKvwbOk5OtnnyIYApLUanoV+FAYoSDW3UJxLE6OttNv8K9O2UssLNWctJ9SG1EYLPZAP91bgeCgGo9RAkaNjT0LHbqfjhkkaI8vPvwUqRC1A0qSgzCQC6iXlBm6K/0hSK7CnMFgTT0hxacc2b+fwPyFYNSD6G9NHN8FjxSDAg8I3ZuLcm2+WJ8s3pba1NkLkDU3xReMbhw4KSu8bX7ARa+ULwTfvTp5qnzQ+8u2s8YmAc4wEXzW/27uR8LsmN2Ja4YeNT92bq68mzrZdawJxEidM2zFYolLRwhMJM3lOUuTnuU+HZHJFrFQAjEP2GiwXDZZVtJg9GdjcBrGmQUxlYCsYhXpGwdpoJWPIquNGfmZakSvbrmhXoUKgnXalXqm/rrRfzGCU5BRa1/HalKySTK04zaIJWdYBpWiSAhglZoyb5m0W2FMQFUBKU3UQihIP/2Em4WRkG6Biup1FuN5uBV2tYk1pFMeh7GBvsLKYkVEBk8qTDe5pzfQ5kh46jOJXRZMi6OAx9XQEq2hSBG9XJ0/4vynYlaCkK5xtR2SG4ksvQ4wugFVEeaJYxmQUc6xBLexEHGuQ0lQro5GpyDZApejiFAxiCpoVMFl609WcCpgsPStJUJxORO8AplJtpi2HFUC5YV55bDgByzqeHEny9YR1pY6nQNJ/mKQVr4ApD+e6XnXuEnV+2z1TrlWNAGuJZ2JuGLupi4piCzgiztfo8zFxO/6gwbFg1JHh5mKC6s/UagFt0UUyMyZh3Y4tIt+aDe0vagtWsUv2YqXWiPzETUyWQK4GnF2tLleyq0XnXYIF318Gtpc54tUtYc4yALJfANaH59Y0yqYyqBUMhm7krI0GGekFIp7y9MtL0YExUVXgYv3zC1EgZzJAAfPq9OzlKMHZDFHk5udfX5AiORtVRT56VrJ/IWoUXFSVuHz9UopUzkRVgauzyxeiQM5EVYEvp1/OX4gGnIqqCqvTl5IHORNlBW5ejAI3gxT4ujq9Xb8QFQou6kp8eDEqfBiowEupypzKABVeykCJU4HZFVTZb+mch+twRNJhypKzoNhJqZe2shujZtpiH0elbHsBEzDJuVaF68W0I/uLzqF7hdmnabvGT539X4XZtCPXztFphde0A9LOQWeFF57lmJ9Pv/zw/dvXpxMSrMNKMJ12ONk5ZKzymrQqyNeEr+dXU+Yoh5NhNvGwtGfoWeM25Wizc0TZzE2Qlfj23GxfeG/m5oTUOJ7SWnRj9BaHvtI6spJ3DRFue/fAA72DCsVmYxlYekuUJEK7yA1noVsgy2/hWva9GwDthXSwLeEq2daOgTw0+gpCrOamQdMAbd33UVXbxKflZpslQDvPfSWWIyvx3br+LGxzXCWuu8jxrVnYFshKfKPwEU+uEuthngJRhVdj/jhPqchxlbj+lkTuLGQ5sBLbeIYeooSrxDW1Nh4yQd02e1g3GAzgD+iwJMVezYGpSOv60sfeNLPmyEp8s7ma5mxIy0wSmY+xFUUzGblOQI29FyUJGVDPQ72MDuhQhi3jRnsUW17OQuyKM9y5zPZC+97MIsdKEexMsakK9S6rE5D3AI+BToT0EC5hS3NlE6m52Bbo0nxp8wd0xEyC8AFemjFtAefiy8GV2BYt35ysyyTk2SdWnM5Hm6NPeCyr0kiB9jcCxUm/Uyeg1ErOQbiErdhKzsVWbbHLyp7MGG1RjAIbmQ+hl4K6Affw72QjbX/Lma+s5NgKB9WA57td5dpWm/ISVCu2UqhTnTKUKxTkmQMvOXYxVlt2tKFdNzqpqi3qO9Z89SzHlm+T4Zdyu9pk5eVcFOxJqBEHfk23i3aNhDT7OUmrcp1gIbqDrfpi9A58ibSLruoyqft6vjYix1bgOl/bm2PLc52vD3bV+t4wsnygw5ESZAt0+a1f6O2SDraqWya/ofmaghxbnuuc7Zby9g5JQEIGpfNN5KoU5BeVoTelOigrb0zN2vfGyn1v4rgh8MmrDr4FugJfH83XmhXo8nxnXDdJFF2EEt+dsSjk4PJsZ2x8VdveZFayymyfkzidb5B7gFdkbO6R5ZDtiXmZl2mo7uvMxfwAr7avMxdf5W316bbUe1gP2lZnOyuz0ebo0nwfLGc2tjm2NNfHLAb2v+kgy8HlAvN4oIGieuhW0GX55uF93prRPG1bkwKgSwg2zkM9iHDrx8GD/0Y2wkqXz0ibzVkQMnNLohmYbpBuYbeFc90NohaLgtpJpOMgqlAbItbc48ylYuZSpcmitR70RdGV3u2uWYRS4Dd7TLGvWbEGqVItDFRztFksplWjm8igfNXTiNC4ch56QB7AGZtNCNwxl+gXnVwOKr8Ll0zhXypiWkaWX7iO/Gx6qhxVfhXYk70sRiPLHFSe5BymVLbkRAcnhFyHnZqI4hCTSUKcFAUJtLuDkLiAgfyqKtrBr/eIWJeAFcimGE/+viitdEvQ8oShF3uEVBWXeLCAaLbCWwNXWtUxydRkjm63Bi6/2DCN/76IsoTzvmgG/+gAT4lFXDnqeGdXpXiztN+Hnf+UlOWuvRxVabQxOcscdEhHPj3XCrZSWz051xx0UFs9Odka+KC2enrSVfABbfXklCvQam315Fw5qr7Fv5YXbY/r15eRu9ES0Ymx+l1hyA/j55YgzyXT+pbnhba5ybZbkONBnLHBCBlNvKZpZSNZozgOQpk41u39pHiJiQoGtAWVX10hLCDHxVlpX9OpaMrQtI98agrSayM4Ekgsc3ZhMUQRIPcgu0GK4sDCj1LLBmgoC3MxNWpLxkICEhGC8kucaUubup6bPk/GXIArT5ipTVTesauMFW8l1UO/k4WcMof8s2PkkGm15UG2rG2lp4Y+ScMiKARwrUyueL4oL8DVUfimpN/JQlGZ6YmDtfKu7YuvKhUpH5GYUXCVjXAxalBCpduowWVLg5owP2SNvnVj/9GKkZlFuDC23LpR0bKWgl1HD5cTdYL1BwV+b/YIE8LlkxzxUZnXNtoRd1VsAgTZQ5W7pgOaTL2ZaBjDCEqMXVr2pPGsZoOHaOY2xm8ewxh0tFhxkhAiDy856T5GloNbD5ny43IfmcxLXZYSAayBFJoX5IwO7OG678MkNX3hJdvNGkbApUZE7uPTu5PvycWzZmBHJgEBNBHXIX/ehg7ST1v7DZuybK0WZ6ZhkzJSLYnzDFaLuGPhgg5owIoSxTpQE15uOBZ5VroN42kJ13DVmJpc8FyUKwRUZlybbHLOFWAprqTZKqrkxHQb2BKM64U/nLfuhQolmmUOc8uZpVwU0JNMvfOYECAXLbRoWwpFIX/PAgtZAHendw9XmQu+m3zZmHVyrgWsJE//7eQcGaQkv3D6/A5V8jm6x4OnnQ/gFNDDsgwsyZVcdDo5Tw4qyZEAk6Z7b8UOneEmKM0ATsD3sG6nIauHZwXc3xfKd7pPByEFFf4O2rrBDC1uHXzAOGlKygJsRcZAPsiSnHv8kOujJIY0Mduem8XHrxi3DJm6VpcAF2aDxupSm+ItJx6CJLUCiHBYdY4CQBWeZA2TauiCtjM1snVUecbg47oK0+7hnJghXc8pqs9UTJuo8ozdMCXt5j2CHEVV2FYR5ZnSgCJ26IHPRCpsm6jyjEkYlMlbgwaoAt/USrPpiBZo8gwnzXqlHH90ty5875zzq4FJr6ELHwseNh41H/Qfia0fhEUp2e8wfSuwdo2LpSSWmkV7tBsrcW19jhvF6cEq17LHiRCxngHtVHFzQt5NxbMMJ0/SRnGKS5htpXjuleIOeyqrinAlaEPTa9IQVySZPad2DxAYJdi+Ug1HXRnREF2kiqburk2dkhNLZx8nouYE8KRyDBk6bqSjN5PgVAaSITYNKyVKtL+bhlcVSqbxAWbUpCHTYTcq8CZx8lF3d8/bW9ExJWejR+kDJxZxgEvutDr+ymXetlAkSuJlmHyHv/0OkksB0F0a8QhxbLTwFh4H0d0MNEwj2xi0zherDJ41NRB1+Of+1iAbGwGwBTsTRffTMWbfhFbsdI3Y2RRk5ECdBYbASOPGN1REMSmKecCJXC7AeOYgf1SmVngfAmXkcjsLVPFlvoGQxRpcHdv5NFH6Oj9yXQeIjRqy+5i4r20oIhXRfTyiRx+KR0V0H48ELmMSlXzJAyVCUalJ199E8sW/rlaSDBjHL2bQYaduv1RO3yDS+Xy2AwlqjksgtWrCJ7VcMMx8ts1SowpyVZPyep4Yqa+KsaR6Zt0d3FQm2zSFzp2Qdl79Ox9Cchqmku2cpCePtPTCkIBpDpk/Q+eQsdHU1ZXWcjqAEeEtWssZgP45cbMRaMThdByXDIfYoc7RmyM5bx5mUyTc6ORj+4mb6NkDr3JpCO7hoafBqXFob2QaAz6E0A+vXuuoyFUODcE9PLQs4tUotK7XtdUljeAtJaCvqaj/rNazmPMo1bZ603JY+k9qDUxJZ+uRRKJwMe/yXsEyQv7SHhQut2BmlAkY/TiGBMUodh/IVsY9eqZJyd8QFFtwOiniihZvXeThaZ9FE2klJpLeTuf3LEbmxvJS1yfOIdoJtcpvpWRbRIJeGlWZ7dA0TwHgG3J7KPDSpJlBSWwrAbIShmL9NmjK7aGg3QYNsa0EyCiOuu9qxa9LbdefzrTLjaBmQ7TKl6SkP2vaxLcS8p8d9HDvpvUuQhulDoBeUtrtIxDcl1XOjkjQ3qC3STekJzJhuq95oFTou4GDnpb71B8SSfdAmqIYVWGtHCs/qz+q3HfIvg/NxLk33y7fLk8KLepLV/n5vI4hV+9sTmJ+lIa+K+vwvcOkDc6rPhcqBHUPeTeu7DCvCy2XIjPXCpPXaue0unBFIjtJuKUk6Ck1Uyu5J8Fq3SSVdoTtINQrvttCKMhkq3WXVbiYbjClKzE6i1rL7RbNOa6uXJfLazxqjuVDjnQBHgT1AHo4o3XgcTl9+imsoHTrJ1ot6V/v6V2d6q4PcktWQuL8RG+PeKkaMJpGSZBUgRwPeBAkUyA14HE5KmsYjeJydrW6XHX1j2d4Otm99XMZ2F4miMZTnW/RQkGWZFz5EFZV9SlVg/AxckijKbVzPZp9TiJryu8xS3IoCe2kkP5OlgBTXDN04VclSui/s4cNZlp1zwV2Q9NVCenDwP3IJXmdwH6UmVbsP3yvC7kiUIPT2u36evVaWw3Dze4IRSmXWu7mEjuNHJK7RmzZXcN+5JI89W3PFYXqNhYGILF1XekYmu2cGZxRlah/X0ri9AE7LGG6gdvj/yizWV8SZgb4n4PsxN29SsJ49yWSD7PF3UAaUkpFmlQOjlbla2gX3BCPKJDlj89KLmlUFnIhPP/KQntdl/JvzTgdNvLpI8EFwxSgQtUxBaegznfASkKV7AdBggvWUGxZyPLxhZbJobVqVJFhcopBc1WqhG8X+fwexQGSvQNCgcZBLkzBLWs7ptjk5A8uaWWpajbUTuMgV3+fi3wnfujqcQXLi+oZhezQkXUeqhqG8iuvGxSiNFRkwaWeyscoStcrjlBQeF1j/8kF/LEuUCkwc4cCFA8cX4g0LcnrB99bwyqXCDiXJQWa3x+qEfogsZ+AFw6cdYiwubABLqJ9HVrtolEaMsdzg/uBK24l9qRH6ZYuaUVdPLgwdSt67qbbivgDCnF29ZYuOljjKGNxhkAkRD9SV01t0RZ3c7b8CeW6vkRNkjslKVpdCLncYdWQsKNd10FKx/pp/tXIpraG2dbQjnMII15m7RuT6d6NHTOy4vS5q2BsY4TiNOw5oEQXagRVR2ZZpHUVSaGwEVWNkkYGp82WkPgSSUV2o1WSum2obSWspgXHV10O61OEr0/V5QuCUwjp+FEYKnXTyoQKBCn7yrTKbIDdV7qiME6tjSfOmcqXn87OZAIxn95ekUb5o0ycUQKup7wyZQ2ujYHJGpyJwWEkopiRT33LjmWPS46lVWCBxC67Qv6VJRN/fY+syHwLkxU5CeOAMS4kjKibk2lb1N1BenXMkY02B5EGB5zZGTk2DcKiLLyHh4OiGBGHJYdFU6Su/kq739Ks2qF6OKIH4uu2w0pFMMzqAD18PmIxZOcAhEtZeA8PVb8IaQ5iR4kGPoZJrB3K790DYdKE6OMUZSwYmsqBB0VODQgJTo+xFUXkckIoRmWAHj68QQbhUhbexyMO/xu3BDA2KQvv4fFbhjIYYxSSexgkyI/2MDXoILqPA7kO0KTDEJgcqQP08FmRz6/g6NTk91qH7rIANnUNhB5GxC0LhAgX3IdPQgLDZM1BtPQcUzS00jmILQvtXI6rjCV0MqgL7mSh6qHWiy5yVWtpSHXCFhK796gqVUcnfkNyt2sYrjl6M72Q2I3L6otW4EKkjh1ltOuPpUk3Ax9c9Ni7DHbx6dPFbd8kj31l3q7Xo+zCqBsHdgYTbFTl9/Vl9ONRbWUPEZkO9cAYkAiXP3T1SmnJtnYCpTzHj6KlHUVSevpWmmKNSPBaK6EjxweXuIIadFUZT+ljoxBnNKF8yw10Yh3kCcDoVgeJLPK9ZLGWAKzKFIBa0pFga2jlcuNvkJN6CX4XhWTTJrbiZ8MqB32tYibBCU6YyPYnyshl+S34j7GbQuIX8gX4GyzlO9mFOFXwg3ARsrsjR12AkAvhAmTbhoLNJYsw3WgvPWhQhi2EtyLTiTsofIEg4pAHs6AXfECxqGOIeKSx6cQb2VMcyhRK4gXoyHb2QMhctBg1seBgmWwhLlR5yyWLMU07ix/AOpIqgIhBkMZhJBvRRxn+IF2Avb93tkDAXLQI1bdsyEpVkS8aF8kG01QF9p0WRBr5BAQylyzCvLeTb6FQuWwRrtolsMrQjbteqzPuvfX62++AsA/Cxcjfnsj6NgxAzoULkJ++ffUOCJeLbkElFzlYYCPRGoKw9ZY9PNKBnUfFZF+YSRaRnQKegBNL3eDZjp8jdsgkxxUzMnX02cNImR19On6po1cfSqy914/jUMMwdwgvjixgFboaOokBnHJcYXs+C6HovpWPphnCIFJdc4foXtP8fBCzrrl7dK9p5j6IWdesvriPnCyrzcOvzqC9z5HdQdBLsAzewo1a2LSTedqzBgMBy7OfL2/OSIrw5vTzGienP4auEpbzjTNeuX8lN3MaXUgtxK7IYHY4E+qNnBuqIq4F7iZnjr8L42edyCLJ7TofX1meR+6o16p6RapovJricpZF3Kn/dnXyhP8DLrh56Gl2eavx4fb87Zs3P5wW1OkZtlZeosXQSPZ+Zcm19L0Q5oZ/fcXulxmaVejJ8iMPv6tXmgMdMZQMp0F2GMKox0ZneyuQP483gk8JSMDnNk1XdJOUfwaXZ2IoGU5QNhIBdeYZrmxW4CRTFOwqlhQr+KJUQupidBGQY5CT1P8KlAwncCOVgLr4rHBbituJKWxUgZLhBG6jEpAEn/P8wOl0pqogKjCcynDn9SO4FXanWDyJXoa//4t0NGh1Uk2YXi5XeEQOmI1tYHK8prEThxKNNZ8Tcj0HiWydbfR3MiLxAhYf2ObyIGvUB9kHWe1AmsbzZWkisGecCevwZxIlVQtcRV43oC4NaxJbpkqfw91OPsJjD6bHhBk1wQLoNXpKrwPvmX+lSWlOQCheQGNFy/oZvWVsuAmas/Wq1D5grQsFdbkCcA1Gr+P2GZrUtzc/sMHOcEMniExiI+vZCy3HqMvsgx2hbgewwmKIljomlivuH0jAFebqNQha1wpEfl1lhU1OuHLBH2eej1cwZYyJvMoTerManllEOMWG3guCX29ihyBa718tyf+dvnr9DXn06tXrw6M3ODFRpZ7Wt7OlFfvLx62/xBrsMD//5OTE36UbEq8gDUPP3ltuUE+H3yXOff2pHRIpxFRL/HZJT0EssR1Q/J6Gxl6a7958/+rdd9++e3WIku3jcuS9d1Bix25EDPGnH43mM+YCWTEYfvYjPfuC7BT/+2hxtLq8uvl8eXa5/tVcrb+eX16bV9fnXz9frI7eH/3n37GlMtdL3eDiifreJ3dH7//zvxbkNhQ/fEAO/rm1vAQtig/ZUXD6HfUfwtjFobVDA7co3t2uL8xzFmAtz+jDu4PLqEmSVl/SU/7seL+DNtkuv/qx+lHkBsLnJHy2+L5I8TdpzA5dV97Su7LzoBwPlpex017EOjgvMnJU+v3d0Y+5td9fXdGH3zz5XpC8z5/+dHd3d7RP0+i9YTw+PvKSgAuFkSRkWE4+WiKiJfnym9zINFkaZ+yh69Dfmb1kuEva+CwzO7d3sba23Nk2TRE5fkXEn+5oWaEHLMiYMsEFn9RTBrT83+RPg3xUFB6uzp+oTXJaWGEi8R+LseXGytIQs6aF46y45SfPAP6SBUAhrZO5Cs+WnlN5y0JWs5SmbaUW7ulrAkjUbnpqYm8Fjscc8bpeN1IX0dXYF0lTQOOLdhk0vKBLfeHsF1eS1sgnHTj6A5al5jRy0T5tFL9iaZqdeP3h4Ts+U1iU5zuL5qh9UZ85LLo3hxaNMfhCvIGxqO3lLNo3W8SvmOjKKG0hXl5etK5gCt/UUlQW2EQvat9XFuRFL2rfVxanRC/E31dWaDres9TipWfRC/Z9c9Bdf8i+E85KFoXH/aJ0kGFRbMMsat7xi5q3+qLsPb4oO3QvCh/rRdnzedFwRl4IXIMXVTfdReEXtCj74ywqTjCLg3PtouTxuii8UBd139BFxVVzUXJcWTRuFVGMLD5IQB4ae1DaUkhx1fSl4NiKSSvRrRXTViOR9yfuiDatmrgUMFo2aR6UqBq3uStx2w0bCmmK+B7yafJrXOQTiO5fUUg9IA2/20Q+RXGZhkKS4j4MlTTxoRuUS9N7W4yEpMYNFPJpiksdFJIU905Ip+m9o6NLUnvQ0f5UjaCaiknKwT9Vk5aCvKkmzeNhDkh2iGWpmpgHo5RM1xLLUDJ1fxBJVUE8CmR3OkHwP6UE8pkqCILYlUAx3LuqqI4Y+DKiumKJj0nPw4AryRDEgx+Tnodyl5HRGZd6sIBDRGkFEeL44oMFHCKDC0QMDwqoQ1j7CG1IGEglMYJoGkPS12NhDJEhCrg1Rk4tnswYUZ2jzEFR4MbIaY/bNkZqPebaGFkdY9ERkcZGSWvECBsrrRzfa4yscnyuUXJK8bXGyClCZI0RcghyNUpKLUDVOFm1aE5jhPGITKNkFDGVhkjpmNSMDMWqV2IRRXWU2Gag0iHiithFQxI3Ag8NEVJEDxqUuIgAJJNaIjyNBjE83I4uUYwVP0O9qJ2ZXpRjuyz4geZFPQjLojjGtiiOIC/4QbJF5QTXonReeFE5QbWonFpaVM/3LgQHhxblQ7iL8rnYRdehg4XYS2zRdA5YkNvdiwuj31BTv1meLN8YxbZbYtD9U+4JQAbhceKyTVUNIoR7twME4kH5LnLDsbzIFOe1rUFK9OhrkJJo0QjnepyOUopMew65bkuKqC2rOg4dVVq1VUJVOWxiQqqImz4PlZHvewzl4CKEfnj12iyP/JRlRKOSDzRAPoHlqWWzkq2BFMWBOv74dkb3MAx770aUFm+DZDnJSSX7XIqmkhPsWKkFIhg3sZWNB41yKyv/GuWyDzXLJU2GbUcwpKlw/ANALNuO1C83Sqp+TJplx3gwuc0CG0B6PnAAYU+fwxRAJtp/0F1ISuMona0daZzdm6uvZFlhx/7lhCl+FvmZaUVuorG9bsNq5LZ5GALrwiUYDkJR4uE/zCS0AeSX64Ne6XzgzXJfs+xywdIjmnk68qY4w92S3mp8kE82R3BxckM8xXGR5rYud9jMc7cFaqzFnNgl/hz54jf7lbsjnLMffKGd/TTPTr+M1k8dVE93qox7sf55FlDz6vRsHuCbn3+dHvhjaTN6QtjL1zMY+erscnrQL6dfzqdHXZ3OoOrqZgbQr6vT2/UcsB9mAZ2hLGFUoKZ4gi7mAqw5/wRW3MEaR7AGEHdd5ufTLz98//b1KQwCWIMGlotfz69OYcaIVDIMZ9qYwpCGa6fB2uLSgbPGmSI4qMbZtvFQEYqpO77lmfwRWVmwsiczRlsUowADPoTUy0/LNKoH2Ca+gGYWOeSAEDAWmcFZzvh1KgkY23YngSEHQtmRqinQ4mlsN379WgpFX/3tgHGsSWyGtlkySf1BZLtzEqBgTw4zOubW9afAmwhmFzm+NQlQEk3SBLmvJyng7utJqqs7SUMaRpY/fm9AAih6nKRQ/4YmKWq/TVSkCQ5zsZwCLQ7AB1kUZqqGJ3HcUN86YieQjyYp3sk0PV7iu9PYbZpalEyFQ92MpkMy98hyDv6uoIgVz1pQpGyi1oHgcIfzSfASS4M/jATQgyXv5DYC5jGLx2/99+DUDzBDAuWuEm/NCLSYkw3cylKDrpWtFiw+XZ4Ahs3IJgCqeG2DItHGaCKcojGaAo82RrqA4tDna4bFeN+y712N40kBBHlke/f6amsbhsbFpjYIfQ1cC4LeZZk2EK1LMi0gOpdHWiD0Lo20gEThI4koaD2AZ4vOuXcLhNYJcQuGzulpCwQ9GWXq8uKWAtPjQtkHRTpMaBCtA/cODO2D9jYsL0oS0sNoA+Jn6YXBQnRvczXAijEu80ZnMT9MN0i3MJg8VEALpq7BRztmLbqJNkDqouqhB3bwND8XpG/OJZLvJJrrsAiEeFmDAkSeBysfmr/+LlsIE4c2SpIQQ6Eg0bijKgKL0U7rOpwYI8VpVY43DkLRuPYmko8fRFPkCKnoJpm7AbcpAP2bAOXx0dE3uSkD8KksbbUg2vbiIIXnwco/tCqQMKR6QMovVw9QnFL1gMQ5VA9IFFo9xgA8uiRsfZIY1n5DBlYo3lr4m5Jfzlg/1h4ETYdI+1CoDSERijikINJDUOtE96np7/xxDW0PBjk4BSmfCMW1BQ/YY+fRivHMmEbthkT0rIBPTDRMvmTQaGwpuJpSBOuDAyCb0PS+C3pscQKY8fOeFiDeBJMZIil4/ltyYBKiMRYhhWBIJPhGNa4KJITJPwPEYtP5TQaPwQobGEq18miBQXEchCyIAfmXnorChFYsw6Rrobx1Y5+28Fm0iy0StLH6gEXCmRBKi832YZKS2p0/L2W0GdiRSV7r0cm1/Yhf3BIdYirrlqrFKHmnRMcPivFaZOTytUXVYDYqsklZodRdXf12BUDnwLYimJbHogECAHDDlJTue6Rp2FkRTp0e7dDTObOoABDPSsiCoxjiSV6wfoM8ultX67jRp5evGD69ySUP76ep72YtFGdOGivtbUsTYvweXZ9wWlF3LAZc7aYmPYB5X143mM48KUG06wQAmG/2YFg7Rg5ZaLY8vaWtAUSb1TyQmz1ugeiAFTe6Sp1y8+4MV8kNCQC7xVMd9BjGeslXujQtgtN9jCyH3G3KRs/URy/zUpe9QPJrgLxHMUhzV276pOtZqwSRWTUIVe6d2mUNHAy0C1Trhapy8gFybv1h+ZcXuDbrywrlm8ebxMnHMmxfHqXORlq7diEu/t932O7faZKFRY2VlOB6peBu1iZFbdzXJuVZvgi1iMgUHM4KTwGUkkJi+uyutVJnrFHUgOorIdQJtIpTDsEpIVO7wAHtTZvUouU5jOpUW4qGyGLUo1GUubES1zZtFKdYeXItAR5pHO6D0QkxYMgkLRw32eRi0x7JcXEXc0nfxLAeye6bi8jFp4fnSxp7uu9AQL/ICI9RiFnv0TMVeY96Zg8dIkk8vK2LPAf3RFTYIEG/ZzHCdsNDG5/eizdclG2Rn8OTUwtpEMFtO0SC/4yHhfduWs/6MbLG8CFlHsWjjMJEjCJBh8jlWqFJlAZSzo48HVNsSTtPNxkVBITpnnYWDrl3NvW5e9nTt6/e5Y0O+Sdp5Q5HbfInaeknjUlv2gmdtLyYC4uLJndFf/7RLi1+MXbMHWyxFXGKXfKHM+Thpm3c89lFqM/4JZXVzObp/2jWfTEmPKOeCLmMGzxh/4ATO/9jz6H2nN+S/3W0OFpdXt18vjy7XP9qrtZfzy+vzZvzq9XR+6Mf/4wtcXf3DXENw1A/3R2dLF/dHeEniNwZjlsq/Ojr+uPxD3dHf8aYGDTHxJ8Elo9+EtXCM9Y4VFUlQnHaCPe2zysb/42TcllHVDT+AP//j9vQc1B8kM+amso3/Es8rj98d7htxiQESC+MX+YkEM4eZmHykN7VSZ7QEsTM2Smbz3zzUxIPlpexmaA2BH453lnhA6RNdDN2mk67tFwgAwKRxqGnVXjklr2utImld6iybVsHbbLdSIwfDVYnyo9cFov0xkr3+GcWu8RUaea44XveLBi85jBZRXNBf30DU/XXyCdOOUhX5c8HHf21n12/S56Zq/Bs6TkUs2iqD4ZuWl+qAp0VSw95LuoRza4RZ5LxvCW1vHCnG4BdAFna0IMTbusVXlx0y+RrN31DPhR/em2xS9di7eFqDG0Hiko0WUPQOWsc3BiIZ+LCtqExhuC1V/B1Sxq2Jdj6fUsqNnjsSdWa9t3Jk1TSFgG+nSkkb7VV3Ke1vKy9Gw2U1WYjMuMcIbBFbHEJiwbhXRDkdhdtEByo0uy0Xh1zx5sH8v1745fQy3yUGLfFOtbOTY1SFZM5VTLiJptm+6nRCJW7euZRvEJBl7KCHmBuQbVOr3Z1Drzt64jjTV1XqX7T0AQ61SG1KyW4nAleLQEoiGKNW62mUa0BO0Y5DRW02QMFevrOhsGLq/pgDV0EQ+Zw2ktP9ZZLeG2qeNrVqR6LhFeniqddnVJMHHhdSmAgjVT1Ss5pKk4VE0at/DLQiRTK0cD7x8k0qoGCKNa4R3Qa1Rqw2pWrXKELr1QFDkKZKduHChyEMocgqpMoc4CDahymrj8VSP3TnNptuvA61RH1q8ROAkzYItQRgVTK71KeSp8cbrbpzLjE0vvJI2TqWpVsNDPtV+zCZb7sJb+zT2+FY+ra3crQZmqBHWUb6WIwkX7t6MPUHJj5Q5M1NnZYKCRtexyl6+r0tir08lGI3Y7z8WQ75V+yTUnYjQ6mROmS1DuAqsDDZg24Fhx0g4OjsZvPX4bmjMsUaud3oL8ItXMuE6nNb2F/KapzPlOpn98F/2LUz/lMoX5xI/2LUL5gM4Xq+cXPL0LxnMsUaufXUr8ItXMuU6j95fTL+YvRm5OZQvH8Lu8XoXfOZRK1b16Q2jeTqV1c/P0iFC/YTKP6hxek+IcJ1X45DRsnM5HiL2fAzsn8M/pkQXiNaFgvFZYNoVf9NMUjX9tqMtDvsiA6ezO5klUGL2vddr5lHK0rNjWtLqaep19omIjXdPg09cDsk4bRV02HqWeOGmaHNQ2mngRqmOjVNLj5+Vfz8+mXH75/+/p0UlXqwBp1mnqqpmE6Vtdg4sqtv25/Pb+atjxxQJ06TD4J1DLRa2gx7XxOw5xNVJaAdtXby9LYTXRRWZpUCY446T4wV742TzncoKh3FxbMa758dzBklvVcWwziHle5iXcG5QpsGOe/w8XSE+tWQgbLt/yu6TmKZAzn9Hy44noOxeAcUSsXa89R0zg2mHb5Zdsz6JYjg2lW3PE9g24FNph21ZtKZ1CxSgBOz8e5ymeODKYZv1F9BtU4NJhu8Sz9dwkZTLPGBfIz6NjgAKwt6LEFKV3hjjEUOPwSujl05Nhg2mXzdYUZdE9IAMzH2Iqi2TKwTgFO1/KNznMoWsb/JzmyQm4ajvYotrxc46HO6y3yQY6v2F5o35tZ5ORRjyHLQtNA9PxKnQLMiecYLMZEj3oldBDN2CLKfLoV+CDa0S4NLECQhHoHAiD60V5tPu04PJhuRW82r45lGjC6JhaLEz+Xkhx/dr+RKRbmK10G8AhGYHQykqlTAOu15lGvhA7Ya82nG9zWg5U9mTHaohgFNjIfQi8FPgjao20nH5C8tZw5S22ODhQOCHylras+2nCLbYShxaK/zqhghQSMnuCbS136wW0w2fDOuZ2KwW1SO9acrUmODtMHTrEh2NUHgm4KomBP4mg7U+wMdilZowGi67wqQmo2yeZnh26wG6C7CbbOupSD3D5zX8/ZbuboQJrN2dfl6DCazTn6cuFGXWFk+WDBsCRUK/BhHM3gXQs6dIN0L/gNzdk85ugwms3b8oM6ThDhJr0EfWYNDyRgNjLhnUM6FAR1EJl51BWDjroSxw3BY6R0aFfgA2nnozn7gwIfRrtZV20TQHfxxHdnLZQ5PIxus3Z2kH1dMrNqoLo9J3E65yTuQABQP3OPLIdssc+tZ5kIpC/DfHoeCMD5MsynHaiD4JTOgT06gjsIMl+CGZXk+CDaPVjOjLrl6CCaPWYxuHd1h2ocXn84dQ/4WoIe5Sr4ENrlAdzfmtFcvUOTxD+JUy7Otof+C0IVhQYPvuhC0/FktXj4dpcydn2IuSWRQU03SLfQTmt5DhjEaOzevE4q2gJ+tehPaJh7XPwp7HzKN3loaFn03RcJ4sUnzBlqCAdtrWwyT6lKrpDmr4UDdFlsVoCpFe+m8iJK5EvqTOhdOB56QN4fJHrFJgQf7JaMUgwFc1gYf6BkmjN2Ir3K2DAbwJGfzaEYx4XZH/W8OXTKYWFUmiebQHNpskAAQs3gowBEcYiJJyGGQUEC7+gqVFPAAWYHEe2mWPUX6ViCBlItxdxc+DikYuVK4DDqwS/5CxUDXOjHYNGMla4GD7a2b5LFlXkGXDV4mGXhqc6MixTUdmC8Vb3HRwd8gVGkGcd9aQf3wG5LpGNN6HWXkqH5EUmOCzbGnUGnHBZ6QDiHZhV0sJ52Bs1yWPCedgbVavDgPe0cKlbhgXvaGRSsgMP1tDNoxnH/qFtbVUOWAulCGrIUwEfdcsoKqyeoD1se3Rh5eIqtPHZpSPKRH8bPgy+greSWb3leaJubbLsFCmfC9TYYbaOJCJ55LXZEcRyEuu7xHTsqbduKoSRB84UiVPcAC9CXEJl77B5EzbJMN4D5VM2gZBpVYP0hb7rGxSFA6UuvIJgi2RdEcWDhR6llgwxOisxlRqltmQspaIzNz0BNOgpKXc9NnyfUUYCsXzVmSGLEXWyRZUxzbwWOBxOfqUPRTh561T6UHTtGDlm8tTzYHrit7Nbw/wkbfUExhuwBckPnjgwC5Ckr0LSKdvIAUnsOFf+wvbpr+5GeuUNujojcKAHZbBHGRg1soOnbFYAsQg0FBpQdXdm/dWP/0YqRmUW4gjpDHVBrlqxJpRUftEzU1ag/KBhoKShC4ZAlRk69GYvR8NF820CIreXAjn/KA58Dnq62ZLLhOVNDy5i81QPU8rwNnsiY2xi/eQxj4FlVxf1aiD1XSU/3MbIc3GrrKu8uP9GQeanLpCOQHYPCuoUKRgf6XPbdh0lq+nambZ2BqKd17uA+Pr07+d5MQtsM7MgkhEGzi1skf96G/4ccfVr7DVsq2VqDD79ALy2RhpUcgMBZQI4d4UYINLsrJin2hJoE9E6eIs9Kt2E8tWo1ZBidTE5gPuUqFCDWjTbZDNpVoLVqRbqlopmbXLEGukbd6pU5nLs1CQFqHisY7AjCTKWyAP8nXL7MYxD7wLPOqnVLwY/98W7FwgC6qWWmzxECOVzco1UVXLdmbG45g1YFsGaN/LczaMNANWsSzlHaQohSFt3jacLOB3EO7tGnDK1ZqyyZpeZwWM3aEIKk491bsUPX7xKUZiARR3v0ayeiW2PPCvipXbhT033aCklAaOqgrRvM0oPV4QHH49MqJ0AH0g3svK6kdlrO7DZH44zR5HqVcP+Qa1PDB+3dewGgW4tBYy9guPlbo0EESWoFMBd51DURQOrWhuyiUVu5wO12TaU6rl69JphzVPQZO9Vo04OufhdNy3T6NHH16uWGKenN7hHseL2iUxVTrz40hLYdehPM4Cs6NXH16kXCg8/Q4jVgNWuVWmk2pToFnl49Ji5s2svYo7t1pxif5VrU4F7SMRmpT3s+6ns9LOheM6weSokXgulbgbUj55pk1Bu3SSn2VNtYiWvrdBcu4m5VNSx7RAsx5QtSlyK4KSdvp9OiDKhHBRvFKa5jtpUi3ObhId50OSJCHqkUPHlZkioNyVgPkC7PYiiDMM+OGhK0YdQmuGKzaBsatZmm5Gg9cDwkJu4EU1DOUcaSdSM9Ix0JxmWosbSn4qyNMB0fTcW6Cja2qQbnK0tSx0CupcHaJE4+fx025hrV4GETOBtdZj5owuK6ctmDSwGW4LJzh3AUSwBjeX6H5XwHy7SAGF6z8Nxp/D3MLSwPwofz07J81MZvwDpRnd+ztma0Tu55XJuZjb8dqoVZJn/z0xwz3k1oxY7qfJctKcwwzWUhejHjseNuKqRYGol58N9c8oseZx+YjiywFRscAiDnkgdXpUJKvrGexVqOQ7WzbeKMGTrtIjcEsm9D+hie7msbjmZF+BiW0aMPx7IifAzLBDLLE105nl/2BUe0Jv+ldll8g0e11yLTq3mWZunETv+JOW4Ig8jna2gdWC9/XY2Q12wVvpDGRb/ovr0j/0ZW+KpVyrs/YqwxDRUTq2sdsIO5ruU/Kk2vX0E76zF+BC3UtSxQtTPWsiRF6x8UxZfcXTEvU+UplmSX0zS0phPmjDbvVaTPkQ9bx5NtNAX36zmOS4b9LLCWBheGXG9+fZ5IvFRJazC1/cRNdPn8VVk2RA9kqKvJrrFTbaYFkx2E0A+vXutp6KrsGqIHMtS0IVMjp7j30t4aaKUlXdKGNqXtL+otR8zZt7YfzQb5sDHet4ZVs6f1SOILu9gi5d31ZYT8pT3wks+Cv1GmZfQj9Ri+lXgUuw/EJeAePVNx5G8Y4i1IysRx4xFvXeQ5eKhKBWmmK5KvRvL3LEbmxvJS1ydOvAA0WxGUiNoWkaqbXFWqGiFaUkBINSQPIMZLrnZeJcFKtMgeA4oh7NWUPIAYgL0agpVokbkBPbinmVVdrpqt6KpbuXnXbrRWhBFEITK3DUCJpv/soId7N613lBqJdkAMogpgS4HoIZnt7IhUgC6sTX4bSdVpdpjuez1vayq7gYOelvvUH3bL40FRim1UxSnrJXghelTXe4fs+9BMnHvz7fLt8kRggeYCeh6DRmHIPGgNY8SsPg19t3qsNEkzxw3fG1hTgytQn6wXqYbNlTZuqgqZJxmzIhAmr0WxOyTARekHMXFLotBTaqZWck9uQ3STtHZirJ9Vr6xhtkJB5qvah6cZhmglCYqVC8Qh1dA1mEElYVz+4ylQXI9xK4F6SDUQ1cOFQhmUJxqqaWORUEpTubW/Yaufg9d6u+uaLakjD3HVI2tULVLjUko1qjwroh5SjSnPqqA8EcxKWkuRO7taXa5U+/OzMJYLydhIeRnYXqYUS7o+H6dlkyxIuvWQ9UIbU/0MQtjIsY2miMHbWUwUuWWq7tikRqYkYTCX9HeywJ7iKjuISDX5SIvsbIkxW6s18tTDOdCVsVogKmkKpcSDGfhRZlqx//D9IAqV1DM71t+ur1evZ63ruB+StSIlWytMefLBWRkmZura98MolBJDu5us6BR0uLExVXLjoFu7FalXVYZrVJO/1C34gcd62blo0w3cgcdLxvqNlQiYAf5nfxZxJ/RSSj7QEAl76c5QDc691VFkgEr0oaqwmZtZN8RjTmS1r8SAFi+OLl+seApepsoSRjkz53LMOJUYQPex4VJeeukurCddqguNuX9ASYI2+49mw6XMXLnYTbrzVC2GTdsa+fLMEhUTw6qIkZ7jRNQ9igPkjeRzEPLSq1fZgNJlOtf54BJfFqEvD8bxOQh5qSMu5Dvxg+p4S2qXAaKwIDt0UH+eUKXKK4NFuplbuuDBfzPHAWgMSwKMWpknUcOY9Ugadii3lnj4AWIsaBD6aFRzhwIUy4xKRbqXEo9jsbck2hMRgzzhaHRz61nJfjCHQ/JxTLxQZr4uIsFTgh+QGTOqKFcYHrrYc4N7mZX+ktKkN+8WpSEXBhHiKaFzwXM3w3MBJ6Zkz67e0gVLS0FTnNYQpH+5fXi/pXTtZeFBjV0PjtViTmJFUmZKSV7QkQTOSqIdIorQUcQhyYB9pzy1Sl9UA5bvieDduYlnubxbSrp3Y8eMrDh9Vi2n2xihOA0HBjGga8xKjcjYZVth3h+WutvqDbGnUTKTwfVmy+R8GbciSKH9Vw7/OH6zQGgIrlXnrkGfLfjqfV2YclDMFm5+FIbtIzVldoU4kPwa08Gy6emY6hGFcWptvCHloibp09mZrlsET2+vSLf9UefFTUTRAZWXWdjgZjKwlganZ3CZGu8MICJ9y47DAcW3l2sh+A95KcAV8q8sXZe/7pEVmW81FIiclXEQ+JJC+7aY0tXTGfQ4j/YaMadhqLmTdhDCxTsjEcjGUypLGknKQVGMiGu2w+6Iocdc2x3jpCm2yx1JGD2QQwI7bIFIA826tJHkPmIYsjE+nlhZ0khSne6V0oRU/C07yPgoSawdMjfZdtvhwSVNqylvLMEoY3cHNM/xDiPYkKeB4GNsRRGKNdErSxtJjvc844mVJY0lFYf/jRshDdYqSxpJ6rcMZRrMVIgZSSdBfrTXUB8PcsYSSkkwGjoo1JBxdWkjya2IuCtN3GrCRtuN7r3ral8b4kbSI/7k41lxKWPJkHvVNOTgQQ7oool4NDl4llCWMHixvzKAGkylLmUwnU7X+14acj74HfCsNR6MXyQf7mFQqa6DiTTEDPdLxzV1RMEokg8nwOrmcAZF+rn9mNBu+KUy1GHkwUWPo1arLz59urgdOy9nUszb9Vo+U5juxkENg0kxqsLG9upUmHx30MNKxzjjoJ4uVlzYfMvFGrePhCejq6tAUbS0o0jyXLdvpSm2Hbkcy0rocP/BJUdzDLo3ZqVhbBQCjXZQ33IDvagHiR2wdEeZRLr8XjrqmwR0VWoHvKVwQ1MNt1x+/Q1yUi/B76KQ7JPHVvxsWM3LmOroSXCCBSTyNyspcygj9DB5jN0UlkmB0MFkg6V99xaMxkF8Fwd3R85fg3EoxHdwsG04ArnsLnQ32tcGY1oJFOJ7OdCVHmAiBUYXmzwUIL3YGo5PHaWLURqbTrzZwZEpAXTwQLazB+PAhXfjJxYkASa9kwFcCc1ld6ObdhY/AHZlVYguLkEah5F8BFdlIgf5HSz2984WjAIX3oXvWzZs1awgdI3q5K8bUaXgN+4bqS9hI7geLJfdhX5vJ9/C4XPpXQxytzU4EiWADh7J3nr97XdgLA7iuzl8e/IakkMuvoPD07ev3oEx4MJ78MllsxbgKLuG0dlr2DpY5PdwsC/MJIvIHhhPwCmmbvBsx88RO9CbI3dzM/WMKYbRMyXGHPgjPaOOoRT7RyVxHGoZwg9hyLE7+IWuls5pALscubP3mIla1DhH0GCmbUY0iJ7MXCm617aqMYijzIpHdK9tvWMQR5m1EN7B06jlczGtc+jv4eTv2tZLtQzfw5Ja37STudrHBocOvmc/X96ckZThzennNRZDfwxfqy3nKue+cv/69O7ke6MLq4fiFRmSj+FET8fkxqsI7AG+yXXB34fxs14OItn9dji+sjwvtLWboyK3a7Sd4nKZRfx83O3q5An/B17U8wur3EdalD7cnr998+aH00IJehi/lVnX4nQUSUe5ldwN2XcC3vBUV+zm6OEZiZ4sP/Lwu3qFOxATg6mwG2ibIdwk7Xa2t4J6rAAgZiWoDma3abqi/gH8c8gcFYOpsIOzmwhKKkdxhbUCJ5mmKlTRlPhNUeRKWDLcLgISbGGiVqQCpsJuAsOVoGSYrXBrjVudaexWAVNhN4HdSlAKzM7zCBVTmq+COYDrdMY8F8fvqPE8xTAkgjJO9xeF24bU6TWBpFld4RkGaCa3wakxnMp2HKxrfPyckKtDyf1J2QaiSxMBdPD5wJwSBlqoPlk4SOuH1DZDKcvrgn3GWbUOfya3TWgCrkiUg9andU1mz9Twc7jb1YPVj0D3mDijJrqDxBo9pdeB98y/1mYITkUI0EFoRevLWRhs3VFmaa5nVOXKUtC8qFKX3EFDS5bUGchmA6m9b35gw7Mx2ZAgMrmPrGcvtByjLlWWwCgTdFAYsJikqaaKJXf3UCRaHnNhHEhC17oNDeBX41Ohnh8pubHSvaY1Gr0AvJuGkVqvdHpRaFAizQLbVn/1AxXH4nUK/v333828pCCHrOYd8+iLOlHKtSfw2K0ebK8dRKkqnBtOBKW4dUEjS0FxETk0TIQGYudaJwBejWcqJf29x0hiLfc68zCzm8Qxk9C+R4BYLP56MbujGvt2ZjixSy5CyYNQsV/5XQPn7MellpKlg5ZNx6HzE4EtDQFKH8OYHJumGxbzFYk49LnlJ+dAQuMZ9t6N4KA3oRU7HHk2I0copg78lmfyRzMZe8bKVRT9PDDsdCweXdJbJAlx9wrCmbBd269P66aA3bqx/2jFyMyiXWxN2cIfNJ/F3olnBlP3tQW4td+YboCHHVuL3OIwV7vDpsMeemABFWBI0Ag8dujRK7s1w4hvOgYByG83hrDQIZ69hPyeuzKHSmjehjZOUuU6rqGiWq4SGiqufnvAcDl5/F5FAZIBRrVJrYZK0S1WGE5ybE6XL7QZl80Kw4hBZ/rHanq4VqQmiUZKOi8C991V1nXN1a+r9QW29fWXj5efzK+rC3N1ffaXi/VKVsrF39bm5e3/xRK+flnTFdkHy8vIm++/lZXx8fPp6mfz5vQTRr/8fxcVKW9fvftOVs7Vh4vz9ecV1+bj5eeqrLu7//VbFqb/J5+VH5O5wHG+4nFs812R/CNZ0Oub9eUVZm2uL7/8enb7683aPF1dyaa+ub3+5XJ1ef3FPPv59MuXi8/mxZfTD58vzivET2TFrS7/+jezuWbZ/v3t6VXT5q+eXr169Rb/79XXz7KSDtrf3GJ7rC9/uZAuQ5j1J0z66uTk5OrT+sOpdLrPl5g8VXeQsXjyD6/GCfibeXX2Nc84kxXmD6eri3Pz4m8XZ1/XOHd1yD+7vrrC5YTJv7o+v9AjlFaV29Pzy2vzw/Xp7Tmlbv5CWiQQ8Vjy66GicxFfTq+qyrMayxfZ1OpvIfX24heB0NNXr5XlfblYk9y6uf5y8QW3jl/OPn89JzVaToBprtan68sznPDz5ZeqotSfw8YP3MBzg3of3F3NSb7Wq7ktX8HbmorXW1ulnRDWmcubq6/m6c2lfIvxORdC669JqF0V3diw0tXohFT1uf748fP16TnJ/b9e3/6F5OLZX+SbQJVWu8DM7XetYLiy+S+/rC9uP56eSWu7PjXPLy7wsOEz+bPcIg1os6Wz+q+XHy9HVCjWs3/5iovv+V/M9c+3FziXPl+fneJCtL6+JTl+c01NsaoUndcKFA/szk7Xp5+vP+Fu8GJ1QYZEcjJu12s6/KJjh3IFkyZRxIQyVxdnjU6HNWKHj4SNWr699LGy6b6z60NK4WdBiIegMh8mqWP/+793fik3Gpe4fUfMQEofv/ZRGobedZTbnPy4dPBXh6fLzF7yu+ip5UP6uOOrpf30ZJK4Jo4VO5XMwvZpVBAZfPLL3ltu0E2g+GxpR1kVGE/C0NOx/3YK8G0NfBs9vD1O6quWMNBeaKWmtXGrvWu4TbdD8NlVW93g+XVc7K+VHbtRWgH/FxKRmKzkWFka7lBgsC/J+o+5Cs+WngNfILdZYJOHZoLo30mFYRpn9REHAAfHSi2t+AqZE1hBaNombg/m0Dz03dTcxrj5MaOQLufOQAIbAD3ZKJor+zF+nKbuxBnPHWeurIj2AtPrbbf0BO/ewWM/WnHgBrtkaXneDKoX8Ogpja05CUTIsQIywap0S5aXTEmC7J7RQBHJHDTIF777Oz02UO0c3d/l4H3rHpFeCyviL8mYLLXiHUrr+C2fNYYkxz5+8pPawGQkhXSf+ZsaifwZPHh9VHTs4yc/5WOjY+ekvhoKQ0I4PsJUyPNj/PwnhbFSA+TQ6PXyOXza1kIe4/nET7LNZIf83LtajgyJHNs2eGeEpEfwmimJ+s/jbRAes6ezEGoZUFBa5XfTlSXe2Jr1Hu/4r/TJtGaCZTPILs2u+Piv+bOZbAPGSMU+rb3j8fXo8qtqGUguKjZpn7Adb8m748O7aQ00GTEVa3VPsY+3/P1sVpucoFL9654iH2/JB8f0g+Pig4mr5VQUxdPbls/a153GLTeNZEUXXBrLLcd4pIvs5Cfydkn/OQUXvgZgst+mb0VVVn/L5d19c3xlRT/9y79ef13ffF2b55e3/2b8y7/e3F7/x8XZmmxO/tuSJh5T8Bqs3cQW3NdBxX52k7QQfegd8egqC7IEOceRRYpbo6T1iujJh970dddf42AEBdPswyT9w9lGTVes2ZK5VS5dB+F/vnnNNHbSJfPZdzaZ6zl0CXu5C7JlqdUhwfvrJikJrH3NPloSmy2pEyK5KumlWa83vfCKmI7vt/ntksceCnbp/qf6thp0bpHJkEp+lb//nxwbnWNsK5HmVr7NUs+A3Cc0jOqrXo+7dyff+ycnJ/4u3QycbLR0RMrNVoZ/mFGMBw1bky7FKNr0r97i+Hhntwzd5JLjkkTmZMfx4xOWs/OJy7HeZn4ia8G30PnAQqp9Jt/ymu45W8/aJTOUDd2ZO6ylVLBb/jW33D+L3RotCDlp7W+0jH2e8I9C3PGjm+6PmyEgXkjdUBVnu7GdeVbsoAgFDgrs52HbSS9HowC3ik5jhiS/GTSmBmpQ41CbFbLmRyPvpOmvb37885PvkU9RTOJh4I9PlsxjF5EIJm6ww4++rj8e/3B39GcmgHfyhTdPZi/90MlwjaKRUJYOi3O2QmlKN92KGBtL6u6EE2IREYrT55WN//6JXT1Nxw0GHKvMvmGfqPApl4H6PY/55F7UQFQjqyzt2DZzF3g7ZogkL7F1iszBj4oAE+LKVL7Db4JMPKNHb3KT3eCK8oGWvoG2i7AAWnxXKYr+hOlXfk+k0hhlWFYcLY5Wl1c3ny/PLte/mqv1V+J6fXN7fXNxu768WB29P/r73VEppMz7O/zgDo90rQeEVQ3t+1+s2CUncRLy+D35g3xA/sMtQOTir5z7z6HNFp7zF+/5P3bIvg/NxLk33y7fLk/44wX/BzkFcx010zUG2vnzf7A/sICjPDbhH4byP3BWMHTiU5Jgy//nfx394/8DO9vBgw===END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA