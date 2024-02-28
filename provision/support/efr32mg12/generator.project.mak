####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH ?= /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.4.1

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DCHIP_CONFIG_SHA256_CONTEXT_SIZE=256' \
 '-DCHIP_SYSTEM_CONFIG_USE_SOCKETS=1' \
 '-DPROVISION_CHANNEL_ENABLED=1' \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DconfigNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS=2' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCHIP_CONFIG_SHA256_CONTEXT_SIZE=256' \
 '-DCHIP_SYSTEM_CONFIG_USE_SOCKETS=1' \
 '-DPROVISION_CHANNEL_ENABLED=1' \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DconfigNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS=2' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../../generator \
 -I../../../examples \
 -I../../../examples/platform/silabs \
 -I../../../src \
 -I../../../src/include \
 -I../../../zzz_generated/app-common \
 -I../../../third_party/nlassert/repo/include \
 -I../../../third_party/nlio/repo/include \
 -I../../../third_party/silabs/gecko_sdk/platform/bootloader \
 -I../../../third_party/silabs/gecko_sdk/protocol/bluetooth/config \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Include \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/CMSIS/RTOS2/Include \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/util/third_party/freertos/cmsis/Include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/util/third_party/segger/systemview/SEGGER \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/common/toolchain/inc \
 -I$(SDK_PATH)/platform/service/system/inc \
 -I$(SDK_PATH)/platform/service/sleeptimer/inc

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lstdc++ \
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
 -Og \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 -Wno-unused-parameter \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c++17 \
 -fno-rtti \
 -fno-exceptions \
 -Wall \
 -Wextra \
 -Og \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 -Wno-unused-parameter \
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
 -Wl,--gc-sections \
 -Wl,--no-warn-rwx-segments


####################################################################
# Pre/Post Build Rules                                             #
####################################################################
pre-build:
	# No pre-build defined

post-build: $(OUTPUT_DIR)/$(PROJECTNAME).out
	# No post-build defined

####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o: $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o

$(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.o: $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/CMSIS/RTOS2/Source/os_systick.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.d
OBJS += $(OUTPUT_DIR)/sdk/platform/CMSIS/RTOS2/Source/os_systick.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_syscalls.o: $(SDK_PATH)/platform/common/src/sl_syscalls.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_syscalls.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_syscalls.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_syscalls.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_syscalls.o

$(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.o: $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sli_cmsis_os2_ext_task_register.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sli_cmsis_os2_ext_task_register.o

$(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o: $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/toolchain/src/sl_memory.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/toolchain/src/sl_memory.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o

# $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# 	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
# 	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
# 	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
# OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_hal_flash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_hal_flash.o

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_lock.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_lock.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.o: $(SDK_PATH)/platform/emlib/src/em_cmu.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_cmu.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_cmu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_cmu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o: $(SDK_PATH)/platform/emlib/src/em_core.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_core.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_core.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_core.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o: $(SDK_PATH)/platform/emlib/src/em_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o: $(SDK_PATH)/platform/emlib/src/em_emu.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_emu.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_emu.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_emu.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.o: $(SDK_PATH)/platform/emlib/src/em_gpio.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_gpio.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_gpio.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_gpio.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.o: $(SDK_PATH)/platform/emlib/src/em_msc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_msc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_msc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_msc.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.o: $(SDK_PATH)/platform/emlib/src/em_rtcc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_rtcc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_rtcc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_rtcc.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o: $(SDK_PATH)/platform/emlib/src/em_system.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_system.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_system.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_timer.o: $(SDK_PATH)/platform/emlib/src/em_timer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_timer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_timer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_timer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_timer.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_entropy_hardware.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sli_psa_crypto.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_driver_trng.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_psa_trng.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_nvic.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_nvic.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream.o

$(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.o: $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/iostream/src/sl_iostream_rtt.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/iostream/src/sl_iostream_rtt.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.o: $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_kernel.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_kernel.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.o: $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/cmsis/Source/cmsis_os2.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/cmsis/Source/cmsis_os2.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.o: $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/croutine.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/croutine.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.o: $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/event_groups.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/event_groups.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.o: $(SDK_PATH)/util/third_party/freertos/kernel/list.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/list.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/list.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/list.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/MemMang/heap_4.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/MemMang/heap_4.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.o: $(SDK_PATH)/util/third_party/freertos/kernel/queue.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/queue.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/queue.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/queue.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.o: $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/stream_buffer.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/stream_buffer.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.o: $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/tasks.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/tasks.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.o: $(SDK_PATH)/util/third_party/freertos/kernel/timers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/timers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/timers.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/timers.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/aes.o: $(SDK_PATH)/util/third_party/mbedtls/library/aes.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/aes.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/aes.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/aes.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/aes.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1parse.o: $(SDK_PATH)/util/third_party/mbedtls/library/asn1parse.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/asn1parse.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/asn1parse.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1parse.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1parse.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1write.o: $(SDK_PATH)/util/third_party/mbedtls/library/asn1write.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/asn1write.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/asn1write.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1write.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/asn1write.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/base64.o: $(SDK_PATH)/util/third_party/mbedtls/library/base64.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/base64.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/base64.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/base64.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/base64.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum.o: $(SDK_PATH)/util/third_party/mbedtls/library/bignum.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/bignum.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/bignum.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_core.o: $(SDK_PATH)/util/third_party/mbedtls/library/bignum_core.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/bignum_core.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/bignum_core.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_core.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_core.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod.o: $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod_raw.o: $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod_raw.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod_raw.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/bignum_mod_raw.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod_raw.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/bignum_mod_raw.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher.o: $(SDK_PATH)/util/third_party/mbedtls/library/cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher_wrap.o: $(SDK_PATH)/util/third_party/mbedtls/library/cipher_wrap.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/cipher_wrap.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/cipher_wrap.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher_wrap.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/cipher_wrap.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/constant_time.o: $(SDK_PATH)/util/third_party/mbedtls/library/constant_time.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/constant_time.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/constant_time.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/constant_time.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/constant_time.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ctr_drbg.o: $(SDK_PATH)/util/third_party/mbedtls/library/ctr_drbg.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/ctr_drbg.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/ctr_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ctr_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ctr_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecdsa.o: $(SDK_PATH)/util/third_party/mbedtls/library/ecdsa.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/ecdsa.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/ecdsa.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecdsa.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecdsa.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp.o: $(SDK_PATH)/util/third_party/mbedtls/library/ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves.o: $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves_new.o: $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves_new.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves_new.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/ecp_curves_new.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves_new.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/ecp_curves_new.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy.o: $(SDK_PATH)/util/third_party/mbedtls/library/entropy.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/entropy.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/entropy.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy_poll.o: $(SDK_PATH)/util/third_party/mbedtls/library/entropy_poll.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/entropy_poll.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/entropy_poll.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy_poll.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/entropy_poll.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/hmac_drbg.o: $(SDK_PATH)/util/third_party/mbedtls/library/hmac_drbg.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/hmac_drbg.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/hmac_drbg.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/hmac_drbg.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/hmac_drbg.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/md.o: $(SDK_PATH)/util/third_party/mbedtls/library/md.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/md.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/md.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/md.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/md.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/oid.o: $(SDK_PATH)/util/third_party/mbedtls/library/oid.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/oid.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/oid.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/oid.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/oid.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pem.o: $(SDK_PATH)/util/third_party/mbedtls/library/pem.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pem.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pem.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pem.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pem.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk.o: $(SDK_PATH)/util/third_party/mbedtls/library/pk.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pk.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pk.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk_wrap.o: $(SDK_PATH)/util/third_party/mbedtls/library/pk_wrap.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pk_wrap.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pk_wrap.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk_wrap.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pk_wrap.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkcs5.o: $(SDK_PATH)/util/third_party/mbedtls/library/pkcs5.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pkcs5.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pkcs5.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkcs5.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkcs5.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkparse.o: $(SDK_PATH)/util/third_party/mbedtls/library/pkparse.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pkparse.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pkparse.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkparse.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkparse.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkwrite.o: $(SDK_PATH)/util/third_party/mbedtls/library/pkwrite.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/pkwrite.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/pkwrite.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkwrite.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/pkwrite.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform.o: $(SDK_PATH)/util/third_party/mbedtls/library/platform.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/platform.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/platform.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform_util.o: $(SDK_PATH)/util/third_party/mbedtls/library/platform_util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/platform_util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/platform_util.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform_util.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/platform_util.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_aead.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_aead.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_aead.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_cipher.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_cipher.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_client.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_client.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_client.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_client.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_client.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_client.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers_no_static.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ffdh.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ffdh.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ffdh.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ffdh.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ffdh.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ffdh.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_hash.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_hash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_hash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_hash.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_hash.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_mac.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_mac.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_mac.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_pake.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_pake.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_pake.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_pake.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_pake.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_pake.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_rsa.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_rsa.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_rsa.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_rsa.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_rsa.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_rsa.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_se.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_se.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_se.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_se.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_se.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_se.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_slot_management.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_slot_management.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_slot_management.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_slot_management.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_slot_management.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_slot_management.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_storage.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_storage.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_storage.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_storage.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_storage.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_storage.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_util.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_util.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_util.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_util.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/sha256.o: $(SDK_PATH)/util/third_party/mbedtls/library/sha256.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/sha256.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/sha256.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/sha256.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/sha256.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/threading.o: $(SDK_PATH)/util/third_party/mbedtls/library/threading.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/threading.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/threading.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/threading.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/threading.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_create.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509_create.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509_create.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509_create.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_create.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_create.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crl.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509_crl.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509_crl.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509_crl.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crl.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crl.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crt.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509_crt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509_crt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509_crt.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crt.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_crt.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_csr.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509_csr.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509_csr.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509_csr.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_csr.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509_csr.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509write.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509write.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509write.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_crt.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509write_crt.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509write_crt.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509write_crt.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_crt.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_crt.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_csr.o: $(SDK_PATH)/util/third_party/mbedtls/library/x509write_csr.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/x509write_csr.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/x509write_csr.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_csr.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/x509write_csr.o

$(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o: $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/segger/systemview/SEGGER/SEGGER_RTT.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.o: ../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.o: ../../../examples/platform/silabs/provision/ProvisionCommands.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionCommands.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionCommands.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionCommands.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o: ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.o: ../../../examples/platform/silabs/provision/ProvisionManager.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionManager.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionManager.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionManager.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o: ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorage.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorage.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o: ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o

$(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.o: ../../../examples/platform/silabs/provision/RttStreamChannel.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../examples/platform/silabs/provision/RttStreamChannel.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../examples/platform/silabs/provision/RttStreamChannel.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/examples/platform/silabs/provision/RttStreamChannel.o

$(OUTPUT_DIR)/project/_/_/_/src/crypto/CHIPCryptoPALPSA.o: ../../../src/crypto/CHIPCryptoPALPSA.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/crypto/CHIPCryptoPALPSA.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/crypto/CHIPCryptoPALPSA.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/crypto/CHIPCryptoPALPSA.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/crypto/CHIPCryptoPALPSA.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.o: ../../../src/lib/support/Base64.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/Base64.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/Base64.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/Base64.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.o: ../../../src/lib/support/BytesToHex.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/BytesToHex.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/BytesToHex.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/BytesToHex.o

$(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.o: ../../../src/lib/support/logging/TextOnlyLogging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/lib/support/logging/TextOnlyLogging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/lib/support/logging/TextOnlyLogging.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/lib/support/logging/TextOnlyLogging.o

$(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.o: ../../../src/platform/silabs/Logging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/platform/silabs/Logging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/platform/silabs/Logging.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/Logging.o

$(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.o: ../../../src/platform/silabs/SilabsConfig.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/platform/silabs/SilabsConfig.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/platform/silabs/SilabsConfig.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/platform/silabs/SilabsConfig.o

$(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.o: ../../../src/setup_payload/Base38Encode.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../../src/setup_payload/Base38Encode.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../../src/setup_payload/Base38Encode.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.d
OBJS += $(OUTPUT_DIR)/project/_/_/_/src/setup_payload/Base38Encode.o

$(OUTPUT_DIR)/project/_/_/generator/app.o: ../../generator/app.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/app.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/app.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/app.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/app.o

$(OUTPUT_DIR)/project/_/_/generator/main.o: ../../generator/main.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/main.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/main.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/main.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/main.o

$(OUTPUT_DIR)/project/_/_/generator/nvm3.o: ../../generator/nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/nvm3.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.o

$(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o: autogen/sl_device_init_clocks.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_device_init_clocks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_device_init_clocks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o

$(OUTPUT_DIR)/project/autogen/sl_event_handler.o: autogen/sl_event_handler.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_event_handler.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_event_handler.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_event_handler.o

$(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o: autogen/sl_iostream_handles.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_iostream_handles.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_iostream_handles.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_iostream_handles.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA