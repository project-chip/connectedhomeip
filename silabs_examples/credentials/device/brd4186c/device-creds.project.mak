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
 '-DEFR32MG24B210F1536IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4186C"' \
 '-DSL_BOARD_REV="A01"' \
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
 '-DEFR32MG24B210F1536IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4186C"' \
 '-DSL_BOARD_REV="A01"' \
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
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
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
 -I$(SDK_PATH)/platform/service/hfxo_manager/inc \
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
 -I$(SDK_PATH)/hardware/driver/mx25_flash_shutdown/inc/sl_mx25_flash_shutdown_eusart \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/peripheral/inc \
 -I$(SDK_PATH)/platform/service/power_manager/inc \
 -I$(SDK_PATH)/util/third_party/printf \
 -I$(SDK_PATH)/util/third_party/printf/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc/public \
 -I$(SDK_PATH)/platform/radio/rail_lib/common \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/ble \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/ieee802154 \
 -I$(SDK_PATH)/platform/radio/rail_lib/protocol/zwave \
 -I$(SDK_PATH)/platform/radio/rail_lib/chip/efr32/efr32xg2x \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_ieee802154 \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/efr32xg24 \
 -I$(SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src \
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
 $(SDK_PATH)/platform/emdrv/nvm3/lib/libnvm3_CM33_gcc.a \
 $(SDK_PATH)/platform/radio/rail_lib/autogen/librail_release/librail_efr32xg24_gcc_release.a \
 $(SDK_PATH)/protocol/zigbee/build/binding-table-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/binding-table-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-binding-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/ncp-binding-library.a \
 $(SDK_PATH)/protocol/zigbee/build/debug-basic-library-cortexm3-gcc-efr32mg24-rail/debug-basic-library.a \
 $(SDK_PATH)/protocol/zigbee/build/debug-extended-library-cortexm3-gcc-efr32mg24-rail/debug-extended-library.a \
 $(SDK_PATH)/protocol/zigbee/build/end-device-bind-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/end-device-bind-library.a \
 $(SDK_PATH)/protocol/zigbee/build/gp-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-gp-library-cortexm3-gcc-efr32mg24-rail/ncp-gp-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-uart-util-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/em260-uart-util-library.a \
 $(SDK_PATH)/protocol/zigbee/build/packet-validate-library-cortexm3-gcc-efr32mg24-rail/packet-validate-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-pro-stack-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/zigbee-pro-stack.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-pro-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/ncp-pro-library.a \
 $(SDK_PATH)/protocol/zigbee/build/zigbee-r22-support-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/zigbee-r22-support-library.a \
 $(SDK_PATH)/protocol/zigbee/build/security-library-link-keys-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/security-library-link-keys.a \
 $(SDK_PATH)/protocol/zigbee/build/source-route-library-cortexm3-gcc-efr32mg24-rail-ember_multi_network_stripped/source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/ncp-source-route-library-cortexm3-gcc-efr32mg24-rail/ncp-source-route-library.a \
 $(SDK_PATH)/protocol/zigbee/build/em260-xncp-library-cortexm3-gcc-efr32mg24-rail/em260-xncp-library.a

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 -mcmse \
 -fno-builtin-printf \
 -fno-builtin-sprintf \
 --specs=nano.specs \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
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
 -mcmse \
 -fno-builtin-printf \
 -fno-builtin-sprintf \
 --specs=nano.specs \
 -Wno-unused-parameter \
 -Wno-missing-field-initializers \
 -Wno-missing-braces \
 -Wno-format \
 -g

ASM_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
 -imacros sl_gcc_preinclude.h \
 -x assembler-with-cpp

LD_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
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

$(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.o: $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c
	@echo 'Building $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.o

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

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.o: $(SDK_PATH)/platform/emlib/src/em_burtc.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_burtc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_burtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o: $(SDK_PATH)/platform/emlib/src/em_emu.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_emu.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_emu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_eusart.o: $(SDK_PATH)/platform/emlib/src/em_eusart.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_eusart.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_eusart.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_eusart.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_eusart.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.o: $(SDK_PATH)/platform/emlib/src/em_se.c
	@echo 'Building $(SDK_PATH)/platform/emlib/src/em_se.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_se.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.o

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

$(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.o: $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c
	@echo 'Building $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.o

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

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s2.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s2.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o

$(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager.o: $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager.c
	@echo 'Building $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager.o

$(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.o: $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.o

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

$(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s2.o: $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s2.c
	@echo 'Building $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s2.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/power_manager/src/sl_power_manager_hal_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/power_manager/src/sl_power_manager_hal_s2.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
	@echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c
	@echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c
	@echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o

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

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.o: $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_sha.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.o

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

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o

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

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.o: $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c
	@echo 'Building $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.o

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
# SIMPLICITY_STUDIO_METADATA=eJzsvQuT3TaSJfxXHIqNL3Z3zKKqSlKovXZPyFLZox3JUqjKPdO72mDgkrj3souv5qMenpj//gEg+AZIPBK31LG7O227qoiTB4kEkAASif949vnLp/959fYmuP70+5e3V9fPfnj24z8/pMl3d7is4jz76euz87PnX599h7Mwj+LsQH7x+80v3uuvz/75z1+zH4sy/xsO6+9Ikaz6Ic0jnJAvjnVd/OD79/f3Z1WcoF11FuapX1X+dd1EcX6Gw7zEBJSULnBZP16H5N+kHEf7+oxAf/fdj/s8iXD5XYZS+scwz/bxgf+N/jVOcPe3KglSnOblY9B+dXYk6E0Zkz/Rz37w/5InTYor/+ekwX5Vo+rol3GIyij3U1TXuPTDEkeVH+G7OMT+roxenL9+Ff6T3+L5IgG+iElRoSAsH4s6d0VFKEHIhZJ+uHgZ7BMiJKiOTR3l91mAmwqVtUtNqQiVMW5xgziL6yAKo9AhzxVRMnYJPqDwMTiiJLiP8oNDcnJJMm7H/UMepChDB1w6JCYRI2P1R3zYYUx0vWsOQVHGmUvLWxUmY1glGBd1nDrVmlCIjFGR35Pv3DelTI5K30yoGZymb85EydiVKE6Cpib/IIp2SE0mR8gru0svSWX2qEmcGb5EhpAP7yHMHF3xkcgQ8kFFEXA3IMaVK0ZSKSqWjtPmRIY+lSTkhtMk3gXUf+KjnCNmK3KEvPh0Rfy8NM8COvU7IrYmSMjs+urXX6++BF9uboK35EtwQgL8jbmwwiEhUD8GSZzdBrf40ZndK8rc4LuLM+r6BzXaJdg9VZk4GctdTqTQ7+oyTxzSk8mRjyG031T3bqfJpYyN1iTjYECEhbfuW1IkSsauzm9xdgKfRyZHxitsqjpPebEjRmRF6oKWRMwGK1KNZo/CmpChHeY0HNeFbljfocTk89bxdG5/YmHSdUCcFmTASXBE//fc5WJAKknGLc6rusQoDdpl9B2Z/hzyW5e2NbvlTUkcmjJv6hNMFhJpaz4KXVK7dU6mElR8zOPpVlNH7dUUOs1iCm2x2tXENDMCS0Yf0th7eEJCCWpcguoifOGeUC9GyCrd4ahOnLmTS/jN8equcbvXKBXUMvvRbzeRRRvKqKnzA86kO8qtv9etq2nfOQuhKsBl+1IxMsWSlWxCJNQxWfzUjwWG0+mIkkiIkNCXmyuy6EmLPMNZXcFzWeJLHaLuq4DwRkkOaGwjxQilqIzvYZKHt5UT+xGLkZHCd5T9EWVRAukkjvgsJKhRcaGahYTN4ar91oElS4QoE3KhHpGQTULMzuKMiMxC14paylKj17qtQ0GnypNJNKPqXp8CiRunXbs8r5OcLvCGwWafO6G6KVHB7Qk4HjxBuRzp+eu0RF2irNrnZeqOo6JQtaMnl4OzVJLaubo7Da6K2l4uxRjj188vzl++CIrjY1DhBIduPEcFmVp02606Nmmeju9MqMLmjNtxXSZIi5iLXi0TJN7qiLNbsmgkvwl2qMJnSQTNSCRh9cSx3QdGSbKj/xHFVYHq8OjCCVWRaEYV3OBUJK5R5cdgp1TrukhDsq4Uuy5yjWwWFidV64o8E5quFLoib40m2k8OChyMkCuChMTOwrIP+CL/CU1nAb+9H3XA4W0eVNFt8OLs/Ox8tC81+axIUE29uf7viy/eMVKjvy++uI6TmJD7gHbV5LPFh1e/fLm8+PjrxYvZZ0tEtg+/+Go+Xz5WNU4DvC8vL9LDxQupmf5KT278Q1z7B6IQv6uz31bNH/H3e4p+y8EXCfE3eNVkTdQUromJpMyZTSxEqu73WZg00aa+e0m7i/Pn+/OXl6/i9MVraeczqhyn4stlbSi/LxigMC0ck+tlqJNa8eegOE09uW1KafMQutdUL0WZ2K4pUeqaVy9Eh1YdnoBWK0SZVpg2rklxEcqUaBi0a06dDHVSBH9twwmK1yBGnVqKSLkqLOOizuXOIRTDhTR1okWSOKfHZSiTwu47ANbsAO09BeeseinKxPZVGTrvAr0QZVqHgni3rmn1QjRoxc6V1cnQIhUUuXvzmghSpnfcn8DCeiEatB5OwOpBj1R84dzouQh1Ssj91N3JUCcVovAoD1KAotVLUSZGI6BDJD89AGI2EqNMLSHTvGtenQwtUg875NzFGcvRJkejaOLM/XpNJE+dLGbXwJxzHMSoUzvFDJBozwDJCWaARHcGSFGc7PIH17xGYtSpFei4O8FCfCxHnVzlfJbiItQp3TnfS+EilCkVYebcS+xkqJNa2beH4lTqDabk+6CKDxlKTkFtLEqZIlmu0zOSAkWuGU4lqRPEJxrLJoLU6bnfJKg0Nwkq4tStRJ9Dseql6BA7wU7nIEWZ2ElcHn2HpzmJx9Pouzwn2YXS34S6i5Bz6+pkKJOi6TFck+pkbJFKebyiIz5j+K0jzfkpqBNGIilKJ5qCXy5+tfzFPCMQDe5YO+eOs3Bxvj0N30J1nsb6Nt2K9gm+PwHxV4XhLIpRRhaM+s7KVOAUaEtoo78SmItrBE7+QpVVhQ0GrZkqe5B1YW0pS2EDyLqwKokr24r1GBuialQ3tuYxgPhr/WvRXapys7u0zaMbHcHpEXx/AqKidjtZPYaeJuo8T8IjirP18JilwgQh4CxvmGk1eiJdhQY4f2tAVRgOZXSDEh9i8/41sOYWuUBdTBJircHKVxF8CEOy1MNxO8GBEVjCbjaf9nw43JpYmxOnuXDW+vv0S20LHujM8u/4S2C93omKeI335NKsHe3YX4CtjFnT27pVnZfoAEtgDLpCZHyBc6RoXWOeMZGjbugEl2VOVkAG3UmgiwmYamNACJ6AKVtBgcrKYOm7agQDprYxgvIYgW4QKXGF6/VrYxoMpmirY8fWmPn24/X767Xh8i1LVbo2q8piMSdKCNMqrtj94Xjt4q1MCYymT7n0S68l4urU1n7O07rCyR8Broqv/2CZk/CDvh8tkD1FU6g3mX7h6szB1sXSzGlhenkJInYMtio2LZoAlemdfoyvQOwEDN5dicr4bt1VYdmt7D3wPkmW9lTcUvQZQOeAT9Dc+OCDCN1WnBDmrucETb8ZFzVIcBSVd/bNQnBMG6Sl0LUIB9ry7IcLdDBip3huLIGCG9oAJ8uNgANp6AhE7BQPfgwhbtDWXZdRfox1m1XrmYtkG7p64pT9EdTQU0XoW202LmUUv63ISBa3vcbHJMJXkY4ksncry5YrOrIQwjU+RoEtinxkAS1rfLI7g61wRT4dtv44qTbFT+00qC60B3lRRfo5Xwiv2RMckhrQdfuDQ1IjeN1e4ZBVYsaK2a8jSh02gHM2zs5v76KN0YwrPwbpaj8H3mqJyaMDNL2ihX1s8Rnhb9CKp+XYxkiG9O/ZrDBbEeHG45y0DEhF+GQwB9bSLSSTJTJAv+tyLNn3uQ7J2L47gM62x4Bb/axPFcUWjeAUBlhlIjS3dVynwWt4MhNoZUIlrlF5wHVQ1fQtI3BWS3xlajQFJjyhDlWdRuWGR6VL5M6NQu7EGoEagfv+ajrm9cT5yDsG1BwAoCkMsJvDv6CbArGRYRuMSeD6MSPUmEQLbnJpJOGB8qaC5CGA1RyB4BUiC5jcGIHAidxpa8RkM1nFWMH2lLvE6ZYO1HBa6JmHAHQVH1ixOUCGvdoIokIeKgqXpDp8fWI0r1yEkjxzqrSZGFWaPFwPntgAvH6mV5rvAMyEc6j1mqMKeyUmUCFO13Iy6lZcgLtKJIrRISN93mIDZEZhirgqHKc70kLFUT+CTiJ7ArgqmmVkr84qGLkD2qrQkvSM3HzxNxM6oK2fzrOUaW1ZKNFzTChneX7+Ey/mjEX12q+M579RxdgMyND8AdQshVeYlzV+SC+3yI/6ClAFOsn+FNqsGsspe1GDI5zu6b+Pmlrf8g6AqMmwzbwDh6Q6fCvvwCW/qRhN7wCemOyWgJwKxkWZm6/f5VQG4FUqqAITzqHWa97UNaDme7RNZwxqPBQnF5pdN0svL4I0DkvzddRM7Axywwu6eHjwWNyjF+G9edTBnIMAd4NI+y7o6ns+2iSmmKsEYCeTTXFsVCqboq481l5QskW4q0RMQnIkslVigZhZAo+pc0wFAvT1ITBjn0FuiwcVvCmyQ/E2XnDUFC6AVVmAAEkf0NaFdiMQmNwx4PoumYPFwXY4NF0ceZOXND1aFFfeORSVLRmmBMHGwC0ZugQd8lLZax09/khH90PZ3pQ55rlFlODcvLbFwO3GDm9sW27KEgSgzbuBEttjmAI7OI4bCbBswRHz1q8+qs6E9FPi9td5mFsPUwIWY2jQnXyWNAEiTHwCCLA9xXCG4PEFvKtQ7qkg+xGhrcdyWBAnq7BpTfrqA8wwQJBsW5CT6QcADumo92cGidYlhLsel4myqts0T0GmqHoP1tVaONtmalFm3WyAdtrJuBjLduM1mHawARqg/SZvoNm33wTOuP0mKF37LaC3vKHp82524WTbjNQDy+bPzlmFkioQ0wgmnRYkhnaHavOdVhG3FRlu+uPUbGCqwvvjAtrEIt0x0oh0cqckATbAqNUmfbIfrnjyqNWXtbdq3WL0V/KmiJu33drPiQcc4qoKyGIvtliiCKkssd10tHHNTS2IV6C7ujdFNNMlKJUlNoQxJxgXLHMggEH3WOZG1EMMGYzGoJvN0H/dzjxtqkYXbGb4msTa13Kc8erhNwffaUFzg50zEiE76vqDgQCw79OVjUE1tAjHYY4K0NnbmBQwf7uFo5vMpoY8IdTZ8gR2q19NEFzw0PD5+wKjjVM3jKYC3PSrVhgqYmOTntaB96wJrF7rOuCh4T9PCswf2nTITSTKxBLdMNQ7IxAUc2VhQiEm/NojmcD8XEqZ5FiSBVPzAypdpoJjKqN5qYlwgh7tJ6QWx3jMbYt3g+0AttUW7ZcsV9CrlKVHugZkMMN1M9jzypoaDufNLWYAg894gtN58p3trMj6GsFG2foYt3EuWizJ02fSR7K7Raysbg+hENSvopHY3VFY+469dpdpq0oBmbGuI6/2H1aUrln2CaqOIEQmaNvCaaYZELkdkIMuOlawoV0yrtQq51ibGoIQqCTIaAkqqp1KaNvMVIDEDmiKhgchtwPSN7wk3q0bHvmAiXj78fKSTUHInC/B8kV4LmaJecX0Un3j1HCHhgnm92xkuzBzSfSRYis5HGBDSl7qX2qbiuEI63KwbW2wSm2w2SX7qSDJjfq5LPbQqZWkDmFdDn23y0oMB1iXQp+YspLCAdallLZWUKpYQWVp0ZWCPbeb75Zyeox1WQBGrWjTRqFEE0GiaCGFPDELR0c87urPipRaG00ve4p+MSIeY/3YlomgDmF7fLcTI0r1J5DCsiEjE/9wKmyEsy7T8MmTiTylF09Yo+pHkc6sYitXO5vjjIIdpxpUeVaFDnEo3TeZpa2PQNalmaa+nkiTprsWehuWWiy3MqtzZ8FKjDCNpsTbsBMkyZ4hlgXSlRdQCn6OlUDxk/AiP8dKjPCBU4EUECVOcbZ9Kythwvc/Rb6V3ZihYvOVZf+tFHqv4UOSUzmSZyIlfpytLI6h4sdZiVIcLYxCqSeCRNHS2uvtApdxccTl7DK83qJ7ADGN2xgQmJcqBAT1V5cSdNtiRJndVhMBWjVNiaI4X2uVEsVJsNwHUtlWn2mDImkrgPHzOxLd7nkHtXFVjxRK9/qdWi6zg9uW2/qt7XM8Ad3n1x/6V2iI0bdZmV0YXSGidl2UfbrHqG5KWPFjTP3dze5uz1YalV2yfL5GVEPynW3lOko+wfLHoGbpR2KM8evnF+cvXyhVYPgcrB4DpC8QYVarP+7RnVqLsC/B6sLQ/CmwUg1UjJFuFmwZIntIVljx5WcPh4sHwaciLVHRQVXgMN4bpNGZd08C1j4N7Pc8fLGYue7k7W+qVNTU+QGL5qX5MQdjWOIEo2rTtLrPu/q9YO8X8sLaJyAz/XHK/oyTvykUzBKLpDksnvYU+wVNPenSm5mmSHUExeiLrOFtgO9oujXbzsrI+yIxvqJ4gVmqVKI4PpJVTYLDJ6nDVLrZuFogL8wzvokzf/tS8H1vitvjzIR2gYKwKe9oLNPzaGftn3F1Tdn3g89MZQLZy/ZWI3/xhOQv5OQlbT1fmqBgJLAdVbQXVEp1kUna6GZDXR1zmwlRoLWojRMbkElS1FvrmrsmKJKk2bBOua3QUhsTR52vnj+ruzFQ1DGQ0U4wfZEQnSmLFoGeoIS8AHSOsjqI4vlreuv142XAdc9xfZkwnTboikG3wypH4/YIc/wQVzXOQrFrrLaoFnw78l+aZYSbQIkjJh5fRcI08whYtHYl9HyxbJHzsE0apt3NSMs8HqnbIPcnVLYA19ThQA98d2gmBHylub01nKLl1vBylEub5SJdOGyQD8lij/7SUGmEj98h+WLUrUD2rggwA+XNuxGbOIvikix2vL83uLFRyRJo5UQlodfFPVLMQuAEQ0WWZ3YXXyDRk1y6n6b2I9+FxzjRz6c4SJxgbMkiHRZl1tIGlA15ewJgmkNwKnOGtCEXRKSytAKFtxiongusLdlH/ZsiM4lH0fWQuRybw4xOkuT8YiqrSerYstdPMDbq1X6b4fo+L/UDwKf1W2CtyK5CpB//M4jriq9IaIrCevycYOgcfc5/nM7GR1RG96gcO7jzU6CcfGJxfk1mPAahldGjo+Wzot3NrynOWpN235K1cV3miVaw8prsOR7osflciqq1zAjz62YLKBV96WQKWRMryg+ifUbfvh2+fquNZodueLLLnHxNXx+yvzEZ5srR4L0W+EvnYkqdFXFgN7cVKbhu262z5m3KgQGu66UPFy/bu0BBdWzqKL/fuJMobq1ZrpZAAMvD/BS2UASFTRtfANW1vJyi5K/CW3qmZ1sqj6Y4UKOhNYrUyE1RX40Wh9V2UyvdhJJPq/UxJuNkQYg/ro5w5WNRb4xo6Q5HdbI8uBKd9pbz++SLpkRYeeyjNfRHFfFbtj4n5HOJPofc3CCPdK7+qQofwW4QQFV2TqDUr88o138MrEDivoxrJyR64A0S9DGwV8p7jKoMBtQN8SEL9oMWP6AqiQ/uS6SclVaPQw+tQQS4V8ygN4hg9fy8qgSwKD+vSDA/0HIgf4S8SSNSvoOvTiAS3sRfiq6QA9ktqIL6u8flA6R+x16nEeb4G5TSCFoZLeKmWCfdcAS7NSTHh6xRvnCpPCT3qFvisyBtlFekyuJ71C3fQP2uqbJfILp6uhSsnANDWa4gGYZArBNHZAS7ScCJEzKC3STgZAIewW6NfXTjpFBOm6Q85A2wagSCIk8SRyx67C1HpC6DqNwp341W9kJGuBsUjikKnXCYAOs0CfRcOMPeiqs4onNoVXSY26IvXr5yIJyjbot/ea4cGKchnqNu9YU8o09R1gFNQQreIebgG2TqY4lRpJFRU5XIBHhrqOanFeBj9QhXkQKL/HHFowffspAjDm+DFkzzUURlOxGL2DRdnWv86jYre7Boref0fj88HZkUTXp3qIrVb9GY0hukKKxBgzRWvyWusxDtgZUWhG40M8feWpEVlWZeCuVF2QRZgQa7j+iCRQ+sQKLUinnQYVFKE+wKaNSlxuvZOix6YMWFA/Q28q3qSr2sEN3LCI44KbB66gNVIgL4LYepStp3R7oywQFnuES1+uOxyl7UuigFoi5GuDGuAgVS9PwyuMWP4G23RN+gk8fg210cckPww8vnf4KW3GEqiCauDUbw2w8zaDUi4P7kGFeNgnLcjCYF4Xs4QgoV+HHMGFeBAts3cqWKCbg6GTdKmYBvkIngD0YitcNaMgt1y48kpjdIoRddIgHKpNyx0dMNIq6TQ8108BqN5eRoVShAnVQb9sH8K+bSuGMnkKROs0rymj8DkLq1eIEkDZp1XpKCLukNEtRpOThDXqKr0zlqpFU34HOU5FmXE6Khve74cHR1OiX8qfMSXcOondqzpimzYRd6fbmE1x7V3VEaBGiQ0kh2aUJJmAlzjdB88HfHTSBJb6B0Rw2LnpneHCjd8TlK3gWQE3K1/ygRoUGsdmlTHF1vQnFHR3inZ41OibIoT4M4LcC3/OVS9OY7h7RadK35zh0bURLSVTIzd9Qhs6UkfcfXIb1BgmFiuixMmkghnYAspntRf6R+F3Crwpxc/zMSpzhcUqgy5WfgtDlwbCUSbWyRQya9gE06PNbZEZcBfZNISP6VJDHY4DanMsbfJgPqpy6orDipgoBmRyykDpYoptgZh0jiVAmji92xkE17y6BeRxxSyUpqQeClOwYvlSiUcYHT6PzVc1dEJgI26bBYMEdMOmwVEjQmzB0Njq5ChEaHuSPC0bfnlTZg29W80qNvEinUs8brsihEyeQFFDSSS+tSkGSWXlDoonldjaAD/Pbc2sXSuppdR/ibZIagWkdsJgK2ncPSnQPUYW933SZO6M34vfJzItrddyJh22JCZ12YQ29TOCLyfxfO5rwxviKZIk+c9eepBAVCLMoTNIJ0SWkqY5sU4L7Sgox0V2kZyVqg2rs4U07KpU1lIkGBEFVgULjzpqcSVAiNIsndcZoKUZizNZJ66c/awjxfIhpHZ0Mfh1ZYev2tQLfOmmYEv00FMn51QUQewzqjcXA3Hx0U56PjbbR35rJw7O3FaFvA8cC/lLJNDKd5+Rjsmv0elwFKktzZTCARtUkxw3VQ5TRxm7O1wUzENqW4qoPbe2d0BvhNKjT20xENDr29bgRL8LFYNkpyfCwIhJW7pfyArkTE2a5TD75No7tl5YrJCF+ZjFOvZSFEnRa7AeaaVidkmxZZSJxfPndnRCP8bTJlfIdqHKAwxJWzwXcpZZtYhdy22wh/e5fX3QJBesa8vOLgbCOzUtMCvQcRkqWwsy4+EaBGhx0eVU1cu9tVFMlRI5fnt7FbZfUSlAjVMfV/XBIaJGwSGu4qO+IzEbBNJ05dcunRN4lovm2ty0T66rWACrv644hHh61Egt27cUikw1cl46wPjfEVyVTO9gDG+IJ4HsWXFshEq/DMAmMCXREi2h+Qtzff2nAhuh9fxxl7KDuvXMwsAy+JMF2qxL9K4zq+OwnVqTBVqnrPlxvQk71rvkbJxU7NhJL6/vwkBBn2hryA2UKONsGsxg91dZpOsi7UlPpJOs26UFXqpGgJvgIZsezxVQm52n4YcdLagehDZLnCHfKayFAmFv/h1Mx6fGVCddmov/1pwqgXoEpJ64UMA0bS179lhO5Q0jhlNAhQdq+Mc4mz1SIZNjOcLfOCC5KK83OGqimKXJJJXJDUX+VBLfqdF+G7OMQe3RBKcJvA3lbR4xr6yyrwo1h/nYDikbFXJV4r1kNJ8oTVWOdh7rWLMvgvdEHYdfnvumTwtvf9tmrP8/KLxG5vgfRwJ2I5krZNDgcAOdyViPWSlM80WUxwAJFPWYWgWKoy2epofQdWhyUXZ97VRK88LCpHWgwilfNWxeh7DSNRKsFop6I1EqUU7XQyXiNZKr0c7kaWCruFQKUQ8lPpbixLMaz8hNRG0hRDzU9IbiRNZ+I7BbmJNPVzgFMpbyFQJcTpVOQOGqMcDxY7FbWZOLsli2jNQY9kBa+HiT9X809jBlqX9ql25xoayHJvbypKoU/GIwytZ/YsKc4k6jLle4Yn5TrIVGJLJj1+zBVEuMBZhLMwhneuBZxXJKsy51XudttZysGTEJcI1uRNigFmW9JivhRtwJ2MMTRWJT6Jia+J1uQOkbNMi7E8i9kqT4CESVo0pSmUVllW8SFDdQO/n7HKdSJVkzFMijgtumtJ48Rc6xJlFRFI0J+C96p4izpAZCUzroE8Z5ky/9ONHSvCLfifbEyRyzZh/xTTzbp81VrkBfp7c/ohXShWk/NTKF0q2kzfpxwyZZLNmJ9uoBHLVdkpoXhxXQXZXXrpnOtCmott5Gk4VrNLYvGXYl3AnIKuaILuLbSs/IVIYYPJFKIcejaraP+WYV02VY0jqJRUW3WukL8mWlR3F1cAFWiu3wZca4/NfQS35KUyrfYR3HNekWyyLjwJ4ZlAs/XLKZlqZJ5aXayfkvNStN3+zKm56x5GLdbypySsmtdg6nPsmyykjXOicUIkVZMxSOSVFlv1WCzBuuXECpaKNuF+QlULxUKf2nTHy3BHNsGACet+97Cd7z0VpLI2GEqAPFmozG/lDcNVki5Wims0tdaH44JAz/aqM1190HeVKvxm3xpPjd29cTGXuxxrdI22OOYAR5RFCcDrqHq0x1I1STs6LVhjrHtUMC6L6hpX9YktYyZUde7siueV/Zuj62SF4kxoBjsy56a4PjnhqWDHAYnT+dNRRXk40VSQprVHeB9n0D7XCsWxQE2qDpzDNaI6TvjCGTkZS+UkDdNi/c4axIuQ6mwXYnVHEKgHltcpS0XqOpiAuQ7Vlayd/XBW/NR0jYk62BNbJaqzISbypk/GVCfFrsChPhlP+Ts5Ou77yega7S5KHemT0h5LNXbfT8Z4ItLCfT8Z4ZlQ8PuKol/NDvNKMk/tZ46pwhbT9Kgvr+oSozRo0YyXP21x5ogLIP2t2iq53KvETRueE6cNLIBUIj5iBaNFqeIEoizrLa7qtvFV+LDc45x/81jVOL2L8f3W/drrq19/vfoi3DEd1bj9Kvhyc2Os4Ja2PzDzW1B/ii3ozUsixqrfICEZTiSacESiw54TMR6+lr+YGUucxMQvDBK0mz4XtfyOfMGeTVy3Pp56VbYbv7gP1CZn1TGsMWV/xMufivYn6AuFinnoNKs+D9WRbaJQnsRWSaEJPqDw0SsQzX7mtSVh6jRl4csEraq5/cz7e4MbPRdHldVcgAobwzNoTUprx80qRoDv6OkZq9i6BbAPIVU8kuzPwDersTUazX+cO1p5nYd5Mqrw7Is/4sMO49VhqyYWuj5g7Uvy7/u8nH82UyzClYfDnepI1ZH3W44+I+L3svwR3KqVhmHqkbLKZ2dbcsd4q4Lb8gFLih+EZFWTYeW9oC0SMmwFQh4r5OGHmgYjKUcUqTESgKtQartFktOZnSi32XkHrHyiochMKmN9YqvLPDt47bOmHipiKFZCYP1hrcuttOWopnFY5lt+qsZbluLqdWx8Jk72gKVx5hvZ652zi/kh4V8rb2GJa9Ll+BmjrU+GcUZvIns12iXKE8e67AWk2jgHInuMtir2UHhahx3rYsdo6/0yb8oQe2XeqL84utHSM8RV8TjdkVEEsOIzwPVBM0kgRU/gVGvt6Z5uKFffWzvEWNIBlK9Yf60ndFSqLX02ZxkhTr5V3jnZED2G2xYMKHR7JDvGiaVf0g9jHZRCu6KE2V2NPY3oVJX2XQCrGFlTP3rhYwg1l4hQFWiwjeLQ08sxqMJkAbxKhgipNK5xrBMYga0LbZI69jJcU38NSPQcUoFAcQRyZSZwq4I5QY+6clqnE+sEhLAb3jcp7+k82bgxx0/wVkWzazxeiOlkBCN8jqi2VmO5DRkQ0GwvBl4lU6J7D7QLzgBXhdN1fpoCGcAIbN2bfySjNd3WiWuNa0Ib7vwCU2EAHq0PAUffKeoqjSTelagEGoJGYOsjHwq9BD1C9bwJ3Hq753md5Or3rDZafIS27vHsbnGX/RNnhzgD6mdiXO0dGbpRAjn4iMDX7WF/IKYDZAw91uYiC6zXjcD0t3gOxfqGB10z57c48/SiocSMD4UvwNvaASBYD48QWx+t+BnclvQqzm7hhE/R9JtLkj5ZMLa3XzalVvy2mDpPJSzBXR97AKxmksl427Vi1gXg03CxMzwF2dY+9USyikstaBm6+EbEJ7d0L+RNPxGgb8jd1ue6KS+nGLa5bmfOnWjBBNaja06o3vmry9tzx9ymMnQZXrx2z3AqQ5dhVCHvDpfx/tExTYEgE640OO8ETCdiLHR6CgOQilvlHdN3xokTE+YRaP8W4prMuYugGuH4O9l/AahHe76/Aq2yv4MgTLQNm1kAKsrXywSqxECe6HOrXWynQ3GTqO62Eepw3vRMJ1pudV8MkoKZJ0CWL+u9i65vqMkp5zcVcyUw/gRq+xgMrKk62ettpKStZUzNgvv4sLFdbHhNESHbs8zuhzV0/QqlaCOkuTUv61MTAuBPoPTDoxbcUTFfPWsEL83mGDqgbYVQ8MAcIjbokTUTLc+VQ7DG4UcMy9+QsxGGKyzNGiEwD9BS5zmWZMQ0wbg4Ac1OjAlHvaRY1m2+HXotLM124tyT7MUYaZI4iDuLvQINXY4kqTHNwkl5J31HJEMppEoQLNaQNcB2fpc2iI/dsFC6Pj6JRmTFYPTQ8vVHdHypNJWrV4KSIDalzNP6vtWiqeh50mZY394jNtR6NjAVZgHPc1iwMD9hnVQ3pyTRgB6KohJXlde+Sm9hnm2wd7eAleNvDCC2YYpyKsptsYyR/6NanjWIeGOPfmq1bO35UyRfhLqhQPZth2qtRcZiAanShpw1LBMZsgYhqwhBOZu1/GOL5sFZo/ywrELTdHAqotlgZX+EL2AhQAYb+nB68er5xoY7cQ5QFnnsmjQuK68LEz7gjD4YiZXzMovqygj4ajI2jpXEEDYNsU5u+5R9XvBARXpFft8eQzvTnlTOxtYYAbPcmhtIzdHW465wGaOkjcTdo5BYOyqtfLqWgxR2PRotLNrDPev5pmWxxNOzGsgZUGIwKtOhkCR/nRyuk40AtSzGnoEIUaW/8DWc1Yg/6TJjQD1LoccvDsa7DlaPjF0MzQodxYCaZTkWAOh65pJI0aPKzxBcc5WJMRif6JLEJVWxED2i44BYp2TlgvQI/5FYufcSdhzVrJEd8FG+8rasSRveR2S5UNMEXP8U44HM++ueNf3Chjgt73cgq5qjH1nuFvbCZDuCc3n2+z+9yJU9n4XUCqVFgr2Q9MA8hVsqD1w2BOgbithOjDZShTYHbndUFXwLctUE9bYajY7b6JOT0fqB2+RWq8cjwomfWRI/M730DmHo4X15eZEeLl54JYoTj53mB+xkMOBTZFDVZVwUeC5rHs4tknWGzFTN6ubD0hfDUYZGdtuv092qVSDJTqmQ1EVgZgpt9/N3qIpDFVbrOhOA2elMk53oexu1dCkeoDQzx4NQjg5HSREzFRGI7oYVtUTH3VEizU6D0FWQAZop+FA41ukgwE6NAET9KReziUGNx/awD6UXdU7+UqxBh2RbOXS7kW3cu+6QYmmWHRK4CjJAMwXzPGZ3KIlpoJm9sUkA7XRowFJWxkxNwyKWb0G7McC5GDutgZFeIJmPZxTCvZM7kmI/3EFQngNZWWF5ceFVTVEQLo51KRcIYpmQFVnBNFN2v1PaMUvoBUP6qIAjZcsF2inbQUVWMA2VPY58dmvTIlGWCoYkL0QzH291uW2PrfD6M+EpLWTjZ7J9SWslLbEgHEhVboLPlVSilbF0+sPkSgGK5UEPPycNuzNQHf0yDlEZ5X5K09qXfljiqPLbZabfYfhL+PaLgH1uK2aOtSIOr0RRaErD48iJ6bEMIyLbT94Sw2OPexCpANN6TASIa/Dl6s27j1dnaWSnqhEMFzK5Ht1Je9fekiayErTDyeQ3u5ygv83TgpTY0YCvR7qrXEYvzl+/Cn94fkb//5vn59/RXz1/foH6X12SwjRf+bxsGjZnqEzPWGc7o73trO93u4vz5/vzl5ev4vTFa1K+zvMkPBIjnoOQv5FuPv8tUexZm3f4jPz1rHWwiQuDyx9enJ2T/x8Q0n96efHq5YsX/cjzY5pHOPkhwlVI5g2qlj//6C9/13bWifrI736kI83fcFiT/372/bPr9x8/f3j/9v3NX4Prm9/fvf8UfPz07vcPV9fPfnj2v//j67MSp/kdnVZ/2KOkwt+3xwV1nF09sDQZFfnL//4/w6+v2bDMftulKRgleQ/ekt9QC/2+/yON5ieMSI1pYN7otZ7hk12dBH1ISRDut/4cVBfhi+k3OCXjIUvt3OXfFchp03vz2xkBsdS1r44oEf6ZTO9RnYgrkt2llwG/4C/8gD4l2179Ff65SgJm3PSPdZmLCdBXRdi5Gs3G3RCd1KQ7ZoeAnf8FR4yiNqeOqMTqN63iqnspNz7MxVlcB1EYhSrf4bRR+ey4f1ASm6x8xzCWr0JNvulf5WgqMhAEd8QWNj+9o9sxsq9G1nIf5WK7o6ny2+T5sr8+XLwM9gm1yOrY1FF+nwW4ZSgpweIWtypLXQb2wFtQIIVviMMq+aiK6XEuqWtE//dc+hm9m1XHqZxRa38btPkdDX5cFLBTuI1vR/c5Nr5kYZ9cfetfkkGruxS3+l23XAroMok9TLdVgA2hAfNshZ9231F1jj6gozCZBZoEk9H367Mf+Tj/w8eP7JffPaRJVv3Af/vT169fnx3ruvjB9+/v77tJiNi7X1X+5/ajM8wu6X+lk2o7vLNiddm0v4wj9nMTnrVyzypcN8VZE75tSfGjg9ZtIO4qK1RE6QTlz1/ZRMXSLlX0cW8yB1O/oJV19t/pP336UT9zdTX6M1MKZ0bqTBH/83u7SeubUSKfJq5xTUocqn9EXX59hpo6P5BVzllY9rNBSF+J+mbU3IRd+X9oBX+5uQredm+JVXy46P5IBx7iHRF3PNihCp/RaI/Rn6ceS8B/PcNYeCaS78gwRjy6hHj41N9kj5wuv2idmM4RohM3XVRMv+lfRgsIFErywxJmPPGHSU7DfRcw7UMePCBt688LCf0c334hwF98IcdgPFnylCzc/rCd3ofP5ZKFny/QuSPSpTdkL+Vy9bWZVaefz4yiLlFW0Ryw8mafuhxSjQ8ORYwxfv384vzli6A4PpJ5MqHdUK1AO/G2V5QXJUbuyIr+hF9Na9VdON5PHGPJV3zZ0IWeB1FckbEhPC6UsP29EJ+FDaqCyz4WInMvRhVb/vm35YTc4JS+bYH/MUf20UbI96Ptm/6/25ac7Sx9v9wlI7864PCW+KrRbcD2FPwjGX3vEX21iY7D3XOTk5Ulh9cpyYZxpWL8efrpUvZQxLka30nx0eQhLRaV8R3dZxrvhASkZUq6mO0qEebVFvt1HE6L4qjxEazqOjLyBZ/kr4rURSI5b32R4lr2z8m8/Xj9/tp/S1MZve9S26ZVXLGZPR6mW30A0oWNyxItVDyLuW55unkUppeXBmXToglQmd69Nihb/xGMLnStFX7XbqFet4+afaBvml398uXy4uOvFy96uH7nMkBhWkBDYlknMkZMm4cQmuaOdN4UHrPeNEtNzDBtgBHpthw0JPl8cCHBUFMUdJvZ+eZQoQteJLIZzhQSgzdVO/gCg+7JaA/dVIeCLOnBMWN4mnEe0NAQYNzjHl6ldKsaGDK+gG6jGIGPJXGbiQUWlG65hmhzytdETcgI5QDyYYegR7sOlm64xBn4DJ1gtqUPjeqgVyXwvSpFcbLLH6BRC3TcwbsoaQXdX9M7aNesCDPoAboooU2eIAY0fzRZ1QMjE4eH7qUU7fsjkMDYjalW4H5PRUbr/ogfEBTePXcx7jUuBj4XnuRdhKD1SQ/IYSEn0Tkw0CnfNwdBI2ZZE8SerxVou31IY6zKuilGoBsbNUqgc6IbmMMxg4+K2B8fCQ3BPlu1nYHQOB/2GCVNPG9QdshKo0d+WtZCblCgstoeMVYhKrIapi+wWVShx9DnUeIK14HKcn9SeBzhNftxqyI88pDvi6Kqwtsj2axMnafx5lA1LTMk6FUvg7Mopm+6V5t2PS/XbDp60xJVEleaSqBPdzaqxPiOMFe2WgPxMi01tSJ9qGRH8hCGxDIwf4BLkewCpQ2lsitNDP2gsEu8AOn20lsKG2rge/IsNqkT38fYbYmeFOZih8JqkhMcReVdJ5r8pCh0Wm44R9UrzkknbVIf7XIjsRvFcUqLjfoDVplA2lI0YpOVof9hUKQLcjApekSbm5SSYu1JjUlhGkWhUS6Jd/R/rOjbj5eX7GgGKRenTTmLiWVn4m3Ailq7ToGGyhsUZpXfLEfrzKwoVZyOJkWUjglmJTbH7cn34THeXJRPC2wv4ebfszM8pFdztTl1VqRU6KejAgq78NPvlVZGgiImClDZ054UUNivmX9vwkthf2TyfYnSfZPpUSs1G6bSa3e1PYN5EbKi0SqibyyKp82TMiqr4LYAHbj6IUVp3OIlaI/X+p72Q50CWFMA74c6RVbiRcQFaG/S+Z52C53vS806V3oq5faqU0Rfqcz2NgqQhVtcHHGJEmaxw4+Km2yj8lTqsvyG/BJFcc7eHaIXAEoc1t7fG9xsDhhDuYQldaYPTDOP2aCgehEqhczIyaZzPCvSZlLUK0Qcn9qbXN5RLGhSZvFQm3pRnp3DRCp/tlW9QJ/6XrHI+CkrtSIsPjVtHvpYVfLfdqXJ6pNec1QHUTneHL5uikLHihkrOkYM4eQ79rsSJxhVuP+ZbQ4+sKN2spDnf91aDcxkUOe13b71O7iH9q/0L0FV4DDeb+/mzFHbtR/92bwk9/bbLciALiEV7UoItseobkoriHS/6b6slNboGH15nrAyzPFDTGajLMT9zeDhV/aY/VX4UcB3E44/8drfqk0VMKKMalUgmoyde4FVb9IvlrfvmvIOV8H582in26iGoi7ARLWXB7sf22HArGVUgKEYUy20ncABMLQWxphGPAcLQFkdRPHd1Cz4L834KmJb8h46qMotEvfCxjdQLKUVdby4amvbFEJMTZ7d2LhLcPtb8h+mGCONsj8OP5si/nGP7jgv9p9bOGTyHifjoBcHhl33xUUyGDCVcFtFKIWtLUUkleBCRSiViCpFqOxu260TQfVnH7PcA9Xm6KuIRjMUgIGxHARgaAkoGmsBRahxLoWuNce/U21JAU5sD8QrN8ExqZgAh50wqKtcADetX3ssn20f9XSI3bXPTuvdz7blA6W9kE0UhaNLFYwgrtNgM2JnE6lR2LDdBlHZ9t1EubPgEsPpJrZUDu8SvdUp9gNZeW51tiglrlF5wDXxyqLtbeFNuEHJtkiNwg7sNojKPu4myp0Ol0UqJGY+5N+e2jHeNs74vQ8zJFrBESO9mh359jWqlEfOWckhwslMJwIcjIlWAPgMj/WhYvMsWAeOXpOPUJJnyt62AqopVFPXxkrvkij6UYwOGeki6s7nHGn77pOkJN6nlxdBGoel8pQ5R0gvHh48FpTnRXivvGpZwLTJ0UaZhnQhFGJV5CUtSjMjKpuirjymTkMYheAlSUnWfHYDQAuhtB+7hmBYtvu74nGMBKUk44LxyNUbsGH59tVJq8KWGJM8b9QmDzw7wDHP1TcTZqgs54g3SeLnUZq48s7hIU27jgDSAInO5TQ3ERnMyGchTofELtowi3nZGmc6nwLCjeZTOFRDKDqRmZUczaFmACwnNjvbNSvPggers8qoMB+6zCS3eYHaX+pDsHgLwQiiEgIkQ+vDaqdoetxo+qCxs55tp2CQIHROOkXQ48CyNe5n2ml/qcmFI001w5EUOU2SW3WcJr9U5bSNpLcTI8KLIahxdU2RjNQlQtJb+Cvg6e3LiQBnWqMWckfTRylqb8huOtzT6H5jjBHbgfS3MnoMRf2sYTBVKwUdKqOpRX5J4eIZnrKmWExd31ztHbeVPFZqAGQNH+KqCog7pBD7OYPq9DPioqoVEcCMiyLUJA1vV7v2l6jYPEZUQNEcmLaR5jn6YJD1XVpVPDg9jjBbTz5QXhzoAisvEabA3CLbX9JVnpEVTlA0p6N1pFErK+I1EU7QY6ex9idV3fCynAQvqyd3UpblFXuVsps312KYzfe6N/KLqb/33T3hOnmvHQTSnlu4u5XO5eooLD+4xxwFyFqmZJBGB2DVpftDEu/s68yDfIHZsbzouH38u6qbHRAifZgKmiXBtNdiXTZV7YWYLtMhGQ7v89lz/EOeF00K0r451KWDlW52bZVf3aqRFmYvQbH3DY1MqN31G23IrJ2RKaNoqGBfohTTztUF0nYph4si6P/W59qFB9bVuhYsNNvVYDtjVLZ0AFcBT5KsY5IyYLqVMtKtGSQPEGwfzGZ7H77weQy34CZtR6MXfbSnO0ge89k0UEiZjhx9FI39Q6M4E00nKPYPby1Tw3ZpViP2am+8lvxkG2jr5HwVYTzrggFpjr1ikNUDMBlKNz0Pj1KiKCqJM+W1mQQ1SS3gNPj0BuZViKYm8No3lYxVPeDpWr2Jqfdl1AeY9oXCzuVnz/DYv4AphJNdIhISakehHariUOnNRMH3BvLI8glnEY40RM6LaEk95cvimqQsX6tUl0Y8V57an1oNgArEgFqkDkpVX+dxMNMH1WPXfUDerJ6BaZNR08XsU20phaMnurVIgD5hqyxZ8sz9qmBJGS25p39VWp3aSd5jVqYzLM75MtX2ifMeyYTGCd9cV6DGqsFf0/DXo8/WSrbBA5NHOTyeT0jVndzCU3SIJjBtXMzKffXtwm076xUvMa/A5H6+joe+jqRPpy1eoExvj0eIYFaNoXyrVyNbG+NoFR1WsghXHlZ2r+elwzClFiE99NgoTvptTubyNtrFo8dPZji8209dWEOFcCy2r+Ml+eFArI02Md0etaLXPfJ7pAkZVRddLdShoP4X+dvDY7sIMShd0anLtLCxjfK8iczMSEczKjtZfpkh8Ig0s8KPtf4WyRSCHvN47UuFHs4OsTxUfR2m621GhdcSvqyWbEdclLA7YUQXK1k6FHBWA4lUyjfEHwsfQ0NTaEFGI445CM1fStbnZBBr5Le7FXDYJpO3dedOFci8vFlJmnlkLU56u7RRyYPe5ty0cOcTmpSlCXwS9GiorvZM1cx0tY5OZ0WN3aUpgq7r2JXuXD4aOYDWIpBWUUp079koUHMDdVZ4tOwyA2BrrbU81avFx0e1RgC9k8Q2k/T9+BnOxEE6KO8Fz7CSxHgQpmX1hwClhFrCkn3kC7vZ4q88Kycs3m/jL50B7/zV5e25/kJiDfLiNThkVCGPvjACj3qHy3j/6IozRwfFNQdj7/oS06XJuvRhuh/Ge0DtsURTRCuBwWKwJGG9iLKQ5pVcLamwLmjPsNp3HAL6lK7fvqjLEnj6u2a/HwU9tz967Y+p/GTdBHQtIaMWXhus32UMbP9ohNs+9M14tf9twZHniO/jCRPyn0mSy+KXDcFWmNXHuIzoaxK0w7C+0j3iPvgQ/Gf50KmNVMayB8+0oapMFqRqAnVPOvtqS2rg0Utor2Q5ibTByAguDRnQBqM7uWtvb+gChijFSQLWqGTFDIV0ROT/Lp6DwhV5InNG9QExje3ZHIl1IFkOXCgwubulDcXSiXsXZ7KUpvqAVGtBUYGZXU4neTK209sfUJh1GUTlDqpx17Zh9LGgxvPoCNVfcRjJHpswgAKzDBz+rUDSmG99NFk4jTYSffC+gBqM1rZ1dLEOYEP48TaS7VJpQ5HRDLI38n/Djt+pbLtXH+glFFLrWHKHGtKnzGiipZx65FBjURZXdXB7D4SWx1CtUUhfiNBGkm0A6QOF1bnsJpYJGJS9dXs2wHCQ83qPSQtCYRK/8vzyOZgW20vHAQpD+Uty2qAVgqxyGZN+EZ2/gnLQS7ApvzoiqIUlgbp4+QoO7CVYp60qqIas6FOHKy+uG+GxlUzVkCU5lPky2Dy/jSF51jGdwIAA62OJ0co1RW28OIUDW3+ESBft4eXzPwFCBWEJZc0cDqpNW7jK1rcmYy//ExhQQLeC6jhjj33lFUBPE2CTiSiN6/gOEnv1cTIjPAAXfoTXPjS5fn3NBpbdsq6cNNtchovmI8Cl7Uw9ggNyGEeIFeZ6AISM/4BU4WoAiwHe2qGsAdwdShpjPB7pwY4cVk5CVBCq7Jy9lQ2A0x4O2OHwQwFLkPYwwBIko+HYlg3E968BxtAesd2+tqtbCxLcl0h2HdcEybJiNlPBADLeoras2hhrK3m8KegdqmLpgxrKoN0mul2FI+vRhG1P20MEaSx9j1UDh6x1bbnYdg9Mb3myJ4UAgDoTDJD0lWs9OBDj6/bYLevXogRFniSQUHZ1GzbE7SilsuQj6gAQQ2xaVHYe9xjI5jRkjFOuZRHTASIeszQhiCoO3c+2a6kCRSsvuyujSN+fVUa4tQaA8A86FEt13EL4qMUthIfar6VgYNrdYkusCnEfEwwoQKRbAqNZ2sCABuIDL/Hg+CWxPKm2CR59FByMHd+/oN2ywNKXtwGAwRjb+2EzMDBmR1TZOrxzNDBuQP7dGNH4FFoARiOnwDRnHoYlAGuvUQVxWtj6j2NQ69XIDAyMmf28OsGC45XkdTCKmYYjOQOGY1znJb3FA8eUA9oxJNZCl4zBEScA4zI7+LWrIT/wtQahB72WIOxhRnqBmSsnGNJFWqqJhpfbb2Ow89OkOr8MWH4OO7Dh7NROa+xg0h6CWDlG1v54f7QJAmM7zPRHmfYwbLECRIljafNi4QBpkWeYJhrE8wzWw280LVMZN0B1jdmVZO1NC3UZFlHmGkJMFhka8G7ZR3gfZ9rnVer4ZpG66vgGrrU6eL8xazBGq0sh4KQZWNCay85AxbAsxfphMepC6IVLVDfay1p1CSbHterobNMGEjwW2ROQhO6dimGwhpkEFriTwdqVDKPNH3V4s+MMdXyDHQN18NkQ4VJMP0S4EjIMEa4kGOy8roNPOnFeAXfgBXqwQyVZwdaWcsg6pfUYA55cbpoorkutQMOkE1zaTz5b8qqku5VOL5Q7FEtHXoQrg+NUFeQwTF0h01NJN9D8Ipcj9IMzlfQ9w2BDRwn/iM5dUWebIe7AX55fuALvf+0AvN+rcECejqfd71icSGAQuKIjhDSDA3Ri9PoBgErASecGkWm+jO6R5SQsldKZjx043TBtz56Y5dAgS5PgBAXYfqHH0hbhyHCDdktSs0visPuDUaDohgT2AmP/K7OYEAURfZCwyfmvFj7A5sqGBMDVt4IksMlsVZL+qZkafF6gvzc42DcZewvRgfEOQuzX+TIBdYmyipSj2WzcVmUsyV19+OUkYsUFzSSbhfK8nwaCupdiyW/iugqyu/QSbljvX41djFkuRaw8CWopoC4tF85C5Nlw6xC/u7nE9jsdygHY69mQALhjoiDJ+DRbS5J+ZIUWPMzGjEwIH9ZdG/JUjEM7mwpy1TRcygmseTxXuW4kgSyHLSWQZr1ZqyHLlWmMRZ3APoRehqIg9nLdnvkscV7VJUZp+5qdcoIbjsAL6Yk1kkRrPueqKLbCB7a5y54Pv4vxvX999euvV1/4v1SJbMAEX25ugBgxqJZVimJmPv+H/nceNQn++uyHr89+LMr8bzisf/j4kf3yu4c0yaof+G9/+vr167NjXRc/+P79/f1Zm5vxjBiVX1X+5/ajM0yjXemX33EvgxUjC+72l3HEfm7Cs1buWYXrpjjrLfOa/cj3rMMSR9XZIQxZ0SJKJ1h//vo1++67H9nlTGqz1XcFPcAqW4ln/53+06cf/ejP6vVnpgTOj9ScIv7n/3n2/bPr9x8/f3j/9v3NX4Prm9/fvf8UfH738frZD89+/Geiiq9fv+Pd46evz87Pnn99Rn5DukhO97nIr36/+cV7/fXZPxOZRCiXST7JUIp/Wta5Cd+2h/6L2lJcUrzApFWvQ/JvUrqDe8bQyQfk/37c5wkZDwYR7Rb85Jvuy5i0cf8d3btpkx4NcQfkr33w1E9cy/SX9HUh9ps9SircqnQVfBL5DI5OqT9cvAz2CRncg+rY1FF+nwW4qUgfcCSwbSC2oAiiMAodiWlTuJJZKwnuo/zgSMpx/5ALwk4gRSzfM3UkiL1IS69iuqoJe77csbbG1pXQxnEjhr5Txc5MA6IxFzLonkHA35Nygc+tirW5C3z6LDEfdYnTc4LGxmnjQgpOk3jHLp10j/vAy+BDVbuLE9CR2IGQwW0J6EzpaJTqcqAHFINtibgdFfnDQQFLkO5I1C5HxCukKU7KPHFmydS2qntX4xVXF+mR3RPhTsSwZPGOR/j2fWMu6thfmHUggtSj2aOwJhZNTcyhQN48hxITEe006WiOpxd5MHGOIvq/546k9IvA1o+8IyObW7uu2GMJAXssweEITZ1J9zPZ8SRuC3IhYlcnbcAm6TakIfaO0YPqInwBKqI7m3fcL+4a+/XVj367YB3/iudW+ozqI/mRjFu0KGHxg/+XPGlSXPk/Jw174aM6+mUckokt91O20PfZitlvDdHfldGL89evwn/q4tE62f3yn/30nZul/A1O6ZVwDLiYR02dExUrrebbCb9zvruTL/JJvwkztNCy2VQMgXjHCdE+3Xhkh5zcBmDwv9xcERePbwFVoNDjbc+A8EdJfoAWMB4KQ5o+ooLWfvsoCwva7adyR+DAzPsBpMUHb9wFviv+rHXZO0lZ6LAaTEzrhAzCnNZpLgy4ZtwR6V47ZZeReHdpH2gBEzadCQM+fkLXZyaFnc+wTCCO5E23oFz00sWGrauqDL5cjDF+/fzi/OWLoDg+kjV4Qs8OgM1cKK5dS7IhD1reaKXirOsKZUA2E/PfSvq7gCZzPEsiQPBu7dOu51GS7Oh/RHFVoDo8As9sm8Igm4YL4xtiJ6raijQHdctoMrzTVEwmykGt0H6yTWLcmU60tukXBSdb22wdzRqvb2bn2cJ1zqxIl3VL8K2kxDtGV/q9pNR1+wbhB7SrNopKAK5++XJ58fHXixdKxWUs2BaRBoLQzttwgADvy8uL9HDxgnei1hqruoni/AeftMLwlGyrM3+kBL+vj9+S8kWoyy6nR7QmfmhTgDMVwapTFfRrzWZ8344Ctu3Y899dnD/fn7+8fBWnL17z4cpUR++7J8+l4JaN2gMHKEwLaLY9KBzL3p0CJNliwnFMm4fQgS57WDCmu6ZEKTjRHhWSZx264NmigvEM0wacJccE40gDZMBJdqBwLIn8Yf8BkOiAC8c1RQS3CsuYLNFLeMoLeDjmRZvDGZYvBwVjiR30Kgzcq9r4NniaPSwY031VhvD9qkcF43kowhJ+oOpRAXnG8OrsQEFZBvRusBOqPTIY3+PehZX2qIA8H1zQfIBlGV/A9ySOCccROfBLOlA4lv0rg7A8e1gwpjRSLkQZONURLhjXhPg44EQ7UFCWDzsE7+GNgcHZjnMLOmE9FgDHHrOwcXjSAy4cVydzVQI+VyUu5qoEeq5KUZzs8gdwoiNcOK4FOu5cbKGMgeHYVvATLMeE43gHv3HGMcE4FmEG70Z3oHAsS/ghn2NCcmzvuydOuI6xwThXYUlPO4s24w4o5Sk0HGPsakydIMPxdbDfUwHv91TECe5jkQFp9rCQTF1sng+wYEzdeHzw/l7jxuFr4D0+N5uS8HuSdxGCt9AOFIwlvfQLzrIDtWWZ8rBNKIJjPNvAgXksAgxFEayTuAGlTzc+2vrzMlMADR3TjYuJs1AxHmZxfaDO01ihD7a8uvykQykVxQsi27MoRlmGKwX3byp5WtJUeqOwcpvLbZQXZQItVxVWGZBnWu5LmUkd8k/qSB1KmUmtkrjSrmpfyFBmjepG25aGUutS1caBeZ+sSuM+2bb7ZpQVrw1PETiUsmk3TaF9IRcarPM8CY8o3hoOLRtAppA2E4uyRnq2fRbevrxSg2jMS6bD/3pNgxIfYo3xYqhwl7V6DqMxK8u0b0nGlsUhDIOixDxg2JzNEgfMJoAdkOE+kK4TMs3TYTL2TRG2e97A1Z8W9ZdILgYoVMQm1ZxcPtasZewvShsM99Prz6P3Fc2ZjFEMGI2vsY4abrPDzSjJYQy1xDKeh7lK3xdoZ1Latp2MGExKW1tKQZ8LV9iqWTWUAQTMcu0IjVAMGZW4wnWgFrgooDItbjVS6Q73bz9ev7/WHenf0lSCRj6Zfsz9TNthWsUVuzkeJyqWyOrnU8L9/sESwthDaKH4rR4LMiMEYy71HyyxEH5QWHwJiEyLW2qEuDsW2uClzTnQFFthenlpxmFc2phDWjQBKtM7hYsgAg6T0t+qp9gmqdUdO1h6qqdZ0PWZsbY9HZ6Al5Xo1nOT4t/6km4gu2l/k7ry9cqk+KkMUKKKBEdRefc0JkNkKxtLy7OzFl7SZvE53Ck35DAF+NZNNsEKJ+vTenJr5SWBdG3GYQrwrY7ZxPE2uXc7SqRj1g/th7NFJp/NZuJ19Udlh+FNBGdjP/NUxGD09G9YrZNTulSjyE37Ms12tjowbvqx9Ovk1IIoFcnpB0+uk8vuVM7wFMl1YK7nC3t3bpb9u7rYnidFOuj9OyEeYMeDZDjAQXY/SIYjPMhOCEkxgafIeg8Uvw7siT3wcWr6p/HDxwzUtTsu1al3jmTT7pOU/TSjq45pbpEbAVpwjKeY3Uub6pOFgOYK5re+xpi0vpkO+Iw5RwJrIytaS6gnHjq6lIZPM2x00tV7ZVei65FjBJuhok/tyHZW7PkMOCCsaLr4uE6D1wDMJlgg7Epco/KA66CqCR8AiktAEJ40FTMAuw4GhlMFRKqCZHUHpKo7HV095cTTjyPKo3tfZz7hjBEARyJrPgOO1RQoGD5MqcnAgEdKe83Bs2uU4v43iTXagf5rLWpFSoADOC4CqEr/UsTmuGjP6g5UV0rHSCrW/g04pMP7G0/gkg6xH55GAFin2YE6mwdlYMZNLgL0UFGAMuwAYVnSjMsRSvIMVp0zXAjOwwPrtiwHJPNIiVJjr2rGhJc11wmqsFdiIibsn+42U4kAyJhVFKNDRkYsnU28GZ8phDETnO5IIxdHhQh0CZEJgjEP9nRJdVYZkhiKGzMoSR/MNdbwMwZDcfPYKpaJusU15jEH+daWLMuj4lhxklzRXIui7j2MdMb8B1bcH1BOmSA4zMsaP6SXtjoYDQimeuio+FOsU2pD1WNaUcTRwhLov48nsIFtf820BjIweH8NkmEH6MxfAyU7xQX01wBY6l2s3OKFcVHmGttJcl4DkjEvVJkz4WXNddLUtU0D9cWtvGjjgV03K+kiS0F6eRGkcVhqLNFnHGYYFh7rxcODxy4zeBHea4RozQkJgCxYtc8ujx5INGE0BTFmYzkFWslmw2TZFHXlsSY3JiICMmalFHIpIWIb+Mls3naIn4NYsqGPOJp3nRmGHRc7FlbyOwne5JVkfSYCHNt1qCmVobg5g25INCcxRjDfHIZY1dnddqKLaW/yrLZHYXHlnRvz2gJ1wdZ8FN4ChWQLSdL2WGL0gjadiA5le933mOc6UeFzk9zG/TYOLvjbcqg6PsH5BZFquoE98GabYVOkb20rTFZpXesaVbpd6hwh/AQKQ5ZsdR7m+mOwgNIY69swctoHaYK0p7r0NSFhsuHLCg5XwRZ437TBCygbDKutCpZjq07iO/eWRl+0fLqxlEjXti7OuB9FOcY3bVEdT10z6urajVSZ+jto7k2nIM5MvX/SIaqloG1CbbHZ8DRgfdOmtCCsa1O88tOBacB6YtuavCr+NLY1oaBuW5NinW0tsGwc/+mL65qB1Nv0YEKq58/C610KUWAJdC1kCkrs/w7VGocxIqIroN/6kDK1U0Mt8CFlgQVt84D0gIJ3AdUnAHviIbnNN/w0YzHPdUwvA6oPIm2hPgnEFMIqSUILRdaLIa6qAIV0g8SS1xLsWx8rxgpVNnVe9y5ZxBQCvk3seC3BnroLJhgXLKH/E3XDXr6GtfdlhmS5YxSrRu+RWnegfZMBhNoMEJBl++wyHMkez2rqmoJqdJw5PRHUNz+UDRZpUvE+efcYBag1LAjNYZ548GrjPp90SddSoKd+yt1vwrrrgRMcm6Fhgg5CCmiN2YONzquA6E0Rv/WhoaWNili9I06rzweHCQ6cyUCQAlqiTcACtOe/0A1c2CYqwoa2dSC6cKe9AkgwqxSiQpNtj+QDjagEZcZjaEe0NcITdGkrByk4nJubCCfo8Wkm5Va2+uzSft9NK0Npm5ZvUViu2VcpS7x7bUNnBvStz3Fch8oWzqvMTXso/a1me8TpduJUsxeoXDQIVkuzzyo1fskHayXYdzeWZHeKV4VgRxIqlrrk9J5dF92RxNmtil/d6pIisH68DmU8zjBYumLfJ6g6mrGaFLdjQvN9mpHoSn7T49q4EVV7E6sm7UvzwlaaNpJuLVVtG0dUb9v4+pmhmnIYigPYuRGJrqRrO0/inbmdk8KM7NuPl5fM4UAaVSWFfRHAtzuTb+sK6gU6nKpu1TJW/Kq53nbsUmSYNpoCeQlDcXmpkCpiKo8XMROIteuHbeqHFfN+TSVqJflaCj0UKtnZJiK7ImYC00rXSnkJM3FFqfAo2UQcL2EmrtQ2mdLGZCrdDlFZdIf2pFFXYF/ITKhJn7DsEmoBxBOJajHChuk4FT1L2ZSg4E7QmrRXJocyhmP0MVYIGZ1I7IqYz0Ga8tQz4AvFsWeokJKfPpU6KmgmXPVJ4olgqxeJmUEo3NCZmZDp631sxla7tTFVrs37x3QMRum+yXS7yqiUmVjlR8gmYjUfHpP4UboKLk3f2uNekZ48jVcqpH6UpkStpIAyoWZjw6KshSunJ7krYu7K6cnjJYzFmel3WtDcj9STykuY+5Gao5FNl6l0B4TKYjggLmG4Vzi2nArsC1k5r9pCeSEb51VPpuU4pHb1ayJR7XYX8O5Igcu4OOJyM0MV1BbJIFA59m4owvx8IcI35PEv+W3awaiGLPOCCOGkZlEiQlLXIkoUJ4HqxqTN4ZlQ7VT6tqZZxfyOaXcW1pW1yG9BANO9wsAmJ9CVtyPRrjPa57MDerynMF+ucBLD2VFUTNOywso+SQuD2WNUN6UllzGI613/7rq5TXLKXaLzhrRceQRHW28df58U9scop8zGGGOMXz+/OH/5AkQPA5y5OgYMX4B5SuX8cY/uYOyDIZmrhBX3p0hOFGHTH+memU1fxPvy8sJA2yKYh8PFgzaUvPlo1YKqwGG8V0m+Oh8gSWmf0fJ7cr4YV71RTW38NLaAmjo/YD0HZnmoy3RU4gSjyrobdnBdC7ygR7Ud+PaZ76xJef38GUl/U8o312uLpDnEVg3FKtzUk5HaOt0zUakANqhqFN4G+I6mZ9ceTVlNfRGuryhPu3uq1ao4PgYVsZHwNJWaijvlhFogL8wzvllcwY31+va23T4FCsKmvKNx0c+jnf5ihrfMtMr9BDBrHYEwXVvTqdHFKWt0YVojbXsTqoASGei2I/P2NotS1WXQlsPEoDposjNUAJ6L6sMYlQwaSLPtmhmcsQga2BZgyRrxtJ8GRqNEHYO6CgTPtLtMQHwRKuTsTyGt53oh0SdrS5TVQRRv3RPQUxPHtG9TDuTL0CHbtoO1bt9V0idu5zDHD3FV4yw0WYrZb92t4o2c20Yn5H2zXUe19vgelKEpjpBEW12Ety8WZuYHqtbG0EbNamPu1Rp6gqYOpO1pjFobQCifb77PUP/htpHMziRTpHomKZq00kZni1Ay4BOQoMT0l6qtSUj7XVFfDGNzE7ODs6Xj5ARn9oxEFJc4rL2/N7jRUt+ypEFcQkLTg3kETkfypJCNUE8xBZtAtKeVa23+HgMpHx7jROGtjEH0pJCpUDJQoUxf7FDMUPCeACu//jAVPitqSMBMtrXYAoW32LTmi8KmJI4K16Jnoo/qd6GXArWO0zuRWifoc6FNUse6w8ikkGFNW4wM1/d5qXA/b1rjRWEDElWIFMKEB7nd9waimqLQH6snhdwEDMn/MPc4jqiM7lEpXzstQxNyUuREcWhk3mfi1hNEdnXw2bddxoZpQRM76jDCPKvLPFm//rVGYg7wDcXBzTlKDXlWP54jYlHWRs+rGSfX5KvlmQSOvovK+G4z35lgyUafWGv42ys5QaDvxj9NgpYwl1/369XdVtMX8+7MnCN903kDOM1N+1qvMLc7jvTEGTnSh4uX7W36oDo2dZTfG+Y10bWkRfrSQECFX9UA2HcVgCsbrqBsZ7VyzpK/aubmOE2ghO0z2E/Qdqp9UNR2vAPqt92Thqa5d+ToHry6E1cfYzJvFkRdW8nBljNY+VjUhjNWusNRnegEWIjDr0q1lGYrlomwfO6jqvRHGvLbKvucvc8p+BzD+iQ1Ws1kospmhGPJCFXZORG1cl9eWUNjJABW92Vcw7DqkSxZ7VCFX8kPX1QpDTCWfEJ2J8WazwADwie4L5H8JSg9Uj0WIDPbvjfDsmSGV17NUmWE1V/NWmfCAzIgCI2grHlF8jxr6owizWxrMi4VgiDTogC0GHu+OkNJgFYypOm02xzQkmMaWaurhbDmAdP7Rzi2s0l8yBp51hjl2aSHseWTBWkj3xNS5tPD2PpKKyl1lP0k9Qw7a0zkKRaViSjnWlzlAeOpjXCsGcF4aSMca0Yw/scIx3aUpvulhTw3svLgPODAMAqKPEmgaPVgtp5aXQZRuZPnmVJ200ZAlpyOKQphSE2QIFvReuafgdmGIh7RubWyOhB7LhcvX0Gw4TD2fF6eywPqNfhwGNsel2dVTQM66Rsz9t1ujmbJrj6WGEVrL5eoMpsg2c4y/KTXfpoZAQFxYhG6YMR6NFsrO+LwNmiFBe15i/1KXIxp3R9WE62pdwSd1Guq/bNfngHwk8EC871DVbxy79yU7wALsL8QpPFKyi6dTYYeCWRtD6S7OZjtWrqottIRKi+nJ1AAvFheFBBaPRIAq3I9AE6HVmnwXJKUV12ilRBwHVo9EtD6zfqE5BZqn6asEN0LC444KfBKsjtVZgI8W4+yStq3jzvM4IAzXKIay0Ndld3MdWwA5iBD7xgIgBOBPr8MbvGjfXMv4Sz55bH9lirHsGTy8PL5n6ypdCAAXIgriBHAbtQMC4aZvUs+BoLhJA+z1OSk+Rr3CqfK/jxzDATAie1MgilrggbHDkhtEzRLdhHAQWIEE3VBZtxulZjENI2K9epYhAjGEpAerPYQ8T0hddfhAbYvTEiEEBGOZRvyxhxU5gIC0hVAw/Gukrzmj2KmwN1IAA3Iu85LAgzKd4CE4wkRDLKEg+N3XHvyzoDgUfsNvC2G9KYNIEEOB8evBAgfWcIB9hTYTgLcP9h8Yb0zsMQDn38AOQ6IgCzXHrow4ajxCoYaw/k0BkhWAA07ggNy5XDAIzggwaP2645bDME2vSWYgExrULvkcLBzISA/jZvCavxKlEV5GsRpYX+aJYeFnbshebZwoHM3ID31h0wU2c0cfEiqS2j4tQMk3wHypGnWszBpIoDMWfo3e1ZUjVZSJWzplteo/xnpvkGwxqvKzuGIcTAwZm24JCS9HhGEI7/jAkVwgANhF5J/JUlsPpzP+Y0BYRjarSMW/IwXEUJ2Nl7unJqhiyvhFZl7t0tikZFrK2Nm4T0IqJk5D0Juqfmaek4sNVpOS1i9BKT1EoxXGRc4jc5fPQdjN0EE4cgiZ6HodWBQzGgULSA3DgfFjsbUArLjcDBzanvTB2xO7eFA2BUrjwjqUivU3xbc5LX23pYuL+3HtlZ4ddc5wOaEAQ/GA+nuT4D5ICNAEIbDZQooihNEGF+9BPQyOzCY0aSJE5pNaS9/Gld7RJlAwlhhCDeqcCwYXkdE/u8CzgkYAwIyLPIEboiZQgKxZCH8dvcBljynoDBMbbZTFwwNN1PFzNgLzt7FmTzfrja/CSQQS9ocQQG4DJpCQrEcXV8CJDpFBfJs1rL36vs2Ggl9t7kd4cZtjgW0Dv9bgW7hWnaEB8PP6vrCgp3pFQYhtwPgXHwAnIuPt9EezgXkYDB7GC0o9Py2hIVhi9O8fAx2zX6PywAlSQ434UmwQXhnuA6qnOaKhlvpzTBheMZVHdzew3Ec8ED40UsBUNw4FszOgnm+tMXGglHKNAmrsALcKxrgwNjB7Z/2aDDcujvPYPRGgKAMYb3ABSosV3ZRG5xrhwrDlSwdzy+fAxrmCBCGYRnfoRoHKAxxBTedLGFh2FYIuNlHgDCnJYDLPcOAGSGvqoJTGseC4hWEKDzCjToTRDiO7EC5auIacHtdBAzHOM9vY2C19pBgLOuY+pygLAdIEJZDGhUokhNEGI5xCkqwhwNhx19ABaM3wgPhx27kQpHrwMCYsRuwkOw6QEiGcD14DAjIsILbZBoDaoddAj2OSPwWoJcRWV2tdUP4+AMUzM50GztKz9jqOKNJU4q8Apl+B7ISdBf8iUecxnV854b/FB2Sf3syAsl5QITlCbLlOOEJe542uY1jmU5IQHcB7IR1VuOHunLUG9eluKyPm965LgWyPgS+tF94jqj3gJAswTbCRkTB98L66yC8+SDJTkBB2cZ/wJpuDwjKsi6bEHQuGRAhea6/Z2lAU+utS1WWdyhpYGkOiM792hO9H8U2LcgkkuFM570n4UNS/BCyaooiN3o9SvhaHtQD5xTLi/BdHGKP7romuH3dTds+xirzl/XmQSj+ukTACBqvSryWmoeS5JR1Wxf8VEs/vefsVtRL6t9lwe7eJtPOhbClUP4YnUgOzBZiL9IV9RE8DGMcmDzPpcS2hwYN5mCXbwKjV2lUWIvFgNagOuonIdGhzvGfakTQeyRxRV/Eeoye1dnSFX3WcIQNFSjtjOsIGyyU1h3ZETjUCGVxbV2F8kIC2CUwZ1oegwNeDHPJdwQPeFnMJeMRPLR34ITxBB72sNGZmhcSoEJlnTE+AI/GPITZGd8Z/re0lO70sVzd0kAYpUffVeDgVigxI1aXBs+hzJtqqCF37afYQANOPJITxFmsnzNSkfdMhAv6/CjBbQUGIWBVIO4FDxEIIlzgLMJZGAOsuQQVWREFWR2uq+60jyVWd1MbiSQHlSHQNqlVtaqzlOWoQmQcpIGPsZtusybLQYWMsh9rVcM0H7ICeZPEqFrcDVOlKlCv4kOG6gZgO261AhMxDqphmJpaqw7myarXKlCXKKsIKcLgJJVZlee4YkYpjo2rZZoAWbNSDkeuFWmOK+VuRJMLc1Wlk8yV6wIhq5YX6O/NCaYeoRwHFTlJ80hluWsZp6O4TJS76jgc5sSCoHblqMy4roLsLr2Er8AC/h/xYGYe8NvsktgUba0ZDKNIVhqB7oG1dP2FDEMDMmsPmSqNY6eFeuxeEiXTTVPVODJOoLul0gr5a7LMVCuskllSBgX+tvkZ7OxAYa8LuM5SIc73uhxUZEWUq80HN7WYSXC39HVKHzjF7er+ktOKLGW53210XiEX59uL7SentYDM2DV1KvdNFtKmdjVKicQ4qIZZlLBWFWDjhgXLYtdNIZXlqkIuG0Uo5x/heLcLt/m2znaDgZflGrDH6RaAU2SoVeuA2uaMcEa6h3fAHGQLZI07+MbHGLxL+uuO/kiCA/4Au+1r5IG318fQoFuCa3Vwth84F3JEWZTE1pElW3UZi3FQE6jDwrVquDgpHOOjusZV7dqwZlIgPZBORF4h/VwN6zUQ4rviHuyId5Pi2n0tppL+ETdoV7wZKN3xINMpsoP+F+F9nFm7zSu8xxIc8Idw+tfYQy/DFq6eO+qgyc2m0P1mNAsSc1eFhRwXo18nBLgeUhkuPHub1PLqzeEk2fxMhPM6OGUPsfm7yh5651e0/nFHH/rhFsESyB150/d/FZiD7rmv1cHZhrt06eO2LmMxTldY7qoxkeF4heWuFjMp39LWqNKnah8t4glKMrHvldYRltup88iDvKpLjNKgZaC+dG6/Z0szAYZKs9nk5NBbdG3UWdmYeZ2p0QowwOs8T7Js0j6aTSKUqasfVZWYdZUKH1RPIZZlH6sap3cxvrdJ5HJ99euvV18MzkEmym1Rgi83N+qN2tbdH6rhtyj+FEx70BQxU2/2DVZGw7hUV1CsOjB1ZieeKbb+vLDtOInJmiRI0G7rlXFRWVKKTNGl2kvny9fN26dK9I8IBbet24dOVvvEuK7+iLw/5eFP4DRaWkZq1fj0SbmYNGYNwx+PsW6YBB9Q+OgViObF9lpUQ3VMKfkyZOPmaiG8vze42XC3VSnOEW2pqQYRafLTjxeysSx8R2MYmE7MzYqBWDXViIY/QwPTgOlILf/D0v/P6zzME6kmFyX+iA87vKX55UBfk25mNsTvS/Lv+7xUKy5sbYQrD4c76djeacFvK+cztn4v2B+VN+6CYZh6BFcevbBFYgxgzKLFDtj7gUF4RFmG5dvGW4xkYJbsPAbo4YeaxunKA2nV6AnQbPm13T3JqZ9HmqTZeQcsP/xVpCkFNXcf6jLPDl6JsihPPVTExhSFSK7H+S45rs2KKY3DMrddMKGV8zmx5jrqPpPvI90DuBMlGOVZVC28ZhwSldTyDXexerrsrePi5q5NnNGMQF6Ndol8Ll8nssCwH+XNiIyLG3M4FN76ofI6h3Fx87Enb8oQe2Xe1KaNMocw5oLTHRlSbVQyQzCfP5LEisekPIQ+vM2jY2XFeGYnxCJuNmQANLP+VLOKQgyeZxbdYiM48k3QDR7j8nYsbBjYjaXHONH1/vqBtCtraQsoYUZdY2/tzoqKTSyQbK20qR+98DE0nvREMJac2PlZ6G2kylehtUAyZpbiqlq70brOZlTanEGT1LGX4Zr60qY85hiWbIqjqdM2KW/MgtfEo17y+snuOhshjsWKiWCTiWwvf1Rqw2eZABjzYDekvRDTGdWQyRzCfk3OcuUzIabeixjJmFmJ7j273j1DMGZC94fS1NRoRqXNV0GPZGKhG5ZxvXaDemMZtACxnA9GGwY2k8EUxphTEu9KVJqOe6PS5mMvCr0EPRp36kl5c1vJ8zrJV26ob1jJqLi5b7e7xd0DFTg7xJlpFxYDge7w0X04qxFPhGZuQ/sDMUVTA+oLW61hzTv0qLTr/cNDYb6xRfdA8luceRvRvOLKHgpfAGCzn0PkPDwabXG1XGblbahUcXZrwWRa3LUNaD9TJJzGWpSmXL/kJa41f5VHAmQ+fprY5eSFIDsHldm2iTPIOcwALInor2YmNGwXM4LGpVsviKyUdF0xublMEF33m25n3rznLOdldqil2Xs6HoJpvocD9EG881eXt+fQRKegkHQvXjugOwWFpBtVyLvDZbx/hOYsQIYmTqPcXdCe4DrSthM7keIbVyLOiFTiIIZ5ZDdaCIHc+xqKQYzCurcj/mSbz0QFbWjSCpbtviEy6gRtOOICAYDMxjsPSnRMnnHYbj7tmV/cchCbvaSOFsuZmbbA1jU9pBWfU7hDZNVq3rPpkpcavPzVC3E1STl/Utbu2Nm8/TsiJg1vpXWVSMOVao8DCNolp9cUEdIOSOh+WINzrYsUWdw7avuK/gEmKeFPyp4qsFWiBFSobekAhZAu5nc6S9gGk/GYSVKVoGe19VjRvHVI4XFcKCvsbwBbXoQRojMrCTTiatWJj6GdUE8wLlzw7nBdkN7I82ttJvb3pYTobPvbAese14muyfphp7O9paHtETQM9Syc4MN0SBGokxDWrmaL+N+GrDPtsz628d/sbihYvqRJ5DuDNlR5W0l/xNGXwkNdrRegmxm5Mvlv7D69xFDoybd1rPjeI/2mdZUNtcquHc1xvrnYcU1lQewwi+2Z43goikpcVV6IwqM8n6BI9e09r27vSA5oOVZrB8vLuTmzCElj4T8qncNUefWxR6H0dpB6NdCivgjGsmEYVidVv3UYrQUGhLHwalpSk0EBMtSLaZfTM09MLWhRnDWpTWt25SG4sNHdID5LQEsA9c1NDji9ePXc4uyQ+KEoizyWGwmXldfdzzngDJeoxvKHo0QKZGx8NVCLA3kxvFZTrzO1C7iagx5KTI++8/s2JglOr1Jgi811Ikh3p39gOC9uHh6Myxgl7UWXPQpJL0SlnuPfEpLimIdTh0UbTKE/tbaUlgBwlmY180uMzM4NkDAOc9IiD3qroSm/EQKYlRnQEUHY9j6+aaE3d0064BgBzrroyTTEINvhwDHTjNRc4QYQtrnEZHHv4LOqBBaONz8WBScuwwUeB+nKEpS3GBWO9fgiCSxzOTIc+z8SvWWThCqHgbcFCHIg1+iXVW5j1wkPEAVO0Fyflj4Qd8Z8+UFLa9WZFvC7UsYtQAF09+57yfr780vhBruWvXyjnUoBhQqlRYK9kIwMeWqx/TEQ20B0bYq6lujoEESzR0D2DtoS/KTAsKNot4J+s7kLWtg1cRKZhS1M0pJ4/AIaWWOUZI2RXnqHMPTwvry8SA8XL7wSxYnH4scCFrMRcIcmqOoyLgqsxkHY5EIeZ0jRBpgCfNi6iOEopRP0537n6OlaRMBCsz0g6yECO0VbtAePO1TFoUodzNUtEKSpbk2qou9Pp9EuV9oplDqXZaRXHcKSIqfQLhHY3X6nveUJxw8JE03lQ9dHBniKtjkUT9gcg3DNFgBg7U+Fn2L+VGNtNzsaq1SdoL+U43wEYTup9LiAnQA+5QgiZqI7ggDXRwZ4irbhiaTvUBLTqHC3Ji4Rpql+A8qyMqfQ8LCJxM+qTm/2cwqaCgerwQLpVGM3Ffi0654RA4OhHYL/HOiEtl9eXHhVUxSE+RM2g5yMWX+ArNUK5inaqT+O6eqR0IQX9PXHJ2gnORnNdnJQqxXMk7TT+DbX0/UkEQ3dtoGsiRDtVHOLbk3s5hEA1ZuQlhY63fqBnYs41e9SjtHCQJWo4HNQbQK+wyH61TxJXDyNcqN//sH/S540Ka78n5OG3c+sjn4Zh6iMcj+lbxeWfljiqPLb/RO/Q/HlgtovA1bMXuAcTUEwnkXPWcnFyxC6+RE3o3Y0FsivR/Uwm6LM6zYRtV6rL1dv3n28OksjW0WOgCbieOq2z6g+WgkAh+0u5G7CToab7mmGd22tr9v3fT7Q532ufvlyefHx14sX/vvuZQQzbN587LFILYQjqeU9og8t5eS/DAB6Cl2WRvr2li2Cj4rYFOXtx+v31/5bmrLBTKm9SqIyvqNtP06KFeR39AHa+9xGVRyYnQvY4FS4HI9IcRbXNnA4jco7c1Ma4ZApEaJex/1DPn5A1xave8oUoO0SHFFlAZBqX4ULjigBRGsbMaAjGSjF4D7KQUyWQ9JAIliSw8u1moCyB53J2iWqk6pLKAqMyp1HINTp49NJwKUEfFvC77JTnkSYxWTEO1n6cPEy2CfEQILq2NRRfp9172gL/hTgpiL87Ea/7C69tLHGApNl7RGXML2Z3WmxGAEl7xbDoAD2sbkpFRUKuA2cSohfNDvimJm2WInI73y6NAzo5GfnKc7A+gXrLjF2EGWQMcb49fOL85cvoJH/uEd3UHTDY1z4bAXe/vPhcPEAxbcNFGQ/U8sJ4DXSSigQvaxDrK0ibmTlErvXEXANBh0Vta57rtYpMeRYtyGhKq0lyB7eNsFVfFPZcoFY53kSHlFs5d93s1NbbxAkmsSojlMYN59FhkOuG8jHCXo0QVJ4rBEE0sw5VUnOAwDJL2cDVHOUVg2wosusMiBk6XMA1izbMHprmP5KP6ji2pQsMOSG9CyAU5YdOQgrALEkMzpTzdDUikw7aWPpOrHMkHATHIiGwID0K6dwA2WGSGSU+Tu8p/t1xF/72u+sf3xzc3P1JXj76ePHT78Fv735eMW22O9Q0tA//39/b/L6f3xkO9Lk1+/wHf3nm7fkn88ffvnll3P/+cPr589ftt9pCv385dO739/eBO/fTWS2iJpYf7n67d2nL0soylET6t9fPv9TcPXvN1e/Xb//9Nv1BFAZ6/P1m+Dtl79+vvlEqvn+txsYlODt1RdDqC9X11df/nL1LnjzlzfvP7z5+cNV8PHq46cvf53CPX8+nzaliP1Jws8X589/OX95+er9xxevVUtffwjefP5MbeAzqdP7q2uNgj9/evPlncxYf/7y7sX561dv9UyyR/1y9RcB6Jvn59p4pE99/vTbFW21NzdvPnz6lVSWNAK1BUUM5tkHX25ugjcf3v/628eruR09X6y35K318WeC9eYXovX3pKqfbj58evOO/IZY1dWXX968vVIFevvpy83Vv3+81P0+uPrl4+VF8PH92y+fDMoSW1PuNf/y1+ALMXLl7z+8ufnl05ePwb9cUZUImr+bjrwjpmdFZ0dtY/hw9eubt38N/uXNh+DqN9b7/u3Nzdt/effpV7PuTFrz3c2Ha2Jkv/3y/tfgl/cfZn0hqf9HtzvaZbP+sy74aPhZk0N3tdqVr74oohraVMGH9z8HH3//cPOeDAg3n95++hBc//7586fZaKc8NnWwv9+Qf3x+09EXNTCpAFmvjzYakH4taBf9/foqeHP90ZDv0NWvr97ekHlHYIXDR3rm9/b365tPH4ObT/969ZvcxokSJqvYAO35L8yMnurjt7+ojxO/vw1+/v39h3faA9pvbz+b9aEWgUyL7998OCdqvX57ozwLXf2v68/B72++KA/mv30iFvKzHjXWIYLfrm7+7dOXfw2ub768//z5SllBpA/8r/e//nx1FdAB8frqAzEsAvLm7b8K+9ZcbfyY6Jc2EEPtFIMug8j/6H8Hbz9eXgaHMJwHKakDz9ZXqKnzA87a8yvyuxInGFW4/7nfgqRSu79uSO/uJoVzF1z4mdJHqcpHWV49zjdkN/Xyzdw5NqR66uu4hjQdXGO1YuLg4qchn6e6C2lI1+3FQYtOAHL1zrQNn+g2miFdh7e3DBmd7nqThYWd4gaQnf6e6DqMIemnvRtiSvrklyYsLNbZFQOrcRIwOP9HehD6qeA+Ov3hffQTfdy1++1ZE571h6XMNc/Z79c+OwuLZuLFtxy99HK+7nIifT+Tvi/uXnqLEx83opMc1QHaxRMCNJzLQDo9rSfmUa4L7746K0rcn36ORI0EfoiruhfakSMrbLoyGUovAuz9QaBuFVBVke67WYf+s2+yEnQE3KpB+w3/13VIRqd6YgL/hfTnv+GwHq0X6Zc07D7Y0RVhcgIL2TdZyGK3ySBft8E3I4p12cyPjhxwIL4PApWv0ToZyvIgpMv2p6h5nsZ1+6xPUOQsQfATkCAKwA8hLp6q+Yn8sq7jEzd8l+L0IyrYhHv6eof0XbUsYhPBeGL8059OIPvhQSL9n/7pfL7B5kD+PSqzODsQB5I+5nly1ffi8UNdoqckQBxWlNVxOPVOUFKdkgQqcYDLMi+rp6BBv0jjP9gVoon8Kv5DTXyKbjGdNklF0jO6k1uj8oDruXzJZwvX1EvJb37SdFAtOdTHJt3NWPDfuRc+9469lPzmJ+4je9H5q5OQEPrJhAr9vUd+/5Oyz7wQMYx7m2yGT2UDtFfV0U+qo/QKPn0/VZlMUcjH7JaQ8sANTEk0fXv7LPfa3z4JIYk/w2iN/3Y6W+rG2mA+4Xn/xn5zWjW5ZWOkl+VM7P0b/90T6cYZIx39SCdH75O1/epqxiUXHZ3I14venv7NG/52WgWdjJiOttZX+N6++/uTae3kBLX63/oK3dvTD/gzcP0HJ+6Wp6eoo8FvY/sOUN/faIWk+6ryL7/Vqoi3cSSfyTdYLfdVLWmxncXFvqJHVlQ4rH6ifz1j/3kKLt1mV9D+HKSomLL6d4739TvvIyp++i//9dPvN59/vwnevf/y3/z/8l8/f/n0P6/e3tBA5v92xgorcG6Prs5iYh58p3tOl+fmyIupO9GfWO0uzp/vabx2nC7itTW7+UJTcRVq2byXhul8N2SzDF1v0FO6Os480fVyLYDKCGHD3jbL/xth0GRNhSOvQHQOWUwfShBE3xWNgNrHOIk8FgKIkvgPXJrw6cB2JQqxCQCN2kO1ZsF5bgitMY7Y4Vl7sYd1iCq+vGjtM6rP2jjWiB3nsoPDs0PWnI1mHTpqzQ14BDj7uv3ojFr4WV4fcZmQ6vxfYev/z1a7gsLXp9Z0374UiL0EZ4f6+NM8Hty1rdOtGx1rH3//RPb+/4ztmzC2GJVn+L5gE75ssn9/RbdtPnW7flrT59Iy9PlRbvQXPBKmM9y7l2cvzs7nZFcKcEtHUcQi6FHye4XLp6mWqkOq7Wb9W/K95x1C2WLfjOQxr2pgd/D/mu7vbFmqrfIH8kMP593H9dFji/fTemV8SbUxS+nChXEZNgkqI1zQgPgsfDQ7lf52apSRPhct1r/qZ8pmXoR4GLLB0mmaH32+0mU/fffjPz+kCf20TT5EPj4/e84KE5Sc3gghv/r95hePLG7/uQXoVsr9BZwmPEvzqCE9qsJ1U5xFeI+apL7Gdc3O7vmthDY5LbseRMoSlAKX9eN1SP5NQPr1t++OWBN+bj/RpDS2hHmGbL6DIxompnl3z8Iy7G5DhmUrka8l+iYiv+rfNRZ3qXE26GlTPvv+2fX7j58/vH/7/uavwfXN7+/efxpdCX/2w7P/6LYz2np/ffbDV/K7r1+fVegOR9d1Ht7+BZUxva1U0V//QP9BP6D/7xnNmfSpyLoff+j+Q7IVwv/6ffcfRKcxERLdfsjD9nhmDnTA4W0eVNFtQFwO4nTwX/9n+w+C9Oxda1j/YMT/k7RMC0KjuCrSEP/7P2gh3s4EiLbz9729sVQVtIqtsbB0iW0Sqe4uLVNGXsaHmDhY/POsSRLyW95dyC/Ov2cINab5cH740+s/vb7808vnr/7ze13ho/S0QRRGoQ0N7+VLouf/v7Rz21EjhsHwu/QJ4sSO7euqL1D1djVige0iTWHFoN7tu9czBBgKaifOLUKaTz788RzyR1kdHOdPyrtpV9bZJ7SJRAMkzYRST3Lzu2ojAKREKXE9wZ2XWxOE1T0hMeZYSVHSMcWijSAxKSfJoZJgttH7q/3ivDqjxKiMyV2RZetkN+3zbAoFJmAFIEdNTgbg48VPx0PfAgE5B805U206Hja9t8hVohhMJsidlJ/HraGcHVjbqjOoAnF2oMyVczLDbgLhBCJR2AHy1JHBqRYxUtZYC/G3f4anMEVIY4awuDsub9eu7VEm0ykj4ytiTxayNWnmsFguLhDff3wzmSpOloMz/IoYMZhCOEJwtdHsbI5Z9QdfFihEjDmFxeowI5j3w7q3qW1wJQEkJWVA9hTC9vcYgffVfjM+rvJlIYipI2lQx/UvJvYFwVcIoCBRMy9fMv9F4MlBDLZioi3a2EIwVcJuP34yuvaGAkVZQFN1Qz6ATP7jMxyXQFDMmhRp8Tg1wyke+LeDNG4d83bwRQdyIlBhT3TuNbs7HVf7YXz81pX/+FZ2xByienju7NSvLeyT8cCqCEzV5ftg0NQUDbsFMTULJJ54PDO97qbd72eV84krxBTZtM0jbsPu14eNwP1209pGYEUrQQgW3yheMMr8V47QWK/6/nWMyGY3fKxO63dnxdhMbj0tCtV9XXjGozKewfgmIcMRlPrq/Q+MJzLCNhEkx0jy8EDMUyVJI9sdXPh8+fL5B/2W4F8==END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA