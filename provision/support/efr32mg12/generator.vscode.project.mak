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
 '-DSILABS_PROVISION_PROTOCOL_V1=1' \
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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o

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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA