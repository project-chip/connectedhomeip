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
 '-DEFR32MG12P332F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4162A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P332F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4162A"' \
 '-DSL_BOARD_REV="A03"' \
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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA