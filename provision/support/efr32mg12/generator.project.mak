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
COPIED_SDK_PATH ?= gecko_sdk_4.3.1

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DconfigNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS=2' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DPROVISION_CHANNEL_ENABLED=1' \
 '-DCHIP_SYSTEM_CONFIG_USE_SOCKETS=1' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
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
 -I$(SDK_PATH)/platform/bootloader \
 -I../../generator \
 -I../../../examples \
 -I../../../examples/platform/silabs \
 -I../../../src \
 -I../../../src/include \
 -I../../../zzz_generated/app-common \
 -I../../../third_party/nlassert/repo/include \
 -I../../../third_party/nlio/repo/include \
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
 -std=c++17 \
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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o

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

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o

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