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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztfQtz3Dh67V+ZUqVuJXetprr1sOTM7JZXlh0n1lglaTY3FadYFInu5oivIdmyNVvz3y9AgiRIgsTrQ8tb96Yma3U3cM7Bh9eH998Pbm4///vV5b17+/nz/cGbg79/Obi9+vT2/uPfrlz2py8Hb74cfDn44+BVG+Pu8y+3l1d3ONKPf/kWRz88obwI0+SnLwfLxdGXgx9Q4qdBmGzwF7/cvz88/3Lwlz9/SX7M8vRX5Jc/4ChJ8SZOAxThENuyzN44ztevXxdFGHkPxcJPY6conLtyF4TpAvlpjjAojp2hvHy+8/G/OB5F+3KAoX/44cd1GgUo/yHxYvKjnybrcEN/I7+GEWp+KyI3RnGaP7t1qMUWo+/yEP9Egr1x/pZGuxgVzi0q0l3u4782YekUpVdsndgrS5Q7mPwpJGl2il2WpXnpoHV+vIo3y5VTozo8Goen5+7qw4erW/f2/t69xCEtyeGwcNUkT/GxG6C1t4tKuwaaYOKqygrP9fPnrEztauLycBXh7A3QU+gjN0zC0g38wLdenmYIuRrf5wjd3n++u7Spa0wyZa8iQigrwxjl1i3FpeLq8rLMpU1LiAq7wia5ZEoYind7LWB9Pq5CFEfhg0vaZxzzYbexq2+Gbcp+8QMKyshyrnJ5RIqopfclbERX6/vRqbtNXhfq7cp0g5LJPhQX5ij0vRJLcMvnDEEngvI7E1RcA9/eX+HeLc7SBCVlYUvRmGUqt/0mlIvVe1EKntOMkbhcMg2LH6X+I/a6rCnjk01JQ08kDVsvCSKUW7TXiEdOkD0zjXimBIVpUebIi2lYa+V8gkpalj1T8aiEsqryFyaYL/H3Y7Qx44TIcNCDuBTNnsg5RlmRZe4lxTrN4xeQy+WeFF659XuzLJ9tVtzDLozKMNmzyGlWrtgoTB5RTr5ZRIEdZQMKroyFn7cjH/ynHSEjErHDtEH+Y+oWwaN7sjheLBnHqRcsi7ySFNz291GId1V/yfw+CnEXYn8oTT55D0Uv2Cjg1fvb49X1h+XqZhBuDFlZaxRqWF5LLy93mdsaLJNs5TfYLk6TdKdOocMkw+mUOrUUh8/lCAQ+FyWK96OPRzWU1yswk8b/mPjRLhBav6NaHa/Wp8vVJj47l2wkVBJI5TiTfIJM6OK5nh9ne1HYMiloC/w9SauJFJSt3Qz7DXtS15GpKcQt5x4lNmzyGv14tx95lEhBWf6cVrNTe9LH0impzMp0bxIpl4K+QtpFMlVX9N0isTYy5k3WezIeQ6agMPZwxMLPQ2z4PRXEMaeS3hz9tjedlEteH9pXa4NUWxtUxntSVhPJK1tnO7Tdj7aWSl7dJsOe/37UtVQq6sI9NS8Nk5o2d09+X8clry9c7SlfKZGCssDblzTKJK8tws3ifrQ1TGraXH9PbQlDpqAQYSeikF0mMVXYkSkrdB92spsQYFQ2hOpK95bhPT51nUW5X501n4rOPY5GGDIVhTsv35sVGy55fXGxp2abEskry/xkT3ZrmBS05XuaQKBESsr21rp0XGr6inCTeNH+LMgSyivN9zUuylXHRXkal95DtKcumWVT0Fj6e2pYGiY1be4+5blaCnO0p/aPZZPXWOyrfhSq9WOPPoGGR0B39+2p+LFsChrzRHbLj6k+yiSvbVfszaFqqeTVPe1tHPykPA4mMdw029NYmGWT1/g1kN5sZqivYVLThp3FPXlXLJtIY0z3qtlUxnIoL2bb08XlklrN5nw5+mr8xfC8RhynydyWhzDxR1sd+hu9vDKNQ71Go6Z3MIfTA3JmCf24CAs3LVZuHR+Amgc5KyJkoqBvpVt6xSP2QTYhzk29vrsnSAg/byGU7PSWHvpWaWDmybyiQJo92iD/WyBB/oPlulxeu0XplTu98V6fsANy5mrtqBIWuaASzpcXna04VDhmFhbH0fabiRJiLIMBUrNfmaaRv/XCZH7X1tjMU0fYTNLSimlS1UE6ogZeonmePHVH8sus2nTKaXkeIY86Lr714DXIkG/wMDHLUVj3vKAixtDCrFTuqx/StIxSD38z11/3zzrNtRr9kFolupM0OGPljMFna6wo7ZfXdx/v5pJ9WR1WnaspU3v9ejapmzlypCOUPwXRt0kl1SF6Wh9vjDpbVOvg9HAvrAYGdFZC+TvZhFvith6Kv48okf6N5hTDZNo3/FmEPjU55ebHJ2DMDN4scZztXC+Pn15DMfcA1RujUfUhR01XYDUMOxIGCa20DHKXIs4aOS3cMvQfwZgZPPWee2Lj91AwGR8SDp3mmdVMN0/3EeE7KeyfiTbRM8fS5suTjG/DOeOmk7tUtsPANV07n0HkawyPjNtU1eCraNLdNScpaWKv3Jyi5ElzEkFSUoOvXlNlhgWjKwKKpVaV5SWGDhMmKBRz3bKwjkE57y2qavAB+sHmkKH5SLJBMkp4A9KkmgUV5UB7XjIv9QbnIhENsJ1hbZtUk0ajlU5bDBZUyX42RDTAAMW2nmY2L7R0uprUKqMiU+M0BWaAKjI8Df6I8gRF8DI6XDsFl02tSbGh4pu5xT6qmg3BZXS4EIW3vVUGoAC3WGYFp4VpC08PWGj+7qKcrRfVm0Bs6ekxKAqza6k+hUBaOIhoVmiHqnjolqp/V1CAUtBUvR6wgjVhdQyR4QeZKA7yp7khJmdZUz2rkJ9qzhJX+tilnxYKoDEk17cZNoODG+CITLe+AUCrptfJJaBVXZpHny2WVVTSJqwjr9iCiekhigWQQT4YdwNmoS1hDW1QTiu9pJQO8YSWgiKVInM3KEG55hwBL6UMnlShBCPm9i7zNQGQukOUrAlQ3A2Yek2Iwof5moADVBSX1yfVNL9nJhnDORxIGx3ZMGlqu2+wo+1vQ73tSRV33Tt1KDPr+CSU5qRhn4o3NchhqtbLPN2GrU/IYAl29YCksg80n1KD3TO9VEptnsHhHjQ3svTIHsabWDh5qL9Np59/Mrt0cLjci9e7RLd/YAgZoHlGkyXaHuPksuzYprmuS9qzaM7zRjlchp3ugHO6yx1x61+B0WeduN6CzwfX5Azh5tl1F2N6nNwllyGT9uH1HhX/YPqQS/doXo+Ke+yOwwSWdX0sQYuje0io39xwDwANuep5NXO2DkeGD67+j/Hm+fUnJnq03FkI4w2XtHPXGyMSdWR42IFItPnmVBRFqt01Z2txxC2fMRmSsWLVZhlTNSjils+YioJItEDGVPy54YkWyJitw5FpAYzpuNPLGvtafKyifJ4brLGXQQtm6LobwulVmKJrIgcPa0xYjb2w1fN9FKG8ur1bc4K3TnLvkmtnLL279nyaXHTghomaxkmY5eRCBd3VMX3ZA24J1eQ61xexNI9Y6tiQ1HTxeNUCK8rT7Nndennw1dPskaQTSleKeKSis2R5nuouWCmIa2mEZaSJvheDMVwCYXSI4mnu/FYS1ueSE4Z8vStkdYRRLoGwFmDr2Vc2IBOXsu4Fnb0UtD6dZEO5P31DPr2WkbcCM12/rbb8zWGXjktgc1zZXE9zXUhJFUMkqtl+vB9JDJFIUuz5e9LEMImqy9Zb7kcUyyQWtTo925sshkss7HS52pswhksgbLOv4r6RLu7lNkceeSFwP8JGdErd2D7a0z6dWje2B31DPjn7PXnRTvqhFXP7dXS61xbwxscEOcjDp9GeP73xS23J6rYWUPejk9nzPBoiiRJFfVASgyJZV8inlCz9dQSTHcBqphzwqamka4d71NkxSigtULP05wYoQ0mAEl/3LK6K3hleudFZ7CXeBsVknsSmVi6bSp3ykiLzcvK+V5M9YbbdZw2bEWCUjq3u/jiYVGwnNtMppQF7xC+YBMpulAIPuzcvmISGXs4tCMvCrXbb2RU84oIde/ffdCo8TqBB8sOkRHlCNprnu6JEgVuUaY7bE1gHibED8Y3wR2eOeJxlnIL3YspF5EL19fSobZktC6cC8AuZnP9lU/c0pZZnY1sql1FN6Rp55S6HHpDMa2U5Dfww24pneJX9MJtauWx6PSfZWUfWq+wbV0Sup5+8g/wi2lti3VG2aM17+LHf1e7KMJp+OrHchnngYqXzK+TrHKG8TAWPJ1YXesC8m9jdDaLg9JCkOkyKnEZ2fdVIc5VGD1vPyZm6MWWQioZf9doUUUKae0yG+IKa0QbHZThVOhahLKhlAJtaqg+mikoXmeMil4aLXdAPl5cTLmgv2Nvba3KU4D037MC+hBymvNaJdZrUOFis0yhxGhqeM8dRhMdLudKkp4GslovraE66dtM+Xy8vrlF87SW8zTSDZG+Rl7kndrKCinA6DmknVnoAJdO2qN/eKkwjZXam7nMdacCZjUETpXGNtAoWXKAD+2A58j0y2AnQmsw4qjoq0qqmqUT7W6rX6jc4UZkdZUMCgZ73GIbcMGVFCwsu0BGFao/9SGtogEW7Q1BRYOeYvKm0VhtCSisZU4g0ZTtyCWOZKjut8ppGFBKavuZeliG1N2iUFLEEAj1Ng2xFCwsu0pGnv+KWwI5NWHCBjt92aGfHGC2yaOCF4mxrpwZ10CINpec/upUbYidHhgQCPXck+LU9OQN8oXWq24ksNnUjBtHGBq9QOqgsLaQBFvGTTeZ2sqaDlh5j8lwrSCeWBZ09RN7zJSAVDIFnVVQ9OCR7AzjLWjd3kLQt4vzVLr2qA8k/Qp7VQWoObKa3iPO8dX0BJW4hAe5voTtp5qfAovAh9/Ln+VsPFPduj5JJlTiUzJnYn93nLJIljl2YlewRM4sq5P+ahyU8f4s6y//gFejMbEpgSN5BzjKr71wQMk/vReAwV061BfoWd74nUTsOIOSe2PI/5HT9Xf4EXM36sAIFQaF02ECCnCLO8sZKuxqEpDF3l0K/XoUb8tgMaL1qIWeZ0xA2sRRvljNTO0cp5Mx45ymHnI9+cQrL2iAKeIFJJRjheygGU8AN3zsxmAJu+DaawZxvqeqTerBtVYcpw+1maaR0G6ysgBZ4vpcsczfIH5T2F4u7SAZ0ln1LjpeA0/dQ50cd1ZENUPIOct7uaVKUHlkgx146rPGHyPPjjuawA6iGHup87af7DGCrPwMqxe4SSCsSWuR5PyJx453ZmvTIj2ghJTyYdtscrIQxtrTHDqljgCuol6ob1CQq5NQOtOkWwU6GTFMoKXvyivDJbLZdpKyjEI512h2fiofDpEY8Q3AJNRasMwSW9ytAVQyABWMx+HrMYM5zZ4XGRlMxfQ9WqEB5M7GUgMnNwyN+0gMbrnPwBHSwQgVl7qk9jiwloEWd79m9QPUaWHGf3mFKjWhAuR/lyn4z6oLlbjFnuXNyuUtUulsUGa+TDzVwsOc9+4K8pfOw21hRw0UX6vGjUHHLs5SQDlaoIA7VbjeU4m9Ahew44vLYTg/Jh5dU9Iie4YtHH3pWybfTowvQwUcDKGR1fdycA8/7DHBlNMAOvVhQGXazVdYJdvFKax2wgF0bYUGF7FWrbsUAPWRZHRZM0UOe7zfbGwua9hR0NoCHLqnHkhAVi6geGFWxx9Rp0MncgV9Q5KLL6qFHt9q9e5aEcWhkFRZRWmoefVdRyKGRVkjPaNpS1sHLKoJeuB1DyypRPTKvImXqPPyUFsWj7ypSJs61TynJgReYx9DSZddesRWXWH+L/Mfu3nZyzSbstBwfX6nfAB2JjrEV+w1Lajp0aT2KbyIoqeG+lCDfh1mSxaFRaY8tqaLQSu2xJSlbmSeOmPBWhtIT+NKaSmvlh0Kr9FaWlFBo6d7KS4I0dsM4g52Rn6ZQ6UdtKaqhFfpRS0IKpfI7dGNtiRrTqHrLtpRNXZgi+fof9/im1G7iUaI9w+NfTSrbQ1b0syd15svLQ5jKMaKnwCL+Ilna4afAEvxwiwk8EZPLCvztyzZkdNAiDfW+TCsaWmiRhl0YBbhXXpudaJ/U0YMXnX7G/0SRpRrCgot0+HYyheKK2Lce/m91ZEcCAy6lI0ujZ4tKGnihlmrQCDcaHavpE4j0wI38Rkomh31DDVDO6EjBhCs63taUeeXhavHNjooevFALyThyf5AlLSy8WAuzWdKSnD6DSFGzd9aKGAZceKvEw86OiBZZqMCO9xdIeX/B1k6/QnFF92X4Acwsw5C+ARbzW6qeLbJQwa+Z92inTjLYQhUw805jBdxJp4njCVYUdNgiFWAb2UYaZm7CZBVsLPl4Gykfb/sYrK3QN8Ai/vY4gxURLLpACY1j07cbU4g0wawwjHRwFxdG3KeWyE9l2FGc5s/0YLvrRVFqx8Wc4BGoS1DpFqn/iIDmg4eqBvgiNWFRuo9f7SjpsAUqyCFLGwooroA9U3uIV5o94z3MO2aH2Ys7Ipe554Qc+Vya3UE5zd9AS2iw01q0yCIFzakoKyIYcEkd9sZcIwZZRdVhLauKGgaRojR6Xh4fWSowDLhIRx4+eSWq3uws7DTkYwqRpsKzmFEMuEBHHuKWL1ie2Zn866GLlFgaq00s4Y0P0dpZp2iAxfzkxK0lBRRarOHUUg/TQYs0FHaqBMUVs7u+52/ttOg9dBkl1cRssQtLS5NIPBIZXWn6GFo0UQsvoaUMifdsTUsHL9DSHRa3IaWHLlISxtZktNACDfRlBSsiGGyBiuoEiQ0JDbAEf3WGw5aGBlxOh51awoJL6SjsTMOx4KNNKFK3dvNfNOqvayg/PynUTx7TkX5lku5uIWvcZZjUT1kU4N1DJ2mCSU0l9o3jsAyf7KvsM8mprFf9bCnr0GXVgE9K9tTIrjb39roCHkPniFJ94mgYLSnRt7LYQ12YZ9RTbb9uzDPKqcYRc9jBGSOwBZfTYmUCiJGjMAfUbs3UeLxMRVKPQFJT+Lu9ItWCS2op851vrYXt0OXUmN/HPyNG9iZ+Glz9tWgVMeaPQ4tv1C3QZjN6JnoY5rkoUfwUoq/Ch6WuPny4uhU5YXUo9/b+3ujQUi3d6dQ5NbDTxxfkZB3YKAsFQiSKE6PYopAGH6w0jb8YFJwwCnG/5UbeQzH3tBkOh0NUB4bmSyJdBKx3rPMeN2cnFyIcrlomVC1krGyH0eb06Z0ew/wtD21I1exV1zLKYNPn7Pof2GR5Wbbws0wqSUXpFVsn9soSq83y9CkkkwDOBiUo98o0d1owZ0wUe2ECx9ShcagUXvkVEvVf8R2ZTq4sSBluyyW5aUJf16VEz4TomxdnESqY1xircsiI4RPJKNKwgY4egX0ut16SINl5Hm01DA1HzW1Z3lWvBDTBbOUWn0hGkR378GhmcwuPNL0kKOwX5z6TlCbbRYjhmdNzlfhpsI8a3yOSUWTZQAzNnJq75jYG2/bpEckosmyfu+HBulk179Da20Xl3szU41PQtx+jMWwcbW8xLGYgjxH+B5LdLqsqaUwiVHJz99Za/k0wSWnah4EoD0fPX1VO8/VEFLlPTqw6xS4jb9Q5fx0c3uPRaNl/gmjKwpf/9vHmU7rZyC8ezrJFNZQzgOUQ36Nv5eckem5CgSS2oeeCc0TcVQXhUmX6fcQ/LFVDTBGtdsJniadSbGzuIavIxKQkXFZTUzdvP2m3KoS5nuBy+IATtej4vO7adTO3QOUuczPvOUq9wBkiiki1kzpDy6a2XtnZ5VXD1Sig/Q0mxvmDot43ZMbnslqcCh/CKCyfyTDZ3y28PF6gdX68WsSb5ar+k/yVrY5X69PlahOfnePoZZpG/hYPhIcY+LcieBx+66fxoi4lC/zronqBcYGrKsrfnCyOF8uFe3H8+uji7PTiqJ2V+THGiYzeBKjw8zAj6frzj874u3qKoZd+/N2P1bubyC/x3wevDu4+Xt98+nj58f6/3Lv7X959/Oxef373y6eru4M3B//99y8HOYrTJxR8OXiz9qICvapPKZdhcvWtmlYt8C///T/d13d1lpFvm6OA7YO5XX1/1f7WTaC55Of+j3gATt5xxcYoQ8QeGuiCoBi3bdXEDb2xlReITBeQF41JDnMD9C7XGf9cXQf7FPrk6vKwdAM/8GXCoXg3FWx8DoL7M0WbClXPVE38WkQIZdUjcUwIklO4pOzII/Jvvhz8SMvCm+vr6ssfvsVRUryh3/705cuXg21ZZm8c5+vXr005xUXWKQriw5FAC1TNLuKQP9AiUEUr8139ZRhUn3f+ouZdVPV2sfNpaWinWxYb369iZEHcg/jzl6okV/P4xBMpcBUlrUBNtPjf5H8dEqgt2k1y/lyZhMrCCSaIf7wyLdXerkyxauf2/sol1TlNUFKfUnjV/RiFySPKSYO2iILeLzhrcLmOQr+qkdXCyCButdOL4ro4mIe77HEQtqT55OLq6tbAfpj6RUo8+A+i+trDuZ9HDGFKH1esQ3DwRyGmMSqdYXVY1ucEDAdVwqU/iQOWuZcU1ebr6SikfncbNgT4VWMwCvQd1Zx7FBMPA/0D1p3xKOvV5KiK+0sdY9zbD7/swjWO/St2aPJqyul6NXL3Xw0cwlf86c1Xk/No3F8GMXqTPbwfBuF708G8Hwbhe5MlvB/44XuzBjO/17H505+8H+rwY2d8+GUdjjtSecWskrxqp/3xXxvkP6YudqbcyoHqnPHL67uPd84lWeP5SNeE/bgI6+0qYdcCqgPg2qcdl9m0qRqfOD1+fKIRlTxHj/3Zp9caccvfmy0zcpGJ57caJDotVjqR06LadqwStW59SEyyeEsi+/OR31XdKhkokuXBT2T4dvX+9nh1/WG5ummVcL1/kS5VaNfzY2GpVgcNhIVVHXPtZrhvt4Kb5iU8sB/v4DHz57Tyt20g012wsLAFErY7yqDELaVXTMHixp7bjG5TeBNj+Bz9Bg6LLJQzVMbgmOtsh7bgqJvMz+Ebm00WwhcvAurCN7bhCj79YeDBg0a4AlgBdX34coVHmigpkC1ccheCNWx75nALoUemAW2nR4vQzsvh5cYFfM3I/AReaJbD+zQY00bpIrBFuEnwYB8cO7fQPeZpXHoPEXzrkJfiYZ4WqGsLN0fwJbewkGV22hg69w1v2zJvZ4bgQHeFjQbxyYavQEDdNIP3F74GKbxhCShuxKEaxphOvMPA1TvK3U6uGSyd7ShKXJZ2GQtrNOvRwI7EClAf0rQkq7MoHyzlDT6KcOoTcOQkR7V6UhRIXFUGcco0DoU1oR+nnapiTuApxNaIg+g90fIxyOT4Ttgv9+KETMLQt9ItveIR9xSbEGeusBGmSNX6e5sTcrlXx5lnl0Nql9gbK2xwX5fliJ70kUzDCKVeTjWLXSVFOts7EGpQKkFgBhQH+RObqYhZ8hDEIkvhVZxqC716lGYZXSdqtVwlroW8qFsv0ozmriP67Ipq5OYxYMl4ZI8V/v8q6uX1SbUo4EnHJgVgsE2BZK9bryPLlYg+UJd2jchV2oXxSJKr8hdLNsq9KA/ihqsX3t+Gwu6xH0Hsqg7DM8fS5aNJtfSDKLlEfR1EkK8/bESpqWNOFB1LSMx29sLLzOT1IkiM74fhddKRe/F6l6hRyYwOexFqb0ojilY5kBpi9WJILkx2cUjnrlLrSGNDw0u1NE14UneUIrQPokpHQYqaqoKsEoEUZJXwVelSiUBLl0qUuogIYhTI3+UhORLLbFJit6w124zbjWjdhh1yBWCE6p2IoiJiwpPGSYjdwQKJJ+50aMj+ICtJIZXIQ9hOYvdKCdP3Y3hMcq80NOjGglByy6AN0NXpmQ3Y0+UKHrb9GhqWeW/aAnJ3TQMgcmhLdHsTHWj+dZv6XU88UaIDSx8sBoOtb/2HRGy/23qguNWMS/VMAh4s5cFXT+xYqOI39Q4Ytv/OOyByCAFNIOpreequonlUk30NEQKY3OjSvjEBBRgmZHssHjuX+Q67UAH7YCIUB7GzbZ5qqo3evUN2SGe4dCelS8Y1xGnRb1bleNr7fsA5cAPb3fkEhd59JTmc1wRfI6/c5bCWKVAzVnMDlKEkQIkfwlAw3QRTfQHaBaYdC8vCbe7zgIHlFkiaAdW74nthoo/H7YVrKzHNB8NE3qOzQdTVakD0ca22A07OtkBDV4vMgJhTrYSQIq9WBpnTRo2LzjmAJGxypMDI+TogKIkZSUmk5Ek848aDoi3d6PxgsdQxPQeNnDIEA6tSKQnVHO1qrNV8lrXSVHw3LyVcxAEGTUurQTUNg/iVBkmM7qBluyrbfiObjhFGaAZCk8NgqCdmhFEtKcnMREqDmUkLB3DSdqrmRtvMqifVqzbcBOARu/NIVUNjFkaDrDV4AFQDH0L/Cj44ML51RvctrnOE8jIt6uNC7W6Z5msXd3XplKmVwaYPI4mg6L6Y7kzTjK24MHV+YRc7xWGSqdkHYfz6HO8Go2RTHboQg26eaA/rK5tkgHNHbjK49nDSptwCaajZzTvSKK2RDXGw95Qj3yND9gCtSZWdGU5Lo/ay0BArCid3gktjxKgocN2lzwIao2U7st2rTOemBZTQvuZelqHJXdTSWGTua2a3sDxOnv6Ki4Oxnt92aGcspkBxtjXOtOouEjLgA6jC1KuCKU1kz5gxBnEatJNV1TDNhrYpcc6Hy0vn7e012Sz0vvoWHrHKPN1EtrDXKL72ko2zRV6GI2uqrEu2ZuR++dEEIeVGu3+kBUYy9tRbLJ586zcJkYdTO6/lMYpkCYHxNQ9L6dZqCuiBvZVBHyXcTO+elUfZhVHgzhxglUbyvRhFkXlW+b5xovyth/9bHcHgkOclzZFwzP7FRfpY9TyrKUo8eVBCHqPaaXa4WnwzR6qun8kK8+KTVlfulO1bqUZgZd6+JW6CU93UZQ5i3JQGW+O6hfxganOxCoZ5RiP/18x7NM5ishvAFKJeUzeGmVnZlQbZmLee28dg6jSwPAbZLQVRc7j3xumjBeYIp8YQ/PfnTSAHj8cbQXWvv5vA0CfcTSCyyc3Ciq+pmyE076GbohiXm2ZGFAoHpIdswZpHpo3AmIe8jXBGT3AboTFPaJvg9F7ANgIy7zybp6gNMehj0oYop+YVjL7pbAjRPclsDDR4StkYr30I2RSpe8bYBKn3CrERUPuOsAnK/NEN5cd8TTGax3gBcIxzin0MVwen/yitGcLEG7JAoP3nLA1BZw9R6T20aggks1sP7g1TYHDQvGmfCTXEMXWopt7lNMVq3tQ0xWmfwzQEUlrDmsGZPd4wCRSFD7mXPzvTpwHEUYtkiX8ppGfjeQD11K8mAJ3y1Y1dT/UaxW73X+va/yFx4512bDoBatImtlBz+0wlY1fLqRAQumnQasW72Owcp24qWBDj0jFEe/IK3BxqozXzrpppqycYtSNrlwwc1fV3+ZN+U0UQ2pMS0+eqJHHMcqGZ19RNCj1qhEf1szu0pDE0k9HNRWqKiKc20EvENGol4qzQ9PdYBK15ZRaADPXUPRsWAftrkxdICQHIVKKm/TMvmLnURBx98kC5OOqjfkyjzqmJrpvkaqZSm9vIy8oejXysJrpuypvRgC59byZUF0R0GFEBYe7ojzKMrlGZd4aM3LYxEICiKJw+3aUENH27jQoMHTS3O/3MhQ0RzTUaOEcDFHMtM4fAlGHM1Zh6WyyU+jIeB2X65JoqirmW3EuCFI9B40zbjWPR9J37AYq5FoPurgcCoCRKS/GpVRNEAI3NszxgSJqacnKnTVS6WxSZtIx0DUwzOdXKTt0J6fJXx9nIM4CmCSFHVsLJ28ak4uPPy2PjdrBDekTP2unpFqg086ZaCzKIi8sp8vSd2XYZySy+divQLhsZxK/ccVMRFEReSYE21RGs6oDYU4i+0ic36T+yBUoAQx7wBFJUQdWqYi9M2he+mjsCvpuH+NoT13fVx3+0x/i+GzvSG0+xFXGMTfH/DalryMtqboxi3KRF+Veym/3/21PXnv/wluzeNV34eftkMf7zO2pFd34Tf+/W/R/u+9c3767J49c//gWb4MuXH+hemp++HCwXR18O8DeIPMmJWyr81S/37w/Pvxz8BXNiUsqJg9BnxiUfPSag9DL85zsf/4ujNlgHFTQOgP/7cZ1GAco7/DpDe2GakCHO2DZcwXkrGv9KVSCcMbWJyZfVhdPkm6q01facBee84A2GPfFwNxg+991vMHTetR8WaMbPq0OmgPuMOBjB5AvvtjKh/yY7GMvMM/SQKaEjAsvoo1fndUh+dOrWiv2KboK58cot/rjLQ8Jb7oIwfdM02E7TptVYbUNeffrBTqM88Z62drNMO12pdpn3FDwO0PagnbHHOSDM0/EL9WDQ7I1T7Bv1kAT8F+4hGepbF+rH63No+X1wYOXtRUU1PnjmjvBt6a9yN6x2y/jgyQgHLaZLq6ZtmurivGr10xZh5bnYTROh6HYoW6OqXzgh3y2iQA9Xt69pG+q9dTazc0baHc5gyo3b8QyiNMvznLATMeonwibDT8RiXhQTRJ0A6J4hk4o/JaMaFCsg8GsF9121UfHCGQHyYNu46qip5T3XBiGWhyuvlVNZFTOT3uBlmptdAlbHq/XpcrWJz85p06Zto+Z2sUlww0xlHjb0/DiDl9vCAgqtr/OE1lmjAspcu1mYFDakdsiwctO8tKW3gYYTTB8GAtZKUQFl5s9pe+0mtFgWG1QyPZAGr5cCA4otWgcSVCqFhRNKhp/0IiBgrQwyoNzYw8CFn4dZmdoovGMCUPE5+s2OaAoMJxZZacgQdEOGytiGzBoVTuY626GtBaEtLpzUTebnNlyZFhdSamij5WpgYYW6NvzYDhhObLiykf0UFVAmffUdWieFhRMa4ebZgtAGFlao69tophhkQLnk8cMC2ZDbIYPLJXdA2ZPcoMPLtlMueuDwousroi2JrsEhRdsakTHIkHJ3Xm7Hvg0wnFj6pjKwUooKJzPzExsWbWABheY2pmQoKqhMOw1XBwwrtgg3iRdZsi2LDic7tzJQzKEHinkat3fgQ2tloAEF07fWocVSWFihrjWtrhW5ObLRzrLQcIILKxWsgK5gtpwXC64L3elno8iy0ICC82RjQyyFhRO6K+z4hC0unNQnO/MHT+DzBwTRTTMbcwgsNJzgr0Fqo7Q2sLBCsXNsw0FkoU0Fx3STIJhMFhB8/wWQSC6wlQ0YUkEFgUQ/jw81kFNMqvuMwsSX3F802oHbvi42nze1rubRvy6WjOE5m2ebN+OYG41U+HnxtZSEDBT6VrrkkRzsqG1CXMYkfJaeKiGWnq0QfdhFxT5NHD1GryiQTGc9KBNtLM0yoVcSzPLfLUqv3EkMq/usXax5VrkWZFibi1yzNs+XQOH+N5rG9nXqeSyjsqWmhYllw95lmkb+1gtFza5hdk2ZpD6cJm2SVm1jnC6+VJYo9H+63cx8SqtSpFDTuwTT2jeCUej9p6xvKMZUxcb33SxHdNu2vpoxDliZAHZ0HtK0jFJyl6Cqs9M/NafTUvYRxDWv0+r0ozpjJKMGStWIl9d3H+9U7XdJDiprtXTqW48Hhq+7FHIMIIxk/KsqfQ4R3Lr/YwjteldDMc8FaIphELS1lL8396NrCelHN7TIRmYCbNIaG9U5rqEGcozUj0/0JDCRtRWQF3a9PH56rSWhF3tfre9EhSUHo1cvWtfp8+IyVqzEDgoTja6dlWlRvXGiJ4GJbNuzUj6hM04nmRchaoW9GZtUeqilH/17dRnwGEDnEBZzkFavKpi6vZyTvMLySNPqMHEbH48PZ+J9Du9pAJPXgEGJk9p7LKlNecfxvLTkSWYaTVJbA2a72TEduo4u+CiW4vaHZwM6lJ3AAyw+kAo7ONBCBCWvAXthH6Q56/4ykysNu7xVmxiNSVkEk3xuz/znpcT8l0hNg/K9T/m01pNuGtsU03aRRQCzv7GaBuWFK1e9GPYyVYsuxJG2Rr4815Ga0jyAMMleCvWI8gRFhno6kO+9erEGlC7TNM3NWkYfAi4PzPR0IC9dxdorwF6omrX8CqW6jdOW7B6KUSZ3V6JtvajeNggirAcHqBDQdn08A43hAFShsgzl8aC++2arK406CW9aih4KUG4YCBrCfK/zKCgO8ifVWRSpjSk2igvyU5kVsSpR7DJ9G++F+w9yi+cL9ByDy0OJWdz6silxU1jbkiBUbcw8lHa9q2BJg7qO6Nst6qp60c2UVG+DaYloYn7XrS6bibK1qUomqUvDyEaW1mI3ZnWr28Zk5uh4aWciG5d3PQUKPbtMbdPV0EUHqG1aIpqYtmtbFD7o1zYcuRJ7eX1SLYh6CinFcR1O/O/XmxBbCmrDKh4X+ttQYitxJap2A7ooGjvWSGyZNYc+n/zKApeOeW9ejZWJqLkbVj29/Vh6aZbdcNpLr9F+Uxz/QWbbZ4/xQWbL52S+Sm5v7eepye5WHD/34vUukerzGFYmlh6t9JaaHq3iNhqeiXOpgULPwLn8GIFLqOJXDIhVvQqOAMmrz/rUSteaTZFqNlLDuHoSpNaBe8QKq71jOrnrhHp8KlcFjQmlbiXo8SncOMCl08vOfkTNNkrqOHO/gVI4qjwmrCeYFSm7SCakmo3FOLKeCMkJrh635GzWXk86UH9DYrBOEkLG6V0Mg65FkY9GMWrZFSnbSPrNqhojMjFq1Uaq8TVR9JtVNT4aw6BlU+NTWf6YbNnUKLtIJo2KGqfkEgr4/kEfCyyfVQer7GsfmjO53fMyxS4jtwubXOc++fCVUtaxDzd4vo8iche/lNfe2NFhDeOM00hfs3HmmExPGTPQaZyEWU7u4JJaC9dPw4AIIAnksQn7ecBjsXJweqoWKK1nTBjLxenM0+zZ3Xp58NWT6ZalbUXXe3kMpkf18zyVWoNWUNpiGhfABh7elAywoUo6cvRkzmspqewDw6hEvsRzEzoqKbChypZg6wHLHCCbl8zujUL4wtnHBmrGLYkdgu+z3VZbdxS1MXBdW3MCtwM2zEPcBriezPKokkQG1bR18WML+hhUU32x59sQyMCa1tKtt7SgkIU1V7g6PbOjkQE2V3m6XNlRyQAbqtxYqS8bsPpSbnPkkafULKgcYYP2xeCNeB8bti+GFjsEh7HskxftkIXusY/9chdlNWkfT40QhUEePknusJ4Cghhd1llbXX+o78F16ek5bw0qQOGmowGCSJlg5fLxgWplDSh9bkTNyANwWMl0Od6W6A4eQHaBmgV1N0AZSgKU+FKXnaiInyGBGUjHXuJtUExm2sCEc6EhK6WXFJmXk+ePm4wNs621KjrDZjVRW6lNuzBJ2irv8NVIEB5l7Cs9lMpqcjzs/u0rPQ0XjCcUloVb7fkFVD8C/v4nUIZvFheeMgjXymFSojwhB5nyXVGiwC3KNMdNoYHryZibeJ34ozPHolpMJkv/fpIiYgJJTj2bD6q7hVSulqolfjKDOI4kWBKn8a14faC6ufCwstfIK3e50YhyXjhLYNFhBZU/QwLusIIJ50Lb8R3Ivl+yNAxsdhGTncSUzxl0+ZljebmpFd19MtM/DJ2PXRlGk97H+ELdbZgHLraP+l6bdY5QXqay13oOd8aQS+NMJpGU72Hjlsfu7ropD5bY02HM5DTpru+9a+5l6wHt02NVv/2Pa4gmVbPXAIps0VzHNwQzbDRaOFzj0+lDecrqWrjvbqq1vlLDpHqQeWXyoJX5aObD5aX2aGYA9fb2mpyTe6+Jxy0cJKEalbe2sNOYycGpdBp5ToOpN1yYlBl7fj69xmGgtQXWHN9oDiJ0xx6DUnGN4msvUd8lybXzFnmZewJQIKgqpwO0PjYDmb6A6AwET40IjUhlOGqPj8wIwsUbkybTswXSklgkQ1F4WJIj3yNTDAFak3WMWXdcWuI0runWxifiBW+wBTIAmUM0Q3HvMQ256dZcGItkKCoKZ54flhbUoJjuBURFgYeV5H3o9czsjLSsMZ6pwGxHrtgv0/nhnLzAER6AwK+5l2Vo5u1bJXksmqG4pucxF8YimYrK019xIwRgLRbJUNRvO7QDMFMLYzqngeJsC1AfOxxTQaXnP7qVUwiQcUM0Q3F3BO4aSNsAzNhu1X2hUO3rCM50D5pXTF/EIq2qQTEVQ86FAeRgh2N10oTvTWqPElgE7dt1eg6UtpQhiracykvRltHE1qavW2Nt/ja6/uWCveqqLWQEoy2I1FSDgtFG1xdQ1019BW38F77Yj+631JupjsKH3MtF0+MzZpw7CDWyIZXqUFZH+bDTkLxIlhi5UKhaIwkshJGQr3lYGgppIbSFPHgFOlOYIRqq6OJrSxDsMhNK0Nk3xpVQjV9MdbQg+j3hzCE8oQjlg3Zjctff5U8mtbSPYSAlKKbP+UmooNG1BcTT+9CE7LHivrJhtQw35Clc/WrZxteWkIYGyaeRtcmzmesghOSZ/LUQfPJHvzg1oG+iGwgwYTekNuwfGQADEYZ9IwNgIMKwP2AA9NvA+ki/QSvYAZiKcLM0mn6hQlZJi6LfWZe5G+QP0yddxD01g6AtY0tOd5rp6EHoD9WqA5L6Krr4+lmSJkXpkU1FeJBjkC9DGP3BWnOKUF9MD0K/FaG7tQyaEQbBWIZL6My1tDD6Dk7ixjuF3SsjB6eNb+hjtTukDbSMgUBGItqCBiAG1Xp2B7FEfVbfIjzXsgDk1TQemMQnrwifFFZkRBI7PKNRXXsSYO7MttTYbohkKMvUXkMUGD9HX84AxWD4adgMMAD6IrJCdJBArKOHYSRl/rSJlBKN0yUcIcQ3UFkW4ynpMIyklLnnG1SfHoS+g+EFs08jiF2LDsB4yKYv4tG8xjRjTwMRLYC2iJxcmReV7hZFats7hmI4QPoDlYI89Pqw25jL4kIZCfOjcO7wi5SiDsNIShzOXKMtJaRBMJKBQZfHAP0zHwtA2iN6NixCfRxtSd9Ojy70B1VNbCN618e9icmU2QDEVIzBIJNFMJWhsIY/IcNsHb8GKQxWt1gEIxlVp2Jukh4MhCBT4/Rg9Hvt9makphXXnyThQQEIg1AEZaPZSxpULKR+A8NMxhkuJHOhIITRg8ft9lgIhRxMCKlFlJYyt9eoSOVggkilNxmASOywIKQZLeGPcSAkzV57o6JJ/U6baVFzV9eoaFK+l2ZaUm6y52CMA1LQgcq4WfH2t8h/7B5CIrezG8xx8sHAuib9wfcYCLBrgpDVQYEIm3t7TEmWwotkIlHDvg1CHwcTqqWHkEdxwFp6CE1b0xdTGSzzaYQJMBBxJUwZozhQPSOEJIoD0jN6SZDGbhhnBqsj03hQnTeItBoHqPOGUFSAFfahNw6ibowJ6eGDSFS798zwvXPFc+dGe/RnbOmpHDxtjNee6KSfPZDTpl4eatTNkRaKYiqmSJYAYigKgBjNJSOeIo3FI64surPfWFOHYyqo3tNsLqjFMRW0C6MAOyNrhWtMJkX1sEwviMD/RBFEfWORTEX5AHlHQUylbD383+oIQA+DBCIqS6NnKFkNlrGwaqyuOeIfS+ujmYrTHFiPZGmMqvmCtDz3kRxlv50vpnrj+HC1+AYgqYdlLIzkP7mpEEIYi2UujNmCDKGtD2cqr9m5bq6MQTK++udhB6CohTGWA+D0BiBOb7AF6PMoiOl1R36gMSs01NKgmIuBqPktjLGcXzPvEaC6M0DGkjTmFcdyFCcV+VLoESJzOR2QqSS9baAjQdoXjfflbCBc2w2Ia7t9DNbmWhoUUzHt+SZzRSyUoSyKCebSjvFMBWosYI1EKa5dTQg5hVByCiEFxWn+TG8Scb0oSgHc7AlQQ6kJKt0i9R+RzrLCUOIAzFRaWJTu41cAWR2QoSRyVNxYDgUxlEIOjhtLoSCmUjS20o+UQFxtRU6zLxUuwZ4W0+AACAJolVoYUznN8UxzRQwSkCigUewIDkpedYwUTl4DZyovjZ6Xx0cQhYxBMhWVh09eiVzP91EB0K+M8UwFFh5UfjJIhqLyEDfHwfIMYKq3B2UqC2L0q7xIzZVCnoA219KgmIshVxFAyKE45oJOIXq/DsdUUAFQwSiIuRTX9/wtQAfTg4KQVc3mF7uwhJgL5CFCiEzTxxDKeC0WgLAyJMMNGGEdlqGw7joOY109KFNZYQyjqcUxFESfCTNXxAAZSqoOuBnraVAAxFSny0AENUgwogDqHIsEIqoAmGdlkRT2hRk/AKLz9Ohwwa0+NqVtAvKeZYdhuvpXb2Uje0XKMKkffSvM+rhO3wQsrGQ86ojDMnwCltyHhZFcr3eDyOygoKSZTVz3pEFtxOhts9e9MoSjEPq50SFsUqJvZQFdmebh7SQBuHLNw8MkAQPnBkNhRm2LBCPMfFqP0QY4s9duBxc9S6yir4cGJDD8HagYtkhAwsp858M06x0UjDTFh4JmlEE9EUThnrxoB6Ssg7LiCFq8Nr9Amw3+rHVrfvFclCh+CtFXo0derz58uLo19WhrFPf2/l7+VGmddqdLhlOjOH0ww9JWg8kXM4EqgPLPJA9KVQP23RZ/0c+jsh1GIfYR3Mh7EJ1X4cXFsaqzo3oViy7g10eP5ConN6PJnUn18v9snWDT6jDinb4Opwenf9NRizJb+NRFSRa/fb/hzftqcFQmyxZ+ljHWIL+/cf6WRrsYFc4tKqoHqAtnE5bkra5i68ReWeLUZ3n6FJIJLWeDEpR7ZZo7LZwzTRl7YQLJ2eHNkCZP8XGvJBpRNmgzhMQSW0izbmfpbppY13XZ1DUw+ubFWYS6JWBa/hlZfCoVbVp20VEmabPLrZckKLKviyGa0XVblnfVK09NcHt5yadS0WbLZjwiqbxMY9w8B8U+KkCfS0md/aLGMMkou0r8NNhPu9GjUtFm3WgMkYyuu+aOI/s261GpaLNuszv+MXCBrndo7e2ico+m6zFqKN2XIRm+GZVvMTxmIk93/wd6tiZuTCOt6eburcXcneBSUrcfo1GmGWV/HZ8Ll5ZT5D65m8Epdhl5ltn5K/dsOJ9QM3cmKEX2v/y3jzef0s1muLivzRvVYM4AeEbCPfpWfk6i5yY0kAEaIVz4GTl3VcG5HC89KSkZlsMhqqwAA2PMShBZASAzhvyyGUBKzmU1i3rz9pNBe0U01LOxDh9SUBePz2vHQ78QFKjcZW7mPUepFzhDTFl6g+TPCBhbgM5i33jlFmR8DAnftO82MIcFFZIDZwIwXLPYAAn7+++/uzTbUECmNQ7rVWhIDnbqOIm8okC4jc5RltpIUJ8sTFWJ2LnHtnS8Q0+hj0gzSqYhP5Em7er97fHq+sNydeN8NAOvDU4k6iJcXt99vHMuyYSooZYa6fb+893KFArncmW0oLKdGyY4pwzSiOIgfwKwFYqJp6AOMFoEaR+/9uMiLDStNY1Kn9TWK7hCWOImeQ8Rcj5cXjpvb6/dy+uT96Y5Hab1W+gm2VMgH/9K1piiekNJgpKymtenJ1AbH6/e1bMnGtzHIdyhWmcDKJaDhWkouPZRdJPaS2bprRQOcpym3t4BYcKp5U0dXMmVK8O+o0zTyN96oVHL2FTjOt0gSBFCGTlQx8uW2PPz9B1ak44BO4+dB9z1rKvj1fvT5erD9dm5bOS7T+7bmxv35vbzzdXt/cerO9mIdUX/+Zdr9+7df7j3/3Z79fad++nz5dtP7t3959u3H67cm88ff76/ur2rXPJqlweOt1JQdvn5+ubzz1c/37uXb+/ffvr8Aeu8usOfZTGu/3r17v7THQb6+f3HD+77j5+uemL+V1T+K9OsNDsf/6yKj8dJ7uXtf93cf56l6t2NrEh1e3/v/nJ35b69u+7hHknbs91r4N5dXd5//PxzX99vu7T81y5Q/XkATlu19/016TLw//Sn2ZByLR1xNPD/k79J/+pufH/hySjAAWWCSQWKZQIlKa70g4CkTfmcUZuTDx8DHLL7drHzF227U5k+rb6fC4aHnrteLuGmsETfDuOTfZCvB+Tr7OnksBiOhe1Q41F46XoPYU9Aka7LtQ4/6fxwBubz9E2oBfZeWndgvEuBRPoUFmVL2sqLSJF1u9ijqQunI1RNAhkGxg/CNLTBvstERGHyKEpBHYb+c+fnYVb2CsE/ZXn6K/JLx9uVKR6KO3VIMuBdRIH9wrHeJT750sXeFfm36Kkr893Qh7SgIfBKD5RfIWMSL8Hdl4vbxZdIeRqHpbvOcTPsZmn16uwLiEjI3nIfZS+V/Zg/L8twzxnfTHtde1nVG+4/3b5Lbk4MvDzod4oXF3vg/vZtgv1Pf1ou7fN/9fIkTDbFwouiFzB9S18dqnhJARkKvKQM/b5r4kXFPkV4OapfDS5eQgYJEYe/Vwu0fQcp/F2OPvYeUdVjenm8IH526eUbVA75J4KN3NLDGH/zk5pzaiih3O7ih4EI+p198qFnfBjjb36i/vFhsDzbiwiuj4ylkO8P8fc/KfjLI5Ku4RPq6YJOtdCHeIz4k2wzPYNPl/fkxJCdvFONdi1IuuUGlsTrvw/XSXpYf/sigiYcmkoW+9v+ylLT2LrDHu/wP6tv9msmu2q07DLuig//k373QraxpkjFPpO94+Fn4/KrahmbWlRsMj1gPFyT3w673/ZroL0JU7HW/BD/cN38/mJW27tApfo3P0Q/XJMAh1WAwzbAnqvl/iWqWPD7mLoDtPd3mqDJOdXpkN9rUvjzOBPBpidX9edUDRVVs4qjOcVDPJxCfvET+XVR/bkPLc1El1t/dmMv66v6PxTvyw+H11720z/98+df7m9+uXfffbz9F+ef/vnm9vO/X13e//z2+upfFlVkCc314tgixCWDTnAP5dKNQWnW9yTQOj9exZvlKlsdr9any9UmHi3/mpkpLHylsn74n9Grw8ONP9X7wLRC5jIFhUsYf7ibRi9d27Qo95swNaFY1qLeiFEVziI8XtVyg3JR78YIqpfKqtW7xSbZLZjGn7z0NkwPAzgIXQdakAQv0nKL8ggnb795yj2cPRN+HaOiwDY4jFCyKbc/DZfjbZuaDOBVjM2G/3/d3Ar+gHIV/IY/tHCHX8NyezjeRm69HtIOTVAwVOH8MPd3kZcHKENJgBL/WW894PtJUYLb4GDkfcjP5utVXH53a4KlkjU/OtTPqD798ONfvsURCVrf8YkDLxdHVWRETkGEyQZ/9cv9+0PsW/ylBmj8lHZbzM5fxGmwwzWqOk2xCOqTgHeoLKtVk/a8w6LasYMjYogM5eXznY///YlcaU5dH8eeqstqD+BNHewG5/xfK3MqiGMLRIYBqvy4K1H2Z2zU3uc9GdokMRYtvfOpMF3jDu92ocMTXlPcPxW08HO/2WPn5zUjvRmyrQb4q/YkB7/ZYi8f6efiwauDu4/XN58+Xn68/y/37v6Xdx8/M5soD94c/P3LAXO8580X/MUX7OV7TwiXjNR//Bt5t/iBnNP5Qn7+Ugcg/4dbgCzEoYLHT6lfzxzSH940f2yQ/5i6RfDoniyOF8vm61fNH2Rf7udsHI8/VqA//lH/D0Y5oEd4/7F0/4EzpZZAticUOA/+u8sEhEexb0gWv2qLWrUvl6SQ7lknG0HrTcbdDZg4dJqHmzDxIho82UXRq/ZOZfzF8lWFUKKE2Ovi/OL8+OL06OyPV0rkzO5McuxQi/tw+fr89Pzo4uJ0qchebbmkrbVJ6pcXR69PTs9PVytFAZydsRr0x2fL49evz86PFNlxzjOnbtzAD3wTGWdHy5NT/N+Fooz3OULkKNGlPjXmPVodqRc/bIFu87lJ2g+Xq6OT5cnZ+ZlqJnhZ5tJOIUSFiYaTs5Oz09cXq2OzYoDinZklzlfnF8vXqxNFGdWJq+rQg1u9f2ok4vXxxer04vjotbotxm/Yadjg4miJc+P1a9W6wPDTPDGSsTzGrTJWIV0vmolQrAMXzCisO7TqFk1dSxyfv754vTyRLg6NhNv7K9wn0NM7hV67vDw5xj3n6etzjfS3J4dcbAQvSjXzYHV+htvnizOdLGCrpR9hL6QgCyrqZsCd49H5xWol3T8yGtATMcHWS4KovmBHvXs6PT1bXZytpOviJLtW4lfnuDlanp5L980MfXNUkSrQLIUnJ6uLk+XREYgCLRscnRwtl7hRku4aeAqqYhhWr3j72qY4v3h9jtsEDSHhoGV26U+6TeP56yNcN8+lW+gZJWXuJUX17pqJpvOj47PT09WJRjEJq6ONAHZ5jUvp8fHJsUZLUWvonk4w1rK8WB4fXxyfHUu7VBPLeBql4+jiNbbDxdGpKvVoBK5Dfrp6vbrADdcf/3Pwx/8FRCD1ww===END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA