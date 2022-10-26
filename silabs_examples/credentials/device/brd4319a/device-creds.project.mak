####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH ?= /git/gsdk
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= ../gsdk

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
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DMGM240L022RNF=1' \
 '-DSL_BOARD_NAME="BRD4319A"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DMGM240L022RNF=1' \
 '-DSL_BOARD_NAME="BRD4319A"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I$(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Include \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/hardware/board/inc \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/hardware/driver/configuration_over_swo/inc \
 -I$(SDK_PATH)/platform/driver/debug/inc \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/crypto/mbedtls/library \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc/public \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/common/toolchain/inc \
 -I$(SDK_PATH)/platform/service/system/inc

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lgcc \
 -lc \
 -lm \
 -lnosys \
 $(SDK_PATH)/platform/emdrv/nvm3/lib/libnvm3_CM33_gcc.a

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
 --specs=nano.specs \
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
 --specs=nano.specs \
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
 -T"autogen/linkerfile.ld" \
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

$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o: $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
	@echo 'Building $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/startup_mgm24.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM24/Source/system_mgm24.o

$(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.o: $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c
	@echo 'Building $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/driver/debug/src/sl_debug_swo.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.d
OBJS += $(OUTPUT_DIR)/sdk/platform/driver/debug/src/sl_debug_swo.o

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

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_mgm24.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_mgm24.c
	@echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_mgm24.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_mgm24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_mgm24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_mgm24.o

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

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c
	@echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_retarget_stdio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_retarget_stdio.o

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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA