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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztfQtz3Daa7V9xqbZu7d61mpZk5+FNMuXIikc7VuxSK7M7M9piQWx0NyO+hmTLUqby3y8AvkASJF4faE/dncrY7m7gnIM38AH48I+jj9cf/vPi/Ma//vDh5uj10T9uj64v3r+5ufzzhc//dHv0+vbo9uj3o+dtjPWHX67PL9Yk0nd/eIyjZw84L8I0+f726GT14vboGU6CdBMmO/LFLzc/HX9ze/SHH26T77I8/RUH5TMSJSlex+kGRyTEviyz15736dOnVRFG6K5YBWnsFYW3Lg+bMF3hIM0xASWxM5yXT+uA/E3i1Wi3RwT62bPvtmm0wfmzBMX0xyBNtuGu/o3+Gka4+a2I/BjHaf7kV6FWe4J+yEPyEw322vtzGh1iXHjXuEgPeUD+tQtLL0ZliXMvxxFGBfYI/UNIU+0VhyxL89LD2/zsNN6dvvQqXE9E5IkUrS/evbu49q9vbvxzEtKZIAGPUE/yEJ/5G7xFh6h0nUkTXEJdWYH8IH/KytS1KiGTUBMp5A1+CAPsh0lY+ptgEyxQr2YohSp/yjG+vvmwPnerbEwzlWdFhHFWhjHOF8gtIZlQGcoyv+5oQly4ljbJplLTcHxYuKL1GYUacRyFdz7tsUnMu8POtcIZvqk8jO/wpoycl62QSaapzu3lpI0IK4XfedVwKhpa0aFMdziZHFtJpY7CAJVEgl8+ZRg+GbUCb4JMmMnXNxdkzIuzNMFJWbjTNOaZKvOgCeUT/ShKHZQ3l1FCNpVuJojS4J7MyRxqE9NNicMPNBV7lGwinDvNsxGTmiSXWTVimpIUpkWZYxTXYR3W+AkyZWEus0tEJhXG6mGYFCVKgqUybsw5ITMcjCp+jeZS5hynqswyR0mxTfP4swgWsk9KZ5P/BXNXzDcr7+4QRmWYLC5zmlcoNwqTe5zTb1bRxpW2AYlQyCrI2zUS+acrKSMa+XRqh4P71C829/7L1cvVCTet6gXLIlTSCtz+Pgrxlo2i3O+jEOuQzJXS5D26K3rBRgEvfro+O716d/pyEGyMyLJrFGpYaZ+KEsd+m12KXf6OZIrXpNurkudxafBamV6lwxMReRJtJcrLQ7aEOBHTUF2vpkxm+2USRIeNNN9bJvTi5MX25MXpyzB++Y1iJ6GRwFqON80nKYQ2oo+COFtAYMujLkx5JmCnqz/4S2XdHXIULyGsJdKRVgYLSauIlKUF8WEJYTWNsixqyFtCV8OjLozgJ9t0EW0dlbq8GJF4RZCHWZmqLgntVI4Y1cVmUbSIxJpHWRheplFgzUaBDwUZNxdR1jIpi9sWebBIs2iJlKXtMjIBXUJaS6QhLVwk0xoeLWE+ndovpa4lU5a43y5U41oiDWmPCyl71BMWni7SEGoadVlomeG+4VEXFqBgr2q3t5PWMimLu8dPRYBUTR1W6jgqZXkRmRosoa3h0RL2eIcWmR7xXNoC6WGAMFlmuSXiVBeM2ZbyIjo7KnV5S40UkfZIES00UkS6I0WMwugufVxCG0elLi9D+7uFFvw8l7rAYpERraZRl/WwiFGpplGWlQXJIjPNhkddWL5I71vT6Mjyi3CXoGgpeTydsswiyFEZ7DO0WUJln01dJF6wr+uRqUtcxiBRaBokCjIp3Koe1rBT1jLpiFvI6toxKYtbbLqkP1k6LDZbOuhPlxazgOkbwB42aJHa1vAoC/u0UT5QZSWs4ZEJi+tTTg418RSyXc/hRqkzVSImpU1PwZejr8ZfDG8BxHGazG2Jh0kw2grvHwtCZRqHZnW8ovcIh9cD8mYJg7gICz8tTv0qPgC1CHJWRMhFwY+lX6Li3s/xLiSFadaD9wRJ4edzCCcHsyVQP1camHkyVBTYsPcdlH8LJCl/sFJXK2u/KFF5MJtR9wk7IG+u1Y4aYZFLGuF8fTE5sFELJ8zS6jg6pTFRQ6xlcECSAqNmyShSPTo5R8lD6RVamaZRsEdhMn+UaFy2U7exbFLTimnS1UF6slFFYUyYvEBGK4ldW+2U141ohDwaLcW5B69BhXwXBGSdjMNqtAcVMYaWFqX2BOEuTcsoReSbuUlC/5rOXFfVD2lUoztJg+tB3hh8tsXK0n5+tb5czyX7nN27nGspU+fQenlS9a30/kGofmC/nydMqkf1tPPKMepsVa2C1/dUYTVwoLMSyt/oydCSDDBQ/H1EhfSTJgWb9hpwnppezgriszMwah5wljrODj7K4wezM48C6h6gfnc0akD0ruQpWBsj8xeLhDItg/KtEWczOS38MgzuwZg5PP2xe+JA8lAwXZVSDpMOmtdcH+3tI8IPU2RaKDvbzd2hmq9PKrMbwYUsk9KtZXscXDO4ixlks43hrWeXqqYOSUqu5LqUNHF0bk5R8mBou1CU1ODrt1SVhcHoljvpjkyarCgx9UJhgkKz1B0L6xi0y96hqgYfYBxs7sHZryUbJKuENyBNqnlQWQm0V/ry0swmIBPRALtZ2LZJtek0Wul1j8GDauWfCxENMEC1rYzb9pW2NpLTVmVVZSoczrbDo8oyvg5+j/MER/AyOlw3FZdPrU21qcU3Js0+ql4egsvocCEqb+sUBaACt1h2FaeFaStPD1ia/Z2flz2Kmi1qV4oGHJriqltLTrW1FJrS3BZkn0IiLRxEtGtTQ1UidEe9U1ePgVLQ9Aw9YI3chNUxRIZfA+N4kz/MrYAFm736RYWD1NCMzfTxG2ItFEBfTZ2kWaaM97NmkUAKw5I3xJutejQwFKkSmb/DCc4N176ilHJ4cnLTjkpELOyWJkj9bYSKPSR1hygXQM04YNwNmAPDwsDjIG2wfuVDwmjM49TTUWUeXacYocT0EBWLEYq7AdMvxii8my9GEoBRnF+dnbGNCGSnmeB5IkwXQ9kwcXoHIshKwHz2yMhZAfEwMwcOSDB6kd2aqwaRMKU5BqCqUea5MESqsEqq2EVUa6oGZZ6L3o2wpqpB5pno8X1rphpknqkAqBOFQo2oltkAXC3OPJ/5QqtHJ1xMKRhd588+tt2D2ZBO9bFpcDzloGPU9veh2TWdHlmDIu/R7KlE+zECJnZMAZnOuvuEHJbkBCdIKvtA8ym1OCnZS6XSQUlWrcy2JQd1U7DzOB6VjI9k9stP5UQmCZejeHtIAFoeBzTPaHMypsc4eRpGONID5GguWmMLuCxXhAPO6fWgaJZhTSnc+RXOMqypxE4zRLMMayrhNU4BE1j/2ceSz26sCYU3HAWzG2uiQqEVWFxc63NNXEubmE1B8NU4Knxw7XyMpzKbs6YV2lANjgwFREX5NLvM5NxCS/aAOo/htfNLmWfIwQMcE7nGu2pFQYAjnDNf3obG6SrJPXfX3lh65wZ9mlx2f4qLmsZJmOW4wIaXuW1kD7gVVFMnrp8lp0XEShfBlAx84x0XoihPsyd/j/LNJ2RoVVBOaL3LJSKV3QzM89R0s01DXEsjrSNN9EUyjOOSCcPMOaZ7US2PRFATGQcbUrXJn2YWWy1xYk5FocUeLaewJpPXtu6lnUUqXJ9OscNcTt+Qz6yHFG0GTrdzpyNAc6Oo45LkOWl8PjLcotRSxRFJJAVBvIwkjkgmKUbBQpo4JnkXHaME7XDsfC7EqtWQTiZvj06WyTOeSS7q9NVXi8niuOTCXp2cLiaM45II2y3VGnfKrREHv2boHi8ja0AmkVbuc4zoW4zLiBvRaU0AlhiJ+nR6E4AF9A351PLvAUUHQ4+JJvnX0Zm68hBZGCjyJg8fRgdSzVaAVU6WebKDnbh1MntztoZIsUZVCDZHvvXkDfj0VNb7Fgvq7BgVlJKJQL3t4G9whpMNTgLT69c6emd41VTXiW0e1aH+ZpcQPUGrpZlE4mdfy6keE2vrJj1J+FAZoZbV3SfW0o3IsLqg2oZOSyNZKSwosWbTUsjcOZYHaGvgrM4ep5baIMz2xgfxTaR2hGo62bNqGcrpm4SLa54lN9a/Nz15aa9+P3FEU1n7Un3EDLWx9oX6jmlmfeXLDyXz7GopSDP098PSXbaQVEvv8pk9SWySz8t1i1O8JqqX6lDErGpL37AsfHYtxK3OEResZb7/1mOBBIEGyQ+TEucJvUqXH4oSb/yiTHMyK4Y1AnD5QNf/5KM3RzwuMkE1+2zKZeRS9dUmqmuZLYugAYgrmdLq3aXsSUY9O8MWs4kysCFrXivPaWFrcK14hld/HbmA2AGdybpnOZUdoY0tYTm9Y2Ib282yuvW27EYr/uXExkoex/szGHrYmhbJIv2BiFNLbfmULdNzDfn011qLZuwksb7uxbJYSAq3O9NstcNszfgdHuS0vQVtnU70aOTriS68fyhDUyc4utpaLi2B8CvKOYka60g+Wn0ecimVHJ2WTGij35xGZSsfH8md9WNOqoHpYxh9j5JNFMJuwcok85xagp3sDsyp1dsa4GOissTU3f6CtWFAqTYWNpHTwth3j5JQIZm+xPohgLw0daVrKndI7PLcJzcqOkli45moR6NVu21eANWXp/H4Zz8e+ORuTqT65Hk0tVhIYcOl1+xc2HyGKicJtfKyNR2yAxMLZeqIVC93m+iL5S5PqDef3ePg3q8uDC2VuUNOPcHLSjUUCW5KmxWpbkcTTdsXUsnR6a8SFtK4n/C6pbNKWEiqgTFycsa+oGSe03CVsJDaHqHxKmEhsQNKUwuU9n3njNRA2vmgyMKxVgdi4zu2Q2GzfSEoqKeeMYNJYXOy2U6kCHTmAvwovI3XBpmWCS8Osloz/NjPZTp/5bJ5+N7ZPsw3foby+Tv12xxjuoqbv0/PVnsyU+qaZY9sQdc91KJRV2lSPS5FXiO7eveledekh222JJ16vmaQioZf9w0bWUKaR2WG+JJutA1O+rxUa/KsLahlADPWV17CZbWL3ulAd5G4ZHoh352fTxxH6QV7c31V+U38+eavwvCjGEma+GyIEYkQlAqVDFPLqyzymjzwSBI9Xr/XSfMaVtHpEKFEVGi5kINTWRMrC41RkGtdaoKT2lJr5epnkVoTi4VOHsiZO6szfYin1zyucHyFEpEPlUH27DHK/Jdu6lwtwus4lE8lKdssVQYI/WeYpWmsmb2ph5lHGkiVJaCJ1sRfWQUPLtGxwVmOA0RPrzHTZai9M66sappKdiX2ge5E70iiMjfKhgQSPT8RGPpmmxMtPLhERxQWWlNiZQ0NsMwZCC4KslDz7w7brZ5hSFnJmEKmKTvQh03LVNugrq5pRKGg6VOOMrL0cKeIJ5DoaTpkJ1p4cJmOPP2V9ARu8oQHl+j4+wEf3GRGiyz1dhFnezctqIOWaShRcO+zyZSbEhkSSPSsafArd3IG+NLcYe99OezqRgwybwyo0HoiQVlIAyzjp74F3RRNB61sKBBNrSAnsTzoSNTkZAVSwRB4VgUbwSHZG8BZ1qq7g6RtEWd5+00Hkn+EPKuDthzYQm8R53mr9gJK3EIO7LQmTw7V7j/m7ZhReJej/Elga+KXbnquGEbJrJV4NZk34Z6vz1kkJyS23sMBcmYeVcr/KQ9LeP4WdZb/DhX4KzuTwJC8g5xl1j+XKmWePnwqYGaTagf0Le78SBLActd4Mk4/OOQPwM2sD6uowE/wJ0cqGuhZJbHWfVUpeyy8iNpvaeEuOdgZfEctrYVUYNZ+MkeRfuIRHbGGOIXN9z6sogI/R7B1bww9qyQNYTOhxpvlzPSeq5FyZqJna4ac90HxCpa1QZT0MptCy4OtQudSI0rSC5xYBUb4uQuHKeGGn7dwmBJu+NGbw5yvXfpXVeT1a/o+iojbz9JI69KRqoAWeH7+VOb+Jr/TuqshnzxxoLPse+pnFpy+hzq/HmXOUUHJO8j5fE+TokT03h5Zv8Fm/hB5fkXa+O4E1dBDnW/99ekf2ObPgSqxa9/8U5Ywec+vr6NADiRwoPPr1ojMeXRvhcvXrTzq/Gwu8eOD3YGb0SyuhVRYMRsdmFdcOs+ejZ+c2YPq6OOqzuwdSKhhdWb2blQ00BpKSDk+oCJ8cFI2Ig5lqwekngGuZATT9TCjMHRNuZCZHjv9MM5gG64YXkeRi+5kmkJqpWmvE2l6A1ey0wzBFdQ4aEhDYPWZN6iKAfD8zk+clna7ckP6FlFiLaPeJECJO0gZM3iPxWHOc2eFgU8sOX0PVqpA25eYkoBJ32Ejfjort9wVFwnoYKUKyhwFsE2/hzo/y0Yb9jQ9JDuHKbNyOBkWBrhKlhZgfqX211iDYLlbTKXVlaMSEKHP6snpq3hR6e9xZH3SbKhGgD1vASkif4PvDjsnaoToUj1BFEIvRvuwUgVxqPfUrRJ/AyplJxFPztzMk8Twiop0r8BrqJm66N5T8vjqxbegFpIGUMrqB2SIA7aPD3BVNMDah3hQFXa7c0oT7PKzSlXAAvZ0AQ8qZYffG+mhqvE7KYAesqoOB0XRQ5aO5dVDSU1/Dm0yHaEr6nEkRCdHdF136+THlIPuydKBPxIkRFfVUzv4bE7f+0nqsyvrWte9dSTOEqqqLqK0NHxyR0ergEZZYe1K25WyDl5VEfRxrDG0qpLtVu99YB0pDbaqFl13hzpaptwbTmnRfIBCR8rEMxNTSuijiq6kNNiqWnLggy5jaOU27a45y3Ok9s9U9/Pa/pbkhnQxvtYYC20/GGBrjrGO1Ew7dprUA713KcDW0eLC4jtNYTgTcaVOQGM/V1pGa59QZ9x3pK+G1hr3HUlpsLXGfUdaphyWTWlxYtKawFfWBLwJN4bWmRU5UjLxrMHsrMiRlAZbeVaEkk0aw+/yT1PozNdcKVLYTe3PqhwJKbTKariMdCVqTKO7WnWlbOpdKRUXTlNOUZTu6I0SjSydKjSpbF0X1J+RkicFlIcwjWNEXwPL+IvkxA1/DazAD7fhKRIxufUpvhToQkYHLdNQ3W1yoqGFlmk4hNGGzBC2ds6vJnX04CVaAvJXFDlqITy4TEfgplBqXBn7HpH/Tl+4kcCBK+nI0kjLGbGmkgZeqsXMIbaGGi3v14AWhpESVb/RAdTEeKRA7bWvyu3o8enq0Y2KHrxUCy04H21+PRSlH+EdCp6ajSI34mb5TNRuczLXhjqBqKSXZ9RSTGd2LHIFtIBiAaO24uJAbSi4XFa1gFVLeVHArO/mldYsWsrYKRH30hoaNW2OG5Bya+GumjmS02eQKWpuHjoRw4FLdLBDb05EtMhSBW7Wfxul9d9m72ZmWeNK2HEAZH8d0jfAcn5HzbNFlir4FcyIN9bw65QRb3zdw5ECoTl+4nK3EwXKz4nAHfkfaZh5MpxXsHO0ytsprfL295utE/oGWMbfXgZ3IoJHlyiJYjc9co0rYa9Du1xbjilkmmB20kc6VF4fjzevHJG/UmHHcZo/1e7qfBRFqZsl7gSPRF1CpvFFGtxjoL2xoaoBvkxNSGbH9zB3gUdKOmyJCuqYx4WCGlfCTl30uGCvcWXsMHemRuQq3kupm6ATu+dBpvkbaAUNbnqLFllBwdfOFHytoqDxqeFEBAeuqMPdmnPEoKqIOdtwqqhhkClKo6eTsxeOqiwHLtPBHojDPgoIlZuhZEwh09R4RXGihgOX6MhD0vduTr5ys/3RQ5cpcbRWnTjEMLxetkdudmobYDk/9dfkSEENLddw5krBmRr/K0djbAct0+DI/qtm8WX3Q1GwdzOi9NBVlLCtseIQlo6MeCISFV1peh86zKIWXkFLGdL1gzMtHbxES+fqzIWUHrpMSRg7k9FCSzQ8kBql+fCosggOW6LC2Y6N4h5Ne7PWlYYGXE2Hm1bCgyvpKNyYQXnw0TFApdfIyKxN+uAm/BY7YfXUd9LrCyLVViE60DPpCX29B9Yo0WmaotLSWW8m0peXMxTmvv1TT3LBk5wmyounJE2egO22k5p5ts9zmo1K0zvJVqWDxinDpHovugCftnA5JmbSU0nWjHFYhg/uVfaZ9FSaP+xuolT7Qff2DpTue8866sQvPs+pAd/c6KlRPTXXu9UEeAlNIGpEoqktKfFjWSzQcucZzVQv00bkrGbq3fdD84xqqknEHNZAxAlswdW0ODjRxolRP7lWX2VyYRLn9GhYxdvrOnWRu5LUI1DUFP7mroK34IpayvwQOButOnQ1NU6nw5rz3gcUHdyJ6dCVVmhGb5cVeLfD+fzTZcVTUeL4IcSfZLes1hfv3l1cy5aFVSj/+ubGymFCJd3r1HkVsNfHl5RkFdiqCCVCFKoTp9ihkAYfrDaNvxhUnDAKySjqR+iuf/NuHI6EYJ4B5mtifTCjusU4rLVDc2dEwrGjG7qVjJftcdq8Pr3XY5i9S9mF1C1efS2jAjYoyd7H/gc+WSjLVkGWKSUpRmVJdOY4wqhg7xc/hNQw6e1wgnNUprnXwnljqhiFCSRXhycgSx7iM8Vao0DVoAmIaIrVaoRi9u2FNB+b0FdVbTHNSPyI4izC3dZ6XSM5OWIqFU1G+WCiSJJH53uUJFjVCm2hhyMS6LkuyzV7nbUJ5q7MxFQqmlzlkYhotszIOh4lm2KJit3nUlLlvipxTHOKLpIg3SzT/ntUKpqcZxJHNKdn3fjOc59HPSoVTc7zaD10xTCr5y3eokNULphVPUYNhUtlHMcnUPeGwDMPTWnyJ6xqkNEXNaaRavm4fuOwFCe4lFQtk0k1k0DRjzqeIAYyijyg/k684pBlaV56Pw5cP4iIDEthgmoqn398Iqm/Sf+IVW9ny+h6ePOEUCkcIApIz/94+fF9utupH/CQcEYVmDcAFlDf4MfyQxI9NaGAEt0IEMILZKxZtT/X2eARKBi2oiGqjNgi8bPUU6kGyPQhryyjaY04Z2a7j2/eW/SmlLsy/3liyIme4+ybanpjXsgFLg+Zn6GnKEUbb4gpo7VI7gwxn+JqH/GQsy670VCPt4SalBKOet9Qm9g52woN78IoLJ+oGSE4rFAer/A2PztdxbvTl9U/6b/Qi5MX25MXpy/D+OU3JH6ZplGwR2EyBCG/FZv74bdBGq+qyrIiv67I2BKQP+n9/tcvVy9XJyv/27OvX3z71atvX7SGq+9iksro9QYXQR5mNGE/fOeNv6usML0MIN99R0ezX3FQkn8fPT9aX159fH95fnnzF39988vbyw/+1Ye3v7y/WB+9PvrbP6rDD2WYXDwyO3Nxe/T6b//z/PYox3H6gDfk4xZFBX7eBlxXpUbDNffnvZ9yjK9vPqy7pv+8/a2zMfr05/6PKCMlnKckM8oQ83fduiA4Jh1d5R20evxEFIjaUsjPrIiFAXqORsc/s5dVHsKAvgUXlv4m2AQq4XB8mAo2vr4n/LlGmwpVGfMmfi0ijDN6rSLnQtCyIzXlEGFSQrdH39V14fXVFfvy2WMcJcXr+tvvb29vj/Zlmb32vE+fPjX1lFRZryjoJJYGWmFmgCUhn9VVgEUr80P1Zbhhnw/BquJdsYa7OgR1bWgtUatdELAY2SbuQfxwy2oy2+qgk7CCtFHaHVREq/9L//RooLZqN8n5gWVJLYskmCL+/ty2VtNjWkS1d31z4dPmnCY4qS7XPe9+jMLkHue0T1tFm94vpGhIvY7CgLVItnc0iMuO59a4PgmGyPg9DsLXtIC+i1VQc2E/DH6gEHuyyI8qD/5zP48YwrRg5pQ6hAB/FGIag+kMmYeJQBAwHDQJv/5JHrDMUVKwGzvTUWj77k4zSfBZZzAK9AW1nBsc04kG/idsO+NF5vPJRaXwlyrGeLgfftmFa9Yzz/lV2fPx2uL5cH3zfGp+9ny0Rng+mD0+F1uGn08aH4W/DGL0bGOiHwbhe7Z00Q+D8D27kugHcfiecWXm9yq22Gos+qEKP565D7+swgmXNs+5zabn7b4J+dcOB/epTyZcPptkdfP286v15do7p1tll/XWehAXYXWCKux6SX0A0kKN43Kn8XXjM7fp8dmZQdw4O/hk1vvwjUHc8rfmIJRaZDo9PB2kOi1OTSKnBbtQohO16qJoTLoJTiMH85HfsrGXLivpNut7utS7+On67PTq3enLVki7RPBREEtrni5kO4ZCId6RNUIMj1lKq70mZhAfgBHpfB4akgSvT3VDosbIb1Z4qbQr0gXPoggYEoMXFT4UKJf2KZqg2yIPoItqlwU5dKXaZSG8zDD1qY0QGHe/hc/S/fYRGjI8hS6jEIH3JWF7nRUQlD4CGiDplEITNSI9lAPIxzsE3ds1sNQsFCbgg2mEmeUFGtVBq4rgW1WMwugufYRGzdD+Dn6KEhfQ7TV+gJ7sZUEC3UFnOXSVJ4h+Ee4SsvoHRiYTHlQG+6x6sAsSGLupqgX4vIesS4LtDh4Ufnruot87uOj4XMwkHzYIOj8/bVLgcu/tWMFAx7U1GgStOojut3qtQOulfVGSwj5kHKjNCr8BHQqVYN6laUl3K3E+2NkafJThVPcP6d0PtplQFFhekQdxyjQOpTW1H6c1ynD3HzViG8TB9Wsj6jGorfggHX56cUIuYfix9EtU3Ps53oWkbKX9WI3E9qPbklArvToO7dhRFKmWeRVrXrMaUrtP3eTdLghIHcT1jSLFlI9Qqj1Ju9gsKcqVpQOps7SWIMkGHG/yB74qYG7fQBKL7iezOOyQvn6UZi/aJCrb85G3XVHUPZIaeCai+duofn5QNzLdmNSIR08tkf+zqOdXZ2fMbI6Uo9MaMNjsp+XrV7uxalWiD9Ql3iAyS7w0Hk0zq4CxYl/ei6JkYh3EkPaQvfDBPpQuaPoR5NPfYXjO+4B6NKUhZRAlV2jigwjqTY6LqGD67IVXse31IiisW4fhTTJZYZ3YC08W6NtDoiet0CsTtbXQMEqJpXMJQRSjsldaAvViKG7wdXHoHECnpdEuqe0slHqkOgZl0ApPW5hOBKxJwBqKTgTaUHTC0xqvE77QS29dFXWiVPVJEoOsHZiTQRSxCtJ9VFzmy+Irdh4cDDsQO5IhSUaBg0Me0lvJ3CEo8m17ZbWZ6bffyBTpI/qoO3biAH34LBw0fPvWGzSwK8WKJngDZO7pEGDkvcJ82AA2TOjhLNJa6P6MA/yxOx0HBOwwYHcyChKeGZjLg3wWZ4DdevEAxm2cVAPAhjzuFrOcgJIciuqhNXZj5ui6a9sBYITY667h0ev+FB646ZzgkfcK61UD2EHn4Yag7Tzg4bvOAx6bNXIY2F5DTAuwRjjCra2Iealw0m6Co7uq0Nw1ay8gdAe16XsBEc5thhwlnjROwox0iFi+W2pCQ8+FO0kK7X8RJvkktwhqYQZBDI9JH+GCBq1fuQPH3TnIgLYNxZDVjCHv0Qm8XPYuggvYVyen8LDt19Cw3Q0vF8idzzVA5NCV6NbRPWj5MWMFeybRbBgUIzbfscc4ffYmqAt4Up9BcUkvgeQbuXqQUTNjJHOkfPMJGc9jJvGbtgcMy1VjWOQQAppCVH42K8NXgbrXPqEA22V9mR+KEm/8onelBoSDZodrnibLR46QHYBbLm+F8K1LVdI65afJzJCt7G8SbBDzjQIHwAxnliOWH5PSBE4z9PcD9uneE80a6FpTw9uYiKag2f3VjPTmSelMPs/hIg31/hWpmxlONjgJQhgKbgAJy8JvfCLCwIq6Mjfg9O4zNHSZG9tIhJiDrtEJMvcAgvkkY5bBylgnwQYxfylwcJ2vEw7a+ToBtrWvTcHXna+76tkncFKH+hTwRVDjO62j/CjirjAELE5KRMBjYTfXYIEvfJ7EaQ0QjvNSipwdpeb8lTSmE4ELE+mkQQlM5UavIpTCQS5FpORBfo5IBFVPgkYeiIpTk6wXoFE/RWBgLJWKUI1zmCa3ms+quTQV389LhYXLAKNOS6tBNw2D+EyDIkbnqqndPm6/UU3HCCO0A2m2uDoM/cSMMNhxWqVDaMpoqseKFOHskhoO4JTznR0Kawu/OnrIVg02APc47xypqEJ09xJaDaq5IQKoNYghzN9NgAMT587okYxtjjHdKK18k7R3lZqvfTJ0plNZrQ02vR8rg6pvJnUOVGbySghTlZcX5CkJk0zN5KXxK89iO4KSTU0QpBj1TZTWfaB2lgxw1tS34hUiSZuaZihDzd6fUkZpM9kSh8zGchwgau9lR+nCOROSMmqvCC2xorCY6sqUMWJcFKTtkuFju50c0dTRsgO9cVemc+YwLbRPOcoyPHlcXhmL7m/QJ2CtcfL0V1IdrPWQZeXBWkyB42xvXWjMOypdPwE04XqWBlOb6AU8aww6aTBOFmthhh1tU+O8d+fn3pvrq+rm1c83f/USMpSyJWJVLZ0ToGJq3g3LYZjJyhysilqzXOH4CiU7b49R5k/dLpbCVe3XMHK/lRiC0NZhPAuom4Vi7KmHy6fdpalD5OGUbxt1jCI5gcD4lIelcp88BXTHe8M0Rwl309e01VF6D2HbIAUoxlFkX1RBYJ2oYI/If6cvYHCyNJq6yKCBhGVXT9Sx5va51VGmt4LVMdjNpOPT1ZQfFw2kyu1v9VR89Ubt/AksO+htnsbU1AoLTm23DLl7ZBcWnPmnL3DphKAolNfRaoCPr158C4QIU1TUB3bJLE3WYGXub/I767bMXOfbg1iPsZu9dafLjgjaY9gXdH2o2R5m6vK9OsTsLTh1mJlTccogO/thdX+/2Vpj0GPsEC0niq0rvfAtCHO0qWNtGgivrCEqa2u1bPC1DK1TkAkZboo0uMeldYYnZK3s33+yhUlD66zOJp0SqEMoGx6mEYLiRNngPIdiXW8oyNfWIPXGBBQOyEjdgs3cBlUHI0uBk7MX9tnNjglgdqupsG5WdGYIkbo8JM1ic/KV9boptx/E6Z0cAIzTV18BoEy5mtLBeGXf0gEm6AU9bjLj1VgPiC1Ei0NY2k9AGV6a3ocgyuhTAlh542MKqb2uYw0UxgAo875qlGEgVmUUgyzPratjjWNdUhVOYTxJptc47C1hFMXOZtEhNGto+hCSjxNqljae9IxR61U1PbKWoXDeA4M5fPGUpMmT+TydA+7ekIqztLDpcASgSpc7TIDJMB+HZfgAonbWDZAekM1yhwNSuYxkhMferylgC3sIDl7oQwLQwieIufGcisOxsxtyQLZzfQ6qPccOgRX+BpLfRZkfAojGBtWzzl50ngSKwrsc5U/e9JVYedQo3CUzF9LkAEVyQn6Z9NamBFDtuxkC1PttVrFljnnkMNV2nakIFnvOyZ8WhFUa/Didunqgg2Ctwc/RJ3sdDAVEC6kiD6ggHZkxXOLHB+MKVm8x2ozxLdTctQ/F2OxYFgSEaRqMZiVdbH6zyDQVPIgfxpl52faRLLuidvPKMF3VLo1xZONaQaL6wSF/MB/MOgQ/wcadB0VpL/xPe/JQxLHrM+Z9malG97M0mj0rroxhmIxuW8hQRBSn+vsTTeTYeDyLNyZb01xsmy4uzgrDxRePYLS7yANQI5n+5J1HIGubySdMpAB0N8iw9DK0mXGTL48+6U5YHvXePKZ1/08gbAbnJroxO92vMua2Wkpk91YLiSa6acqbNbMpfW8ryxRE5rhHA2HuFrI2jGmmdjB209YxEICiKJz2JqAFZLFqG8BYDhkcWm3x0r1/oY7oJyl7WWjyYisMtr1ui0ntAMVey3Y76apNG8ZezczNfm0YezW2s20eyny6yaFMuyTQRbHXwg7N2YsxOXsngMlRskljq1Uzj2a+YB2g2GuxmL70QACURGkp93pjgwigsXakB6Bt1iWfEpLtxIsGsp6+59QdclT6exzZDLv16RjD1LDzGtXsxpSf+a+4O+ysE0LvlIeTrzIpxSefT86sh4cOaeYVCSlKd+zEsGzYCQ+LuKTBYGS+SmoPh9jFN+6O2sMgFvGtloktgG0qahD1pBR4x5wsMBcQDyH+5K0v3r27uK7/Uq2REhj/+uYGSBGDqlTFKExWQZaxD43Dwf+hP6SbQ4Rvj17fHn1H71bjoHx9dcW+fPYYR0nxuv72+9vb26N9WWavPe/Tp0+ryg3Fiqx3SMv0PlaBVpgugmjIZ/UxDRatzA/Vl+GGfT4Eq4p3VeDykK1aH01r9pG9TobIgLLaBQGLl23iHtAPt7fJs2ffsa1cemyreJbR5zLyim71f+mfHg30nTdI1A8sB2pxJNkU8ffn/2i/unhku8MF+elvNH9yHKcPeEM+blFU4C7umvmkaMN9Efl4zhafNcbHtCh/pMeu/jc/TfOzfm+T1EoSY1f8b0aaZuQ/fZW8PaJnBYlqbxXkQbMDTP75BfWih6CJv3ju/s/R86P15dXH95fnlzd/8dc3v7y9/OB/fHu1Pnp99N0fSBbc3j6rj9h+f3t0snpxe0S+wUmQ0rkY+eqXm5+Ov7k9+gPhJKQ1JwmSoBh/L0rreVUE/aRS0PrF8ad1QP4mURusIwZNApD/vtum0QbnHX5VoL0wTciQFGwbrmhfc+4OAZBfaxWYFEyVxfRL9jwv/YZVqyo/Z8G7UdunqQPFHrwfDK69ZwcFRxc5CnRA0/hkOneSAs6JmgPtKMv8uuqHuFigEDB73xichb0RWtnSqwWsm5TUKwLH6HV+2ZF851W9Ff9VfYjxIyr35OMhDylvediE6eumw/aaPq3Cajty9umZm075Bsd0Hw1Ddcv1oKvUL5MGEIUBO3DMDobW2d2OoF1mj0tAWqbXNxekT65XKQUoNO+j1if6UZTuoAkELmkDWIbKrxp75q7a3HMHDqy8dW1a4YMX7gjflX5WuiE7yxaAJyMc9Jh+3TRd0zAn0Gxf3hUhm7m4TROl6C7ZOKNig0hOv1tFGzNc07Gm7agXG2xmbUbGA87A5CYceAZRmoMjgrATMd6y3ngy/ESsdeUE9j1ZL0qiTgBc/HR9dnr17vSlUvQpFWxNrIEgbhSVG128zc9O493py7pP7NctUgqdO94qzzwuE7w2PY1/WBHquNHoCS1RTqoXuFIRrLpUQSvVLMbaOa9tObb60YuTF9uTF6cvw/jlN3WnZppHjePgaXDLQm2BfRTEGbTaFhROZTuaA4qsMME03h3yysM9qMoWFVJnGbjQWaGC6QyqBxpAVdaYYBqb9yhARTagcCoJf33vHFZohwunNUYEtwjyMCvTHF7yCB5OeVbdT4DVW4OCqcQOWhUGblX4UKDKaQOszBYWTOm2yAP4dtWiguncZUEO31G1qIA6Q/jsbEBBVfrMb7YLqS0ymN791kUtbVEBdT66kPkIqzI8hW9JNSacRuRgXtKAwqlsXUXB6mxhwZTS83YBSsClcrhgWiMyxwEX2oCCqny8Q/AzPB4YXG31DouDxaiIAE49bh8LgxXd4cJpdTJWReBjVeRirIqgx6oYhdFd+ggulMOF05qh/Z0LEwoPDKe2gB9ga0w4jQ/wpr0aE0xjFiTw0+gGFE5lDt/l15iQGquXmiMnWnlsMM1FkKMy2GfV/VxQyX1oOMXYVZ/aQ4bT68DeUwDbewoyCd7u4GW2sJBKXRjPO1gwpW5mfPDzvYObCd8BfsbnxigJb5N82CD4GtqAgqn8tEnhW3wDaqsyro+LQQnk8Wy344c7/DASRbBOduOVgkoCyX4eH22nl4J0T5uESaB4ymR0DrN9RXa+ZCpdzePOXSyVjBccoWzeBuY8cOjwi+IbKQk5KPxY+vSZQD/Hu5BUMYUhqadKimWWV7h+2k4nf5o4ZoyoKLDK8DGoE20swzphVhPsyp/5PTkoLEL6rF2seVa1HmTYmovcsDXP10DpWag6jc0z8hIsq7qlp4WLZVjK1HQdRYUuLx/PRUmXaRoFexTKOnzLijKVLdXlKOVMadU22dPFVyoWjZHXdICbTymrvxp9TJfgut2PYDTmHVO5bynGVsUuCPwsx/WxYXM1YxywOgE8xbpL0zJKqaNG3WlW/9aWSR/dR5C3vE6r14/qjZGsOijdTDy/Wl+udfPvnF6UNerp9E/ADjK+GszoMfQwUpnZsfR5VHC77hhDGLe7Cop7xcZQDIdgrKX8rXmowUhIP7pljpBOxCI36tjmGphLwPjszEwDH9tYQ5wdfJTHDwrHsgUaerGX6n8nmiy9mnv6WVs7mbqq5iITO6hOdXTjokwL9viWmQQusuu5lfYlkXE6qUmGqpWOZ3xS69sV/ehf6qSBrD9MrgFxVznNmoLtxFdwl1RaH+u0elzcZpYnhrOZfw49BYDJ0z+aLr1BD6ZN+xTyvLTkQcWEp6itAXPd7dguXkcuJkh3Le1/RHlQL2Yn8ACrD6TCDg60EkHJa8A+8xykuW39ecwrDbt6rjYxmizlEWzKub11npcKtjeZmgblSzf6tLmn3DW2Ka77RR4BLP+t1TQon7lxVftwn6dp1XuAtK9Rr89VJM62y0PYFG8NdY/zBEeWejqQL7158RmoXKfrNDf7KH0IuDKw09OBfO4m1jqh+kzNrOXXqNVtnLZm91CsCrlzyrVHUXMWCUTaABBQZXXdGE5kiweoEbCM+3gWGsMBqEajHsoTQX3x3WvXakwS3vRoPRSg0rAQNIT5Uu09ON7kD7rWHqXDOy6qCw5Slb07lij+KEMb7zOPc9Tf5WfINt7Npmru0Tgs74aRjdsWc/Btwm7N6jM/TCq2I1Hauch2KpT6dpECjZ58lt3fRvWjOkYauuh2SpoH+PRFNDG/aJvdwKkt7YT8ygmafOLBJZqO6PNQUPXBSFUvOkB9MBLRxHRdH6Lwzrw+kMhM7PnV2Rnbu0UaSSWRPRHAlzufkOcV1EFAsoJVXHMwVaza8HEMztiR6NRfkR5hHcOQrn7/XYevjmJGiLXTh23SxzyB6PE1UcwI6Y1OPb46hhkdvVKoR1fHMKMrdCtLYVFVKjOSLmEbyYxUcQnf41RcphtuvphdSmh7IoWZEE0KW8fEek7dBN3JPlS4gNxjbKKYd5eafOqbxkI6dnQNKS10+qxcRMNLFvrp7ccyS7PqPYZeeq2uMbB6qHDCYlBzlQ9RiIZBtVsT/TK1uTRB4uco3h4S3RbKxTKjVT4v2aPVPCM5MdPQzeBc3awiJNRZmg+IdRfm4qmOHq/GgZaJqY4en46fNPFUR49Pw92FkM6s7+1HNJ9n6bFqOHsQzrP02AqLhqJ6Gb9PqHXVfnJyp01aR7IhNewUxpFtZph63IqGfvDTmgERWD5pL7i51z0Md1y752SKQ0bdI9r4b5986Eqr6PiHGlAQ4Ij63lcaSJt89PiM8cZprF+v8eaYbC+Tc9BpnIRZjgus4o/HJg0DIoAk0Mcl3JeBiMXJFfmpVqBlJ57ILJ+kM0+zJ3+P8s0npGLuUc6retdaxGDrkCHPU6WddA2lLaZ1BWzg4bOSA7ZViZm/e2CFLailugYcBxvSyMifCvsEWkrFBECqiz1yJLdGtq+h3duE8JW0jw3UnTsSOwRfsv/W21WX9TVwQ1xz87kDtixD0if4SGXzX0sih2qpLwhiB/o4VFt9MQpcCORg7UeTGCVoh2PY2SKrikNsW617dOIgN3lYe4Wnr75yo5EDtlf56uTUjUoO2FLlzknb3oG1bRz8mqF77EDjANlSZ7nPMaLP0jlQOsIGnd+AD4x9bNj5DbTYIThMzj6g6KDi89wkZzvsz+durkn72OxEFW7y8EHxrsAUEMTKvSraMk92FrPiLj29CXGDClS5Kwbli0R6WgfgsJLrTTxXojt4ANlkolRvw/kbnOFkg5NAyf+NjvgZEpgk1DnTvA1LH8QAT8EEB2gCCCg/b3WUhDELeCJIzxU+VAZOh4nos4AmApHJhSvpDTaoYLIuc6W3hgaVy/zAlwcr0/Gs6B4BqPQgzPZqV7JMdHfoMKLZw98ZyunL724TMMvkLDF7pfPg9knZax8c10yIkx5nhsdZQlz0RNM08MlwPH7NU8EkJ83Q3w9OhwYhA6h4x8UwyeKiBBz1uFMkLpLgpHsSU8DYG8Ky8NldPEDRI+Avf+tnAEISoA0izOUwKXGe0Avl+aEo8cYvyjQnSwoLAw+X3dS2Qz56cyy61WSyri+TFBkTSHKq8wigultI7WapW+MnC2hs/ABL4SQ8rEloi9laxMb8OS+cJ3BoFgKVP0MCv5aHVj7AdrHCdCS5Q3dp9nEkfszi0gDnMBGw29Mje40j5THIA0792R+9ekILE753ERGASi+fMged4hAcfonrLssnWeAT4SbzhQxf3q5ic77l828p+p0W46VWi9A6reph2i8IOzz/UIZKngB1hbbAoGotLQdzegHtBTxsfXLbiWQOG1SzlRF5TjCY1ZgHBbKMzel2YBYbwu9RsolCi4MIMv08Aah6+72rOemwG1c8MipLTN9Hc1VpBvgwI34DnhZqfgaVVAuR4fXWD7XlpdJ7F6bahyxfvk1yZgZhn0uNe8UeJmg72uBtmFhMPWe08tigmu0my3OK4ZYpo8mUC7kNMGxrt7bvDSVPooPmcmuPZqeZXGT3iAE23xt4N/nOo8PO6vc4uPera6FOsn1IAKveoW5Hiu0MqrOK4aypopWOC8kcNvwqyoXgvbYTTwW1QPbqOd0OjNWTixxX+nkCR6soF9J76M5WUS6UD/A/n+kR2PNHRhoC7SpRtJCzzY5Q+TmCLgpbbAkRviCHfGN90grJpZBt8IsQDHzQjHCU3SrJBCm7WTKtrtM/DEuJLh0mi2n8gvg+zDd+hnJ9ZzfbHGO6nDdzdMPMATbbAtrPTgqrRPdU51Szo/npcdnkNemunvlsnqHsAS1p3NB/7FSYEU2qZl89leVF8/roEMxyaGvhyNCUTq+itNW1cF/czln1gpBN86CXD9FdZFIvBkjvzs+ND+MNoN5cX1Xetn+++ash5gRykiY+m8boJ1ha/WhWGnQPVRl6TUF4JB89PgO8TrPXUJgdr5NoR8W0L2E4+TWLkxTEKMinbxDDpaHlcVYO7tNQs9ikwPBMpFVk43OYg47gCsdXKNH3eScstT1Gmf8SoPHUqrwO0Pk5VZBtE4iZBSrTeMbnpDQTaxleh2PrdCZPCWkyvaRXlsQjWYra4CzHAaLHrdnuSjh/8klZ4jSurXuPB3r4aEdyIAOQOUSzFPcToaGvxNsL45EsRUVhMb3UVBbUoNh6dMNFgXbYvztstzNGZ2VZYzxbgdnBz/K0TOc3BdUFjvAABH7KUZbhGRfJWvJ4NEtxzchjL4xHshWVp7+STgggt3gkS1F/P+ADQDa1MNb+xuJsD9AeOxxbQSUK7n02MwYouCGapbg1hbsC0jYAs8439tY2VP86grP1eoWK6cfilFU1KLZiqJdvgBLscJxa4MSzSeNVAo+goXtmOmYsZYhiLIfNUoxlNLGN6ave2Ji/jW4soN9cjYWMYIwF0ZZqUTHa6OYCqrZprqCNr7Tz4+6x2drDm9m2RxTe5SiX7bXMZOOca+tRHtZSvZrV03ZhPSQvkhOCPPMYmlwCD2El5FMelpZCWghjIXeowF9pWIiGKrr4xhIkly+kEkxuWAglsPWLrY4WxHwkDCxE1JFtyP3gkD/YtNI+BoAUP8GfIOQ0OMaS4mnnHFIZsabXjWFDDXfJQWMfZNRQ2/iWEuZfHlXUofUW6ZyYOLUokj4GgBQ/RxYVdYxjLCkNLbKljmxMns28/Sklz9TfABWT3wfFKwv6JrpFt7Upph+ZUOit6ugWOWCTfEtqy9kVB2AhwnJmxQFYiLCcTXAA5lVRcklUXhlNboKKRfhZGk3fCFZV0qKYT/XK3N/kd9MXIuXzPA7BWMaevvBgp6MHYb7QZw8PmKvo4psXSZoUJaKeAMgS2aJchjDmS/3G6725mB6EeS9Sn9+06EY4BGsZ87f6lbUY3OEfCiqQrRYOwXwVH5EZ26wLG/kqnocwn4EmfnzQOFg4mnm28S0NCfKrYooWBcNbYTMrFnNBfRCIFYutlhoDasUCIKfBAZJEiv8BFeGDfbGJAEHsRMbCBiAWw+askz6F8VLfC9/cyO2HcWbR7sVYUNKsu6VpPCsDV3tzd+7tHiUT1xDJUpZt8xuiwKwfzOUMUMw3BuO01NjpHepoo1vYIKlPLXMFXXwbCXZdIAdgLiIrZH5K5Tp6GFZS5n3CKikx8AErEELXHToHNkRKOgwrKWWOAosupAdhvlZAmygNNM6KjJYKHYCNOch+CBqAWNumbIRYN9/GyGYhogWwXkdCFI4IylhYTh8wj0p/jyO945lDWQIgc1NREfkbfHfY2csSQlkJC6LQaiHex7CSEoeFxnUBkZAGwUoGAT05A5jJibEApM26w9GQpe/0ZiDp8dWLb81NSU1sK3o/IGOuzabFAMRWjIVpjUewlaFxBm9Cht05vAqksDidwiNYybDc1OpB2AuxL5seDIQg21LqwVhNKapHXZvhxMpGPYICEAahCCqPZh+80ckh/WdtZgrO8kSaEApCWO0fvrln4yepz5zYTDtd0dE6iw4hv4jSUuXlTR3RAkwQqfWTMSASOywIaVbnA8c4EJK22820Z2cdTQ0QhKhZd9M6ovTdS0+LmnslTkeT9ltw05LoU/QgmhogCFG5zRmqMQ5IlwDUG9jlUe1zsh5d5n1Eyjc1xGBgA7uVNWUABDiwQ8gy8Ug5I8xqh1oABCXK2sQ+jedgQgQiU4Dpdu7mQHQfHWrWASG0xgGbdUBoaoDAZh0QovTdsU6LsjcCToCBiLPZTx3jQM3SICRpP+QlmaVBaGqAQGZpKNmkseXBj2k8qIkkiDTLPfP+DBBCUQFWjMM1NIi6MSbkuhxEot6zsDb+EPVdUVld253JS6Tji6bJvNbJS/0ZgTigQXlo0DZHWmoUWzFFcgIgpkYBEGO4PS5SZLBRLpRVX/a11tTh2AqqLjXaC2pxbAUdwmhDZkhbDW+Pk6J6WJbCAvJXFEG0Nx7JVlQAUHY1iK2UPSL/nb4A0MMhgYjK0mj6hQpNWQ2WtTCFl1U0pIE+o2Jq5BnJgnpzJDBaTozkwLwKXDl6Pz5dPQJI6mFZC6Pl76PNr4ei9CO8Q8FTs9kIoHQW3IX0bU5WMkYndZXE8/Cg8ukEmYFXRNDyBfDg8osDtYHh0mESBBSgySgKgxX1vOwaElQmO/sErLPBhBEK2QLBmht3TxVCWx/OVl5zvdleGYdkKYqdFbVX1MJYywFYRG9AFtGbPcAcugaxlIIDE3v8UEuDYi8GouW3MNZyfjWz4I4F/apvwZ2QZLCjM5ajuZ0jliJ5h09dDtije4b3eUaC9O/0COXsIJbKO5Cl8v5+s7XX0qDYimmdYNgr4qEsZUUxwABRg1hKqdHAVutjPFuBBudERqI0j4hMCHkFoeQVhBQcp/lT7TfVR1GUAlgQJkAtpSZkEVSkwT022cYdShyA2UoLyXLi3sC/wUhWB2QpifpWs5ZTg1hKoZ7WrKXUILZSDK5njpRAOPKm7t9ONN6PmxbT4AAIAuiVWhgAOV/DyPkaQk7j3cheEYcEJApofT+Cg5LH3B/ByWvgbOWl0dPJ2QuIOs8h2Ypir0tjHwUBLgCGuTGercDGnZW9NA7JUlQektFhc/IVwKZaD8pWFoRdQPs4kFBKsUcAhw4aFHsx1JMfhJwax17QGYicMxgxryBmBh2OrSCIHQKYPQF2dx4Fe4DRrgcFIYtt4haHsIQw2YoQIUSm6X0IlXktFoCwMqRLMRhhHZalsM61prWuHpStrDCG0dTiWAp6ILU0NLm6MVTEAVlKgtkuBNogbP0UgAhqkGBEAbQ5HglEVAFgDueRNI4DWz8FS2a1tgdnbY+iEAke3ImT+m5ctWOODvRiTUJfObSwLnUCp3BBRdd76vf4iegJc1/zmUy5+kkCF8konpI0ebKx+k8mgIf+Ao6nUp2wR1OrRFPMMkzoBcEsLezma1xeimFhJZPlfByW4QOw5D4srGTaKkgPSc0QVnMEgewxNIz06rQhiNQOCkqa3d5aTxrUMdjexVDTa7wChSNEYKFJiR/LArofmId3kwQHjUxO4SYpwF3cPDxMEghwbmHp49S2SDDCbE+lcsrgTp/Wl0OtN1Q4cYB7Ku2Vx7rmgOjroQEJDH8DaiEtEpCwMj8EMINnBwUjDW7iDzzDf0DRAUhZB+Vk1evwtdgC73bks9FjscVTUeL4IcSfbC6+ri/evbu4tl2+Vyj+9c2NuhefKu1elwyvQvH6YJa1rQJTr2YSVQD1n0selKoG7Iut/rKfR3U7jEIyffEjdCe7ky2KS2IxfzhmDas+yVVdr1drnMKCpq6Gq3Ngs22CT6vHiff6OrwenPFt/w5ltvLpi1Ksfk5rk+AH0VeD6+BZtgqyjMsN+vtr789pdIhx4V3jIj3kAfnXLiy9GJUlSXeOI4wKMgrl6UNI7ffeDic4R2Waey2gN00aozCBZe0QZ2iTh/isVxstSRu8GUqaG3vYzN3PEn5sYl1VddQ8m/EjirMId8dw6pbACROT6agzzBsTbYr5dr5HSYKjJZRxVDPKrstyXeYYxU1wlyUqJtNR5y7fRFRKJZrGpMPeFMs0hT6blr4lqhzHpaLtIgnSzVK9SI9MR90CGcdRqShbN355l8i3HpmOugXybS12hCRR9hZv0SEqF82+HqeB1uUyk2Oc0fmG0DA/jWnyJ/zkUN6YSFnVx/Ubp2U8waalb6mMq7lmtP049pGkIajIA+qpzCsOWZbmpfej0FOSmNK4jCZIZaXw4xPJmZv0j/gRLLU9RDVquFQPMGfoz/94+fF9utsNz3tZsEcVnDeAnhFxgx/LD0n01IQGy4hGipBgRtCaNZ3z8VaoppZhWxziqkqwypBZEbKcACmSoQLVYqA16JxZnT++eW/Vd1MVlf3aE4NK+qSzb6oJmU1lKHB5yPwMPUUp2nhDVFUBVlkwI2GcC7Xt/yMq90D2BFiCZsRzgzqstLAspDDAAZvNGljg3377za8LEG+oQei4Oi0By8Kb35MIFQUmvXeOs9RNovp0YboQVd397XBwn/rU+tvWsbs0LWmbxMBtZJ49T8s0SCPvLjrgkijY196BFDTwNuw2FW/xQxhgOsBQc/Z7ynbx0/XZ6dW705fepXLuCrGrWkfLyBTh/Gp9ufbOqV3dUkuFdH3zYX1qC0UqOsuzDcs6P0xIGVqkEceb/AEgr3BMp1H6AKO9tG2OSVNOqc+/IiwMc2sa9R7nCY40Wq4WLJ1D0hPD3rvzc+/N9ZV/fnV25v9881cvSRO/wMEhty74MC2YkdGmtJiSkDbxqDoMluCkZLtFtYOLZj5s17w1aUj/gsmEwzkbQC0dHHeAgqsddds1ZrrrY1M5Mpyza14oclLF6CXb6uiRA3jcbn26xFabjBkdGTDBVdwNthxIyXgfBXsUWg0TTSdWpRsEKcI4o+4BREUeoyBP3+ItHSXJwqK/UPTPP/z80+U7f/3HN6evvqKfbi7++8ZfX/71gi2Y2LklEpb8qAW7/sv65uKqQf9lfeGvP5z/6eJm3UM9UcX8eP3hz5fryw8/++d/fPPzzxfv/Yuf3/z4/uKtGVw7v3rz4uTFTycvTl9eXr38RjX2+r3/5uNHn2j6eHF9c3mxVo1Y9fA//3Llr9/+yb/54/XFm7f++w/nb97765sP12/eXfgfP1ySErjuZ9OphrLzD1cfP/x88fONf/7m5s37D++Izos1+ayKcfXjxdub9+um6H66fN+vCf8nKv+DG0+a4+o/6OJ/XL/xz6//8vHmwyxV7/koTarrmxtW9d6sr3q4L5Tzsz265K8vzm9I/evr+/shLf+jC1R9HoDXw9lP/SMu5Sb493+fDak2xNEJJ/k//Xc1z9oFwQqpSCABVYIpBYpVAiUp6fAGAWl/+iGrM51+uNyQkN23q0OwoqMPAcpZ1qfs65lQKxoiDn9jBvteaTHfn+75g8dH+vJWskH5psdPCvzkawP+dsyZF9AGWwXZoU9M5nv48Tg+O1uCfTtg32YPr46LoZnMDXWUotJHd2FPwJ4VhOtiJ9P2dh48PvRFI70Pi7IlbcSRnpQ0RL+LPbJqeh2hbhKoRSi+k6ahDfZFJiIKk3tZCqow9V/rgEzdy14V+JcsT3/FQenRC607nHhVSGoSWkULVI7tIQnol3T1S/8ueurK/DBcPDnQsEElAuXXKJgEJWT49smw8DlSnsZh6W9zMgr5WRomZR1xWREJvUUU4OxzFT/hz8syXLjgGxv4FcrYZGD5dAcTg/G337rn/oTyJEx2xQpF0WdIekvPrq99TgEZ3qCkDIP+xABFxZIiUI595o25MJQRo3vMhgyUxys6zy5RvsPlUMdEsNG07Dgm33yvOTmz1FDuD/HdQEX9nXvy4czwOCbffF/PD483J0MLgxsRwjkikUK/Pybff688XxxRdJ2OVE0XdKqHOiZrxO9Vu6kZ/HrnW00MvRYwtX6pBCkvYoAlicav422SHlfffhZBEwM6k8X/tlxdajo7fzjiHP8X+2bZbHKrxihfxkPh8X/V332mvHGmSCd/Jo0mxx/UTCaAOeNSi06eTC+Yjrf0t+Put2UzaDFhOrk1v8Q93ja/f7ZcW1ygVvubX6Ieb2mAYxbguA2wcLNcXqJODn4ZpivA/P5CEzRpU5wO+aUmRWzHmAg2bVw0tylaKmJWtZFN7ZgspnBQfE9/XbF/LqGlMfT41Wc/Rllf1X/XeLfPjq9Q9v2//OuHX24+/nLjv728/jfvX/714/WH/7w4v/n5zdXFv61YZJuuY6Q6LAKtqnf8X2QKf0gOBd4cZ4j2aaPuTAoRB/HQsCCNIyk7afzhBMmsgezTovzy8lNWr7XSSlK2qk6JrELSsRTh2WmV4k25qo6KbJiPQ7a7tNolhxXXPdPny4dZwgEOQleBVjTPVmm5x3lEkvf/RW0UOoGYCb+NcVGQrD+OcLIr998P9+ldlzBd2euUMR/+f0v5s5RydT6ClXC9xzcstPqMcJr1F5N4m5+dxrvTl+jFyYstPQIUxqMjQGLOEOUr/Cljo85UD3l5QZfwHxoLkFZ5jMtQL0+oPqqNflGfCGhq6cOr1cvVyVDsTIS6WqPNhh2NQdEvBc4/T7JUZyj6Y1P0/Ph4F0ws/NSik6ZLLTrH+adHgrOLcVJajU8a827t9D6SDy3c8aew3B+P7zE5H03riaOkn9WFC8I8OEQo3+AMJxucBE9m+15fTooSMhnbjGb56rtWZuOguG3ZYP0TFE0Xtun3os02QjvNns5Jh2CV9YqTGT7lC6b7O68eutmnZ9/94TGOKG716gRBPlm9YOoxvXQZJjvy1S83Px2T0foPFUAz9LdnDQ/BKk43B9J/ssubq3N2fPtjFewjaVE/srxq71iu2GlIEp8gZTgvn9YB+ft7+vBXPangE5ARAJbZ6xJnPxD5vc8LJWlT+ZdY47Jk29saafHcqbLJYoeyDkEtzLTIhw74aqOPqIH0ryGvgjxoTi4HecVYO0pvOz3yVXtZVNwT8h7i+nXr6PnR+vLq4/vL88ubv/jrm1/eXn7gjqYfvT76x+0Rd5f49S354pZMnNEDJvU1De7/jPKQ3tcq6Nev6R80AP0f6SOykITa3L9Pg2o/pv7hdfOP9l6k/5LNLOuvnzf/oBc9PmTjeBPT7/rX36s/CMxR7Ubln0z476RYKg302FNBSuFvXTHgDcGjhfy8rWzsqgdN4oQxkYRM83AXkgl4HTQ5RNHz9o0f8sXJc3bBoCQdLvl0fPrqm5PTr87Ovv79uR7zqMJqc5++OP325MXX33z91e//c/T7/wNOX8Tv=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA