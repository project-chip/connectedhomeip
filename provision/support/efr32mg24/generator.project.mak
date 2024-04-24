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
 '-DOTA_ENCRYPTION_ENABLE=1' \
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