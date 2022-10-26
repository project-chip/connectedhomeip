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
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4161A"' \
 '-DSL_BOARD_REV="A03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4161A"' \
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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA