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
 '-DEFR32MG24B220F1536IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4187C"' \
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
 '-DEFR32MG24B220F1536IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4187C"' \
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
# SIMPLICITY_STUDIO_METADATA=eJzsvQuT3TaSJfxXHIqNL/ZhFlVVkkettXtClsoe7UqWQlXumd7VBgOXxL2XXXw1H/XwxPz3DwDBN0Dikbiljt3dadtVRZw8SCSABJBI/Puzz18+/Y+rtzfB9affv7y9un72+tmP//yQJt/d4bKK8+ynr8/Oz55/ffYdzsI8irMD+cXvN794r74+++c/f81+LMr8bzisvyNFsup1mkc4IV8c67p47fv39/dnVZygXXUW5qlfVf513URxfobDvMQElJQucFk/Xofk36QcR/v6jEB/992P+zyJcPldhlL6xzDP9vGB/43+NU5w97cqCVKc5uVj0H51diToTRmTP9HPXvt/yZMmxZX/c9Jgv6pRdfTLOERllPspqmtc+mGJo8qP8F0cYn9XRi/OX/1T6LdwvgjfFxEpKhSE5WNR546YCAUIqVDODxcvg31CZATVsamj/D4LcFOhsnaoJxWZMsItbBBncR1EYRS6o7kiSUYuwQcUPgZHlAT3UX5wx00uSEbtuH/IgxRl6IBLd7wkUmSk/ogPO4yJpnfNISjKOHNodauyZASrBOOijlOXOhPKkBEq8nvynfN2lIlR6ZUJtYGT9MqZJBm5EsVJ0NTkH0TN7pjJxAhpZXfpJanLHjWJK5uXiBDS4Z2DmaIjOhIRQjqoKAI+88e4ckRIKkTFyHHanMbGp4KE1HCaxLuAOkx8dHNDbEWMkBafo4hfl+ZZQGd7N7zW5AiJXV/9+uvVl+DLzU3wlnwJzUcAvzH/VTgk8uvHIImz2+AWP7oyeUWRG3R3cUb9/KBGuwQ7ZyqTJiO5y4kQ+l1d5ok7djIx8sGDdpnq3unUuBSx0ZRk/AuIrPDWeTOKJMnI1fktztw7OTIxMlphU9V5yosdMSIrTwesJFI2SJFaNHsU1oQL7Ssnobguc8PyDiUmn7d+pmvbE8uS+vxxWpCRJsER/d9zh46/VJCMWpxXdYlRGrSr5Tsy57mjty5sa0rLm5K4MGXe1O6nCImwNa+ErpyduiNTASou5fFk66aj9roJnWTZhLZI7WpilhlBJcMOaek9OB+hADUqQXURvnDOp5ciJJXucFQnrrzHJfrmOHXXON1JlMppif3ot/vDor1i1NT5AWfSzeLWv+uWz7TbnIVA/LloXypFplayYk2IgDom65z6scBgGh0xEskQ8vlyc0XWN2mRZzirK3AqS3ipB9R9FRDaKMnhDG2kFqEQlVE9TPLwtnJhO2IpMk74jpI/oixKAH3CEZ2FADUmDhSzELA5TLXfwhuxRIYyHwfKEcnY5MNsLM6IxCx0rKalKDV2rZM6FHSpOplAM6bOtSkQuHGAtcvzOsnpSm4YZfa5C6abAhX8nIDjgfOTi5Eep05L1CXKqn1eps4oKspUO05yOCZLBamdkTvT36qk7YVRjDF+9fzi/OWLoDg+BhVOcOjEUVQQqcW23YpjM+XJ6M5kKuy/OB3OZXK0eDnozzI54u2MOLslq0Pym2CHKnyWRMCERAJWjxDbPV6UJDv6H1FcFagOjw58ThWBZkyhjU1F4BpTfrR1QqWuSzTk6kit6xLXuGZhcUqlrogzYelInSvi1lii/eQEAH5kXJEj5HUWln3IFvlPYDYL9O0dpwMOb/Ogim6DF2fnZ+ejnafJZ0WCauq+9X9ffPGOcRr9ffHFdZzEhNwHtKsmny0+vPrly+XFx18vXsw+WyKyLfbFV/NZ8rGqcRrgfXl5kR4uXkht9Fd6IuMf4to/EIX4XZ39tmr+iL/fU/RbDr5IiL/BqyYroKZwTUwkZc5sYiFSdb/PwqSJNvXdS9pdXDzfn7+8/CFOX7ySdj2jynEqvlzWhvL7ggEK08IxuV6GOqkVLw6K09SB26aUNg+he031UpSJ7ZoSpa559UJ0aNXhCWi1QpRphWnjmhQXoUyJxjG75tTJUCdF8Ne2l6B4DWLUqaWIlKvCMi7qXO4YQjFcSFMnWiSJc3pchjIp7L4DYM0O0N4zcM6ql6JMbF+VofMu0AtRpnUoiHfrmlYvRINW7FxZnQwtUkGRuzeviSBlesf9CSysF6JB6+EErB70SMUXzo2ei1CnhNxP3Z0MdVIhCo/yOAQoWr0UZWI0ojlE8vMCIGYjMcrUEjLNu+bVydAi9bBDzl2csRxtcjRQJs7cr9dE8tTJYnaXyznHQYw6tVPMAIn2DJCcYAZIdGeAFMXJLn9wzWskRp1agY67EyzEx3LUyVXOZykuQp3SnfO9FC5CmVIRZs69xE6GOqmVXXsoTqXeYEq+D6r4kKHkFNTGopQpkuU6PSEpUOSa4VSSOkF8orFsIkidnvtNgkpzk6AiTt1KbDkUq16KDrET7HQOUpSJncTl0Xd4mpN4PI2+y3OSXSj9Tai7CDm3rk6GMima4MI1qU7GFqmUhyc64jOG3zrSnJ+COmEkkqJ0oin45eJXy1/M0/nQwI61c+44Cxfn29OgLVTnaaxv061on+D7ExB/VRjOohhlZMGo76xMBU6BtoQ2+iuBubhG4OQvVFlV2GDQmqmyB1kX1payFDaArAurkriyrViPsSGqRnVjax4DiL/WvxbdpSo3u0vbPLrREZwewfcnICpqt5PVY+hpos7zJDyiOFsPj1kqTBDyzbJ+mVajJ9JVaIDztwZUheFQRjco8SE2718Da26RC9TFJCHWGqx8FcGHMCRLPRy3ExwYgSXsZvNpz4fDJYm1OXGa1matv0+/1Lbggc4slY6/BNbrnaiI13hPLsXa0Y79BdjKmDW9jVvVeYkOsATGoCtExpc0R4rWNeYZEznqhk5wWeZkBWTQnQS6mICpNgaE4AmYshUUqKwMlr6rRjBgahsjKI8R6AaREle4Xr8kpsFgirY6dmyNmW8/Xr+/Xhsu37I8o2uzqiwWc6KEMK3iit0Sjtfu18qUwGj6lEu/9Foirk5t7ec8Jyuc/BHgqvj6D5YLCT/o+9EC2VM0hXqT6ReuzhxsXSxNgxaml5cgYsdgq2LToglQmd7px/gKxE7A4N2VqIzv1l0VlrDK3gPv815pT8UtRZ8BdA74BM2NDz6I0G3FCWHuek7Q9JtxUYMER1F5Z98sBMe0QVoKXYtwoC3Pfrg3ByN2iufGEii4oQ1wstwIOJCGjkDETvHgxxDiBm3ddRmlwVi3WbWeucipoasnTtkfQQ09VYS+1WbjUkbx24qMZHHba3xMInwV6Ugie7cyaLmiIwshXONjFNiiyEcW0LLGJ7sz2ApX5NNh64+TalP81E6D6kJ7kBdVpJ/zhfCaPcEhqQFdtz84JDWC1+0VDlklZqyY/Tqi1GEDOGfjBPv2LtoYzbjyY5Cu9nPgrZaYvBtAUyda2McWnxH+Bq14Wo5tjGRI/57NCrMVEW48zknLgFSETwZzYC3dQjJZIgP0uy6jkn2f65CM7bsD6Gx7DLjVz/rEUGzRCE5hgFUmQtNVx3UavIInM4FWJlTiGpUHXAdVTR8iAme1xFemRpNcwhPqUNVpVG54VLpE7two5E6sEagRuO+vpmNeT5yPvGNAzQEAmsIAuzn8C7opEBsZtsGYBK4fM0KNSbTgJpdGEh4obypIHgJYzREIXiGygMmNEQicyJ22Rkw2k1WMFWxPuUuKbulADaeFnnkIQFfxgRWbA2TYq40gKuShonBJqsPXJ0bTyUUoyTOnSpuJUaXJw/XgiQ3A62d6pfkOwEw4h1qvOaqwV2ICFeJ0LQ+jbsUFuKtEohgdMtLnLTZAZhSmiKvCcbojLVQc9SPoJLIngKuiWdL16qyCkTugrQotSc/IzRd/M6ED2vrpPMuY1paFEj3HhHKW5+c/8WLOWFSv/cp4/htVjM2ADM0fQM1SeIV5WeOH9HKL/KivAFWgk+xPoc2qsZyyFzU4wume/vuoqfUt7wCImgzbzDtwSKrDt/IOXPKbitH0DuCJyW4JyKlgXJS5+fpdTmUAXqWCKjDhHGq95k1dA2q+R9t0xqDGQ3Fyodl1s/TyIkjjsDRfR83EziA3vKCLhwePxT16Ed6bRx3MOQhwN4i0j3yuPtijTWKKuUoAdjLZFMdGpbIp6spj7QUlW4S7SsQkJEciWyUWiJkl8Jg6x1QgQF8YAjP2GeS2eFDBmyI7FG/jXUZN4QJYlQUIkPQBbV1oNwKByR0Dru+SOVgcbIdD08WRN3kg06NFceWdQ1HZkmFKEGwM3JKhS9AhL5W91tHDjnR0P5TtTZljnltECc7Na1sM3G7s8GK25aYsQQDavBsosT2GKbCD47iRAMsWHDFv/eqj6kxIPyVuf52HufUwJWAxhgbdyWdJEyDCxCeAANtTDGcIHl/AuwrlngqyHxHaeiyHBXGyCpvWpG8+wAwDBMm2BTmZfgDgkI56f2aQaF1CuOtxmSiruk3zFGSKqvdgXa2Fs22mFmXWzQZop52Mi7FsN16DaQcboAHab/LqmX37TeCM22+C0rXfAnrLG5o+6GYXTrbNSD2wbP7QnFUoqQIxjWDSaUFiaHeoNt9pFXFbkeGmP07NBqYqvD8uoE0s0h0jjUgnd0oSYAOMWm3SJ/vhiiePWn07e6vWLUZ/JW+KuHnbrf2ceMAhrqqALPZiiyWKkMoS201HG9fc1IJ4Bbqre1NEM12CUlliQxhzgnHBMgcCGHSPZW5EPcSQwWgMutkM/dftzNOmanTBZoavSax9LccZrx5+c/CdFjQ32DkjEbKjrj8YCAD7Pl3ZGFRDi3Ac5qgAnb2NSQHzt1s4uslsasgTQp0tT2C3+tUEwQUPDZ+/LzDaOHXDaCrATb9qhaEiNjbpaR14z5rA6rWuAx4a/vOkwPydTYfcRKJMLNENQ70zAkExVxYmFGLCrz2SCczPpZRJjiVZMDU/oNJlKjimMpqXmggn6NF+QmpxjMfctng32A5gW23RfslyBf2QsvRI14AMZrhuBnteWVPD4by5xQxg8BlPcDpPvrOdFVlfI9goWx/jNs5FiyV5+kz6SHa3iJXV7SEUgvpVNBK7Owpr36/X7jJtVSkgM9Z15NX+w4rSNcs+QdURhMgEbVs4zTQDIrcDctBFxwo2tEvGlVrlHGtTQxAClQQZLUFFtVMJbZuZCpDYAU3R8CDkdkD6hpfEu3XDIx8wEW8/Xl6yKQiZ8yVYvgjPxSwxr5heqm+cGu7QMMH8no1sF2YuiT5SbCWHA2xIyUv9S21TMRxhXQ62rQ1WqQ02u2Q/FSS5UT+XxR46tZLUIazLoe92WYnhAOtS6BNTVlI4wLqU0tYKShUrqCwtulKw53bz3VJOj7EuC8CoFW3aKJRoIkgULaSQJ2bh6IjHXf1ZkVJro+llT9EvRsRjrB/bMhHUIWyP73ZiRKn+BFJYNmRk4h9OhY1w1mUaPnkykaf04glrVP0o0plVbOVqZ3OcUbDjVIMqz6rQIQ6l+yaztPURyLo009TXE2nSdNdCb8NSi+VWZnXuLFiJEabRlHgbdoIk2TPEskC68gJKwc+xEih+El7k51iJET5wKpACosQpzrZvZSVM+P6nyLeyGzNUbL6y7L+VQu81fEhyKkfyTKTEj7OVxTFU/DgrUYqjhVEo9USQKFpae71d4DIujricXYbXW3QPIKZxGwMC81KFgKD+6lKCbluMKLPbaiJAq6YpURTna61SojgJlvtAKtvqM21QJG0FMH5+R6LbPe+gNq7qkULpXr9Ty2V2cNtyW7+1fY4noPv8+kP/Cg0x+jYrswujK0TUrouyT/cY1U0JK36Mqb+72d3t2UqjskuWz9eIaki+s61cR8knWP4Y1Cz9SIwxfvX84vzlC6UKDJ+D1WOA9AUizGr1xz26U2sR9iVYXRiaPwVWqoGKMdLNgi1DZA/JCiu+/OzhcPEg+FSkJSo6qAocxnuDNDrz7knA2qeB/Z6HLxYz1528/U2Vipo6P2DRvDQ/5mAMS5xgVG2aVvd5V78X7P1CXlj7BGSmP07Zn3HyN4WCWWKRNIfF055iv6CpJ116M9MUqY6gGH2RNbwN8B1Nt2bbWRl5XyTGVxQvMEuVShTHR7KqSXD4JHWYSjcbVwvkhXnGN3Hmb18Kvu9NcXucmdAuUBA25R2NZXoe7az9M66uKft+8JmpTCB72d5q5C+ekPyFnLykredLExSMBLajivaCSqkuMkkb3Wyoq2NuMyEKtBa1cWIDMkmKemtdc9cERZI0G9YptxVaamPiqPPV82d1NwaKOgYy2gmmLxKiM2XRItATlJAXgM5RVgdRPH9Nb71+vAy47jmuLxOm0wZdMeh2WOVo3B5hjh/iqsZZKHaN1RbVgm9H/kuzjHATKHHExOOrSJhmHgGL1q6Eni+WLXIetknDtLsZaZnHI3Ub5P6Eyhbgmjoc6IHvDs2EgK80t7eGU7TcGl6OcmmzXKQLhw3yIVns0V8aKo3w8TskX4y6FcjeFQFmoLx5N2ITZ1FcksWO9/cGNzYqWQKtnKgk9Lq4R4pZCJxgqMjyzO7iCyR6kkv309R+5LvwGCf6+RQHiROMLVmkw6LMWtqAsiFvTwBMcwhOZc6QNuSCiFSWVqDwFgPVc4G1Jfuof1NkJvEouh4yl2NzmNFJkpxfTGU1SR1b9voJxka92m8zXN/npX4A+LR+C6wV2VWI9ON/BnFd8RUJTVFYj58TDJ2jz/mP09n4iMroHpVjB3d+CpSTTyzOr8mMxyC0Mnp0tHxWtLv5NcVZa9LuW7I2rss80QpWXpM9xwM9Np9LUbWWGWF+3WwBpaIvnUwha2JF+UG0z+jbt8PXb7XR7NANT3aZk6/p60P2NybDXDkavNcCf+lcTKmzIg7s5rYiBddtu3XWvE05MMB1vfTh4mV7Fyiojk0d5fcbdxLFrTXL1RIIYHmYn8IWiqCwaeMLoLqWl1OU/FV4S8/0bEvl0RQHajS0RpEauSnqq9HisNpuaqWbUPJptT7GZJwsCPHH1RGufCzqjREt3eGoTpYHV6LT3nJ+n3zRlAgrj320hv6oIn7L1ueEfC7R55CbG+SRztU/VeEj2A0CqMrOCZT69Rnl+o+BFUjcl3HthEQPvEGCPgb2g/IeoyqDAXVDfMiC/aDFD6hK4oP7EilnpdXj0ENrEAHuFTPoDSJYPT+vKgEsys8rEswPtBzIHyFv0oiU7+CrE4iEN/GXoivkQHYLqqD+7nH5AKnfsddphDn+BqU0glZGi7gp1kk3HMFuDcnxIWuUL1wqD8k96pb4LEgb5RWpsvgedcs3UL9rquwXiK6eLgUr58BQlitIhiEQ68QRGcFuEnDihIxgNwk4mYBHsFtjH904KZTTJikPeQOsGoGgyJPEEYsee8sRqcsgKnfKd6OVvZAR7gaFY4pCJxwmwDpNAj0XzrC34iqO6BxaFR3mtuiLlz84EM5Rt8W/PFcOjNMQz1G3+kKe0aco64CmIAXvEHPwDTL1scQo0sioqUpkArw1VPPTCvCxeoSrSIFF/rji0YNvWcgRh7dBC6b5KKKynYhFbJquzjV+dZuVPVi01nN6vx+ejkyKJr07VMXqt2hM6Q1SFNagQRqr3xLXWYj2wEoLQjeamWNvrciKSjMvhfKibIKsQIPdR3TBogdWIFFqxTzosCilCXYFNOpS4/VsHRY9sOLCAXob+VZ1pV5WiO5lBEecFFg99YEqEQH8lsNUJe27I12Z4IAzXKJa/fFYZS9qXZQCURcj3BhXgQIpen4Z3OJH8LZbom/QyWPw7S4OuSH44eXzP0FL7jAVRBPXBiP47YcZtBoRcH9yjKtGQTluRpOC8D0cIYUK/DhmjKtAge0buVLFBFydjBulTMA3yETwByOR2mEtmYW65UcS0xuk0IsukQBlUu7Y6OkGEdfJoWY6eI3GcnK0KhSgTqoN+2D+FXNp3LETSFKnWSV5zZ8BSN1avECSBs06L0lBl/QGCeq0HJwhL9HV6Rw10qob8DlK8qzLCdHQXnd8OLo6nRL+1HmJrmHUTu1Z05TZsAu9vlzCa4/q7igNAjRIaSS7NKEkzIS5Rmg++LvjJpCkN1C6o4ZFz0xvDpTu+Bwl7wLICbnaf5SI0CBWu7Qpjq43obijI7zTs0anRFmUp0GcFuBb/nIpevOdQ1otutZ8546NKAnpKpmZO+qQ2VKSvuPrkN4gwTAxXRYmTaSQTkAW072oP1K/C7hVYU6u/xmJUxwuKVSZ8jNw2hw4thKJNrbIIZNewCYdHuvsiMuAvkkkJP9KkhhscJtTGeNvkwH1UxdUVpxUQUCzIxZSB0sUU+yMQyRxqoTRxe5YyKa9ZVCvIw6pZCW1IPDSHYOXShTKuMBpdP7Dc1dEJgI26bBYMEdMOmwVEjQmzB0Njq5ChEaHuSPC0bfnlTZg29W80qNvEinUs8brsihEyeQFFDSSS+tSkGSWXlDoonldjaAD/Pbc2sXSuppdR/ibZIagWkdsJgK2ncPSnQPUYW933SZO6M34vfJzItrddyJh22JCZ12YQ29TOCLyfxfO5rwxviKZIk+c9eepBAVCLMoTNIJ0SWkqY5sU4L7Sgox0V2kZyVqg2rs4U07KpU1lIkGBEFVgULjzpqcSVAiNIsndcZoKUZizNZJ66c/awjxfIhpHZ0Mfh1ZYev2tQLfOmmYEv00FMn51QUQewzqjcXA3Hx0U56PjbbR35rJw7O3FaFvA8cC/lLJNDKd5+Rjsmv0elwFKktzZTCARtUkxw3VQ5TRxm7O1wUzENqW4qoPbe2d0BvhNKjT20xENDr29bgRL8LFYNkpyfCwIhJW7pfyArkTE2a5TD75No7tl5YrJCF+ZjFOvZSFEnRa7AeaaVidkmxZZSJxfPndnRCP8bTJlfIdqHKAwxJWzwXcpZZtYhdy22wh/e5fX3QJBesa8vOLgbCOzUtMCvQcRkqWwsy4+EaBGhx0eVU1cu9tVFMlRI5fnt7FbZfUSlAjVMfV/XBIaJGwSGu4qO+IzEbBNJ05dcunRN4lovm2ty0T66rWACrv644hHh61Egt27cUikw1cl46wPjfEVyVTO9gDG+IJ4HsWXFshEq/DMAmMCXREi2h+Qtzff2nAhuh9fxxl7KDuvXMwsAy+JMF2qxL9K4zq+OwnVqTBVqnrPlxvQk71rvkbJxU7NhJL6/vwkBBn2hryA2UKONsGsxg91dZpOsi7UlPpJOs26UFXqpGgJvgIZsezxVQm52n4YcdLagehDZLnCHfKayFAmFv/h1Mx6fGVCddmov/1pwqgXoEpJ64UMA0bS179lhO5Q0jhlNAhQdq+Mc4mz1SIZNjOcLfOCC5KK83OGqimKXJJJXJDUX+VBLfqdF+G7OMQe3RBKcJvA3lbR4xr6yyrwo1h/nYDikbFXJV4r1kNJ8oTVWOdh7rWLMvgvdEHYdfnvumTwtvf9tmrP8/KLxG5vgfRwJ2I5krZNDgcAOdyViPWSlM80WUxwAJFPWYWgWKoy2epofQdWhyUXZ97VRK88LCpHWgwilfNWxeh7DSNRKsFop6I1EqUU7XQyXiNZKr0c7kaWCruFQKUQ8lPpbixLMaz8hNRG0hRDzU9IbiRNZ+I7BbmJNPVzgFMpbyFQJcTpVOQOGqMcDxY7FbWZOLsli2jNQY9kBa+HiT9X809jBlqX9ql25xoayHJvbypKoU/GIwytZ/YsKc4k6jLle4Yn5TrIVGJLJj1+zBVEuMBZhLMwhneuBZxXJKsy51XudttZysGTEJcI1uRNigFmW9JivhRtwJ2MMTRWJT6Jia+J1uQOkbNMi7E8i9kqT4CESVo0pSmUVllW8SFDdQO/n7HKdSJVkzFMijgtumtJ48Rc6xJlFRFI0J+C96p4izpAZCUzroE8Z5ky/9ONHSvCLfifbEyRyzZh/xTTzbp81VrkBfp7c/ohXShWk/NTKF0q2kzfpxwyZZLNmJ9uoBHLVdkpoXhxXQXZXXrpnOtCmott5Gk4VrNLYvGXYl3AnIKuaILuLbSs/IVIYYPJFKIcejaraP+WYV02VY0jqJRUW3WukL8mWlR3F1cAFWiu3wZca4/NfQS35KUyrfYR3HNekWyyLjwJ4ZlAs/XLKZlqZJ5aXayfkvNStN3+zKm56x5GLdbypySsmtdg6nPsmyykjXOicUIkVZMxSOSVFlv1WCzBuuXECpaKNuF+QlULxUKf2nTHy3BHNsGACet+97Cd7z0VpLI2GEqAPFmozG/lDcNVki5Wims0tdaH44JAz/aqM1190HeVKvxm3xpPjd29cTGXuxxrdI22OOYAR5RFCcDrqHq0x1I1STs6LVhjrHtUMC6L6hpX9YktYyZUde7siueV/Zuj62SF4kxoBjsy56a4PjnhqWDHAYnT+dNRRXk40VSQprVHeB9n0D7XCsWxQE2qDpzDNaI6TvjCGTkZS+UkDdNi/c4axIuQ6mwXYnVHEKgHltcpS0XqOpiAuQ7Vlayd/XBW/NR0jYk62BNbJaqzISbypk/GVCfFrsChPhlP+Ts5Ou77yega7S5KHemT0h5LNXbfT8Z4ItLCfT8Z4ZlQ8PuKol/NDvNKMk/tZ46pwhbT9Kgvr+oSozRo0YyXP21x5ogLIP2t2iq53KvETRueE6cNLIBUIj5iBaNFqeIEoizrLa7qtvFV+LDc45x/81jVOL2L8f3W/drrq19/vfoi3DEd1bj9Kvhyc2Os4Ja2PzDzW1B/ii3ozUsixqrfICEZTiSacESiw54TMR6+lr+YGUucxMQvDBK0mz4XtfyOfMGeTVy3Pp56VbYbv7gP1CZn1TGsMWV/xMufivYn6AuFinnoNKs+D9WRbaJQnsRWSaEJPqDw0SsQzX7mtSVh6jRl4csEraq5/cz7e4MbPRdHldVcgAobwzNoTUprx80qRoDv6OkZq9i6BbAPIVU8kuzPwDersTUazX+cO1p5nYd5Mqrw7Is/4sMO49VhqyYWuj5g7Uvy7/u8nH82UyzClYfDnepI1ZH3W44+I+L3svwR3KqVhmHqkbLKZ2dbcsd4q4Lb8gFLih+EZFWTYeW9oC0SMmwFQh4r5OGHmgYjKUcUqTESgKtQartFktOZnSi32XkHrHyiochMKmN9YqvLPDt47bOmHipiKFZCYP1hrcuttOWopnFY5lt+qsZbluLqdWx8Jk72gKVx5hvZ652zi/kh4V8rb2GJa9Ll+BmjrU+GcUZvIns12iXKE8e67AWk2jgHInuMtir2UHhahx3rYsdo6/0yb8oQe2XeqL84utHSM8RV8TjdkVEEsOIzwPVBM0kgRU/gVGvt6Z5uKFffWzvEWNIBlK9Yf60ndFSqLX02ZxkhTr5V3jnZED2G2xYMKHR7JDvGiaVf0g9jHZRCu6KE2V2NPY3oVJX2XQCrGFlTP3rhYwg1l4hQFWiwjeLQ08sxqMJkAbxKhgipNK5xrBMYga0LbZI69jJcU38NSPQcUoFAcQRyZSZwq4I5QY+6clqnE+sEhLAb3jcp7+k82bgxx0/wVkWzazxeiOlkBCN8jqi2VmO5DRkQ0GwvBl4lU6J7D7QLzgBXhdN1fpoCGcAIbN2bfySjNd3WiWuNa0Ib7vwCU2EAHq0PAUffKeoqjSTelagEGoJGYOsjHwq9BD1C9bwJ3Hq753md5Or3rDZafIS27vHsbnGX/RNnhzgD6mdiXO0dGbpRAjn4iMDX7WF/IKYDZAw91uYiC6zXjcD0t3gOxfqGB10z57c48/SiocSMD4UvwNvaASBYD48QWx+t+BnclvQqzm7hhE/R9JtLkj5ZMLa3XzalVvy2mDpPJSzBXR97AKxmksl427Vi1gXg03CxMzwF2dY+9USyikstaBm6+EbEJ7d0L+RNPxGgb8jd1ue6KS+nGLa5bmfOnWjBBNaja06o3vkPl7fnjrlNZegyvHjlnuFUhi7DqELeHS7j/aNjmgJBJlxpcN4JmE7EWOj0FAYgFbfKO6bvjBMnJswj0P4txDWZcxdBNcLxd7L/AlCP9nx/BVplfwdBmGgbNrMAVJSvlwlUiYE80edWu9hOh+ImUd1tI9ThvOmZTrTc6r4YJAUzT4AsX9Z7F13fUJNTzm8q5kpg/AnU9jEYWFN1stfbSElby5iaBffxYWO72PCaIkK2Z5ndD2vo+hVK0UZIc2te1qcmBMCfQOmHRy24o2K+etYIXprNMXRA2wqh4IE5RGzQI2smWp4rh2CNw48Ylr8hZyMMV1iaNUJgHqClznMsyYhpgnFxApqdGBOOekmxrNt8O/RaWJrtxLkn2Ysx0iRxEHcWewUauhxJUmOahZPyTvqOSIZSSJUgWKwha4Dt/C5tEB+7YaF0fXwSjciKweih5euP6PhSaSpXrwQlQWxKmaf1fatFU9HzpM2wvr1HbKj1bGAqzAKe57BgYX7COqluTkmiAT0URSWuKq99ld7CPNtg724BK8ffGEBswxTlVJTbYhkj/0e1PGsQ8cYe/dRq2drzp0i+CHVDgezbDtVai4zFAlKlDTlrWCYyZA1CVhGCcjZr+ccWzYOzRvlhWYWm6eBURLPByv4IX8BCgAw29OH04ofnGxvuxDlAWeSxa9K4rLwuTPiAM/pgJFbOyyyqKyPgq8nYOFYSQ9g0xDq57VP2ecEDFekV+X17DO1Me1I5G1tjBMxya24gNUdbj7vCZYySNhJ3j0Ji7ai08ulaDlLY9Wi0sGgP96znm5bFEk/PaiBnQInBqEyHQpL8dXK4TjYC1LIYewYiRJX+wtdwViP+pMuMAfUshR6/OBjvOlg9MnYxNCt0FANqluVYAKDrmUsiRY8qP0NwzVUmxmB8oksSl1TFQvSIjgNinZKVC9Ij/Edi5d5L2HFUs0Z2wEf5ytuyJm14H5HlQk0TcP1TjAcy76971vQLG+K0vN+BrGqOfmS5W9gLk+0IzuXZ7//0Ilf2fBZSK5QWCfZC0gPzFG6pPHDZEKBvKGI7MdpIFdocuN1RVfAtyFUT1NtqNDpuo09ORusHbpNbrR6PCCd+Zkn8zPTSO4Shh/fl5UV6uHjhlShOPHaaH7CTwYBPkUFVl3FR4LmseTi3SNYZMlM1q5sPS18MRxka2W2/TnerVoEkO6VCUheBmSm03c/foSoOVVit60wAZqczTXai723U0qV4gNLMHA9COTocJUXMVEQguhtW1BIdd0eJNDsNQldBBmim4EPhWKeDADs1AhD1p1zMJgY1HtvDPpRe1Dn5S7EGHZJt5dDtRrZx77pDiqVZdkjgKsgAzRTM85jdoSSmgWb2xiYBtNOhAUtZGTM1DYtYvgXtxgDnYuy0BkZ6gWQ+nlEI907uSIr9cAdBeQ5kZYXlxYVXNUVBuDjWpVwgiGVCVmQF00zZ/U5pxyyhFwzpowKOlC0XaKdsBxVZwTRU9jjy2a1Ni0RZKhiSvBDNfLzV5bY9tsLrz4SntJCNn8n2Ja2VtMSCcCBVuQk+V1KJVsbS6Q+TKwUolgc9/Jw07M5AdfTLOERllPspTWtf+mGJo8pvl5l+h+Ev4dsvAva5rZg51oo4vBJFoSkNjyMnpscyjIhsP3lLDI897kGkAkzrMREgrsGXqzfvPl6dpZGdqkYwXMjkenQn7V17S5rIStAOJ5Pf7HKC/jZPC1JiRwO+Humuchm9eP78Ar1+fkb//5vnl9/RX52/+qew/9U5KUzzlc/LpmFzhsr0jHW2M9rbzvp+t7u4eL4/f3n5Q5y+eEXK13mehEdixHMQ8jfSzee/JYo9a/MOn5G/nrUONnFhcPn6xdk5+f/B+asf/vTy4oeXL170I8+PaR7h5HWEq5DMG1Qtf/7RX/6u7awT9ZHf/UhHmr/hsCb//ez7Z9fvP37+8P7t+5u/Btc3v797/yn4+Ond7x+urp+9fva///3rsxKn+R2dVl/vUVLh79vjgjrOrh5YmoyK/OV//5/h19dsWGa/7dIUjJK8B2/Jb6iFft//kUbzE0akxjQwb/Raz/DJrk6CPqQkCPdbfw6qi/DF9BuckvGQpXbu8u8K5LTpvfntjIBY6tpXR5QI/0ym96hOxBXJ7tLLgF/wF35An5Jtr/4K/1wlATNu+se6zMUE6Ksi7FyNZuNuiE5q0h2zQ8DO/4IjRlGbU0dUYvWbVnHVvZQbH+biLK6DKIxCle9w2qh8dtw/KIlNVr5jGMtXoSbf9K9yNBUZCII7Ygubn97R7RjZVyNruY9ysd3RVPlt8nzZXx8uXgb7hFpkdWzqKL/PAtwylJRgcYtblaUuA3vgLSiQwjfEYZV8VMX0OJfUNaL/ey79jN7NquNUzqi1vw3a/I4GPy4K2Cncxrej+xwbX7KwT66+9S/JoNVdilv9rlsuBXSZxB6m2yrAhtCAebbCT7vvqDpHH9BRmMwCTYLJ6Pv12Y98nH/98SP75XcPaZJVr/lvf/r69euzY10Xr33//v6+m4SIvftV5X9uPzrD7JL+VzqptsM7K1aXTfvLOGI/N+FZK/eswnVTnDXh25YUPzpo3QbirrJCRZROUP78lU1ULO1SRR/3JnMw9QtaWWf/lf7Tpx/1M1dXoz8zpXBmpM4U8T++t5u0vhkl8mniGtekxKH6R9Tl12eoqfMDWeWchWU/G4T0lahvRs1N2JX/h1bwl5ur4G33lljFh4vuj3TgId4RcceDHarwGY32GP156rEE/NczjIVnIvmODGPEo0uIh0/9TfbI6fKL1onpHCE6cdNFxfSb/mW0gEChJD8sYcYTf5jkNNx3AdM+5MED0rb+vJDQz/HtFwL8xRdyDMaTJU/Jwu0P2+l9+FwuWfj5Ap07Il16Q/ZSLldfm1l1+vnMKOoSZRXNAStv9qnLIdX44FDEGONXzy/OX74IiuMjmScT2g3VCrQTb3tFeVFi5I6s6E/41bRW3YXj/cQxlnzFlw1d6HkQxRUZG8LjQgnb3wvxWdigKrjsYyEy92JUseWff1tOyA1O6dsW+B9zZB9thHw/2r7p/7ttydnO0vfLXTLyqwMOb4mvGt0GbE/BP5LR9x7RV5voONw9NzlZWXJ4nZJsGFcqxp+nny5lD0Wcq/GdFB9NHtJiURnf0X2m8U5IQFqmpIvZrhJhXm2xX8fhtCiOGh/Bqq4jI1/wSf6qSF0kkvPWFymuZf+czNuP1++v/bc0ldH7LrVtWsUVm9njYbrVByBd2Lgs0ULFs5jrlqebR2F6eWlQNi2aAJXp3SuDsvUfwehC11rhd+0W6nX7qNkH+qbZ1S9fLi8+/nrxoofrdy4DFKYFNCSWdSJjxLR5CKFp7kjnTeEx602z1MQM0wYYkW7LQUOSzwcXEgw1RUG3mZ1vDhW64EUim+FMITF4U7WDLzDonoz20E11KMiSHhwzhqcZ5wENDQHGPe7hVUq3qoEh4wvoNooR+FgSt5lYYEHplmuINqd8TdSEjFAOIB92CHq062Dphkucgc/QCWZb+tCoDnpVAt+rUhQnu/wBGrVAxx28i5JW0P01vYN2zYowgx6gixLa5AliQPNHk1U9MDJxeOheStG+PwIJjN2YagXu91RktO6P+AFB4d1zF+Ne42Lgc+FJ3kUIWp/0gBwWchKdAwOd8n1zEDRiljVB7PlagbbbhzTGqqybYgS6sVGjBDonuoE5HDP4qIj98ZHQEOyzVdsZCI3zYY9R0sTzBmWHrDR65KdlLeQGBSqr7RFjFaIiq2H6AptFFXoMfR4lrnAdqCz3J4XHEV6zH7cqwiMP+b4oqiq8PZLNytR5Gm8OVdMyQ4Je9TI4i2L6pnu1adfzcs2mozctUSVxpakE+nRno0qM7whzZas1EC/TUlMr0odKdiQPYUgsA/MHuBTJLlDaUCq70sTQDwq7xAuQbi+9pbChBr4nz2KTOvF9jN2W6ElhLnYorCY5wVFU3nWiyU+KQqflhnNUveKcdNIm9dEuNxK7URyntNioP2CVCaQtRSM2WRn6HwZFuiAHk6JHtLlJKSnWntSYFKZRFBrlknhH/8eKvv14ecmOZpBycdqUs5hYdibeBqyotesUaKi8QWFW+c1ytM7MilLF6WhSROmYYFZic9yefB8e481F+bTA9hJu/j07w0N6NVebU2dFSoV+OiqgsAs//V5pZSQoYqIAlT3tSQGF/Zr59ya8FPZHJt+XKN03mR61UrNhKr12V9szmBchKxqtIvrGonjaPCmjsgpuC9CBqx9SlMYtXoL2eK3vaT/UKYA1BfB+qFNkJV5EXID2Jp3vabfQ+b7UrHOlp1JurzpF9JXKbG+jAFm4xcURlyhhFjv8qLjJNipPpS7Lb8gvURTn7N0hegGgxGHt/b3BzeaAMZRLWFJn+sA085gNCqoXoVLIjJxsOsezIm0mRb1CxPGpvcnlHcWCJmUWD7WpF+XZOUyk8mdb1Qv0qe8Vi4yfslIrwuJT0+ahj1Ul/21Xmqw+6TVHdRCV483h66YodKyYsaJjxBBOvmO/K3GCUYX7n9nm4AM7aicLef7XrdXATAZ1XtvtW7+De2j/Sv8SVAUO4/32bs4ctV370Z/NS3Jvv92CDOgSUtGuhGB7jOqmtIJI95vuy0ppjY7Rl+cJK8McP8RkNspC3N8MHn5lj9lfhR8FfDfh+BOv/a3aVAEjyqhWBaLJ2LkXWPUm/WJ5+64p73AVnD+PdrqNaijqAkxUe3mw+7EdBsxaRgUYijHVQtsJHABDa2GMacRzsACU1UEU303Ngv/SjK8itiXvoYOq3CJxL2x8A8VSWlHHi6u2tk0hxNTk2Y2NuwS3vyX/YYox0ij74/CzKeIf9+iO82L/uYVDJu9xMg56cWDYdV9cJIMBUwm3VYRS2NpSRFIJLlSEUomoUoTK7rbdOhFUf/Yxyz1QbY6+img0QwEYGMtBAIaWgKKxFlCEGudS6Fpz/DvVlhTgxPZAvHITHJOKCXDYCYO6ygVw0/q1x/LZ9lFPh9hd++y03v1sWz5Q2gvZRFE4ulTBCOI6DTYjdjaRGoUN220QlW3fTZQ7Cy4xnG5iS+XwLtFbnWI/kJXnVmeLUuIalQdcE68s2t4W3oQblGyL1CjswG6DqOzjbqLc6XBZpEJi5kP+7akd423jjN/7MEOiFRwx0qvZkW9fo0p55JyVHCKczHQiwMGYaAWAz/BYHyo2z4J14Og1+QgleabsbSugmkI1dW2s9C6Joh/F6JCRLqLufM6Rtu8+SUrifXp5EaRxWCpPmXOE9OLhwWNBeV6E98qrlgVMmxxtlGlIF0IhVkVe0qI0M6KyKerKY+o0hFEIXpKUZM1nNwC0EEr7sWsIhmW7vysex0hQSjIuGI9cvQEblm9fnbQqbIkxyfNGbfLAswMc81x9M2GGynKOeJMkfh6liSvvHB7StOsIIA2Q6FxOcxORwYx8FuJ0SOyiDbOYl61xpvMpINxoPoVDNYSiE5lZydEcagbAcmKzs12z8ix4sDqrjArzoctMcpsXqP2lPgSLtxCMICohQDK0Pqx2iqbHjaYPGjvr2XYKBglC56RTBD0OLFvjfqad9peaXDjSVDMcSZHTJLlVx2nyS1VO20h6OzEivBiCGlfXFMlIXSIkvYW/Ap7evpwIcKY1aiF3NH2UovaG7KbDPY3uN8YYsR1Ifyujx1DUzxoGU7VS0KEymlrklxQunuEpa4rF1PXN1d5xW8ljpQZA1vAhrqqAuEMKsZ8zqE4/Iy6qWhEBzLgoQk3S8Ha1a3+Jis1jRAUUzYFpG2meow8GWd+lVcWD0+MIs/XkA+XFgS6w8hJhCswtsv0lXeUZWeEERXM6WkcatbIiXhPhBD12Gmt/UtUNL8tJ8LJ6cidlWV6xH1J28+ZaDLP5XvdGfjH19767J1wn77WDQNpzC3e30rlcHYXlB/eYowBZy5QM0ugArLp0f0jinX2deZAvMDuWFx23j39XdbMDQqQPU0GzJJj2WqzLpqq9ENNlOiTD4X0+e45/yPOiSUHaN4e6dLDSza6t8qtbNdLC7CUo9r6hkQm1u36jDZm1MzJlFA0V7EuUYtq5ukDaLuVwUQT93/pcu/DAulrXgoVmuxpsZ4zKlg7gKuBJknVMUgZMt1JGujWD5AGC7YPZbO/DFz6P4RbcpO1o9KKP9nQHyWM+mwYKKdORo4+isX9oFGei6QTF/uGtZWrYLs1qxF7tjdeSn2wDbZ2cryKMZ10wIM2xVwyyegAmQ+mm5+FRShRFJXGmvDaToCapBZwGn97AvArR1ARe+6aSsaoHPF2rNzH1voz6ANO+UNi5/OwZHvsXMIVwsktEQkLtKLRDVRwqvZko+N5AHlk+4SzCkYbIeREtqad8WVyTlOVrlerSiOfKU/tTqwFQgRhQi9RBqerrPA5m+qB67LoPyJvVMzBtMmq6mH2qLaVw9ES3FgnQJ2yVJUueuV8VLCmjJff0r0qrUzvJe8zKdIbFOV+m2j5x3iOZ0Djhm+sK1Fg1+Gsa/nr02VrJNnhg8iiHx/MJqbqTW3iKDtEEpo2LWbmvvl24bWe94iXmFZjcz9fx0NeR9Om0xQuU6e3xCBHMqjGUb/VqZGtjHK2iw0oW4crDyu71vHQYptQipIceG8VJv83JXN5Gu3j0+MkMh3f7qQtrqBCOxfZ1vCQ/HIi10Sam26NW9LpHfo80IaPqoquFOhTU/yJ/e3hsFyEGpSs6dZkWNrZRnjeRmRnpaEZlJ8svMwQekWZW+LHW3yKZQtBjHq99qdDD2SGWh6qvw3S9zajwWsKX1ZLtiIsSdieM6GIlS4cCzmogkUr5hvhj4WNoaAotyGjEMQeh+UvJ+pwMYo38drcCDttk8rbu3KkCmZc3K0kzj6zFSW+XNip50NucmxbufEKTsjSBT4IeDdXVnqmama7W0emsqLG7NEXQdR270p3LRyMH0FoE0ipKie49GwVqbqDOCo+WXWYAbK21lqd6tfj4qNYIoHeS2GaSvh8/w5k4SAflveAZVpIYD8K0rP4QoJRQS1iyj3xhN1v8lWflhMX7bfylM+Cd/3B5e66/kFiDvHgFDhlVyKMvjMCj3uEy3j+64szRQXHNwdi7vsR0abIufZjuh/EeUHss0RTRSmCwGCxJWC+iLKR5JVdLKqwL2jOs9h2HgD6l67cv6rIEnv6u2e9HQc/tj177Yyo/WTcBXUvIqIXXBut3GQPbPxrhtg99M17tf1tw5Dni+3jChPxnkuSy+GVDsBVm9TEuI/qaBO0wrK90j7gPPgT/WT50aiOVsezBM22oKpMFqZpA3ZPOvtqSGnj0EtoPspxE2mBkBJeGDGiD0Z3ctbc3dAFDlOIkAWtUsmKGQjoi8n8Xz0HhijyROaP6gJjG9myOxDqQLAcuFJjc3dKGYunEvYszWUpTfUCqtaCowMwup5M8Gdvp7Q8ozLoMonIH1bhr2zD6WFDjeXSE6q84jGSPTRhAgVkGDv9WIGnMtz6aLJxGG4k+eF9ADUZr2zq6WAewIfx4G8l2qbShyGgG2Rv5v2HH71S23asP9BIKqXUsuUMN6VNmNNFSTj1yqLEoi6s6uL0HQstjqNYopC9EaCPJNoD0gcLqXHYTywQMyt66PRtgOMh5vcekBaEwiV95fvkcTIvtpeMAhaH8JTlt0ApBVrmMSb+Izn+ActBLsCm/OiKohSWBunj5AxzYS7BOW1VQDVnRpw5XXlw3wmMrmaohS3Io82WweX4bQ/KsYzqBAQHWxxKjlWuK2nhxCge2/giRLtrDy+d/AoQKwhLKmjkcVJu2cJWtb03GXv4nMKCAbgXVccYe+8orgJ4mwCYTURrX8R0k9urjZEZ4AC78CK99aHL9+poNLLtlXTlptrkMF81HgEvbmXoEB+QwjhArzPUACBn/AanC1QAWA7y1Q1kDuDuUNMZ4PNKDHTmsnISoIFTZOXsrGwCnPRyww+GHApYg7WGAJUhGw7EtG4jvXwOMoT1iu31tV7cWJLgvkew6rgmSZcVspoIBZLxFbVm1MdZW8nhT0DtUxdIHNZRBu010uwpH1qMJ2562hwjSWPoeqwYOWevacrHtHpje8mRPCgEAdSYYIOkr13pwIMbX7bFb1q9FCYo8SSCh7Oo2bIjbUUplyUfUASCG2LSo7DzuMZDNacgYp1zLIqYDRDxmaUIQVRy6n23XUgWKVl52V0aRvj+rjHBrDQDhH3Qoluq4hfBRi1sID7VfS8HAtLvFllgV4j4mGFCASLcERrO0gQENxAde4sHxS2J5Um0TPPooOBg7vn9Bu2WBpS9vAwCDMbb3w2ZgYMyOqLJ1eOdoYNyA/LsxovEptACMRk6Bac48DEsA1l6jCuK0sPUfx6DWq5EZGBgz+3l1ggXHK8nrYBQzDUdyBgzHuM5LeosHjikHtGNIrIUuGYMjTgDGZXbwa1dDfuBrDUIPei1B2MOM9AIzV04wpIu0VBMNL7ffxmDnp0l1fhmw/Bx2YMPZqZ3W2MGkPQSxcoys/fH+aBMExnaY6Y8y7WHYYgWIEsfS5sXCAdIizzBNNIjnGayH32hapjJugOoasyvJ2psW6jIsosw1hJgsMjTg3bKP8D7OtM+r1PHNInXV8Q1ca3XwfmPWYIxWl0LASTOwoDWXnYGKYVmK9cNi1IXQC5eobrSXteoSTI5r1dHZpg0keCyyJyAJ3TsVw2ANMwkscCeDtSsZRps/6vBmxxnq+AY7BurgsyHCpZh+iHAlZBgiXEkw2HldB5904rwC7sAL9GCHSrKCrS3lkHVK6zEGPLncNFFcl1qBhkknuLSffLbkVUl3K51eKHcolo68CFcGx6kqyGGYukKmp5JuoPlFLkfoB2cq6XuGwYaOEv4RnbuizjZD3IG/PL9wBd7/2gF4v1fhgDwdT7vfsTiRwCBwRUcIaQYH6MTo9QMAlYCTzg0i03wZ3SPLSVgqpTMfO3C6YdqePTHLoUGWJsEJCrD9Qo+lLcKR4QbtlqRml8Rh9wejQNENCewFxv5XZjEhCiL6IGGT818tfIDNlQ0JgKtvBUlgk9mqJP1TMzX4vEB/b3CwbzL2FqID4x2E2K/zZQLqEmUVKUez2bityliSu/rwy0nEiguaSTYL5Xk/DQR1L8WS38R1FWR36SXcsN6/GrsYs1yKWHkS1FJAXVounIXIs+HWIX53c4ntdzqUA7DXsyEBcMdEQZLxabaWJP3ICi14mI0ZmRA+rLs25KkYh3Y2FeSqabiUE1jzeK5y3UgCWQ5bSiDNerNWQ5Yr0xiLOoF9CL0MRUHs5bo981nivKpLjNL2NTvlBDccgRfSE2skidZ8zlVRbIUPbHOXPR9+F+N7//rq11+vvvB/qRLZgAm+3NwAMWJQLasUxcx8/g/97zxqEvz12euvz34syvxvOKxff/zIfvndQ5pk1Wv+25++fv367FjXxWvfv7+/P2tzM54Ro/Kryv/cfnSGabQr/fI77mWwYmTB3f4yjtjPTXjWyj2rcN0UZ71lXrMf+Z51WOKoOjuEIStaROkE689fv2bfffcju5xJbbb6rqAHWGUr8ey/0n/69KMf/Vm9/syUwPmRmlPE//g/z75/dv3+4+cP79++v/lrcH3z+7v3n4LP7z5eP3v97Md/Jqr4+vU73j1++vrs/Oz512fkN6SL5HSfi/zq95tfvFdfn/0zkUmEcpnkkwyl+KdlnZvwbXvov6gtxSXFC0xa9Tok/yalO7hnDJ18QP7vx32ekPFgENFuwU++6b6MSRv339G9mzbp0RB3QP7aB0/9xLVMf0lfF2K/2aOkwq1KV8Enkc/g6JT6w8XLYJ+QwT2ojk0d5fdZgJuK9AFHAtsGYguKIAqj0JGYNoUrmbWS4D7KD46kHPcPuSDsBFLE8j1TR4LYi7T0KqarmrDnyx1ra2xdCW0cN2LoO1XszDQgGnMhg+4ZBPw9KRf43KpYm7vAp88S81GXOD0naGycNi6k4DSJd+zSSfe4D7wMPlS1uzgBHYkdCBncloDOlI5GqS4HekAx2JaI21GRPxwUsATpjkTtckS8QpripMwTZ5ZMbau6dzVecXWRHtk9Ee5EDEsW73iEb9835qKO/YVZByJIPZo9Cmti0dTEHArkzXMoMRHRTpOO5nh6kQcT5yii/3vuSEq/CGz9yDsysrm164o9lhCwxxIcjtDUmXQ/kx1P4rYgFyJ2ddIGbJJuQxpi7xg9qC7CF6AiurN5x/3irrFfX/3otwvW8a94bqXPqD6SH8m4RYsSFq/9v+RJk+LK/zlp2Asf1dEv45BMbLmfsoW+z1bMfmuI/q6MXpy/+qewC0frRPerf/bTd25W8jc4pTfCMeBaHjV1TjSstJhv5/vO9+4Ovsgn/R7M0EDLVlOxA+IcJ0T5dN+RnXFyE4DB/3JzRTw8vgNUgUKPdz0Dwh8l+QFawHgkDGn2iApa++2bLCxmt5/JHYEDM+/HjxYfvHEX+K74s9ZlzyRlocNqMDGtDzIIc1qnuTDgmnE/pHvslN1F4t2lfZ8FTNh0Igz4+Aldn5kUdjzDEoE4kjfdgXLRSxf7ta6qMrhyMcb41fOL85cvguL4SJbgCT06ADZzobh2KcmGPGh5o4WKs64rlAHZTMx9K+nvAprL8SyJAMG7pU+7nEdJsqP/EcVVgerwCDyzbQqDbBoujO+HnahqK9Ic1C2jufBOUzGZKAe1QvvJLolxZzrN0qZfE5xsabN1MGu8vJmdZguXObMiXc4twbeSEu8YXen3klLX7QuEH9Cu2igqAbj65cvlxcdfL14oFZexYBtEGghCM2+DAQK8Ly8v0sPFC96HWmOs6iaK89c+aYXhIdlWZ/5ICX5fH78l5YtQlz1Oj2hN3NCmAGcqglWnKujWms34vh0EbNux57+7uHi+P395+UOcvnjFRytTHb3vHjyXgls2ag8coDAtoNn2oHAse28KkGSLCccxbR5CB7rsYcGY7poSpeBEe1RInnXogmeLCsYzTBtwlhwTjCMNjwEn2YHCsSTyh+0HQKIDLhzXFBHcKixjskIv4Skv4OGYF20GZ1i+HBSMJXbQqzBwr2qj2+Bp9rBgTPdVGcL3qx4VjOehCEv4gapHBeQZw6uzAwVlGdCbwU6o9shgfI97F1baowLyfHBB8wGWZXwB35M4JhxH5MAv6UDhWPZvDMLy7GHBmNI4uRBl4FRHuGBcE+LjgBPtQEFZPuwQvIc3BgZnO84s6IT1WAAce8yCxuFJD7hwXJ3MVQn4XJW4mKsS6LkqRXGyyx/AiY5w4bgW6LhzsYUyBoZjW8FPsBwTjuMd/MYZxwTjWIQZvBvdgcKxLOGHfI4JybG97Z444TrGBuNchSU97CzafDuglKfQcIyxqzF1ggzH18F+TwW831MRJ7iPRAak2cNCMnWxeT7AgjF14/HB+3uNG4evgff43GxKwu9J3kUI3kI7UDCW9MovOMsO1JZlyqM2oQiO8WwDB+axCDAURbBO4gaUPt34aOvPyzwBNHJMNy4mzkLFeJjF7YE6T2OFPtjy6rKTDqVUFC8IbM+iGGUZrhTcv6nkaUlT6Y3Cym0ut1FelAm0XFVYZUCeabkvZSZ1yD6pI3UoZSa1SuJKu6p9IUOZNaobbVsaSq1LVRsH5n2yKo37ZNvum1FWvDY8QeBQyqbdNIX2hVxosM7zJDyieGs4tGwAmULaPCzKGunZ9jl4+/JKDaIxL5kO/+s1DUp8iDXGi6HCXc7qOYzGrCzTviUZWxaHMAyKEvN4YXM2SxwwmwB2QIbrQLpOyDRLh8nYN0XY7nkDV39a1F8iuRigUBGbVHNy9VizlrG/KG0w3E8vP49eVzRnMkYxYDS+xTpquM0ON6MkhzHUEst3HuYqfV+gnUlp23YyYjApbW0pBX0sXGGrZtVQBhAwy7UjNEIxZFTiCteBWuCigMq0uNVIpTvcv/14/f5ad6R/SxMJGvlk+jH3M22HaRVX7OJ4nKhYIqufTwn3+wdLCGMPoYXit3osyIwQjLnUf7C0QvhBYfElIDItbqkR4u5YaIOXNudAE2yF6eWlGYdxaWMOadEEqEzvFC6CCDhMSn+rnmKbolZ37GDJqZ5mQdfnxdr2dHj6XVaiW89Nin/rS7qB7Kb9TerK1yuT4qcyQIkqEhxF5d3TmAyRrWwsLc/OWnhJm8XncKXckMMU4Fs32QQrnKxP68mtlZcE0rUZhynAtzpmE8fb5N7tKI+OWT+0H84WiXw2m4nX1R+VHYY3EZyN/cwTEYPR079htU5O6VKNIjftyzTbuerAuOnH0q+TUwuiVCSnHzy5Ti67UznDUyTXgbmeL+zduVnu7+pie54U6aD374R4gB0PkuEAB9n9IBmO8CA7ISTFBJ4i6z1Q/DqwJ/bAx4npn8YPHzNQ1+64VKfeOZJNu08S9tN8rjqmuUVuBGjBMZ5idu9sqk8WApormN/6GmPS+mY64DPmHAmsjaxoLaGeeOjoMho+zbDRSVfvlV2JrkeOEWyGij6zI9tZsecz4ICwosni4zoNXgEwm2CBsCtxjcoDroOqJnwAKC4BQXjSRMwA7DoYGE4VEKkKktUdkKrudHT1lBNPP44oj+59nfmEM0YAHIms+Qw4VlOgYPgwpSYDAx4p7TUHz65RivvfJNZoB/qvtagVKQEO4LgIoCr9SxGb46I9qztQXSkdI6lY+zfgkA6vbzyBSzrEfngaAWCdZgfqbB6UgRk3uQjQQ0UByrADhGVJEy5HKMkzWHXOcCE4D8+r27IckMwjJUqNvaoZE17WXCeowl6JiZiwf7jbTCUCIGNWUYwOGRmxdDbxZnymEMZMcLojjVwcFSLQJUQmCMY82Msl1VllSGIobsygJH0w11jDzxgMxc1jq1gi6hbXmMcc5FtbsiyPimPFSXJFcy2Kuvcw0hnzH1hxf0A5ZYLgMC9r/JBe2upgNCCY6qGj4k+xTqkNVY9pRRFHC0ug/z6ewAa2/TXTGsjA4P01SIYdoDN/DZTsFBfQXwNgqXexcosXxkWZa2wnyXkNSMa8UGXOhJc110lT1zYN1Be38qKNB3bdrKSLLAXp5UWQxmGpsUSfcZhhWHisFw8PHrvM4EV4rxGiNSckALJg1T66PHoe0YTRFMSYjeUUaCWbDZNlU9SVx5rcmIgIyJiVUsilhIht4Cezedshfg5iyYa+4WjedWYYdlzsWFjJ7yR4kzeS9ZkIcGzXoaZUhuLmDLoh0ZzEGMF8cxhiVWd324kupr3Jo9oehcWVd27MawvUBVvzUXgLFJItJEnbY4nR+9l0IjqU7XXfY57rRIXPTXIb99s4uOBPy6Hq+ATnF0Sq6Qb2wJtthk2RvrWtMFmlda1rVOl2qXOE8BMoDFmy1XmY64/BAkpjrG/DyGkfpAnSnurS14SEyYYvKzhcBVvgfdMGL6BsMKy2KliOrTqJ79xbGn3Q8unGUiJd27o4434U5RjftEV1PHXNqKtrN1Jl6u+guTedgjgz9f5Jh6iWgrYJtcVmw9OA9U2b0oKwrk3xyk8HpgHriW1r8qj409jWhIK6bU2Kdba1wLJx/KcPrmsGUm/Tgwmpnr8Kr3cpRIEl0LWQKSix/ztUaxzGiIiugH7rQ8rUTg21wIeUBRa0zQPSAwreBVSfAOyJh+Q23/DTjMU81zG9DKg+iLSF+iQQUwirJAktFFkvhriqAhTSDRJLXkuwb32sGCtU2dR53btkEVMI+Dax47UEe+oumGBcsIT+T9QNe/ka1t6XGZLljlGsGr1Hat2B9k0GEGozQECW7bPLcCR7PKupawqq0XHm9ERQ3/xQNlikScX75N1jFKDWsCA0h3niwauN+3zSJV1LgZ76KXe/CeuuB05wbIaGCToIKaA1Zg82Oq8CojdF/NaHhpY2KmL1jjitPh8cJjhwJgNBCmiJNgEL0J7/QjdwYZuoCBva1oHowp32CiDBrFKICk22PZIPNKISlBmPoR3R1ghP0KWtHKTgcG5uIpygx6eZlFvZ6rNL+303rQylbVq+RWG5Zn9IWeLdaxs6M6BvfY7jOlS2cF5lbtpD6W812yNOtxOnmr1A5aJBsFqafVap8Us+WCvBvruxJLtTvCoEO5JQsdQlp/fsuuiOJM5uVfzqVpcUgfXjdSjjcYbB0hX7PkHV0YzVpLgdE5rv04xEV/KbHtfGjajam1g1aV+aF7bStJF0a6lq2ziietvG188M1ZTDUBzAzo1IdCVd23kS78ztnBRmZN9+vLxkDgfSqCop7IsAvt2ZfFtXUC/Q4VR1q5ax4lfN9bZjlyLDtNEUyEsYistLhVQRU3m8iJlArF0/bFM/rJj3aypRK8nXUuihUMnONhHZFTETmFa6VspLmIkrSoVHySbieAkzcaW2yZQ2JlPpdojKoju0J426AvtCZkJN+oRll1ALIJ5IVIsRNkzHqehZyqYEBXeC1qS9MjmUMRyjj7FCyOhEYlfEfA7SlKeeAV8ojj1DhZT89KnUUUEz4apPEk8EW71IzAxC4YbOzIRMX+9jM7barY2pcm3eP6ZjMEr3TabbVUalzMQqP0I2Eav58JjEj9JVcGn61h73ivTkabxSIfWjNCVqJQWUCTUbGxZlLVw5PcldEXNXTk8eL2Eszky/04LmfqSeVF7C3I/UHI1sukylOyBUFsMBcQnDvcKx5VRgX8jKedUWygvZOK96Mi3HIbWrXxOJare7gHdHClzGxRGXmxmqoLZIBoHKsXdDEebnCxG+IY9/yW/TDkY1ZJkXRAgnNYsSEZK6FlGiOAlUNyZtDs+EaqfStzXNKuZ3TLuzsK6sRX4LApjuFQY2OYGuvB2Jdp3RPp8d0OM9hflyhZMYzo6iYpqWFVb2SVoYzB6juiktuYxBXO/6d9fNbZJT7hKdN6TlyiM42nrr+PuksD9GOWU2xhhj/Or5xfnLFyB6GODM1TFg+ALMUyrnj3t0B2MfDMlcJay4P0Vyogib/kj3zGz6It6XlxcG2hbBPBwuHrSh5M1HqxZUBQ7jvUry1fkASUr7jJbfk/PFuOqNamrjp7EF1NT5Aes5MMtDXaajEicYVdbdsIPrWuAFPartwLfPfGdNyuvnz0j6m1K+uV5bJM0htmooVuGmnozU1umeiUoFsEFVo/A2wHc0Pbv2aMpq6otwfUV52t1TrVbF8TGoiI2Ep6nUVNwpJ9QCeWGe8c3iCm6s17e37fYpUBA25R2Ni34e7fQXM7xlplXuJ4BZ6wiE6dqaTo0uTlmjC9MaadubUAWUyEC3HZm3t1mUqi6DthwmBtVBk52hAvBcVB/GqGTQQJpt18zgjEXQwLYAS9aIp/00MBol6hjUVSB4pt1lAuKLUCFnfwppPdcLiT5ZW6KsDqJ4656Anpo4pn2bciBfhg7Zth2sdfuukj5xO4c5foirGmehyVLMfutuFW/k3DY6Ie+b7Tqqtcf3oAxNcYQk2uoivH2xMDM/ULU2hjZqVhtzr9bQEzR1IG1PY9TaAEL5fPN9hvoPt41kdiaZItUzSdGklTY6W4SSAZ+ABCWmv1RtTULa74r6Yhibm5gdnC0dJyc4s2ckorjEYe39vcGNlvqWJQ3iEhKaHswjcDqSJ4VshHqKKdgEoj2tXGvz9xhI+fAYJwpvZQyiJ4VMhZKBCmX6YodihoL3BFj59Yep8FlRQwJmsq3FFii8xaY1XxQ2JXFUuBY9E31Uvwu9FKh1nN6J1DpBnwttkjrWHUYmhQxr2mJkuL7PS4X7edMaLwobkKhCpBAmPMjtvjcQ1RSF/lg9KeQmYEj+h7nHcURldI9K+dppGZqQkyInikMj8z4Tt54gsquDz77tMjZMC5rYUYcR5lld5sn69a81EnOAbygObs5Rasiz+vEcEYuyNnpezTi5Jl8tzyRw9F1Uxneb+c4ESzb6xFrD317JCQJ9N/5pErSEufy6X6/utpq+mHdn5hzpm84bwGlu2td6hbndcaQnzsiRPly8bG/TB9WxqaP83jCvia4lLdKXBgIq/KoGwL6rAFzZcAVlO6uVc5b8VTM3x2kCJWyfwX6CtlPtg6K24x1Qv+2eNDTNvSNH9+DVnbj6GJN5syDq2koOtpzByseiNpyx0h2O6kQnwEIcflWqpTRbsUyE5XMfVaU/0pDfVtnn7H1OwecY1iep0WomE1U2IxxLRqjKzomolfvyyhoaIwGwui/jGoZVj2TJaocq/IP88EWV0gBjySdkd1Ks+QwwIHyC+xLJX4LSI9VjATKz7XszLEtmeOXVLFVGWP3VrHUmPCADgtAIyppXJM+zps4o0sy2JuNSIQgyLQpAi7HnqzOUBGglQ5pOu80BLTmmkbW6WghrHjC9f4RjO5vEh6yRZ41Rnk16GFs+WZA28j0hZT49jK2vtJJSR9lPUs+ws8ZEnmJRmYhyrsVVHjCe2gjHmhGMlzbCsWYE43+McGxHabpfWshzIysPzgMODKOgyJMEilYPZuup1WUQlTt5nillN20EZMnpmKIQhtQECbIVrWf+GZhtKOIRnVsrqwOx53Lx8gcINhzGns/Lc3lAvQYfDmPb4/KsqmlAJ31jxr7bzdEs2dXHEqNo7eUSVWYTJNtZhp/02k8zIyAgTixCF4xYj2ZrZUcc3gatsKA9b7FfiYsxrfvDaqI19Y6gk3pNtX/2yzMAfjJYYL53qIpX7p2b8h1gAfYXgjReSdmls8nQI4Gs7YF0NwezXUsX1VY6QuXl9AQKgBfLiwJCq0cCYFWuB8Dp0CoNnkuS8qpLtBICrkOrRwJav1mfkNxC7dOUFaJ7YcERJwVeSXanykyAZ+tRVkn79nGHGRxwhktUY3moq7KbuY4NwBxk6B0DAXAi0OeXwS1+tG/uJZwlvzy231LlGJZMHl4+/5M1lQ4EgAtxBTEC2I2aYcEws3fJx0AwnORhlpqcNF/jXuFU2Z9njoEAOLGdSTBlTdDg2AGpbYJmyS4COEiMYKIuyIzbrRKTmKZRsV4dixDBWALSg9UeIr4npO46PMD2hQmJECLCsWxD3piDylxAQLoCaDjeVZLX/FHMFLgbCaABedd5SYBB+Q6QcDwhgkGWcHD8jmtP3hkQPGq/gbfFkN60ASTI4eD4lQDhI0s4wJ4C20mA+webL6x3BpZ44PMPIMcBEZDl2kMXJhw1XsFQYzifxgDJCqBhR3BArhwOeAQHJHjUft1xiyHYprcEE5BpDWqXHA52LgTkp3FTWI1fibIoT4M4LexPs+SwsHM3JM8WDnTuBqSn/pCJIruZgw9JdQkNv3aA5DtAnjTNehYmTQSQOUv/Zs+KqtFKqoQt3fIa9T8j3TcI1nhV2TkcMQ4GxqwNl4Sk1yOCcOR3XKAIDnAg7ELyrySJzYfzOb8xIAxDu3XEgp/xIkLIzsbLnVMzdHElvCJz73ZJLDJybWXMLLwHATUz50HILTVfU8+JpUbLaQmrl4C0XoLxKuMCp9H5D8/B2E0QQTiyyFkoeh0YFDMaRQvIjcNBsaMxtYDsOBzMnNre9AGbU3s4EHbFyiOCutQK9bcFN3mtvbely0v7sa0VXt11DrA5YcCD8UC6+xNgPsgIEIThcJkCiuIEEcZXLwG9zA4MZjRp4oRmU9rLn8bVHlEmkDBWGMKNKhwLhtcRkf+7gHMCxoCADIs8gRtippBALFkIv919gCXPKSgMU5vt1AVDw81UMTP2grN3cSbPt6vNbwIJxJI2R1AALoOmkFAsR9eXAIlOUYE8m7Xsvfq+jUZC321uR7hxm2MBrcP/VqBbuJYd4cHws7q+sGBneoVByO0AOBcfAOfi4220h3MBORjMHkYLCj2/LWFh2OI0Lx+DXbPf4zJASZLDTXgSbBDeGa6DKqe5ouFWejNMGJ5xVQe393AcBzwQfvRSABQ3jgWzs2CeL22xsWCUMk3CKqwA94oGODB2cPunPRoMt+7OMxi9ESAoQ1gvcIEKy5Vd1Abn2qHCcCVLx/PL54CGOQKEYVjGd6jGAQpDXMFNJ0tYGLYVAm72ESDMaQngcs8wYEbIq6rglMaxoHgFIQqPcKPOBBGOIztQrpq4BtxeFwHDMc7z2xhYrT0kGMs6pj4nKMsBEoTlkEYFiuQEEYZjnIIS7OFA2PEXUMHojfBA+LEbuVDkOjAwZuwGLCS7DhCSIVwPHgMCMqzgNpnGgNphl0CPIxK/BehlRFZXa90QPv4ABbMz3caO0jO2Os5o0pQir0Cm34GsBN0Ff+IRp3Ed37nhP0WH5N+ejEByHhBheYJsOU54wp6nTW7jWKYTEtBdADthndX4oa4c9cZ1KS7r46Z3rkuBrA+BL+0XniPqPSAkS7CNsBFR8L2w/joIbz5IshNQULbxH7Cm2wOCsqzLJgSdSwZESJ7r71ka0NR661KV5R1KGliaA6Jzv/ZE70exTQsyiWQ403nvSfiQFD+ErJqiyI1ejxK+lgf1wDnF8iJ8F4fYo7uuCW5fd9O2j7HK/GW9eRCKvy4RMILGqxKvpeahJDll3dYFP9XST+85uxX1kvp3WbC7t8m0cyFsKZQ/RieSA7OF2It0RX0ED8MYBybPcymx7aFBgznY5ZvA6FUaFdZiMaA1qI76SUh0qHP8pxoR9B5JXNEXsR6jZ3W2dEWfNRxhQwVKO+M6wgYLpXVHdgQONUJZXFtXobyQAHYJzJmWx+CAF8Nc8h3BA14Wc8l4BA/tHThhPIGHPWx0puaFBKhQWWeMD8CjMQ9hdsZ3hv8tLaU7fSxXtzQQRunRdxU4uBVKzIjVpcFzKPOmGmrIXfspNtCAE4/kBHEW6+eMVOQ9E+GCPj9KcFuBQQhYFYh7wUMEgggXOItwFsYAay5BRVZEQVaH66o77WOJ1d3URiLJQWUItE1qVa3qLGU5qhAZB2ngY+ym26zJclAho+zHWtUwzYesQN4kMaoWd8NUqQrUq/iQoboB2I5brcBEjINqGKam1qqDebLqtQrUJcoqQoowOEllVuU5rphRimPjapkmQNaslMORa0Wa40q5G9HkwlxV6SRz5bpAyKrlBfp7c4KpRyjHQUVO0jxSWe5axukoLhPlrjoOhzmxIKhdOSozrqsgu0sv4SuwgP9HPJiZB/w2uyQ2RVtrBsMokpVGoHtgLV1/IcPQgMzaQ6ZK49hpoR67l0TJdNNUNY6ME+huqbRC/posM9UKq2SWlEGBv21+Bjs7UNjrAq6zVIjzvS4HFVkR5WrzwU0tZhLcLX2d0gdOcbu6v+S0IktZ7ncbnVfIxfn2YvvJaS0gM3ZNncp9k4W0qV2NUiIxDqphFiWsVQXYuGHBsth1U0hluaqQy0YRyvlHON7twm2+rbPdYOBluQbscboF4BQZatU6oLY5I5yR7uEdMAfZAlnjDr7xMQbvkv66oz+S4IA/wG77Gnng7fUxNOiW4FodnO0HzoUcURYlsXVkyVZdxmIc1ATqsHCtGi5OCsf4qK5xVbs2rJkUSA+kE5FXSD9Xw3oNhPiuuAc74t2kuHZfi6mkf8QN2hVvBkp3PMh0iuyg/0V4H2fWbvMK77EEB/whnP419tDLsIWr5446aHKzKXS/Gc2CxNxVYSHHxejXCQGuh1SGC8/eJrW8enM4STY/E+G8Dk7ZQ2z+rrKH3vkVrX/c0Yd+uEWwBHJH3vT9XwXmoHvua3VwtuEuXfq4rctYjNMVlrtqTGQ4XmG5q8VMyre0Nar0qdpHi3iCkkzse6V1hOV26jzyIK/qEqM0aBmoL53b79nSTICh0mw2OTn0Fl0bdVY2Zl5narQCDPA6z5Msm7SPZpMIZerqR1UlZl2lwgfVU4hl2ceqxuldjO9tErlcX/3669UXg3OQiXJblODLzY16o7Z194dq+C2KPwXTHjRFzNSbfYOV0TAu1RUUqw5MndmJZ4qtPy9sO05isiYJErTbemVcVJaUIlN0qfbS+fJ18/apEv0jQsFt6/ahk9U+Ma6rPyLvT3n4EziNlpaRWjU+fVIuJo1Zw/DHY6wbJsEHFD56BaJ5sb0W1VAdU0q+DNm4uVoI7+8NbjbcbVWKc0RbaqpBRJr89OOFbCwL39EYBqYTc7NiIFZNNaLhz9DANGA6Usv/sPT/8zoP80SqyUWJP+LDDm9pfjnQ16SbmQ3x+5L8+z4v1YoLWxvhysPhTjq2d1rw28r5jK3fC/ZH5Y27YBimHsGVRy9skRgDGLNosQP2fmAQHlGWYfm28RYjGZglO48BevihpnG68kBaNXoCNFt+bXdPcurnkSZpdt4Byw9/FWlKQc3dh7rMs4NXoizKUw8VsTFFIZLrcb5LjmuzYkrjsMxtF0xo5XxOrLmOus/k+0j3AO5ECUZ5FlULrxmHRCW1fMNdrJ4ue+u4uLlrE2c0I5BXo10in8vXiSww7Ed5MyLj4sYcDoW3fqi8zmFc3HzsyZsyxF6ZN7Vpo8whjLngdEeGVBuVzBDM548kseIxKQ+hD2/z6FhZMZ7ZCbGImw0ZAM2sP9WsohCD55lFt9gIjnwTdIPHuLwdCxsGdmPpMU50vb9+IO3KWtoCSphR19hbu7OiYhMLJFsrbepHL3wMjSc9EYwlJ3Z+FnobqfJVaC2QjJmluKrWbrSusxmVNmfQJHXsZbimvrQpjzmGJZviaOq0Tcobs+A18aiXvH6yu85GiGOxYiLYZCLbyx+V2vBZJgDGPNgNaS/EdEY1ZDKHsF+Ts1z5TIip9yJGMmZWonvPrnfPEIyZ0P2hNDU1mlFp81XQI5lY6IZlXK/doN5YBi1ALOeD0YaBzWQwhTHmlMS7EpWm496otPnYi0IvQY/GnXpS3txW8rxO8pUb6htWMipu7tvtbnH3QAXODnFm2oXFQKA7fHQfzmrEE6GZ29D+QEzR1ID6wlZrWPMOPSrtev/wUJhvbNE9kPwWZ95GNK+4sofCFwDY7OcQOQ+PRltcLZdZeRsqVZzdWjCZFndtA9rPFAmnsRalKdcveYlrzV/lkQCZj58mdjl5IcjOQWW2beIMcg4zAEsi+quZCQ3bxYygcenWCyIrJV1XTG4uE0TX/abbmTfvOct5mR1qafaejodgmu/hAH0Q7/yHy9tzaKJTUEi6F68c0J2CQtKNKuTd4TLeP0JzFiBDE6dR7i5oT3AdaduJnUjxjSsRZ0QqcRDDPLIbLYRA7n0NxSBGYd3bEX+yzWeigjY0aQXLdt8QGXWCNhxxgQBAZuOdByU6Js84bDef9swvbjmIzV5SR4vlzExbYOuaHtKKzyncIbJqNe/ZdMlLDV7+6oW4mqScPylrd+xs3v4dEZOGt9K6SqThSrXHAQTtktNrighpByR0P6zBudZFiizuHbV9Rf8Ak5TwJ2VPFdgqUQIq1LZ0gEJIF/M7nSVsg8l4zCSpStCz2nqsaN46pPA4LpQV9jeALS/CCNGZlQQacbXqxMfQTqgnGBcueHe4Lkhv5Pm1NhP7+1JCdLb97YB1j+tE12T9sNPZ3tLQ9ggahnoWTvBhOqQI1EkIa1ezRfxvQ9aZ9lkf2/hvdjcULF/SJPKdQRuqvK2kP+LoS+GhrtYL0M2MXJn8N3afXmIo9OTbOlZ875F+07rKhlpl147mON9c7LimsiB2mMX2zHE8FEUlriovROFRnk9QpPr2nle3dyQHtByrtYPl5dycWYSksfAflc5hqrz62KNQejtIvRpoUV8EY9kwDKuTqt86jNYCA8JYeDUtqcmgABnqxbTL6Zknpha0KM6a1KY1u/IQXNjobhCfJaAlgPrmJgecXvzw3OLskPihKIs8lhsJl5XX3c854AyXqMbyh6NECmRsfDVQiwN5MbxWU68ztQu4moMeSkyPvvP7NiYJTq9SYIvNdSJId6d/YDgvbh4ejMsYJe1Flz0KSS9EpZ7j3xKS4piHU4dFG0yhP7W2lJYAcJZmNfNLjMzODZAwDnPSIg96q6EpvxECmJUZ0BFB2PY+vmmhN3dNOuAYAc666Mk0xCDb4cAx04zUXOEGELa5xGRx7+CzqgQWjjc/FgUnLsMFHgfpyhKUtxgVjvX4IgksczkyHPs/Er1lk4Qqh4G3BQhyINfol1VuY9cJDxAFTtBcn5Y+EHfGfPlBS2vVmRbwu1LGLUABdPfue8n6+/NL4Qa7lr18o51KAYUKpUWCvZCMDHlqsf0xENtAdG2Kupbo6BBEs0dA9g7aEvykwLCjaLeCfrO5C1rYNXESmYUtTNKSePwCGlljlGSNkV56hzD08L68vEgPFy+8EsWJx+LHAhazEXCHJqjqMi4KrMZB2ORCHmdI0QaYAnzYuojhKKUT9Od+5+jpWkTAQrM9IOshAjtFW7QHjztUxaFKHczVLRCkqW5NqqLvT6fRLlfaKZQ6l2WkVx3CkiKn0C4R2N1+p73lCccPCRNN5UPXRwZ4irY5FE/YHINwzRYAYO1PhZ9i/lRjbTc7GqtUnaC/lON8BGE7qfS4gJ0APuUIImaiO4IA10cGeIq24Ymk71AS06hwtyYuEaapfgPKsjKn0PCwicTPqk5v9nMKmgoHq8EC6VRjNxX4tOueEQODoR2C/xzohLZfXlx4VVMUhPkTNoOcjFl/gKzVCuYp2qk/junqkdCEF/T1xydoJzkZzXZyUKsVzJO00/g219P1JBEN3baBrIkQ7VRzi25N7OYRANWbkJYWOt36gZ2LONXvUo7RwkCVqOBzUG0CvsMh+tU8SVw8jXKjf37t/yVPmhRX/s9Jw+5nVke/jENURrmf0rcLSz8scVT57f6J36H4ckHtlwErZi9wjqYgGM+i56zk4mUI3fyIm1E7Ggvk16N6mE1R5nWbiFqv1ZerN+8+Xp2lka0iR0ATcTx122dUH60EgMN2F3I3YSfDTfc0w7u21tft+z4f6PM+V798ubz4+OvFC/999zKCGTZvPvZYpBbCkdTyHtGHlnLyXwYAPYUuSyN9e8sWwUdFbIry9uP1+2v/LU3ZYKbUXiVRGd/Rth8nxQryO/oA7X1uoyoOzM4FbHAqXI5HpDiLaxs4nEblnbkpjXDIlAhRr+P+IR8/oGuL1z1lCtB2CY6osgBIta/CBUeUAKK1jRjQkQyUYnAf5SAmyyFpIBEsyeHlWk1A2YPOZO0S1UnVJRQFRuXOIxDq9PHpJOBSAr4t4XfZKU8izGIy4p0sfbh4GewTYiBBdWzqKL/Pune0BX8KcFMRfnajX3aXXtpYY4HJsvaIS5jezO60WIyAkneLYVAA+9jclIoKBdwGTiXEL5odccxMW6xE5Hc+XRoGdPKz8xRnYP2CdZcYO4gyyBhj/Or5xfnLF9DIf9yjOyi64TEufLYCb//5cLh4gOLbBgqyn6nlBPAaaSUUiF7WIdZWETeycond6wi4BoOOilrXPVfrlBhyrNuQUJXWEmQPb5vgKr6pbLlArPM8CY8otvLvu9mprTcIEk1iVMcpjJvPIsMh1w3k4wQ9miApPNYIAmnmnKok5wGA5JezAao5SqsGWNFlVhkQsvQ5AGuWbRi9NUx/pR9UcW1KFhhyQ3oWwCnLjhyEFYBYkhmdqWZoakWmnbSxdJ1YZki4CQ5EQ2BA+pVTuIEyQyQyyvwd3tP9OuKvfe131j++ubm5+hK8/fTx46ffgt/efLxiW+x3KGnon/+/vzd5/d8/sh1p8ut3+I7+881b8s/nD7/88su5//zh1fPnL9vvNIV+/vLp3e9vb4L37yYyW0RNrL9c/fbu05clFOWoCfVvL5//Kbj6t5ur367ff/rtegKojPX5+k3w9stfP998ItV8/9sNDErw9uqLIdSXq+urL3+5ehe8+cub9x/e/PzhKvh49fHTl79O4Z4/n0+bUsT+JOHni4vnv5y/vPzh/ccXr1RLX38I3nz+TG3gM6nT+6trjYI/f3rz5Z3MWH/+8u7F+at/eqtnkj3ql6u/CEDfPD/XxiN96vOn365oq725efPh06+ksqQRqC0oYjDPPvhycxO8+fD+198+Xs3t6PlivSVvrY8/E6w3vxCtvydV/XTz4dObd+Q3xKquvvzy5u2VKtDbT19urv7t46Xu98HVLx8vL4KP799++WRQltiacq/5l78GX4iRK3//4c3NL5++fAz+5YqqRND83XTkHTE9Kzo7ahvDh6tf37z9a/Avbz4EV7+x3vevb27e/su7T7+adWfSmu9uPlwTI/vtl/e/Br+8/zDrC0n937vd0S6b9Z91wUfDz5ocuqvVrnz1RRHV0KYKPrz/Ofj4+4eb92RAuPn09tOH4Pr3z58/zUY75bGpg/39hvzj85uOvqiBSQXIen200YD0a0G76O/XV8Gb64+GfIeufn319obMOwIrHD7SM7+3v1/ffPoY3Hz6n1e/yW2cKGGyig3Qnv/CzOipPn77i/o48fvb4Off3394pz2g/fb2s1kfahHItPj+zYdzotbrtzfKs9DV/7r+HPz+5ovyYP7bJ2IhP+tRYx0i+O3q5l8/ffmfwfXNl/efP18pK4j0gf/1/tefr64COiBeX30ghkVA3rz9n8K+NVcbPyb6pQ3EUDvFoMsg8j/638Hbj5eXwSEM50FK6sCz9RVq6vyAs/b8ivyuxAlGFe5/7rcgqdTurxvSu7tJ4dwFF36m9FGq8lGWV4/zDdlNvXwzd44NqZ76Oq4hTQfXWK2YOLj4acjnqe5CGtJ1e3HQohOAXL0zbcMnuo1mSNfh7S1DRqe73mRhYae4AWSnvye6DmNI+mnvhpiSPvmlCQuLdXbFwGqcBAzO/5EehH4quI9Of3gf/UQfd+1+e9aEZ/1hKXPNc/b7tc/OwqKZePEtRy+9nK+7nEjfz6Tvi7uX3uLEx43oJEd1gHbxhAAN5zKQTk/riXmU68K7r86KEvennyNRI4Ef4qruhXbkyAqbrkyG0osAe38QqFsFVFWk+27Wof/sm6wEHQG3atB+w/91HZLRqZ6YwH8i/flvOKxH60X6JQ27D3Z0RZicwEL2TRay2G0yyNdt8M2IYl0286MjBxyI74NA5Wu0ToayPAjpsv0pap6ncd0+6xMUOUsQ/AQkiALwQ4iLp2p+Ir+s6/jEDd+lOP2ICjbhnr7eIX1XLYvYRDCeGP/0pxPIfniQSP9v/+18vsHmQP49KrM4OxAHkj7meXLV9+LxQ12ipyRAHFaU1XE49U5QUp2SBCpxgMsyL6unoEG/SOM/2BWiifwq/kNNfIpuMZ02SUXSM7qTW6PygOu5fMlnC9fUS8lvftJ0UC051Mcm3c1Y8N+5Fz73jr2U/OYn7iN70fkPJyEh9JMJFfp7j/z+J2WfeSFiGPc22QyfygZor6qjn1RH6RV8+n6qMpmikI/ZLSHlgRuYkmj69vZZ7rW/fRJCEn+G0Rr/7XS21I21wXzC8/6V/ea0anLLxkgvy5nY+1f+uyfSjTNGOvqRTo7eJ2v71dWMSy46OpGvF709/Zs3/O20CjoZMR1tra/wvX339yfT2skJavW/9RW6t6cf8Gfg+g9O3C1PT1FHg9/G9h2gvr/RCkn3VeVffqtVEW/jSD6Tb7Ba7qta0mI7i4t9RY+sqHBY/UT/esb+8xRcus2uoP05SFExZfVvHO/rd95HVPz0n/7zp99vPv9+E7x7/+W/+P/pP3/+8ul/XL29oYHM/+WMFVbg3B5dncXEPPhO95wuz82RF1N3oj+x2l1cPN/TeO04XcRra3bzhabiKtSyeS8N0/luyGYZut6gp3R1nHmi6+VaAJURwoa9bZb/V8KgyZoKR16B6ByymD6UIIi+KxoBtY9xEnksBBAl8R+4NOHTge1KFGITABq1h2rNgvPcEFpjHLHDs/ZiD+sQVXx50dpnVJ+1cawRO85lB4dnh6w5G806dNSaG/AIcPZ1+9EZtfCzvD7iMiHV+b/C1v+frXYFha9Prem+fSkQewnODvXxp3k8uGtbp1s3OtY+/v6J7P3/Gds3YWwxKs/wfcEmfNlk//6Kbtt86nb9tKbPpWXo86Pc6C94JExnuHcvz16cnc/JrhTglo6iiEXQo+T3CpdPUy1Vh1TbzfrX5HvPO4Syxb4ZyWNe1cDu4P813d/ZslRb5Q/khx7Ou4/ro8cW76f1yviSamOW0oUL4zJsElRGuKAB8Vn4aHYq/e3UKCN9Llqsf9XPlM28CPEwZIOl0zQ/+nyly3767sd/fkgT+mmbfIh8fH72nBUmKDm9EUJ+9fvNLx5Z3P5zC9CtlPsLOE14luZRQ3pUheumOIvwHjVJfY3rmp3d81sJbXJadj2IlCUoBS7rx+uQ/JuA9Otv3x2xJvzcfqJJaWwJ8wzZfAdHNExM8+6ehWXY3YYMy1YiX0v0TUR+1b9rLO5S42zQ06Z89v2z6/cfP394//b9zV+D65vf373/NLoS/uz1s3/vtjPaen999vor+d3Xr88qdIej6zoPb/+CypjeVqror1/Tf9AP6P97RnMmfSqy7sfX3X9ItkL4X7/v/oPoNCZCotsPedgez8yBDji8zYMqug2Iy0GcDv7r/2j/QZCevWsN6x+M+H+QlmlBaBRXRRrif/87LcTbmQDRdv6+tzeWqoJWsdscrJJglCI2CBNSX5blmZTJy/gQEzeLF8qaJCG/5Z2G/OL8e1qDrMY0K87r81eXl3/6p/MX//TqP77X5tClXw3aR5r//1bOHgdhGAajd2FHsmPHP4dBnUAgoYLoxNC701SVSoElDmumpxcncqLkixEkUCYmE24hmD1c+vLUYOoiytVuvQo2V0MnagaZcyvfcCJe9jmJk3NOf8cJ2VFN5qAQmaYlynXNg17L93SL4aBQRjetnqztv/1uGQ5BsKoYAEaUfGBMR71+KJ1sE5AzCySPFPAmmXRZ0I9Y4SKoO6Pmai9fWQdNNpB82t4gW8THr/zI7n5+dsPxWvqFiBmS5AqSZTzsxheYcuFa=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA