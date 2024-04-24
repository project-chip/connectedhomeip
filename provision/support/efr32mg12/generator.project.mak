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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA