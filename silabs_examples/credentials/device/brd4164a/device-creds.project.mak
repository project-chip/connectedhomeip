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
COPIED_SDK_PATH = ../../common/gsdk

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DMATTER_COMMON_NAME="Matter Dev DAC 0xFFF1/0x8005"' \
 '-DMATTER_PRODUCT_ID=0x8005' \
 '-DMATTER_VENDOR_ID=0xFFF1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DPSA_CRYPTO_PRINT=1' \
 '-DPSA_CRYPTO_PRINT_CERT=1' \
 '-DRESERVED_AVAILABLE_MEMORY=1000' \
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4164A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DSEGGER_RTT_ALIGNMENT=1024' \
 '-DEMBER_AF_API_BOOTLOADER_INTERFACE=1' \
 '-DCORTEXM3=1' \
 '-DCORTEXM3_EFM32_MICRO=1' \
 '-DCORTEXM3_EFR32=1' \
 '-DPHY_RAIL=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DSL_LEGACY_HAL_ENABLE_WATCHDOG=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DSL_RAIL_LIB_MULTIPROTOCOL_SUPPORT=0' \
 '-DSL_RAIL_UTIL_PA_CONFIG_HEADER=<sl_rail_util_pa_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"' \
 '-DCUSTOM_TOKEN_HEADER="sl_token_manager_af_token_header.h"' \
 '-DUSE_NVM3=1' \
 '-DUC_BUILD=1' \
 '-DEMBER_AF_NCP=1' \
 '-DEMBER_SERIAL1_RTSCTS=1' \
 '-DEZSP_UART=1' \
 '-DNO_USB=1' \
 '-DEMBER_MULTI_NETWORK_STRIPPED=1' \
 '-DSL_ZIGBEE_PHY_SELECT_STACK_SUPPORT=1'

ASM_DEFS += \
 '-DMATTER_COMMON_NAME="Matter Dev DAC 0xFFF1/0x8005"' \
 '-DMATTER_PRODUCT_ID=0x8005' \
 '-DMATTER_VENDOR_ID=0xFFF1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DPSA_CRYPTO_PRINT=1' \
 '-DPSA_CRYPTO_PRINT_CERT=1' \
 '-DRESERVED_AVAILABLE_MEMORY=1000' \
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4164A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DSEGGER_RTT_ALIGNMENT=1024' \
 '-DEMBER_AF_API_BOOTLOADER_INTERFACE=1' \
 '-DCORTEXM3=1' \
 '-DCORTEXM3_EFM32_MICRO=1' \
 '-DCORTEXM3_EFR32=1' \
 '-DPHY_RAIL=1' \
 '-DPLATFORM_HEADER="platform-header.h"' \
 '-DSL_LEGACY_HAL_ENABLE_WATCHDOG=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DSL_RAIL_LIB_MULTIPROTOCOL_SUPPORT=0' \
 '-DSL_RAIL_UTIL_PA_CONFIG_HEADER=<sl_rail_util_pa_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"' \
 '-DCUSTOM_TOKEN_HEADER="sl_token_manager_af_token_header.h"' \
 '-DUSE_NVM3=1' \
 '-DUC_BUILD=1' \
 '-DEMBER_AF_NCP=1' \
 '-DEMBER_SERIAL1_RTSCTS=1' \
 '-DEZSP_UART=1' \
 '-DNO_USB=1' \
 '-DEMBER_MULTI_NETWORK_STRIPPED=1' \
 '-DSL_ZIGBEE_PHY_SELECT_STACK_SUPPORT=1'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I../../common \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Include \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/hardware/board/inc \
 -I$(SDK_PATH)/platform/bootloader \
 -I$(SDK_PATH)/platform/bootloader/api \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/hardware/driver/configuration_over_swo/inc \
 -I$(SDK_PATH)/platform/driver/debug/inc \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/driver/leddrv/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal/inc \
 -I$(SDK_PATH)/platform/service/legacy_common_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_hal_wdog/inc \
 -I$(SDK_PATH)/platform/service/legacy_ncp_ash/inc \
 -I$(SDK_PATH)/platform/service/legacy_printf/inc \
 -I$(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/library \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/hardware/driver/mx25_flash_shutdown/inc/sl_mx25_flash_shutdown_usart \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/service/power_manager/inc \
 -I$(SDK_PATH)/util/third_party/printf \
 -I$(SDK_PATH)/util/third_party/printf/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc/public \
 -I$(SDK_PATH)/platform/radio/rail_lib/common \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/ble \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/ieee802154 \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/zwave \
 -I$(SDK_PATH)/platform/radio/rail_lib/chip/efr32/efr32xg1x \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_ieee802154 \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/efr32xg1x \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti \
 -I$(SDK_PATH)/util/third_party/segger/systemview/SEGGER \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/common/toolchain/inc \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(SDK_PATH)/platform/service/sleeptimer/inc \
 -I$(SDK_PATH)/platform/service/token_manager/inc \
 -I$(SDK_PATH)/platform/service/udelay/inc \
 -I$(SDK_PATH)/protocol/zigbee/stack/platform/micro \
 -I$(SDK_PATH)/protocol/zigbee/stack/include \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/common \
 -I$(SDK_PATH)/protocol/zigbee/app/em260 \
 -I$(SDK_PATH)/protocol/zigbee/stack/framework \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print \
 -I$(SDK_PATH)/protocol/zigbee/stack/gp \
 -I$(SDK_PATH)/protocol/zigbee/app/xncp \
 -I$(SDK_PATH)/protocol/zigbee/app/util/ezsp \
 -I$(SDK_PATH)/protocol/zigbee/app/framework/util \
 -I$(SDK_PATH)/protocol/zigbee/app/util/security \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin \
 -I$(SDK_PATH)/protocol/zigbee \
 -I$(SDK_PATH)/protocol/zigbee/stack \
 -I$(SDK_PATH)/protocol/zigbee/stack/zigbee \
 -I$(SDK_PATH)/platform/radio/mac/rail_mux \
 -I$(SDK_PATH)/platform/radio/mac \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core \
 -I$(SDK_PATH)/protocol/zigbee/stack/core \
 -I$(SDK_PATH)/protocol/zigbee/stack/mac \
 -I$(SDK_PATH)/protocol/zigbee/app/ncp/plugin/xncp

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lgcc \
 -lc \
 -lm \
 -lnosys \
 $(SDK_PATH)/platform/emdrv/nvm3/lib/libnvm3_CM4_gcc.a \
 $(SDK_PATH)/platform/radio/rail_lib/autogen/librail_release/librail_efr32xg12_gcc_release.a \
 $(SDK_PATH)/protocol/zigbee/build/binding-table-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/binding-table-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-binding-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/ncp-binding-library.a \
 $(SDK_PATH)/protocol/zigbee/build/debug-basic-library-cortexm3-gcc-efr32mg12p-rail/debug-basic-library.a \
 $(SDK_PATH)/protocol/zigbee/build/debug-extended-library-cortexm3-gcc-efr32mg12p-rail/debug-extended-library.a \
 $(SDK_PATH)/protocol/zigbee/build/end-device-bind-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/end-device-bind-library.a \
 $(SDK_PATH)/protocol/zigbee/build/gp-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-gp-library-cortexm3-gcc-efr32mg12p-rail/ncp-gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-uart-util-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/em260-uart-util-library.a \
 $(SDK_PATH)/protocol/zigbee/build/packet-validate-library-cortexm3-gcc-efr32mg12p-rail/packet-validate-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-pro-stack-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/zigbee-pro-stack.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-pro-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/ncp-pro-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-r22-support-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/zigbee-r22-support-library.a \
 $(SDK_PATH)/protocol/zigbee/build/security-library-link-keys-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/security-library-link-keys.a \
 $(SDK_PATH)/protocol/zigbee/build/source-route-library-cortexm3-gcc-efr32mg12p-rail-ember_multi_network_stripped/source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-source-route-library-cortexm3-gcc-efr32mg12p-rail/ncp-source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-xncp-library-cortexm3-gcc-efr32mg12p-rail/em260-xncp-library.a

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
 -imacros sl_gcc_preinclude.h \
 -fno-builtin-printf \
 -fno-builtin-sprintf \
 --specs=nano.specs \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
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
 -imacros sl_gcc_preinclude.h \
 -fno-builtin-printf \
 -fno-builtin-sprintf \
 --specs=nano.specs \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
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
 -T"autogen/linkerfile_base.ld" \
 --specs=nano.specs \
 -Xlinker -Map=$(OUTPUT_DIR)/$(PROJECTNAME).map \
 -Wl,--gc-sections


####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_control_gpio.o: $(SDK_PATH)/hardware/board/src/sl_board_control_gpio.c
	@echo 'Building $(SDK_PATH)/hardware/board/src/sl_board_control_gpio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/board/src/sl_board_control_gpio.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_control_gpio.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_control_gpio.o

$(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_init.o: $(SDK_PATH)/hardware/board/src/sl_board_init.c
	@echo 'Building $(SDK_PATH)/hardware/board/src/sl_board_init.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/board/src/sl_board_init.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_init.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/board/src/sl_board_init.o

$(OUTPUT_DIR)/sdk/hardware/driver/configuration_over_swo/src/sl_cos.o: $(SDK_PATH)/hardware/driver/configuration_over_swo/src/sl_cos.c
	@echo 'Building $(SDK_PATH)/hardware/driver/configuration_over_swo/src/sl_cos.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/driver/configuration_over_swo/src/sl_cos.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/driver/configuration_over_swo/src/sl_cos.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/driver/configuration_over_swo/src/sl_cos.o

$(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.o: $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.c
	@echo 'Building $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_usart/sl_mx25_flash_shutdown.o

$(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface.o: $(SDK_PATH)/platform/bootloader/api/btl_interface.c
	@echo 'Building $(SDK_PATH)/platform/bootloader/api/btl_interface.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/api/btl_interface.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface.o

$(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface_storage.o: $(SDK_PATH)/platform/bootloader/api/btl_interface_storage.c
	@echo 'Building $(SDK_PATH)/platform/bootloader/api/btl_interface_storage.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/api/btl_interface_storage.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface_storage.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/api/btl_interface_storage.o

$(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o: $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
	@echo 'Building $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o

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

$(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o: $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
	@echo 'Building $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o

$(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.o: $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c
	@echo 'Building $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.d
OBJS += $(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.o

$(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_led.o: $(SDK_PATH)/platform/driver/leddrv/src/sl_led.c
	@echo 'Building $(SDK_PATH)/platform/driver/leddrv/src/sl_led.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/driver/leddrv/src/sl_led.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_led.d
OBJS += $(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_led.o

$(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_simple_led.o: $(SDK_PATH)/platform/driver/leddrv/src/sl_simple_led.c
	@echo 'Building $(SDK_PATH)/platform/driver/leddrv/src/sl_simple_led.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/driver/leddrv/src/sl_simple_led.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_simple_led.d
OBJS += $(OUTPUT_DIR)/sdk/platform/driver/leddrv/src/sl_simple_led.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
	@echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c
	@echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
	@echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.o: $(SDK_PATH)/platform/emlib/src/em_cmu.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_cmu.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_cmu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o: $(SDK_PATH)/platform/emlib/src/em_core.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_core.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_core.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o: $(SDK_PATH)/platform/emlib/src/em_crypto.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_crypto.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o: $(SDK_PATH)/platform/emlib/src/em_emu.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_emu.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_emu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.o: $(SDK_PATH)/platform/emlib/src/em_gpio.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_gpio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_gpio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.o: $(SDK_PATH)/platform/emlib/src/em_msc.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_msc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_msc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.o: $(SDK_PATH)/platform/emlib/src/em_prs.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_prs.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_prs.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_rmu.o: $(SDK_PATH)/platform/emlib/src/em_rmu.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_rmu.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_rmu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rmu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rmu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.o: $(SDK_PATH)/platform/emlib/src/em_rtcc.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_rtcc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_rtcc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o: $(SDK_PATH)/platform/emlib/src/em_system.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_system.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_system.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_usart.o: $(SDK_PATH)/platform/emlib/src/em_usart.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_usart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_usart.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_usart.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_usart.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_wdog.o: $(SDK_PATH)/platform/emlib/src/em_wdog.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_wdog.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_wdog.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_wdog.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_wdog.o

$(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.o: $(SDK_PATH)/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.c
	@echo 'Building $(SDK_PATH)/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.d
OBJS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/coexistence/protocol/ieee802154_uc/coexistence-802154.o

$(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.o: $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.c
	@echo 'Building $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.d
OBJS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.o

$(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.o: $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.c
	@echo 'Building $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.d
OBJS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.o

$(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.o: $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.c
	@echo 'Building $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.d
OBJS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_ant_div/sl_rail_util_ant_div.o

$(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.o: $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.c
	@echo 'Building $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.d
OBJS += $(OUTPUT_DIR)/sdk/platform/radio/rail_lib/plugin/rail_util_pti/sl_rail_util_pti.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_debug.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_debug.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_debug.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_debug.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_debug.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_debug.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_swo_itm_8.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_swo_itm_8.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_swo_itm_8.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_swo_itm_8.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_swo_itm_8.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_swo_itm_8.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_uart.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_uart.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_uart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_uart.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_uart.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_uart.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_usart.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_usart.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_usart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_usart.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_usart.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_usart.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_vuart.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_vuart.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_vuart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_vuart.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_vuart.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_vuart.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o: $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_common_ash/src/ash-common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_common_ash/src/ash-common.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/base-replacement.o: $(SDK_PATH)/platform/service/legacy_hal/src/base-replacement.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/base-replacement.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/base-replacement.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/base-replacement.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/base-replacement.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-common.o: $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-common.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-common.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-app.o: $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-app.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-app.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-app.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-app.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-app.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-standalone.o: $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-standalone.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-standalone.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface-standalone.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-standalone.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface-standalone.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface.o: $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/bootloader-interface.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/bootloader-interface.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o: $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/crc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/crc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/crc.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/diagnostic.o: $(SDK_PATH)/platform/service/legacy_hal/src/diagnostic.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/diagnostic.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/diagnostic.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/diagnostic.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/diagnostic.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-phy.o: $(SDK_PATH)/platform/service/legacy_hal/src/ember-phy.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/ember-phy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/ember-phy.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-phy.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/ember-phy.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/faults.o: $(SDK_PATH)/platform/service/legacy_hal/src/faults.s
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/faults.s'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/faults.s
ASMDEPS_s += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/faults.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/faults.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o: $(SDK_PATH)/platform/service/legacy_hal/src/random.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/random.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/random.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/random.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/token_legacy.o: $(SDK_PATH)/platform/service/legacy_hal/src/token_legacy.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal/src/token_legacy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal/src/token_legacy.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/token_legacy.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal/src/token_legacy.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.o: $(SDK_PATH)/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_hal_wdog/src/sl_legacy_hal_wdog.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.o: $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_ncp_ash/src/ash-ncp.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_ncp_ash/src/ash-ncp.o

$(OUTPUT_DIR)/sdk/platform/service/legacy_printf/src/sl_legacy_printf.o: $(SDK_PATH)/platform/service/legacy_printf/src/sl_legacy_printf.c
	@echo 'Building $(SDK_PATH)/platform/service/legacy_printf/src/sl_legacy_printf.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/legacy_printf/src/sl_legacy_printf.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/legacy_printf/src/sl_legacy_printf.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/legacy_printf/src/sl_legacy_printf.o

$(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager.o: $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager.c
	@echo 'Building $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager.o

$(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_debug.o: $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_debug.c
	@echo 'Building $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_debug.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_debug.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_debug.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_debug.o

$(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.o: $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.c
	@echo 'Building $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s0_s1.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
	@echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c
	@echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o

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

$(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manager.o: $(SDK_PATH)/platform/service/token_manager/src/sl_token_manager.c
	@echo 'Building $(SDK_PATH)/platform/service/token_manager/src/sl_token_manager.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/token_manager/src/sl_token_manager.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manager.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manager.o

$(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manufacturing.o: $(SDK_PATH)/platform/service/token_manager/src/sl_token_manufacturing.c
	@echo 'Building $(SDK_PATH)/platform/service/token_manager/src/sl_token_manufacturing.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/token_manager/src/sl_token_manufacturing.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manufacturing.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/token_manager/src/sl_token_manufacturing.o

$(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay.o: $(SDK_PATH)/platform/service/udelay/src/sl_udelay.c
	@echo 'Building $(SDK_PATH)/platform/service/udelay/src/sl_udelay.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/udelay/src/sl_udelay.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay.o

$(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay_armv6m_gcc.o: $(SDK_PATH)/platform/service/udelay/src/sl_udelay_armv6m_gcc.S
	@echo 'Building $(SDK_PATH)/platform/service/udelay/src/sl_udelay_armv6m_gcc.S'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c -o $@ $(SDK_PATH)/platform/service/udelay/src/sl_udelay_armv6m_gcc.S
ASMDEPS_S += $(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay_armv6m_gcc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/udelay/src/sl_udelay_armv6m_gcc.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-binding-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-binding-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-green-power-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-green-power-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-messaging-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-messaging-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-networking-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-networking-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-security-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-security-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.o: $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/command-handlers-trust-center-generated.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/command-handlers-trust-center-generated.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.o: $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/em260-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/em260-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.o: $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/ncp-stack-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/ncp-stack-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.o: $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/em260/serial-interface-uart.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/em260/serial-interface-uart.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_sleep.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_app_framework_stack_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.o: $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/common/zigbee_ncp_framework_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o: $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/framework/plugin/debug-print/sl_zigbee_debug_print.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o: $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/ezsp/secure-ezsp-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o: $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/util/security/security-address-cache.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/util/security/security-address-cache.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.o: $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/app/xncp/xncp_cb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/app/xncp/xncp_cb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.o: $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/config/ember-configuration.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/config/ember-configuration.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.o: $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/ember-multi-network-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/ember-multi-network-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.o: $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-common.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-common.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.o: $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/core/multi-pan-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/core/multi-pan-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.o: $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/aes-ecb.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/aes-ecb.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.o: $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/ccm-star.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/ccm-star.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.o: $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/strong-random-api.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/strong-random-api.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.o: $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/framework/zigbee-event-logger-stub-gen.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-163k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-283k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-sign-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-283k1-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-dsa-verify-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/cbke-crypto-engine-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/cbke-crypto-engine-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/install-code-stub.o: $(SDK_PATH)/protocol/zigbee/stack/security/install-code-stub.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/security/install-code-stub.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/security/install-code-stub.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/install-code-stub.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/security/install-code-stub.o

$(OUTPUT_DIR)/sdk/protocol/zigbee/stack/zll/zll-stubs.o: $(SDK_PATH)/protocol/zigbee/stack/zll/zll-stubs.c
	@echo 'Building $(SDK_PATH)/protocol/zigbee/stack/zll/zll-stubs.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/protocol/zigbee/stack/zll/zll-stubs.c
CDEPS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/zll/zll-stubs.d
OBJS += $(OUTPUT_DIR)/sdk/protocol/zigbee/stack/zll/zll-stubs.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/aes.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/aes.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1parse.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1parse.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1parse.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1parse.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1parse.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1parse.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1write.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1write.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1write.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/asn1write.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1write.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/asn1write.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/base64.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/base64.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/base64.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/base64.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/base64.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/base64.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/bignum.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/bignum.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/bignum.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/bignum.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/bignum.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/bignum.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher_wrap.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher_wrap.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher_wrap.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/cipher_wrap.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher_wrap.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/cipher_wrap.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/constant_time.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/constant_time.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ctr_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ctr_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/des.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/des.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/des.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/des.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/des.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/des.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdh.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdh.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdh.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdh.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdh.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdh.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdsa.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdsa.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdsa.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecdsa.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdsa.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecdsa.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp_curves.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp_curves.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp_curves.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/ecp_curves.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp_curves.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/ecp_curves.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/entropy_poll.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/entropy_poll.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/hmac_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/hmac_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/md.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/md.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/md.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/md.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/md.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/md.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/oid.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/oid.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/oid.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/oid.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/oid.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/oid.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pem.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pem.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pem.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pem.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pem.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pem.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk_wrap.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk_wrap.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk_wrap.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pk_wrap.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk_wrap.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pk_wrap.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkparse.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkparse.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkparse.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkparse.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkparse.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkparse.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkwrite.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkwrite.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkwrite.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/pkwrite.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkwrite.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/pkwrite.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/platform_util.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/platform_util.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_aead.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_aead.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_aead.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_client.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_client.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_client.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_client.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_client.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_client.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_driver_wrappers.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_ecp.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_hash.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_hash.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_hash.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_hash.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_hash.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_mac.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_mac.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_mac.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_rsa.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_se.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_se.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_se.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_se.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_se.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_se.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_slot_management.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_storage.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_storage.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_storage.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/psa_crypto_storage.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_storage.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/psa_crypto_storage.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha1.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha1.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha1.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha1.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha1.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha1.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha256.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha256.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha512.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha512.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha512.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/sha512.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha512.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/sha512.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/threading.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/threading.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509.o

$(OUTPUT_DIR)/project/_/_/common/gsdk/util/third_party/crypto/mbedtls/library/x509_create.o: $(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_create.c
	@echo 'Building $(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_create.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_create.c
CDEPS += $(OUTPUT_DIR)/project/_/_/common/gsdk/util/third_party/crypto/mbedtls/library/x509_create.d
OBJS += $(OUTPUT_DIR)/project/_/_/common/gsdk/util/third_party/crypto/mbedtls/library/x509_create.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crl.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crl.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crl.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crl.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crl.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crl.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crt.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crt.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crt.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_crt.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crt.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_crt.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_csr.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_csr.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_csr.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509_csr.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_csr.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509_csr.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_crt.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_crt.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_crt.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_crt.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_crt.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_crt.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_csr.o: $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_csr.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_csr.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/mbedtls/library/x509write_csr.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_csr.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/mbedtls/library/x509write_csr.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_aes.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/crypto_ecp.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/crypto_management.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_trng.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o

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

$(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o: $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
	@echo 'Building $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o

$(OUTPUT_DIR)/project/_/_/common/creds.o: ../../common/creds.c
	@echo 'Building ../../common/creds.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../common/creds.c
CDEPS += $(OUTPUT_DIR)/project/_/_/common/creds.d
OBJS += $(OUTPUT_DIR)/project/_/_/common/creds.o

$(OUTPUT_DIR)/project/_/device_cert.o: ../device_cert.c
	@echo 'Building ../device_cert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../device_cert.c
CDEPS += $(OUTPUT_DIR)/project/_/device_cert.d
OBJS += $(OUTPUT_DIR)/project/_/device_cert.o

$(OUTPUT_DIR)/project/_/device_creds.o: ../device_creds.c
	@echo 'Building ../device_creds.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../device_creds.c
CDEPS += $(OUTPUT_DIR)/project/_/device_creds.d
OBJS += $(OUTPUT_DIR)/project/_/device_creds.o

$(OUTPUT_DIR)/project/_/main.o: ../main.c
	@echo 'Building ../main.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../main.c
CDEPS += $(OUTPUT_DIR)/project/_/main.d
OBJS += $(OUTPUT_DIR)/project/_/main.o

$(OUTPUT_DIR)/project/autogen/sl_board_default_init.o: autogen/sl_board_default_init.c
	@echo 'Building autogen/sl_board_default_init.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_board_default_init.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_board_default_init.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_board_default_init.o

$(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o: autogen/sl_device_init_clocks.c
	@echo 'Building autogen/sl_device_init_clocks.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_device_init_clocks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o

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

$(OUTPUT_DIR)/project/autogen/sl_iostream_init_usart_instances.o: autogen/sl_iostream_init_usart_instances.c
	@echo 'Building autogen/sl_iostream_init_usart_instances.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_init_usart_instances.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_init_usart_instances.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_init_usart_instances.o

$(OUTPUT_DIR)/project/autogen/sl_power_manager_handler.o: autogen/sl_power_manager_handler.c
	@echo 'Building autogen/sl_power_manager_handler.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_power_manager_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_power_manager_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_power_manager_handler.o

$(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_phy_select.o: autogen/sl_rail_util_ieee802154_phy_select.c
	@echo 'Building autogen/sl_rail_util_ieee802154_phy_select.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_rail_util_ieee802154_phy_select.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_phy_select.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_phy_select.o

$(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_stack_event.o: autogen/sl_rail_util_ieee802154_stack_event.c
	@echo 'Building autogen/sl_rail_util_ieee802154_stack_event.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_rail_util_ieee802154_stack_event.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_stack_event.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_rail_util_ieee802154_stack_event.o

$(OUTPUT_DIR)/project/autogen/sl_simple_led_instances.o: autogen/sl_simple_led_instances.c
	@echo 'Building autogen/sl_simple_led_instances.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_simple_led_instances.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_simple_led_instances.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_simple_led_instances.o

$(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o: autogen/zigbee_common_callback_dispatcher.c
	@echo 'Building autogen/zigbee_common_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_common_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_common_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.o: autogen/zigbee_ncp_callback_dispatcher.c
	@echo 'Building autogen/zigbee_ncp_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_ncp_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_ncp_callback_dispatcher.o

$(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o: autogen/zigbee_stack_callback_dispatcher.c
	@echo 'Building autogen/zigbee_stack_callback_dispatcher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/zigbee_stack_callback_dispatcher.c
CDEPS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.d
OBJS += $(OUTPUT_DIR)/project/autogen/zigbee_stack_callback_dispatcher.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA