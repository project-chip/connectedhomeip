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
 '-DOTA_ENCRYPTION_ENABLE=1' \
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