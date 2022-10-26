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
 '-DSL_BOARD_NAME="BRD4163A"' \
 '-DSL_BOARD_REV="A04"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4163A"' \
 '-DSL_BOARD_REV="A04"' \
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