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
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4161A"' \
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
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4161A"' \
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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA