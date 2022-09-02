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
 '-DEFR32MG24A010F1536GM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4186A"' \
 '-DSL_BOARD_REV="A04"' \
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
 '-DEFR32MG24A010F1536GM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_BOARD_NAME="BRD4186A"' \
 '-DSL_BOARD_REV="A04"' \
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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA