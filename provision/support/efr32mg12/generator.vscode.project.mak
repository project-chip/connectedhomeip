####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                            #
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
# SIMPLICITY_STUDIO_METADATA=eJztnQtz20h6rv/KlCp1KjmxBJOSbNmZ2S2PLHuVtUYuUbPJnjiFaoFNEiPctgFK1qT2v59uoAE0rn0HuFVJTdYiiX6/p6/o69f/c/T17vbfry7v3bvb2/uj90f/8+3o7urLh/vrv1y57E/fjt5/O/p29PejV1WI1e2vd5dXKxzoxz9+D4MfniBK/Tj66dvR4uT1t6MfYOTFaz/a4i9+vf90fPHt6I9/+Bb9mKD4N+hlP+AgUfo+jNcwwE/ssix57zjPz88nqR+Ah/TEi0MnTZ1Vtl/78Qn0YgSxKA6dQJS9rDz8Lw5H1b4dYekffvhxEwdriH6IQEh+9OJo42/pb+RXP4Dlb2nghjCM0YtbPHWyw+p75OOfyGPvnV9THB8n9NE+SZznGD2mCfCgE4Isg8jBdp98El0nxb/HKHPgBp0uw+1i6RSCTp8Fpw9ldfX589Wde3d/717iJ82T9BjoBYmewlN3DTdgH2TWkmXASC9QkgLXQy9JFlvD6TXRC4Pzcw2ffA+6fuRn7tpbezbLzoitXrxPCMK7+9vVpSWkrv5QKqUBhEnmhxDZTJ9eK71IIElc2m74MLXGNGhGpDTBcD9VYWqa6oWDYeA/uKTJxSEf9ltraCOGhlItfIDrLLCXjb0meDA0fSdg6lgq0H50ildf32sQ7LN4C6PB9yAuuIHvgQwjuNlLAg3yU9POgJXeZL27v8KvqTCJIxhlqQWYroGh7PXKp1wMDoLYZNYySdNrRqTV8ILYe8RdJRtQ/XaGqOATwd+BaB1AZCeVOibEWKwkTsfEEIsfpxmCIKTP2ijOA1aEiawkUJ8VLlFe1vwozUDkWU+qrrEBPr/1PnCpmhW+MWOifBkCUbqJUTgtaa/ZQea83z1FevYbGuV62PtB5kfT8Q0b7OUM/OgRIvLNSbA2DtVS7yU48VA1IMF/Gmfo6PN7OVvoPcZuun50z07OThZMb6fxWBKAjBTS6vfOEx/zFx/ze+eJlY97MnH0BTykjcc6D159ujtd3nxeLL+2nutKxnvUMlo+xxbQDKBsn7hVgiWSrXe289HaTbDKi1PMszhVujll0jhFCjhMNJ06Jk6B6vSzOJwIvKQZDA+Dvw+ljd8ocIOZdx15wX7Nzb3a1PJ0uTlfLLfhmwvJVsVEAlBcZ5CHk4l1OBd4YXIQMahIJNjX3oGgFyAS5Bs3wT2XA6GvYeRigFv+A4pCSSMeBy/cHwY+BZEgRy9xPnN2IPwsjlQskiw+mChQFgn+VLpPaYs+bXY2+exkciDaHEjiMzASMQgBDph6yMcZdyAVocskFR8E/3Yw8aAs4vzwUFpTKNuawiw8EPICRJx8k+zh7jDYKxRx+m2CR4aHQV+hyND7B9J8liRy7O6B9PtrFnF+f3kg5YaCSJCvwaGgUxJx9gC/Fg6DvSSRY3e9A2krGRiJGEDcyUtl1/ZsxaCGkY6B+7CX3SJjNxYlkHxMDqZANXjk45FmhxWPgkcmHgc0GmZgZGKwxxyHEoGSRZw/TA/ktUZBxMkTLzqQdC9JJNjRgUzAURAp8oNpPWsWOf7U30YgOJwcYIHEY4IOZdyOZMftKA4z8BAcSJeIpZGIQ+YdSMNZksixu4eE7yrFAMEDaf9ZGvE4pIdSf1PZ+ntAfTaFHhvduXwgxZ+lkYgDimR3RNripyTi7Pv0YDrMFYo4/dPBzAM9Sc8DkRBunBzIXBBLIx6H57X0XmBL/CWJHDseLBxI75ml4cUhpJuS5yRnGaQ3O83H3csitNup58vOV90v2gfxwjCOxrbU+ZHX2UrX3CUMsjj07TR6BZ6DGZyGIWcUyAtTP3XjdOkW4SdA6zM5CukzQeD3zM1A+oj7iFsflwY7facGMNf8eArDaG9nabWZqqWZcRiQYsN2egyt8lcZ4pS/yUqdWFlz0wxkezvzGU2g2pAz1ip1GpkUcRqZ8fJqY6sqjRgm41aXzvbUgRJqHZMxxCkQL6kHgkD2iIkKEmtKrlBkcRx4O+BH41u5u2Vn6EC5zdhWsGW8a5MO760t8M4dPCNPCqHdtqaOGa3kHcud3kp/6k/PKAK39Tw3QdAvumOTQnZNc4uKdAfvIY6zIAb4m7FOXvOE9FhT3HzSSo2qkVsnt52u8dEWhZc2lzer69VYslzmri3GaurQAYNGmhXvDnKW1Jc/gymWZnlUHMJbDSy6VkerQvE4dRUyLSNjdBQx+52cPMrwC3gqvqZFgfTbWpo3HEy7bf/UYBONnOL3wrPJyBh7o2BhsncBCp/eTkXWMCjf2HaqP3G7sTTWQuDepcWEyFlbpYdaHM2kOHUz33ucjIyxJ99zGjit144QmXQhNmy8vtg40RNtTYvmX/KYmncyknEaMF5eRfqePR4IbJQOGi2HMVd2nfoJeH29tueeOalL+zLMtrbiCyIPbMAfI46eLM0MCiKX9uVbEpFhZccTVLqw0qT0RZYOMwcQJEvVzOA1gXTZmpG6tG+gH1H6vtCf6SiVrCZMaaRMFdYoLwcrNx8oszMjxoMsDduZlqmSwmajV0WNtnisUan0nwOyNGyg2hRrZ/qVhq7BkVpttUgWdpiZUdYqL+Po448QRTCYHrO2a6fisKlhs1jSyJULFk2rcnkwOWZt10Tlqdw0GqhAlZbdglmZqQpnwzA3+2rPlDsQFDsj5+JtEEiCz5vSTQQOut8KaLfStKn7rFtqvuqCOFEMy6ahYVgiN6blbFs2PwkCwzV6GpsC6dnrIp/V0IstrRLl/Ox6emXKQGNPfDtrxpx1D20xAYiZPPpte6NFmzw8FZQQjLuFEUSWJj/6Uoqxx4ez1RD3gfU2uwNQ7iYAqZ39jwNotUU+IJlnnIytNGZh5qrl6J00OG7hW9BKn4KJHXkrj1uXKSZTwTYsChaTqdhKY/LFJPAfxosJfiA3cXlzli8zArtRwuacHpM2OgrtqMttecUDMW/n29kznLMVb//aysheLvKUpUWBJkrf1H8PSb7fANjqHzSBGFucrbaTpFLT0HhKWdyy2kgloR2r+LkHS7tDGzAP3Z2hPWXI3t7ZZvkR2TqLn0Mg3OwjW904BogxNE5kc4tOg2hwW043z5CtIVEjx1DfaKiHxXLfu8U03PPusNnzi9ikGvB52M8zXZPdNjdOZ2uxu8HUu6TdJrHmEayB0u/tq81iyx9HA6XX10YPyWRFp2mL02LbOnnfbK57T9W3WYp5e/s0tR0Rnunax669cT57E5MNrN5ZSO1TKrTzZ2fkR+jJoK82IvBOto9CrQi99+zTVHb4bxbrMFAkl/I23zpKaYX/ZrGOQo0ItODWUfrX3gZacOs0tR2RFtI6Tu/ynsK+VA9TZi9jky3sLVuchej6rjV6Pwnv7o7WNaMDqcremAM8DwYQ5ZehWVogK5KkcbuY041afcHcMBzvlDsTNA4jP0HEC5+t3RXq0WqxCcSK3MxzkDnVByZ0ll9oEr+76oyJUJy8uDuA1s/A0htfOCHoToE+KJ4DCYRiWxsaJOArDG4ZLIPPz9xk4YDTITiwdHJPCrzJIgYOPTv3EqmAUxYOeCWwA/OTt2D4pby+g3l++A6O4IvicPjbPGpvhr6NGsPt06xvxvIwds3CyTPcGLjA0vYSKWoGhNcyeeFhIDMgPOQQeAfCzJDwqvMOLA4DmiXhQy/P3xwMNsPCBz9fLA8GnGHhgG8PpTpuhatjtkMQrP1oexjgHRypbsLs+B0cuW7CAfC3ecTS/wkEe+mrme2lf42j6suub/6HKK+R/9Q5s6A2fi5SOndROmn3sI5Go2dYggiUWDoGKUQOIwb9SIK1j4ayeAJLLitaPHKxoHtLDigeNZFATFJYbv1w1zCB0RpGni1fPzLxGeESmz0IQQS2MCTzjHPGpZdGvM7nt7ljFhywyl4/2Vk7ECTdAowCasVzZ2vXr5lY7ga2CEvFEY+IDjiKlE4rhgB3Lw84iiWeWLfLz1I3PxQyb4Q6LGbnlhoPYVM9D7WSx49wNCNyUBDt0wyu3TSLEW7vpu2gMulE+qb4ozMG1s3ynoJ9sDHjwXFjVyx/zB2NiqKnAvYXYrH+8ZzxGkZS6lnOHZVeIrmYbCDI9mjqAet4XFgmjX7y3DEa4ZLuJ88Zl14atZ4H2VlP1uPnzxwenFr8spfkAAreGJjqLBBvz1H7Y7Orss/8gOmrtB1M1ykwtkNpgyBEWdx1i9vca0QcKvJmpQbcE3bWZ0rfjAY7lSQpGk+U0SpcQZauChu21TqRQx4vW7Es7Zt2e8mLaOlnsm2fU/Oqx3EdiI2enpYGrgiMTa0WjmV4pZfM8ZKb8PhDhM+XlwNDhMZjH+5uyFHPT73PttKfGJ+mPhSJ4ZSxdXBknJLUKTH6OtM9xHi8jIwuKmhgVyy9A4HBrvVwn7uRlzcwvAFR32bLVrLsIEjcs3mykkI6NYPwIEN4AC3S9pm/MoebBpTMGbpEp8OICwsWjYyOa4UpWeMcTtzHRdADZLC7hhsyY2+6oydMPYzC2//4RDpLWxzpZB7yNgCH9xOWIR6OZ2FljXM4A9/sDejCjKVh3u5AmKZ4cEMuqt+YPfgkTNpF4DEne3IJQxYbH3SIM3cQBJifEUgSaPZibSliFoDDW76wZmFljfM4UfwbbunmSVPWOIfzb3u4nycxK8u8gT0Mk908LUBtmseYAe/RzbuR8+R4G4DDuyKP38yH27LPTd3cO+6Mr4IOAW/jGkiN+sMSBi0N8/jIIbZ5sr42LTyH0tf1nnKQxBoddXLV6CtOSdg2PEqZ98CmpCsNjlIVr4MpsSqL465BG1V/Sr6O5VFOUvOnLXSVxXGuor5PClaZNOAflO7kHJ/iDvwHBNDLuNc4w2fLOslASR0K4wycH2sypdECh07t1rwOGWuVy/eM/Gx6vsrqKN8DSOEbu1N2bbja5CiZ+Z1vXLLhvWw9ZPmgbwa8yu74m9zscUou28CRyTaT6+3R08TNSNOsIKEbweeZKEvTHNJ1avTYqQAgtTjKFRrd/8eFCnv38zVbOH9L7hqftIWrTAqQGffnI4g34OGnnzGMp83XpllBQheBaets1/QoaexPm4jU3ihTYtYDDpcp6fOE02Z69NLzaalKixyuiaEEiKbv+zI2OWzT93sZmxy26XtvjM3xN3vhY2Xad3ttU4TNTeLA6D1VooCV4fH+eYbcNXowem6O3zlnjI7S7cjB+MnxGlbH54vyw+STwtUmx/M1jtIMkK2Hfjhtu9KxPD5jVB6znpSxYXW89aM7QKdt/hijQnQukZwFsbI8zpmCGRAZo+PzTgHum5re582fd2KtjvfaIzfc291r2emtVyYFZsSq4zbTInZtC48gJ+Vs2hUdQc6ASM3KjCDnoSxNS5DicvIEUv9plrzvYxCeNZ2St2WX08MwfQhLoGsxdMpquO/j+mEybcPUb16GeI7mdBiBOwtcnbY07LhHaB64bVyAdoaGoG1YfOQ4KWXL8PjOgjDO7O4qaeNVFjmz6uRQ1qRgtUke2eQtOmNznC1JLRyr5eM1zHIJjR/NFgIcPIrd4SOjSsu71voAa7Ncwgxhy5MDVlbHR4lgbfpuSf4gsbbJmwWd5bXcsis0Uzsxn1D7Us42T8tW2RSanZgph/usj/Ii4l49yNwdDKzv5G/T9tgenyFNA3cNH/bbWWh7rXN5vcCfejKoaZZLGPpm778S4iuNculwwMXpPP3sfvOCxI/wZfri2TQ9Svr9/PW7SWdQS4NcKtfDXZCJ1y9bdkUYp51/Zo2K0Nndpz1Ax9+rXTyYTru7kDXKpZt+7bxhVYxvlgxuWBblnCGrG5a5fbXyBs3ifTn1klLHuiDvTKAyKWraJaFMeg75GxzM/em3PPdaF+WlzrXK069uhI1mIPONurmUicIokGis0iDOLDmUlYlLD4ZwDKhnwLnIa/OixFNvV++aFiXdbNZGHdXKoJa2RVlNO9WVYR3ymDvEatg5rgzqgOfbIdIEPM5WtUrboqxo4kMCXdPCbdZ8zRU/Rb0dVq5vOSeXAk67kNlvX6oPNPX8X8u2ZB9oJtraujDv1HtvemzLsM6xIjaMoNjTnIu+B0O/r3wYcWkCyfTbZuKnpqX6bTOhlral+m0zsZa2RVlnmRIfsC/MPPEmj65pmV7xTKTUtFSveCbU0rZwrxhE6zicfhfdMIJMf34uYoHdQM1e9UygqVRZaE+zzAXdxZCd7ZmLfOgaChHvxENOVYV8mHQSBVh2WlimQuX6j34GQp4KAfKnqbwdPGqYx5dGi3n4qGEBvuk23PRBDm696XeqMgdmbZrHWPgomIWxMs1j3PvBGvfwNnb9bA9yNsxzWD38TxDMVMNZ4zxOb55Mp3Z5dDuA/1u+ngeRMS7EmcTBy4ykpXkuaz55ON2sZZe2CcDjnW4GsEM6OP3XZpxqYNYhHBiWdQ8yJSA7Xp58n4eyYZ7LSgqGC9a/7dPMDeAWeC8W7myWgB/lUYnNBuGx3lQnOITiwxJJxYj0/PPAhdABxKiHSDpG6Z7MgcLssGLVQyUVszSdZn5jPCaUQoo836U5P3qJIcY+cwUXrs2MK4uZcJsEPOLSs8ossIxxDme+KX8WyMoyl3Ce+ZG10PzIejfPyITa5dBBb6L1ozZeaZjPN1PzU1nmEv422SJBl/G3oUWC7nHqmQh7lzMHnIPNQljb5lFOdqS1wzhywzBLuJ1pFmQrNAuye1xvZsErDfP4Kmdis0Cy1jmkQTjPG4/a5dDRp+ecm+ki8Jin2UnW4ezdRNZhO58J7lyEDoYxeqHXIbggCOJ5ppAGODj0ER6GprH3CCfaO9Gmbtnn0fp49PY4jS+nDmltm0NJHPTOQUjtcuiIq9456KhdHt00PhU6cCK3ExF3wgu7Nx8P85WmBRjnaS0rywKEb2cjfCtCWPq0nAWSMS7IOd+cT4dAlDh3djkrcUnAI46Dl8Xp65mqFGOcx4n8J5BBF3geTOd5lXcReMyl19NZaBnjHE7k43fXevFmnuXrhnUe6UxzRQObCNvuK3Zgnp1IpWE+H/FXPRMhNc1nPJ2L8FSM73ymPlJtmsc40/qZ2IpZ7l8HeLt53ugN6yKk+daKdO9nMy1C9EGIcMfxoz9jElfmBVgzn4yPZ2OtzXNYa1f1c6A2rPNI/XA2zMo0h/EJs/gTnehrQzK2OZSzregLruFXno3mYiyNi3HOU8tZ40Kc6TzLQKzxzjGKoVuaGycOcK+fd9pghi1qmMoR34lGD2AXW1nAnpyZjMBDAKeddKyZh1Ck4kE3uzzCF2zPR272kkzcoehGaJBJJWbpSxRHLxOvWw3GiaU5zN36BF1up34RTxIm8yNyGD2J08m7pUyK95PIxSJBuLuQ+U/zx6JJIhcLUoNwI0ymh6bu0fTEpEsjFpti9/Nc9LV1UdrJF58btKKnAhpeCyZ0YtED3YGQZI8y+D1LD6DlGSdSi9Vh1GE+lVrs5m9nx4nEYoUDomkngJkIVMbFWGfYkc/Aiu+8p64I5lgSZHglVgWr4/K0SM2F3AAQZPZ/n68CVsYFWTO092brDdTWxWhnHU5JjpueQLCfD7a2LjTD0PNl31fNWYgUbrcQtSYi2s+8pBkMn3z4zPOisLr6/PnqjjetUTzl3t3fW3XIV0TNqemdwrDTtM8pCcXDVosAB1SguDIxmhG0tG+stHa/aBVMP/BxL8Ul3I1C130OP5F7hhsv6XTjYuElpV0r2sshAX4u39pouhCz0XIYdqeJ5zQIRn251E+aLh7yrJ0ColASGh+bH9hogyQ58ZJEKsoJip98suKAYxpBBLIYOZWO07URAj8yYqQW6rESPYWnkuWsz0Yp02OBxFGucAyl1K5X/wMOk3v/i6M/Q9nONvwOwiSA9Z6psjTWDF19LsTX1QfpnJMmqYz04Hwtw1zuQBRB2ZUhAZY+CwIg+avRfMoMmhlDuoq8WP56OBkaxoIIiM2EadgYg7kp2lWLqcJYEAGxmSoNG2Mw+I8s9mKbFYk1IYLyl4XVpGmbEUJaToO05COtlDzLydCs2s7jRkE+kzeZtMdcBR7GkEj62Myvhg0BmMt9msXhFEisJQGwj3AD9kE2BVnDVA/azyr+2VLkEfeHTop/jVHm/NzytNZnQTauAzYG4/GC+zL38Z+grJefjp2G0Lgl7Ti1pHqsXf7p+uuXeLuV33rVNhYUKk5LscfmPfye3UbBS/mUbjRLy726PfZXeaG+VFmMI6bblaMtx7OoEt1Rm0Px1EnftkFempJcv8xn675++KIyniBGi+k+p19roOqfXhRdRYWMTGG2T/Cg/iWIwdppi/HsqURwxOJQHG8zcB884YdIgiiNGfuyM8Rttn8cZ8AZ0BckMVG0xlnYdClWsPcoH1KWWPQNhDGwGgwa3zzEAK0v81V6/8EP/OwF/4y/JxM57a9Db38CUHgCN+h0eRJuF8viT/JXsjxdbs4Xy2345gIHz+I48HbAj3qk0/Vj+1sPv7qLuJ7gX0/wkNjD/0scTr0/Ozk7WZy4i8Xy7O3b5fLivJpq+zHEpSJ4v4aph/w8Nf7wo9P9rpj3aSQM/u5H8sb+DXoZ/vvo1dHq+ubrl+vL6/u/uqv7Xz9e37o3tx9//XK1Onp/9F//8+0IwTB+gutvR+83IEjhq2KDUeZHV9/zufgU//Jf/11/vYr3yCu+LT0yOZ8QhHf3t6u6HXxV/VbPirrk5+aPIME1AsU4NTIfsqfj60dgiJv74j6L4jrevofIlA7+Oc/63gcaV2N0f87v+n3yPej6kZ+5a2/tiTwHw/3QY90D/70/U7Whp4rpw4Ff0wDChBz0Q8wTJKdwSdkHEOfQt6MfaVl4f3OTf/nD9zCI0vf025++fft2tMuy5L3jPD8/lwUVl1knTUl3jzx0AvMp42+kShVFIA+WoX3xpb/OP++9k8LuSd7Qnew9WhqqebGTreflIZJ12JD4w7e8JOeLP6R7meI6SlqMwtDJ/yX/65CHqqJdRucPeZJQLBxhovj3V7qlmmycxNTO3f2VS+pzHMGoOA7/qv4x8KNHiEi7dxKsG7/grMHlOvC9vEbmq22tsPmBA6rr4scA7sV0H2FLmkdumk/JrGXzGfhEJHYgWgfFnYJjP3cs+HGaIQhC+kSPfueJYY2c0899jnk9D/qtKuHSn/gPZghEaX5GdDgIqd/1/j6Oft4YdB46oJpzD0PyooT/gHWnOwP9anDG+VVfr6r95a76qhyhvWJHlq+6g6ZX7RHbq6Eu6qvOGOhVq+f8aqTrM/BbodI359zzdTX5+2pojrXvh5ZWY2aw74fW8+zsXd/35TRa72/L7m+NuZO+H3a9X7MTGz0/N2YXen5nJo9e9Y2y2l8Wz/WOE18xq2SvqlUg/Fe1zufmnbW6+3p5s7peOZdkje+a7lnwwtQvtvb5dWMrL4ArunJY5piQbPj8vrDwTCFomOxd3Hd+eqsQNvu93GAnFph0MpetSMfpUiVwnOYH6WSCFg0dCUkW/0lgbzzwx/wNTgbsZHn4CxnwXH26O13efF4sv1YkvSMNHpestAu8kFuq5UXX3MIqr7lxE9yNsKIbo8y8sBfuzWuilzjv2ttQpueazMqmkNvuSIuSHjA9bGJWNwRuOZCOzScxlkfwb8ZloYVyBrPQuOYm2cOdcdVt4iHzjc028c0XLyLqmm9s/aX5+PtrYF40wBXAiqjrmS9XeFALoxTa0iX+/6xp20sON+X2yBSk7bzRArgHyDxumJqvGYkXmQdNkPk+Dda0UbqIbOpvIxCYR0YWXo8oDjNyftq8cMYf5imJurZ0ETRfclMLWWanjaHT7ObTNkPVxJM50X1qo0F8stFXIKJunJjvLzyvY/MJS0RxI26qYQzpHL8ZueJEglvj6snS2Y40w2Vpn7CyWrMepWwHlqP6EMcZWTmHqLVq2PrI0ylOE5OTRvlCTZpCflVphcni0OfWhGaYaqqKOc0sEVohDKR3P4qHIDP0e+57uRHGZyIGv2duBtJH/KbY+jhzuY0wVcr3RlQ5IZZ7NAwuRB4IAtE8L0KNM4spVZsAyrTb4jdkgiA9vyYY845Ksd6rFzqPinBhqUVoklIETjLAcI2e2KIAmbUaTiiyVp+Hyc9hyAcp1/lVgm4bawZSQXcgUAzmbgKQcl8ZfYHJoq9EOLIvDv9/HvTy5ixfSgDCoUkBaO2jINnrFgvdYiWiKVTHXSFwHnduOBLlvPyFgk15I8gDv7lrPO/tfO5LtRmA38FtP884FhEPJvR+aAVBAvW1FUC8/rABhSace4KopITAHGnjeZH5v0YAgVmB9vMq8UAg3OwjOVMiY8pGgKIPphBEqRwIDcwaIQSXM+sw5OUuU+tIY0OfF2ppyudJ3ZEKUFQBmSBQkikvyDIBSEGWeT4vXTIBaOmSCVIUEU6IFHp75JOzuMwuKnZPXbk1vNopV+8oIq7QA4gq5zS27MRh5OPuYAr5030qZsgGJitRIZUIQJxO/O6VlKbnheY1yf1UpkW3FkCJt3MbosvzNzZkzxdL87LV16Zl6429NpRr5yMGlX1b0JXHZaP5Vx/OcAF/ekVFltzJaFK2uJ3QpGL13Q4Y1c3nafLrHvFgCa2fAb9jIatf1jvDskwRNqvsm5AmEoWzq+JVUZSzwg1HqPNSbgkTP0XVXZimBP2I7N/FY+cM7XEXau2mjW2URmyQdLZtJ59qazjTE1nsUFDPN4gnuO5EmUtGTaRLpN5oi9mpfGiZslF/JTicVxTfQJDtkVn2FJZjNXcNExitYeT5Zkwwrwmm+hpoF5h2zM9St3QKY0a2t+CbV2cLJLUEcA9gGkvFJR/T2NoJTCKasYQHFSYNdWu1HXFy+Ma0tOlCO9RKcE2gfD2ROQ5VdtF7TkhxmxwhMXIA0JCUwIykoFL0xJ9x65OiLV3ngGO6UEn6HjVyDNKYWB5LQany7FmZWuVn0VQaCu+iTKCL2NKgcakYZOPQCp8zCGrUJ0GrtdzqG9F4dDR8PZFyibbWkI9MRyNfUhKZiRQW00PzW3LC6ZTPjVaZVUyq5224jsAj7s5DWYZ6Kb1iEE2NPgHK0C+h7tjRnFh/6nS8fG4QhCiL0+KQUbXHpvzaxa+6eCippcWGjzDxpOhumvok1Eha9coU+YW72DF+JhqafeCGLw4ab7FKMvRC52rQzROVNwHpJGnprIivhRuAozbULRCWGt3yI6xSJbKmDu49IegBMmRfww2psiMDXmHVRhZqagX+4P5xYY0Qpimuu2QD/WbwDSSuluzJJrEsHhu4S6k9I5AkcHDvtbAWmfsa2WMsroPi33Bx0Ob52x7utWFSGCY77UzLvaWQIZmBKkx7VWZKE9kzpq1BOg3K0cprmGJDW5Y45/PlpfPh7oZsFvqUf2teMc881UhWsjcwvAHR1tlBkODAipRFyVYM3Cw/iiKk3Ci/H2mBEQw9dGcfEG/9BiWQP7RfW1wjjRYmNJ6Rnwm3VkNCD6w7CXUVfzu851ZcpXHHmo6SB0IYBPpZ5XnakfJ2AP+3fG1GJ4mDoU2yEko4ZNOzkrpWMROqqxIOHq8Q18h3mh0vT77rKxX+cYpbCovrecbXrPWkNygOyaShWXEyC5kr1/cLmRXPPbmlMLNigN5Pbk6wvKPXgKKZrCLOorJ8DkZbLEPuGj1o1+Xcx5y+iPY7dr3TbnShtx7adS6joZ/R0PstAY/aWUy2iehKFJsttGVGlvyFRbb6r9Xd43rocLm4BtlGZ6LmBKF2oe91mqiuttZXONeWKOYhi2GDKzUFOSQZ4ddNGnuPcNCXjbgUHkW6j8+6MrGvndTJ4EZ0cQnhIfmwgpcuhKdix1S0yw0RGfKoJS5Cp+xN6Rh5U1diJIS2GB4KLE5f6yd3fmUpzHdVp9rVivQMTcQO+bharBdvtMdNSP8lTnYxG9BYnr8xoDJ0blBG41y/phvooKdk4wQe1GpXq1woH4im+/JmYW29OH70jZARb3mDDiSElaoNztpCfmhAZfx8krCMiVEZ0cDDc+3iSHW0c6rQSZU7yWSPqv5MWH2NqL5COYYmHoNdGJFpaeVOT1eVjqrJ9dkJ8NHo9lJ1+fQliqMX9X46I1w7W2avMjck2r1F3JBw8xJvTdHRY51yQjrDnb47y0f2Dyvp9d5bb1jceKaP3+CuKV7drq6pozdvOHAjuaZU46ZwXa3yFm9dneqCbU0hUy3r6NGwQaHAf0AAvTjDJ6n4QQN/G40cYuELpNEC/5IKL4X2CRTrbooCdL1NK3R1fEU1GehynSpEHnrsKLiUhFYc3DAe2vMvo6DN4CLwrM+RqxhhwUXkCaS4IVOWi9xwr1zA6BKjzju+kho7ayEYOt+wZEJCNQ5KvZI6NLtYpBoLVsT1w0Q9b5tKmk1RtXilGK9ilUY5sHKpwEFdb4+e1F9mtYIbQeXGg6hUpxmHzz4L6ui1GeUSk2pU6HHgJA5Gd1ELayhGo14WUoQIwlh+faIMHCq/z8K1ytI0E1qniQuTVHHwxSoorS6yAmSSTL7zzirgsc2gV0quAFkNUsy9BKxHfJ7xgw/6m+EHfVQPqd3+Ywmdl3MZXNk6Wa9Stq01lEgetQYSZXDVmJdjZlXzjaUsVRGeuwMJhbHjv9IyqonKXLWo1W3tChkgCvzh8+NSQhqjtpaM5iuDUaMzXrInE8QV3SjO3cQOHtE0o63PrdGpbanos2w266HD69Iy+jQjR+mlZfRpdHvbrJR6d5NRGT7/L6uiz5JvmtOHUdl71yODQLSOQ61RM6umPmBtqeizaHRfGiIGSII44/s50VE0wFje9mhMSYNJt+NFHtLuviPijjHI3B0MdF67dHeMYmzy/RpF70bVfu6JgVyxrRsRctraH3SUKxQef16car8eaqVH+KIcn3rbiWLe5Ds8NMLiCgOB+iip2hyiF165Oao2g2iE1xomVgK6saAi4lFJ4TZ3P5A7R3jy4bOzuvr8+eqO/iNaIjkybn6NsDmpgioEflTdiVv6xzqYW7Irb0Or/OM/2k3ZB5OOl/ngk2p8jdPsZ7Lt6n/TUzU96e0JuFTiENv0fxNSNSH/4YvktyOyVxBTOyce8soVYPznAbWie68MP3nq/vfRq6PV9c3XL9eX1/d/dVf3v368vnW/frxZHb0/+vGPOAm+ffuBbrH96dvR4uT1tyP8DYy8mPTF8Fe/3n86vvh29EdsExulNvEjEQjhT31xpTfBN6NKROn1US8rD/+Lg5ZaR7k0fgD/9+MmDtYQ1fpFhjaeKZ/0ccZWz9VX89SbAPCvlALijCmSmHyZX7ZCvslLW5Geo+L1W9slsTOq3boNxjh7Yx7UuHqfyzsLZkpvRZdWYsC4F7PA3rw5bYJMgPn9Nsat5JdIFHPpxQDWTkzoiMCyOk0vPSM/OkVrxX5FNzF+BdkOf9wjn9jN9ms/fl822E7ZphVaVUOef/rBTqN8D0OyjgZNNcv0pSvULuMKEPhevuE43xhKk7t6g9aJ3c0Bbp7e3V/hNpmOUlKj0qy3VRfzgyDemjbQ41zVM2uh8Di2A9E6KBb37IkbJq+cdBb6xjO3o2+LP89dP9/L5hmPht9qMV1aNW2byd065+vytgzmPRe7cSIm6kM21kwVt/uR706CtZqu6rumaqgne9mMzhkpv3BaU269L55WkHLjSM+zAyGKS3UHnx8IxdzBywk6IFBf3CsUfggjHxRLKPTXit6biDvFC2eEkSuOu1VHjrbvgmMTsH264qw9lVUyM6n3Wt3crCOwPF1uzhfLbfjmgjZtymlUetYdFNfMVOYqcOCFiXncStYgaOHK3jRnoWoQc+MmfpTaQK2VzeLGKLPFW0qbA6aXYhpmpaoGMdFLXLmcNw3LahtFpifMzfNSYYOwadWBNIpKZc2BkuEnPfpvmJVRNogbAiyceshPsthG4e0aMAqP4N/sQFNhc7DQSkMGTTdkMAttYBaq5jA3yR7uLIBWuuZQt4mHbHRlKl2TqL6NlquUNQvq2ujH1sLmYP2ljeynqgYx18AKJ5U1Bxrg5tkCaClrFtT1bDRTjLJBXHLxdwpt4NbKxnGJj0p7yKW6eWw75aIhbh66uB7FEnQhbhLa1oiMUTaJuwfITvqWwuZgw9TGu4KqmsNMvMhGipayBkGRjSkZqmoU007DVQubhU39bQQCS2nLqpvDRlYGisj0QBHFYXX/k2lWRtogMLl/0QIslTUL6lpjda3gImijnWWlzQGnVipYarqC2eq8WOi60J1+NoosK20QGEVbG7BU1hzoPrXTJ6x0zaE+2Zk/eDI+f0AU3TixMYfASpsDfl7HNkprKWsWFHeObXQQWWld4JBuEjSGyQoa339hCLJX2MoGDKFHOQ/xfu4eaiDHwWT3GfmRJ7i/qLMDt7pZdzxvCq7ywus6lEjC92yeLe9LZnyvyNjvC69E4jNS8HvmkgsicUdt6+MyJtBnaVBxtdTSCtJLDWXSpwyjZhGkKRR5WbfKRBVKsUyolQS9/M893uwFhtVNq3WocatiLUi7NqdIsTaPl0Du/jcax+JCea6WVtmSY2FCKebyS+qBIEhl7bLhbOR0FseBtwM+r8HXLChDyVIcixNOlIq2TJ46vFC2SLx5VV9w4zHNy69EG1NHmNb7joxEv2Mo9TVhdCm2nucmCNIN4+o0XR1jZcJwF+shjrMgJi46ZbtZzfN6Km10U4Ff82pWpxnU6SppNVCyiXh5s7peyabfJTkirdTSyW96biV88TIjBxD8QKRnl8fPIcDVwKMroVzvCinm/iJFGEZBmSX7vbyiQwmkGVwzRbYiU2+DqbGVnV1rM+TOIMMzNQQmsDJBmOxdgMKnt0oIjdBTtb4DFZYcyV7OWtdxx1U0FXPYVmGiwZWzMk7zS9fUEJjAtntW0meDuvEkMzKElvs2Y6NKj9M0gx9qlwGPPlSOfzFHeNWqgm63t+cMMbc80rg6TNiyj9cvp9P7bHuIMIZXipmCE9r1LMgmvdd5HC16EpnAE2QrxWw3O7pD145rkXTBb3/60oAOZQf0DBYfk4S1nNFCZAqvFJu5D1Kesp9ncqW0Lp6qZYgySVkFnXyuvA2gTGDmjUdTqhz6lE+VesJNYxVj2i6yCsbSX5umVJm5chXLcPNULboESNoa8fJcBGJmdlkJneylUo8QRTDQ5KlFDr16sQkoXKZpnMtVlKaEuTzQ46lF5q5ilfOxmapZZV+iVFdhqpLdUNHK5NoZ2w4ExYZFI2ANOYOEBtOuqafB6LdEJSpLG69P6uCbrbo0qkS8bCkaKoZyQwOoLXOo8ygwXKMn2VkUoS0xNooL9GKRFbE8UuwGgSrczO8P4j90hmRj3ZaKph4Jk6ddO7By3codpqtY17bq5n6tROZk+uLOBNajEGrb+wgkWvJR6+4moJcUKTHUwfVIygsN5SHKkAc9F9ZyEkwaIbdwKsfveDCRJm/0cSlT5UGJqhHcQHlQgihD2i4Pgf+gXh5w4Bz28uYsXxAFEjHFYZ2e8Ifbm+CnlKmtsnhc6O18gU3MOVTRDaiDKOxaI6FF1hya9sRXFnrN5bsbgNBbu2mVCai4D1c+vs1QanEW3eraiK/WTlcc/kFkw2nD4oPIZtPBfBXcWNvMU519tTg8AuFmHwn1gRirTCg1s8JbahpmJbfR9CUxEhooNBIYiY8Reg3K9DNbhmV7mT0Agk7XmqalHKoNGVVspNph1RCE1oEbhiVWe7vmxBwZNezJOCnqGhTyh9CwJ+HroNecWnY2Ayq2UUIHqZsNlMQh6a7BYoJZ0mQdSMeoYmPRDawGITjB1bAtOJs16RkL2t8QGD6QiJCRQx1C49UiaY8G0WrZJU1WgdSbVTmLUCdR8zZSzl4ZRL1ZlbNHQ2i0bHL2ZJY/Bls2OZN1IJ1GRc6m4BKK8f2DHgbMXmQHq+w9I4prgPXFNuk+IX6NdRzJD165JZV17JURwPNgQG4BEOq1l+nosAnjdONI79Fxxizpnm9mpOMw8hNEvH8JrYWrx6FlyEAUyDUX9vOgz4qVI9tDtUBqhnUgsVwcTxQnL+4OoPUzEHktC6cVXe/ts6DrJAChWGgNWoK00tQugKW8+aRkhDUp6cgRiJzXkqJsCpuhhJ7ARRcqlFRYk7IysAOGMVvK+iWzvh3RfOFsahtqxi3BtsWnbLfl1qF5bYy5V1t5ArcW1sxD3Aa4QGS5XAqRUdVtXbzQAh+jqssXAs8GICOrW0t3YGGBkJXVJ1yev7HDyAjrU54vlnYoGWFNyq2V+rI1Vl+yHYKAXOJmgbKjbfRdbLwRb2qbfRebhm2Lm0nZJxDsoYXXY1N7PhddZdy7UyOEcI38J8Ed1kNCJkaXRdbmjhfVe3B1fBqdt1LVQOGmo4HCiAXcfn1DtZKqip4bkUvklrhZZLocbwu6ljeAncJyQd1dwwRGaxh5Qs5OZOBHjJgZSIcgAlsYkpk2Y+C90uYqZX7vbgIQuXi5zFg/2YkdQZCuoqPWrEZqJ7SN0EyUdtJ7DhUihEcZU8WHmrIaHYC7f1PFp7RlpifkZ6mb7wE3SN8RPvwJlPZtySmQFulNZT/KIIrIQSa0TzO4dtMsRrgp1Oh6MslNep34ozNmRbaYDJb+aaLCs2QkOsVsvlHuSlK6WsqW+MEM6ulIGovisL6VXp9R7l55s9gbCLI90hpRjoOzBix2WI3ijxgx3mE1Bt4rbafvQPb9kqVhw8nOs2QnMtlLYrr8jFmZb2pFdZ/M8A/tzsc+84PB3kfXoe7OR2sXp4/8XpsNghBlsahbz/bOGOI0TmcSSdoPW295rH3XDfVgSXo6TDI5ZbwLv3elX7aG0JQ9Vnnvf70JUcZq1A0gLy1Kd3xtMc1Go5LDNT4ePqQpTVfJHdxUa+FSQ6d6kHllcpWW/mjm8+Wl8mimJfXh7oack/ukqNdbOEhEFSpvkcJOmUwOjqVT4jmlptpwYRAzBB4aXuPQYK2EFcc3ioMI1bFHq1TcwPAGRPK7JHvTeQdB4p4ZKBCUyqkFrY/NjExfmHgZcC454SYixXDkrj0ZAcLFGxuNhmcLhJFYJU0oPCxB0ANkimENN2QdY7Q7Low4rKu7tfGJ9IK3OAUSA5htNU24T9gM8XSrD8YqaUIF/sjFx8JApYruXkCYpnhYSW6m3owcDhLG6urpAiZ74mI/i8eHc+KAHT0DgM8IJAkcuXVXCo9V04Qr3zz6YKySLhSKf8ONkIHUYpU0of62h3sDyVTJ6M5pwDDZGaiPtY4uUAa8RzfvFBrIuLaaJtyKyN0YYmuJaadb7i/UVPvakdPdgwbSYcc8wlSlii4MORdmIAdrHauTJv29SeVRAqug7F2n0YFSRmmrKOPkvRRljDK0svmiNVa2XwVXdy7YqK7KIB0ZZSBSUzUKRhVcHaCom+oEVfiZHfvR/ZZqM9WB/4AA4k2PjyTj2EGoThpSVIdadaQPO7WNp9ECK6cSVauDwEpogTwjP9MEqSSUQR5ACt9IzBC1KerwygicXWZcBJV9Y70I+fhFl6MSUX8TjhzC40JIH7TrGne9PXrSqaVNDQMobgSfTeCUOhpI63T46KEACQ2uDBAOb43jWg8lt7q1Wwp/S+4FVm8pqvCaCONOTQQ5pNycjMGEsUaWNDUMoLgIaNSUro4yUuxrJAsNrGw8GXEmwjWeiDsV6Tf+6KXnGubL4BoAOtY1TWv2rhgBDQjNnhUjoAGh2ZtgBNRfV4VDCI0XVi2gC+EmcTB8v4koSaWi3tXLkLtGD8MHj/j9PEZBGWNHzgbrcTQk1Af6+fFadYo6vHqWxFGaAbIlDQ+RNfKlLaM+1C/PoKrDNCTUWxG610+jGWEUtDFcYk6fpZJRB0qBLgujoD6KD3CHaXTPLH8Uz0qodwAjN9xL7AXrdPyq8JoTCdV5Aw2WrpCRAYM6UFPExIBBl4VqmBowGMApdQwh4ex/Aqn/pJ9tfYJG5omUwVoiGq/N0fMdAu9L+QMcY29u1w8TjXrfr2UKTbtZGtbTmuCqzmeNedIQmuJqK2li6Va/toqZ8YM6TktFfWEwjDOJld42RxVcYwqQHPBQJ6jD6yDoNYGMgDpEkvKOuPE5GhpaKOPnIIVIFM499oCQcYfMho0+klpDCyVDwNNoQhoS6mMFsB69xIk/VKgFdKaD9F9BLRHtuSkdEO3qW06yaUBUAtrjSBOZ0yelDIaIy9sgc3cwkNue2cbqEVKfKkrJRe0P+60+Vq+UFpgX+FoD8aaGFkroj1yDIQRSKmhhYNHFqYGeXL+WAbRH+KJZhJo6ykjfz1+/U59KKkNrmXc9/M7VWbRoiejCaEytsQq6GBJ78AYw9PbhFSKpxu4UVkELQ3NRqyGhD6KfNw0ZE0C6udSQ0epSlLdFFa8TrTnqjpQBMBNEptJo1NuTTArJu3IayTjNHWm9UibAqAeT8pyNG8VumoHMH3YAJsM6qm4CPw3iTMQ1ngx0j6YRVOomyQhirWUCTWt/YFfHBNJmsx72qSfDVAqZgBp19CcDJe/FbxhqzFmfDJO0J75hpAQ8minnpZAJKKSzz7KrY6RJMNQa6KWRt4PeY30fJbkkR2NRo1/M2ItdazalJWTwxW4Cq5YyAqa1Qt0jZApKe4p9WM9Ch8gIZo+m3b6bBeimuqlehwlQqmOs12GCqRQy1uswAVUKmYDSnwQcEDMCp7Oe2tUx1UszgUR1jPXSTDCVQkZ6aSBax6Hmxo9hPVMdSSNommvmzR6gCaLUWDa2x9BG6LqaJsflRhDlXCHruLCTd0WldWx3JC2BjC+aMvEqJy/0MzDigAYgX6Fudlioii5MGi0MwFAVAzCKy+N9RAoL5b1Y9LCvNlOtowtUnCnUB6p0dIH2frDGPaSNhGfDQaiGliaYh/8JAhP1jVXShfIM5B0V0UXZAfzf8rUBHkbJCFQSBy+msEotbbB83khx9qmL1lTThVOc5OlgKczw9AMpDSc6ONKDiX6YOExAdrw8+W4AqaGlDUby3wXr3/Zp5gZwC7wX3k14EqSj4jbQNwiPZJR26grBs/JG8UkHORcvDJnG75E3jp/uyRwYzCxGoceE0WikqcKIehybShrFzPc+GeYsNc2AmqyBxqobc07VBFtTThevPN6sT8YoaULle0X1iSoZbRwDg+i1kUH0emegD01FNFGgpzIf32YpVfRhTNT8SkYb5ze1Gdwu0G/yM7gDSAorOl0cyeWcfhTqZ0IfpxbSRVI7z9MBUr7LrImzNTFU3hoZKu8e1xt9llJFF6ZygqFPxEppYgWhgRcEFdFEoWrGRutdPV1AhX0iHSjJLSIDIOcmSM5NoMAwRi/Ub6oLgiA2MIMwIKqJGuFBUBp7j1BlGbeN2BLTRfPxcOJRwb9BB6sW0kQirs20caiIJgpxdKaNQkV0URSOZ3ZITDjyJt7XFhJXfg3DlDoGgAy0SpWMAZy3ZnDemsApvRvpEzFKhqAMje87cqbwcvdH5vBKOV28OHhZnL42UeYZJV0o5D+BDLrA82Bq4DXX1dMFLN1Z6aMxSppQyMdvh/XijYFFtYaULpaJeQHp7UC9KOkOGNh0UKrowxBPfiZwqI4+0KkRnFMzMOcmega1ji6QiRUCM2sC+dl54O0MvO0aUiaw8kXcdO9nJqZs+xRNQMbxo28q8SotA2CZT4ZiZsBqLU2w2rWmNldDShfLD80wVTqaQPTCeH0iRkgTycxyoaEFwspPgRGgUskMlIE6xyoZgUoNTIezShLbgbWvgsW9Wu3L0TW3omAEx9yOE3o2rlgxB3tysCYitxxqzC7VgEO6RqHpmvojfME8PnIlr8nk0w8asBGN9CWKoxedWf/BCLDSB7A9lXCa3ZpaRJpoZn5EDggmcarXX2PSsl/WLDIezod+5j8ZRm7KmkUmtQK3kGQaQquP0IPdlTaDXuw2NIJaS5lC01tba6CZ2gbbOBiqeoy3h7CjaBg0yuD3LDXdDozL24mChUrGN2EnKoabuHF5M1HAwkhjpo+hrZTMgOnuSmXIzO0+pYdDtRdUGDiDayrVkUdacozwNdQMAfq/G6ohlZIhsAztPTMvz1rKDJq5jr/hHv4TCPaGyGopK6Nei7fFpnC7xZ+VLotNX9IMhk8+fNY5+Lq6+vz56k53+F6ouHf39+JefIq4O3U0nELFaYpplrZCTLyYcagMlH8meqaoSrGDLf68nztl2w983H1xA/DAO5PdFxaHyv3hqFUsupOrOF4vVjl7M5q4Gi72gY3WCTauDgPvNDmchpzyaf9aZbTwyUMJFj+rpannh76vWsfBk+TESxImNcjv751fU4hSJ/TRPkmc5xg9pgnwcAKALMMRT1D85JOJe2cLI4hAFiOnUnKGrYXAjwyZq6VG7EVP4Wmj/KlaK4VGbJH47wyl427U0gccNvfAFEd/hi/SRuF3ECYBrPfX0CLO0HQtCON8XX1QyGFppsrMCNjXMuzlDkQRDGykVJ8NCaT8NW8jtQYNicBdRV7cvujDNBdjQwbJbmI1rIhg3RRvAqspxdiQQbKbUg0rIlj4jyz2YruVkDUiA/WXheXkahuSgltOBbcUh1v1OEsyzbXqd6LEQfpM3qQt14eWyBhTMmlmNzcbViSwLvdpFofTwLG2JBA/wg3YB9k0jA1jI5A/dz0XiRClyCOOw5wU/xqjzPm513FRvy35+A9Y48btBfe57uM/we/68WtIidk0EM+W2Ijdyz9df/0Sb7ftnVYqZoNCx2lpjli/h9+z2yh4KZ/Wj3rJ0Ks8QrLKK8Zld9lRFKJdxdqCorbVkmDUOi/ueqnfNi2a4qSUXOaTuV8/fFEbPxHzxXyw06/GaVROL4qur1KGpzDbJ24CXoIYrJ22nKhltUiP2ObF+zYD98ETfpgkluJYui/bQ/zW8I/jDDgDFiSZzBTGcapuWtGVhq8g2+lOaRhSLt/hhuXaKWVIHueBOaVy3ceQ4u+//+7S3IFrMuF0XOy4MCTPzt1HAUjxw5mDYBIbjkbTjh/L2mDnfatC8BDHGWlLoEixbSjQUZTzEOxhhlV21LePKslH+OTjyK6KyegvpBJffbo7Xd58Xiy/Otd60SwynCSWqsLlzep65VySaXFNlkLp7v52tdSVwuUlT7R1nnauH/mZThxhuEZPBtIKhqRnJi/QWQrbIIgrU0xc9qV+qphaw6qPEEUwUKxCXFnSLSUbfp3Pl5fOh7sb9/Lm7JNuTvtxmiEIQp3sSaGHfyUrjUGxeSuCUZav7lCHFGWfWq9CS5rBLQrE3Rvr1gwUy9b2BFNy1LG2Xu0lSzdWCgc5zlps8jGRhEOL3Cq6guuXmu8O/I4LvB3wtVrGshoX8TaiFECYkAPtfdkSAg/FH+GGvBhwH7U5BnMvb3/5dP3ZXf3pw/L8Dfl0f/Wf9+7q+v9d5R3wfKcNfhb/KCW7+uvq/uqmVP91deWubi//fHW/aqguRDVX118+/Lxyv97d/uV6dX37C/nr/vby9ov7l0VD8bWw4hdG7fJPH3755eqLe/XLh5+/XH1UY6w7KsvT5afzxfLzzZsLCZwPX78SpK9Xd/fXVyvRgEW7+cuvN+7q45/d+z/dXX346H65vfzwxV3d3959+Hzlfr29xrl610z6pQTZ5e3N19tfrn65dy8/3H/4cvsZc16t8GdRjZufrz7ef1mVxeHT9Zdm6fo/QfZvTCtdbtr+g6w+Hvq7l3d//Xp/O2qqcYmSpKm7+/u8OH9Y3SgWvGoDj7u6urzHxa/J97d9nP1b/VDxuSVOXxKfmhs9srX3r/86+qTYi4P02/D/k79Jd8Xdet4JECHAD4o8JvRQKPJQFOM2tPUgaaJvE5rm5MP1Gj9Zf3uy907IOw0LoTzl4/zrkadOyBOh/3u+Dt/IrNwBpn373vfv5PqpaA3QumEf5/firYL96jU2DlA9duIl+6Zh3ImC34/DsymMb1rGN8nT2XHanryxYxoPizMXPPgNgDTeZBsV+5L5jjvDVe+yu/WJBPrip1lltMILSJV169CdCTinNigbBTK1ET5w41A9dpCRCPzokReD4hn6z8pDfpI1CsE/JSj+DXqZQ451bmHkFE+SSZyTYG2/cGz2kUe+dHFnnfybNugytG8PSSwwrEEGjNqXyJgIRPj17eL3whwxj0M/czcIv4bcJPajjAacFiIiZ2k8mMyV/dg+yjJ/4owvZ3FvQJL3BqaPtzfwNn73zr7tZ4AiP9qmJyAIZoh6ZT4/xDUnQALXIMp8r9k1AEE6JQRA0M19EqeKGCF4hPkrA6DwhHS0M4C2MGtzDDzW6Zcdh/ibn+R6Z5oI2W4fPrQg6Hf2jbe7hsch/uYn2kE8Xi/akxZ2IHo7iRiFfH+Mv/9JosPYMVK3Olye+tGhJuoYDxJ/Em2nRvTp8qwYDNkkPzSCKYCEhzGGkfpeYMebKD4uvp0FaOCNnmOxv01XlsrWzm2/co7/I/9m2mSyS6OULt134fF/0O9mShtrRDLpMzhtcnwrNmliMGVsssikyfCI6XhDfjuuf5s2gSYDk0mt8THu8ab8fbZUmxxQqv6Nj1GPN+SB4/yB4+qBiavl9IgyKXgYc1cG0/tAIzQ4qTj85KFGpX8iY+Cx4dlF9UlFTaJ8Wq0zqXaMh1PQS38iv57kf07BUs70uMVnNwRJk+o/qd63H45vQPLTP/3z7a/3X3+9dz9e3/2L80///PXu9t+vLu9/+XBz9S8neWCdpqND7aeeVNE7/g/chd9H+xSujxNA2rROc8aV4OQDN3y7s6NW2Hdxmv3DpY1cXHHMToptJCc+biRS/3RZxHidnRR7Sda52758sehkG+1PmKaW3MjdThJGsPV08dAJSbOTONtBFODoHVrqccP3+igYeX4TwjTFyXgcwGib7X5qL6Dbzi0y4pbJL/b5/80x7RwrNiHkuUUX0toZQPezxklzwAY36HQZbhfLZHm63Jwvltuws81G720k33YFr46Pt95AJ18sOC4OZPR+jJ6/Y51tCKNMq/2S6GNJx/c7/lDJHT/72e64e5jCemtLOwmcuisr5/nI2wcArWECozWMvBe1RY7DiVGEX9brTo9OfIlCrW3tr1s6Wv8AWVM/W74igvUmANs+l0ZTNwhaSS/4gmRjPmG8f3ToKyT/9MOPf/weBkS3cLSPlRcnr3N6SE7J+dEWf/Xr/adj/Nr4YyFQvoKqnWV77ySM13vcfuan7U4u8/2/X4vHvuIa9XOeVtV5r5N87xsOj5USiLKXlYf//YncdURfbmwEEiyQJ/Yqg8kfMH7j80RRWheHvVcwy/K1TIm4OPaodJLYItbeo2CqWd72OUYH+H0VpHk88sRDXrlN1UOFReobumr08FfV2bb+lpB1itUsW0evjlbXN1+/XF9e3//VXd3/+vH6ltmHfPT+6H++HTHnGt9/w198wx048ARxeY29x78A5JMzLin5+j35H/IA+T/cRiQ+fmr9+CX2irl3+sP78o8t9B5jN10/umcnZzib6Nevyj/ISYHbpBuuvxtIf/x78T9Y5Yi6NLDMvTDL/XecKQUC2eGS4jz4rzoT4BrLkSx+VRW1/KQAiSE9RUP2UhfHHmr/1/jpGPlbPwIBfTzaB8Gr6mIT/MXiVa6Q4SYXf3p38e7i9N356zd/fyVlnNngTI6gK9k+Xrx5/e718uzt2YWk9XzXMm3adGK/ePf67dn5xflyKQnQs7lcwfzpm8Xp27dvLl5LWsc5z5wDdNfe2tPBePN6cXaO/3snifEJQUgON16qm8Z2Xy9fyxc/nAL1cRiduB8vlq/PFmdvLt7IZgJIEpe+FHyY6jCcvTl7c/723fJUrxjAcK+XEhfLi3eLt8szSYz8DGh+DMvNt8prQby9uFguca2QT4ruHdsqDRKuj+e4Ssq2B4x9miVaGItT3Ci/fftOuFqUKwmYA5fLwC/eZ7lzZ0WEd6cXb9+9XZwJlwYGoTpN6GIOEMTKzdLFu/O3b1QSga0XXoC7ASlZE5R/P1zgF9TFO1wiFRjgE0mBHYjWQeHkTv79cH7+ZvnuzVK4NgxaV4r88gK3B4vzC+HKwJgvTy9TglTt9Xx2tnx3tnj92giBUhq8Pnu9WOBugnDb3EeQF0M/InvjPOWkuHj39gLXSgUQv9U2uvQn1cbp4u3r5cWbC+HuwghJhkCU5lch6zAt3r57t1ienom/QVkk0perbzzSTiDcmXj9+u074e7EwCKwQlG9eIvf4Gen57KWO4NPedunuAv39t25tOm7+ys8fqBvC7Wacbx8javo24uzdwqtRJH7RqoF7kZenF2QrsN/H/39/wPlP9LH=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA