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
 '-DSL_BOARD_NAME="BRD4166A"' \
 '-DSL_BOARD_REV="D03"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG12P332F1024GL125=1' \
 '-DSL_BOARD_NAME="BRD4166A"' \
 '-DSL_BOARD_REV="D03"' \
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