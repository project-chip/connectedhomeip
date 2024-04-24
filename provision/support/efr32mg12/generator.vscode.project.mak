####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Resources/git/matter/release/third_party/silabs/gecko_sdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.4.2

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
 '-DOTA_ENCRYPTION_ENABLE=1' \
 '-DSILABS_PROVISION_PROTOCOL_V1=0' \
 '-DSL_PROVISION_CHANNEL_ENABLED=1' \
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
 '-DOTA_ENCRYPTION_ENABLE=1' \
 '-DSILABS_PROVISION_PROTOCOL_V1=0' \
 '-DSL_PROVISION_CHANNEL_ENABLED=1' \
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
 -I$(SDK_PATH)/../../../examples \
 -I$(SDK_PATH)/../../../examples/platform/silabs \
 -I$(SDK_PATH)/../../../src \
 -I$(SDK_PATH)/../../../src/include \
 -I$(SDK_PATH)/../../../zzz_generated/app-common \
 -I$(SDK_PATH)/../../nlassert/repo/include \
 -I$(SDK_PATH)/../../nlio/repo/include \
 -I$(SDK_PATH)/platform/bootloader \
 -I$(SDK_PATH)/protocol/bluetooth/config \
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