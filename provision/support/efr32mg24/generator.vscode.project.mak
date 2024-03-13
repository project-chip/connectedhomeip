####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Resources/git/matter/stash/third_party/silabs/gecko_sdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.4.0

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
 '-DSILABS_PROVISION_PROTOCOL_V1=1' \
 '-DSL_PROVISION_CHANNEL_ENABLED=1' \
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
 '-DSILABS_PROVISION_PROTOCOL_V1=1' \
 '-DSL_PROVISION_CHANNEL_ENABLED=1' \
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
 -I$(SDK_PATH)/../../../examples \
 -I$(SDK_PATH)/../../../examples/platform/silabs \
 -I$(SDK_PATH)/../../../src \
 -I$(SDK_PATH)/../../../src/include \
 -I$(SDK_PATH)/../../../zzz_generated/app-common \
 -I$(SDK_PATH)/../../nlassert/repo/include \
 -I$(SDK_PATH)/../../nlio/repo/include \
 -I$(SDK_PATH)/platform/bootloader \
 -I$(SDK_PATH)/protocol/bluetooth/config \
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
$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/AttestationKeyPSA.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/AttestationKeyPSA.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionChannelRTT.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionChannelRTT.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionEncoder.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionEncoder.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionEncoder.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionManager.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionManager.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionManager.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionManager.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionManager.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionManager.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV1.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV1.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV1.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV1.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV1.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV1.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV2.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV2.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV2.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionProtocolV2.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV2.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionProtocolV2.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorage.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorage.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorage.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorage.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorage.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageCustom.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageCustom.o

$(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o: $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../examples/platform/silabs/provision/ProvisionStorageDefault.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/examples/platform/silabs/provision/ProvisionStorageDefault.o

$(OUTPUT_DIR)/sdk/_/_/_/src/crypto/CHIPCryptoPALPSA.o: $(SDK_PATH)/../../../src/crypto/CHIPCryptoPALPSA.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/crypto/CHIPCryptoPALPSA.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/crypto/CHIPCryptoPALPSA.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/crypto/CHIPCryptoPALPSA.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/crypto/CHIPCryptoPALPSA.o

$(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/Base64.o: $(SDK_PATH)/../../../src/lib/support/Base64.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/lib/support/Base64.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/lib/support/Base64.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/Base64.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/Base64.o

$(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/BytesToHex.o: $(SDK_PATH)/../../../src/lib/support/BytesToHex.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/lib/support/BytesToHex.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/lib/support/BytesToHex.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/BytesToHex.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/BytesToHex.o

$(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/logging/TextOnlyLogging.o: $(SDK_PATH)/../../../src/lib/support/logging/TextOnlyLogging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/lib/support/logging/TextOnlyLogging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/lib/support/logging/TextOnlyLogging.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/logging/TextOnlyLogging.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/lib/support/logging/TextOnlyLogging.o

$(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/Logging.o: $(SDK_PATH)/../../../src/platform/silabs/Logging.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/platform/silabs/Logging.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/platform/silabs/Logging.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/Logging.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/Logging.o

$(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/SilabsConfig.o: $(SDK_PATH)/../../../src/platform/silabs/SilabsConfig.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/platform/silabs/SilabsConfig.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/platform/silabs/SilabsConfig.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/SilabsConfig.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/SilabsConfig.o

$(OUTPUT_DIR)/sdk/_/_/_/src/setup_payload/Base38Encode.o: $(SDK_PATH)/../../../src/setup_payload/Base38Encode.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/setup_payload/Base38Encode.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/setup_payload/Base38Encode.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/setup_payload/Base38Encode.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/setup_payload/Base38Encode.o

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

$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA