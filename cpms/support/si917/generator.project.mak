####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################


UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif

SDK_PATH = /home/sharath/Latest_stash/matter/third_party/silabs/gecko_sdk
sdk_support_root = /home/sharath/Latest_stash/matter/third_party/silabs/matter_support
chip_root = /home/sharath/Latest_stash/matter
wisemcu_sdk_root = /home/sharath/Latest_stash/matter/third_party/silabs/wisemcu-wifi-bt-sdk
examples_plat_dir = /home/sharath/Latest_stash/matter/examples/platform/silabs/SiWx917


# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DSIWX917' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4325B"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DHARD_FAULT_LOG_ENABLE' \
 '-DCORTEXM3_EFM32_MICRO' \
 '-DSILABS_LOG_ENABLED=1' \
 '-DNVM3_DEFAULT_NVM_SIZE=73728' \
 '-DNVM3_DEFAULT_MAX_OBJECT_SIZE=4092' \
 '-DMBEDTLS_CONFIG_FILE=<siwx917-chip-mbedtls-config.h>' \
 '-DEFR32_OPENTHREAD_API' \
 '-DPHY=EMBER_PHY_RAIL' \
 '-DCORTEXM3' \
 '-DMICRO=EMBER_MICRO_CORTEXM3_EFR32' \
 '-DPLAT=EMBER_PLATFORM_CORTEXM3' \
 '-DSL_COMPONENT_CATALOG_PRESENT' \
 '-DPLATFORM_HEADER=<platform-header.h>' \
 '-DUSE_NVM3=1' \
 '-DRSI_ARM_CM4F' \
 '-DRSI_BLE_ENABLE=1' \
 '-DRSI_BLE_ENABLE=1' \
 '-DTINYCRYPT_PRIMITIVES' \
 '-DOPTIMIZE_TINYCRYPT_ASM' \
 '-DRS91X_BLE_ENABLE=1' \
 '-D__error_t_defined' \
 '-D__STARTUP_CLEAR_BSS' \
 '-DMBEDTLS_PEM_WRITE_C' \
 #'-DMBEDTLS_USE_PSA_CRYPTO' \
 #'-D${silabs_board}=1' \
 #'-D${silabs_mcu}=1' \
 #'-DMBEDTLS_CONFIG_FILE=<siwx917-chip-mbedtls-config.h>' \

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4325B"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DHARD_FAULT_LOG_ENABLE' \
 '-DCORTEXM3_EFM32_MICRO' \
 '-DSILABS_LOG_ENABLED=1' \
 '-DNVM3_DEFAULT_NVM_SIZE=73728' \
 '-DNVM3_DEFAULT_MAX_OBJECT_SIZE=4092' \
 '-DMBEDTLS_CONFIG_FILE=<siwx917-chip-mbedtls-config.h>' \
 '-DEFR32_OPENTHREAD_API' \
 '-DPHY=EMBER_PHY_RAIL' \
 '-DCORTEXM3' \
 '-DMICRO=EMBER_MICRO_CORTEXM3_EFR32' \
 '-DPLAT=EMBER_PLATFORM_CORTEXM3' \
 '-DSL_COMPONENT_CATALOG_PRESENT' \
 '-DPLATFORM_HEADER=<platform-header.h>' \
 '-DUSE_NVM3=1' \
 '-DRSI_ARM_CM4F' \
 '-DRSI_BLE_ENABLE=1' \
 '-DRSI_BLE_ENABLE=1' \
 '-DTINYCRYPT_PRIMITIVES' \
 '-DOPTIMIZE_TINYCRYPT_ASM' \
 '-DRS91X_BLE_ENABLE=1' \
 '-D__error_t_defined' \
 '-D__STARTUP_CLEAR_BSS' \
 #'-D${silabs_board}=1' \
 #'-D${silabs_mcu}=1' \

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I../../generator \
 -I$(sdk_support_root)/platform/emdrv/nvm3/inc \
 -I$(sdk_support_root)/matter/mbedtls/tinycrypt/inc \
 -I$(chip_root)/third_party/mbedtls/repo/include \
 -I$(chip_root)/third_party/mbedtls/repo/library \
 -I$(chip_root)/examples/platform/silabs/SiWx917/SiWx917 \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/inc \
 -I$(wisemcu_sdk_root)/platforms/si91x/core/chip/inc \
 -I$(wisemcu_sdk_root)/platforms/si91x/core/config \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/config \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/rom_driver/inc \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/inc \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/CMSIS/Driver/Include \
 -I$(wisemcu_sdk_root)/platforms/si91x/boards/brd4325a/inc \
 -I$(wisemcu_sdk_root)/platforms/si91x/core/cmsis \
 -I$(wisemcu_sdk_root)/third_party/freertos/include \
 -I$(wisemcu_sdk_root)/third_party/freertos/portable/GCC/ARM_CM4F \
 -I$(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/inc \
 -I$(examples_plat_dir)/device/inc \
 -I$(wisemcu_sdk_root)/sapi/include \
 -I$(wisemcu_sdk_root)/platforms/si91x/boards/brd4325x/inc \
 -I$(chip_root)/examples/platform/silabs/SiWx917/SiWx917 \
 -I$(chip_root)/examples/platform/silabs/SiWx917/SiWx917/hal \
 -I$(chip_root)/src/platform/silabs/SiWx917 \
 -I$(SDK_PATH)/platform/common/toolchain/inc/ \
 -I$(SDK_PATH)/util/third_party/segger/systemview/Config/ \
 -I$(chip_root)/src/ \
 -I$(chip_root)/src/platform/silabs/ \
 -I$(SDK_PATH)/util/third_party/segger/systemview/SEGGER/ \
 -I$(chip_root)/examples/platform/silabs/SiWx917/ \
 -I$(sdk_support_root)/platform/emdrv/nvm3/lib/ \

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lstdc++ \
 -lgcc \
 -lc \
 -lm \
 -lnosys \
 $(sdk_support_root)/platform/emdrv/nvm3/lib/libnvm3_CM4_gcc.a

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -mcpu=cortex-m4 \
 -Wno-empty-body \
 -Wcpp \
 -Wno-error \
 -Wno-missing-braces \
 -Wno-sign-compare \
 -Wno-unused-variable \
 -Wno-unused-value \
 -Wno-unused-parameter \
 -Wno-implicit-function-declaration \
 -Wno-unused-function \
 -Wno-builtin-macro-redefined \
 -Wno-unused-macros \
 -Wno-implicit-int \
 -Wno-missing-field-initializers \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c99 \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -Wempty-body \
 -Wcpp \
 -Wmissing-braces \
 -Wsign-compare \
 -Wunused-variable \
 -Wunused-value \
 -Wunused-parameter \
 -Wimplicit-function-declaration \
 -Wunused-function \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c++11 \
 -fno-rtti \
 -fno-exceptions \
 -Wextra \
 -Wall \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 --specs=nano.specs \
 -g

ASM_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -imacros sl_gcc_preinclude.h \
 -x assembler-with-cpp

LD_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -T"autogen/linkerfile.ld" \
 --specs=nano.specs \
 -Xlinker -Map=$(OUTPUT_DIR)/$(PROJECTNAME).map \
 -Wl,--gc-sections


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
$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_dev.o: $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_dev.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_dev.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_dev.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_dev.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_dev.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_intf.o: $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_intf.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_intf.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_intf.c
CDEPS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_intf.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_intf.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_prg.o:  $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_prg.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_prg.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/ccp_flash_prg.c
CDEPS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_prg.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/ccp_flash_prg.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default.o:  $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_default.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_default.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_default.c
CDEPS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.o:  $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.c
CDEPS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash_ccp.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o:  $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_lock.c
		@echo 'Building $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_lock.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/platform/emdrv/nvm3/src/nvm3_lock.c
CDEPS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.d
OBJS +=$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o

$(OUTPUT_DIR)/sdk/platforms/si91x/boards/brd4325x/src/rsi_board.o:  $(wisemcu_sdk_root)/platforms/si91x/boards/brd4325x/src/rsi_board.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/boards/brd4325x/src/rsi_board.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/boards/brd4325x/src/rsi_board.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/boards/brd4325x/src/rsi_board.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/boards/brd4325x/src/rsi_board.o

$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.o:  $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_deepsleep_commonflash.o

$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_ps_ram_func.o:  $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_ps_ram_func.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_ps_ram_func.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_ps_ram_func.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_ps_ram_func.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_ps_ram_func.o

#$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_system_config.o:  $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_system_config.c
#		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_system_config.c'
#		@mkdir -p $(@D)
#		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/rsi_system_config.c
#CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_system_config.d
#OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/rsi_system_config.o

$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/system_RS1xxxx.o:  $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/system_RS1xxxx.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/system_RS1xxxx.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/core/chip/src/system_RS1xxxx.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/system_RS1xxxx.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/core/chip/src/system_RS1xxxx.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/clock_update.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/clock_update.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/clock_update.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/clock_update.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/clock_update.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/clock_update.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_comparator.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_egpio.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_udma_wrapper.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/peripheral_drivers/src/rsi_usart.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ipmu.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_pll.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_pll.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_pll.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_pll.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_pll.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_pll.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_power_save.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_power_save.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_power_save.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_power_save.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_power_save.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_power_save.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_processor_sensor.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_rtc.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_rtc.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_rtc.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_rtc.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_rtc.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_rtc.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_temp_sensor.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_time_period.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_time_period.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_time_period.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_time_period.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_time_period.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_time_period.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_ulpss_clk.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/systemlevel/src/rsi_wwdt.o

#$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gap_apis.o:  $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gap_apis.c
#		@echo 'Building $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gap_apis.c'
#		@mkdir -p $(@D)
#		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gap_apis.c
#CDEPS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gap_apis.d
#OBJS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gap_apis.o

#$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gatt_apis.o:  $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gatt_apis.c
#		@echo 'Building $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gatt_apis.c'
#		@mkdir -p $(@D)
#		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/sapi/bluetooth/rsi_ble_gatt_apis.c
#CDEPS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gatt_apis.d
#OBJS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_ble_gatt_apis.o

#$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_bt_common_apis.o:  $(wisemcu_sdk_root)/sapi/bluetooth/rsi_bt_common_apis.c
#		@echo 'Building $(wisemcu_sdk_root)/sapi/bluetooth/rsi_bt_common_apis.c'
#		@mkdir -p $(@D)
#		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/sapi/bluetooth/rsi_bt_common_apis.c
#CDEPS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_bt_common_apis.d
#OBJS +=$(OUTPUT_DIR)/sdk/sapi/bluetooth/rsi_bt_common_apis.o

#$(OUTPUT_DIR)/sdk/sapi/driver/rsi_bt_ble.o:  $(wisemcu_sdk_root)/sapi/driver/rsi_bt_ble.c
#		@echo 'Building $(wisemcu_sdk_root)/sapi/driver/rsi_bt_ble.c'
#		@mkdir -p $(@D)
#		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/sapi/driver/rsi_bt_ble.c
#CDEPS +=$(OUTPUT_DIR)/sdk/sapi/driver/rsi_bt_ble.d
#OBJS +=$(OUTPUT_DIR)/sdk/sapi/driver/rsi_bt_ble.o

$(OUTPUT_DIR)/sdk/third_party/freertos/croutine.o:  $(wisemcu_sdk_root)/third_party/freertos/croutine.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/croutine.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/croutine.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/croutine.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/croutine.o

$(OUTPUT_DIR)/sdk/third_party/freertos/event_groups.o:  $(wisemcu_sdk_root)/third_party/freertos/event_groups.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/event_groups.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/event_groups.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/event_groups.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/event_groups.o

$(OUTPUT_DIR)/sdk/third_party/freertos/list.o:  $(wisemcu_sdk_root)/third_party/freertos/list.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/list.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/list.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/list.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/list.o

$(OUTPUT_DIR)/sdk/third_party/freertos/portable/GCC/ARM_CM4F/port.o:  $(wisemcu_sdk_root)/third_party/freertos/portable/GCC/ARM_CM4F/port.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/portable/GCC/ARM_CM4F/port.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/portable/GCC/ARM_CM4F/port.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/portable/GCC/ARM_CM4F/port.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/portable/GCC/ARM_CM4F/port.o

$(OUTPUT_DIR)/sdk/third_party/freertos/queue.o:  $(wisemcu_sdk_root)/third_party/freertos/queue.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/queue.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/queue.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/queue.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/queue.o

$(OUTPUT_DIR)/sdk/third_party/freertos/stream_buffer.o:  $(wisemcu_sdk_root)/third_party/freertos/stream_buffer.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/stream_buffer.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/stream_buffer.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/stream_buffer.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/stream_buffer.o

$(OUTPUT_DIR)/sdk/third_party/freertos/tasks.o:  $(wisemcu_sdk_root)/third_party/freertos/tasks.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/tasks.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/tasks.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/tasks.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/tasks.o

$(OUTPUT_DIR)/sdk/third_party/freertos/timers.o:  $(wisemcu_sdk_root)/third_party/freertos/timers.c
		@echo 'Building $(wisemcu_sdk_root)/third_party/freertos/timers.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/third_party/freertos/timers.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/freertos/timers.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/freertos/timers.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/UDMA.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/UDMA.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/UDMA.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/UDMA.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/UDMA.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/UDMA.o

$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/USART.o:  $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/USART.c
		@echo 'Building $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/USART.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(wisemcu_sdk_root)/platforms/si91x/drivers/cmsis_driver/USART.c
CDEPS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/USART.d
OBJS +=$(OUTPUT_DIR)/sdk/platforms/si91x/drivers/cmsis_driver/USART.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/aes.o:  $(chip_root)/third_party/mbedtls/repo/library/aes.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/aes.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/aes.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/aes.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/aes.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1parse.o:  $(chip_root)/third_party/mbedtls/repo/library/asn1parse.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/asn1parse.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/asn1parse.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1parse.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1parse.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1write.o:  $(chip_root)/third_party/mbedtls/repo/library/asn1write.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/asn1write.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/asn1write.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1write.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/asn1write.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/bignum.o:  $(chip_root)/third_party/mbedtls/repo/library/bignum.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/bignum.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/bignum.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/bignum.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/bignum.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ccm.o:  $(chip_root)/third_party/mbedtls/repo/library/ccm.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ccm.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ccm.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ccm.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ccm.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher.o:  $(chip_root)/third_party/mbedtls/repo/library/cipher.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/cipher.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/cipher.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher_wrap.o:  $(chip_root)/third_party/mbedtls/repo/library/cipher_wrap.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/cipher_wrap.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/cipher_wrap.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher_wrap.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/cipher_wrap.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/constant_time.o:  $(chip_root)/third_party/mbedtls/repo/library/constant_time.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/constant_time.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/constant_time.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/constant_time.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/constant_time.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ctr_drbg.o:  $(chip_root)/third_party/mbedtls/repo/library/ctr_drbg.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ctr_drbg.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ctr_drbg.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ctr_drbg.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ctr_drbg.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdh.o:  $(chip_root)/third_party/mbedtls/repo/library/ecdh.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ecdh.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ecdh.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdh.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdh.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdsa.o:  $(chip_root)/third_party/mbedtls/repo/library/ecdsa.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ecdsa.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ecdsa.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdsa.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecdsa.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp.o:  $(chip_root)/third_party/mbedtls/repo/library/ecp.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ecp.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ecp.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp_curves.o:  $(chip_root)/third_party/mbedtls/repo/library/ecp_curves.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/ecp_curves.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/ecp_curves.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp_curves.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/ecp_curves.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/entropy.o:  $(chip_root)/third_party/mbedtls/repo/library/entropy.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/entropy.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/entropy.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/entropy.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/entropy.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hkdf.o:  $(chip_root)/third_party/mbedtls/repo/library/hkdf.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/hkdf.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/hkdf.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hkdf.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hkdf.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hmac_drbg.o:  $(chip_root)/third_party/mbedtls/repo/library/hmac_drbg.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/hmac_drbg.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/hmac_drbg.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hmac_drbg.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/hmac_drbg.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/md.o:  $(chip_root)/third_party/mbedtls/repo/library/md.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/md.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/md.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/md.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/md.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pkcs5.o:  $(chip_root)/third_party/mbedtls/repo/library/pkcs5.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/pkcs5.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/pkcs5.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pkcs5.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pkcs5.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/platform.o:  $(chip_root)/third_party/mbedtls/repo/library/platform.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/platform.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/platform.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/platform.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/platform.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha256.o:  $(chip_root)/third_party/mbedtls/repo/library/sha256.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/sha256.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/sha256.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha256.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha256.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha512.o:  $(chip_root)/third_party/mbedtls/repo/library/sha512.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/sha512.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/sha512.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha512.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/sha512.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pem.o:  $(chip_root)/third_party/mbedtls/repo/library/pem.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/pem.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/pem.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pem.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/pem.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509_create.o:  $(chip_root)/third_party/mbedtls/repo/library/x509_create.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/x509_create.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/x509_create.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509_create.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509_create.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/base64.o:  $(chip_root)/third_party/mbedtls/repo/library/base64.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/base64.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/base64.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/base64.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/base64.o

$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509.o:  $(chip_root)/third_party/mbedtls/repo/library/x509.c
		@echo 'Building $(chip_root)/third_party/mbedtls/repo/library/x509.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(chip_root)/third_party/mbedtls/repo/library/x509.c
CDEPS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509.d
OBJS +=$(OUTPUT_DIR)/sdk/third_party/mbedtls/repo/library/x509.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dh.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dh.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dh.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dh.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dh.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dh.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dsa.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dsa.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dsa.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/ecc_dsa.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dsa.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/ecc_dsa.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/error.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/error.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/error.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/error.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/error.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/error.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/oid.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/oid.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/oid.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/oid.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/oid.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/oid.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk_wrap.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk_wrap.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk_wrap.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pk_wrap.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk_wrap.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pk_wrap.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkparse.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkparse.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkparse.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkparse.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkparse.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkparse.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkwrite.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkwrite.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkwrite.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/pkwrite.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkwrite.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/pkwrite.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/platform_util.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/platform_util.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/platform_util.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/platform_util.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/platform_util.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/platform_util.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509_crt.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509_crt.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509_crt.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509_crt.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509_crt.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509_crt.o

$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509write_csr.o:  $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509write_csr.c
		@echo 'Building $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509write_csr.c'
		@mkdir -p $(@D)
		$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(sdk_support_root)/matter/mbedtls/tinycrypt/src/x509write_csr.c
CDEPS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509write_csr.d
OBJS +=$(OUTPUT_DIR)/sdk/matter/mbedtls/tinycrypt/src/x509write_csr.o

$(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o: $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o

$(OUTPUT_DIR)/project/_/_/generator/app.o: ../../generator/app.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/app.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/app.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/app.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/app.o

$(OUTPUT_DIR)/project/_/_/generator/commands.o: ../../generator/commands.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/commands.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/commands.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/commands.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/commands.o

$(OUTPUT_DIR)/project/_/_/generator/config.o: ../../generator/config.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/config.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/config.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/config.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/config.o

$(OUTPUT_DIR)/project/_/_/generator/credentials.o: ../../generator/credentials.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/credentials.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/credentials.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/credentials.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/credentials.o

$(OUTPUT_DIR)/project/_/_/generator/encoding.o: ../../generator/encoding.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/encoding.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/encoding.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/encoding.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/encoding.o

$(OUTPUT_DIR)/project/_/_/generator/main.o: ../../generator/main.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/main.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/main.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/main.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/main.o


# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA
