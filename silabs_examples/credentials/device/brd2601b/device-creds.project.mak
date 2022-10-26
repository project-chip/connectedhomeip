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
 '-DEFR32MG24B310F1536IM48=1' \
 '-DSL_BOARD_NAME="BRD2601B"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

ASM_DEFS += \
 '-DDEBUG_EFM=1' \
 '-DMATTER_X509_EXTENSIONS=1' \
 '-DEFR32MG24B310F1536IM48=1' \
 '-DSL_BOARD_NAME="BRD2601B"' \
 '-DSL_BOARD_REV="A01"' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I.. \
 -I$(COPIED_SDK_PATH)/util/third_party/crypto/mbedtls/include \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
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
 -I$(SDK_PATH)/hardware/driver/mx25_flash_shutdown/inc/sl_mx25_flash_shutdown_eusart \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/sl_psa_driver/inc/public \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/inc \
 -I$(SDK_PATH)/util/third_party/crypto/sl_component/se_manager/src \
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

$(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.o: $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c
	@echo 'Building $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c
CDEPS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.d
OBJS += $(OUTPUT_DIR)/sdk/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.o

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