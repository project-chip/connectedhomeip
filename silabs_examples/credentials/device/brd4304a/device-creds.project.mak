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
 '-DMGM12P32F1024GA=1' \
 '-DSL_BOARD_NAME="BRD4304A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DMGM12P32F1024GA=1' \
 '-DSL_BOARD_NAME="BRD4304A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I$(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Include \
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
 -I$(SDK_PATH)/hardware/driver/mx25_flash_shutdown/inc/sl_mx25_flash_shutdown_usart \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc/public \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/common/toolchain/inc \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(SDK_PATH)/platform/service/udelay/inc

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lgcc \
 -lc \
 -lm \
 -lnosys \
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
 -imacros sl_gcc_preinclude.h \
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

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c
	@echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.o

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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA