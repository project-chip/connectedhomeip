####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 5                                             #
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
 '-DSILABS_PROVISION_PROTOCOL_V1=0' \
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
 '-DSILABS_PROVISION_PROTOCOL_V1=0' \
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

$(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/multi-ota/OtaTlvEncryptionKey.o: $(SDK_PATH)/../../../src/platform/silabs/multi-ota/OtaTlvEncryptionKey.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/../../../src/platform/silabs/multi-ota/OtaTlvEncryptionKey.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $(SDK_PATH)/../../../src/platform/silabs/multi-ota/OtaTlvEncryptionKey.cpp
CXXDEPS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/multi-ota/OtaTlvEncryptionKey.d
OBJS += $(OUTPUT_DIR)/sdk/_/_/_/src/platform/silabs/multi-ota/OtaTlvEncryptionKey.o

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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA