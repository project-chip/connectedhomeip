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
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4164A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4164A"' \
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