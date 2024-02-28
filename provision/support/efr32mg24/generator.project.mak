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
 '-DEFR32MG24A010F1024IM48=1' \
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
 '-DEFR32MG24A010F1024IM48=1' \
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
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/CMSIS/RTOS2/Include \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/util/third_party/freertos/cmsis/Include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/include \
 -I$(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/peripheral/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/platform/security/sl_component/se_manager/inc \
 -I$(SDK_PATH)/platform/security/sl_component/se_manager/src \
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
 -Og \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 -Wno-unused-parameter \
 -mcmse \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
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

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.o: $(SDK_PATH)/platform/emlib/src/em_burtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_burtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_burtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_burtc.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.o: $(SDK_PATH)/platform/emlib/src/em_prs.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_prs.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_prs.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_prs.o

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.o: $(SDK_PATH)/platform/emlib/src/em_se.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_se.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_se.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_se.o

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

$(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.o: $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/peripheral/src/peripheral_sysrtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/peripheral/src/peripheral_sysrtc.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_hash.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_hash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_hash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_hash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_hash.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_signature.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_signature.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_signature.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_signature.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_signature.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_signature.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_util.o: $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_util.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_util.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/se_manager/src/sl_se_manager_util.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_util.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/se_manager/src/sl_se_manager_util.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_ecdsa_ecdh.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/se_aes.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/se_aes.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/se_aes.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/se_aes.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/se_aes.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/se_aes.o

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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o

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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s2.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s2.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s2.o

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

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o

$(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o: $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o

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

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.o

$(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.o: $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.o

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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA