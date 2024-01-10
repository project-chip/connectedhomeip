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
 '-DEFR32MG24A010F1024IM48=1' \
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
 -I$(SDK_PATH)/platform/bootloader \
 -I../../generator \
 -I../../../examples \
 -I../../../examples/platform/silabs \
 -I../../../src \
 -I../../../src/include \
 -I../../../zzz_generated/app-common \
 -I../../../third_party/nlassert/repo/include \
 -I../../../third_party/nlio/repo/include \
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

#$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
#	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
#	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
#OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA