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
 '-DEFR32MG12P433F1024GM68=1' \
 '-DSL_BOARD_NAME="BRD4170A"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P433F1024GM68=1' \
 '-DSL_BOARD_NAME="BRD4170A"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I$(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Include \
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