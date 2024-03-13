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
# SIMPLICITY_STUDIO_METADATA=eJztnQlv40h6979KwwheJG/aoiXbfWVmFz1uT6+z7XHD8kyyu14QZaokccxrebjtWcx3f6t4Fski6xYZvAk205bE+j+/py7WXf88+np785+XF3f27c3N3dGHo3/eH91efvl4d/XLpU3+dH/04f7o/uj3o9d1iPXNz7cXl2sU6Ls/PvveqycYJ24YfH9/tFyc3B+9goETbtxgh774+e7H43f3R3/8w33wXRSHv0InfYWCBMkHP9xADz2xT9Pog2V9+/ZtkbgeeEgWTuhbSWKt02zjhgvohDFEoih0BOP0Ze2gf1G4Uu3+CEm/evXdNvQ2MH4VAB//6ITB1t2Vv+FfXQ9WvyWe7UM/jF/s4qnFHqlnsYt+wo99sH4JvcyHiXULkzCLHfTXzk0tH6QpjK0kBcneQsafXOyzlWRRFMapBbfx6crfrc6sQtWimbFoPOvLz58vb+3buzv7Aj1pCIdihUoTPPmn9gZuQealZiNowBKVKkqA7cQvURqaZaLaoRKh5N3AJ9eBthu4qb1xNo7x/DRikMr4Ywzh7d3N+sIkV9/IUHwlHoRR6vowNh5TVFNULhBFdlm1uDAxCzZoiyeHQT87aAZr26MSQt9zH2xcP6OQD9nOLN+ItaH48x/gJvUMpyrVDouojOlDgfXMFXzfWcVrk/YKBVka7mAw+A5FmdlzHZAiBDt9iaBuJ0r71oApagTf3l2it5sfhQEM0sQUUd/KUGo71VM2ogdeqD2liUii2uKpWBwvdB5Rq8sYGd3YEBp8wj7sQbDxYGwwvnp2+IDMRVPPzhCQGyZpDIFfPmssnw+Y4sYyF1U0U0ysPP+5ATIUOIeJtL7FAUi38waxSzVzkGMWeSHTGATJNoz9CXCptgfB82b9wWKWbm0U7iFzvdQNDgw5bJUK67nBI4zxNwtvY4asY4KKsXDiuueD/jQD0jPCbjDtoPMY2snm0T5bnOFxkBq+9VjkgRRn3Pr33hOf8vcl8XvvibWL2kNh8AU8JK3Heg9e/nh7urr+vDrrPNZXzCOr91Q3u74kKfTtOrpkqvh078YbOwJx+mIVAz5WHXNWFTlWEQcW4ahV+2IVsBaNxmI4kCK7WTQbD2g4XRdaeW4wAa8Cx8s2zBSsLYGT5cl2ebI6c/2zdzIVjYZYKJmtYShGctYBbeD40Vy8qGH46eXaI2bg2y0VJvtDFgN/NvQ1jQh/6syJv6Dh5nf8bDb0JQs3Ox7EnA18BcNPj/SDbTgfBxoefh98gMIlTuxGaSjV3TbjSg+L36PI8+bjRwnDTQ9nVJqhYGmGWYIA5oNf43B7sE1iZz7luabh5t9FqK8yG/6aRoDfnU/0VzBC9DbuT87KhZqI24/9dk6loKYR4H+eE/6zGL27mk8JLln42cGM2nMVDD+9A5y91DyWGf4ah9uDR/iSOEBq3NCICwQPtw8eavvNxoEKRoj++QHMpyVNAgl7gZfnuMGMBidoYPxewXxFyHycaXj4fZjVu9kTfjd7c3o3e6LvZh+43kP4PBsHCB5+HyKwf5jTkB0JxO9FMp+GRsnCz/40n6HqkoWbPXKC+XRvKhh++ng+r7KSRYTdTtxdALxZ+UAycfuSODFInX0ENrNxpY3E7wmc2zuhRcTvx4zGHRPBcccEdTK2UkvrzODXOCIezGkSqsHh9mBeLWvxdnU2r4Z1Jt6yntfgu/jY+9MGzKcEVDDc9N82cot7jdBXMCx6v1yaOzU4ycFawNNd8zMtOg2Ha/0O5cveV/0vupvcfD8MxtaJuYHTWx/WXh0L0tB3DZa7gtFCIFbLmjVK5fiJm9hhsrKL8Ifio9kdJXWJIPA5tZHlRzuGOxdlC4OvwxY1k2E8rmGQGRwLaMdvZWucCCQJNPkq6+TJ2hojTx42J/LlPxtZSDODHcI2VWPNGqu4evVQEjPqofE8bGyNZukdwmOWo97CzIFcexhWwhojf+AJGM+T2owhw0XaE8sjaRh6zh64wfiK5n5WGtrebdzlmrhyvrFrsV72HK/qwW3rOE8eoCZq3CtLf898r6VDT4eJQHkId45jRzF0i+bc4Un79pk5R7iZ+BCGqRcC9M1YU7G9N3mstm4/aa6UNdydjdNWn2C0qmFF0MX1+mo9FjcX+QkUY6V3aPV9K+KK1wvep+lKbm3ki7jcHwtD1/2UvunRklE8Xh7rMQEoYXmUM/0N79FJ0dv6oJBtsxwxiYr5BLFYWh3nw9vpHf/09LB8pNVRPj/KbBD7Twa3qFD4WlbFa+Re9YDPx1hpq0FQA9V0bOTAnexUmh1NrjCxU9d5PCweYVS83TWw8a3rFR7owTaMve1Ix8rdYW2z+hsGqAfB2mhIbN0fz748zVfKOQDG8knpm0XYrNpcdAxWS7F79s7k6EPbVRhnyEzOPbCrYAw7eDI5RMnJXUGIVzE8Xdbe0U6osjVW19A8LruwAxyCmWwO9A2GcFabGr2C0NDyqM6iUB9UqZTMx05lqYoa0jIrLeuzN+LU4Fgci7SybmYYqI4P41Vi7V9ZH5KWhVJiMtLKuoaiVMz1qRekcs4Ql3TzObQwRozOkqZZSVg+/gjjAHoTsTbGzRQmMkqM59LSw2o2pW1aLDWmYW2M6yhQ9SGNGgpVrXWAfFrbqvNqyzozIZvDKffAq1ZbTYrdARH0oDi0YHoHag5B/hnkmzYHg9/tBDxAZdBFpyEYqqCbsnVIN6t6r2VdIF0mgO2a1z9iBP1N/DQ2XkRZkiSe6NAJTc7F5U6QCxtqexreafhQa0X3yXOxTccCtpXHQdfoaE7HDx+UjIvI3sEAxiZHimhxRhhlExqtq2l01Jp5gMzeesjYwfkas2xKPFp7WMDKooFRv87597g6sovzD801RQgX8Xt8HEEk1xyUuGWWM9ccFLCyKJ5rPPdhPNegB3ITF9enp/kMLziAY8ioRTNsooXRjQGxpYSoj2q4N5IT5klN2hpZhYcew+eiHQaotMTACWN4KJ7S1DgQPFj8QJ74yU/+OQxPZWocCO8wPgxPaWkcB29dPQxOaWkcJzlUZk44snIxUHYooNrYOJThAY4WE3UQg2OianyrSl3XGmz9YSfy/qA/dAxnryLduwa38reIKlPsF82BeGjT+BScfNEhMNqRbVMRBhlbdw4XX21r43FmevdLK764Nr/k5cHgWp1OyaMsx+k3YMzuxWnnKZ6tOOi5GPjbLDhUDUVYG8cyvoq3hTW4cpfaBj1UAsa04T0K0CGGkDpgwwNItEbyYbioS7SojeTD8NBP4qQ1kg/DQz3wh4Jz2Hdf2yC7BX8YKuoxN5QW/GFoEo6KwPRxIm2ggcNCBroVB4MqjfFAHbjS7Bvl6fscho069SaxLttBqOnL6DAYceEbY/VEcwtged0N6y6YzuW5A1FLXsoEHAd6MM5v6DM58VnES+u2O6vvX3P/4TAh65gJImjoB24UwwSaPPlMxbcOIIdr+PKn+aYZjY7rUA2uOZj+CgNEFIfRi70H8eYbMDk0yh0b5foQGhnrUJc4Do2uZRHwoGZhZskq+EzA20AsepjfmTMT8hqGQV0Fhs4GFTf0X4NzeEIe0ME4vUn2YGZulETsEtBcPT4TD3pMnC+WmTnRhZJ7k9BW7QzXYtO/TquTEBogRuqhWsMGJhccCaETNAxux/FnxE3QsLh94MwJnMBhv+98EIAd9OfRIs6zepeJ5cMeLGcU+yQOm3x1/mZe7AQQm/58uZoXPQHEoN/Nqq7Zcdc10Pk1Ao9wRuwdIgZ/uo8h2LjBbkYe9JiEWnjz8KHHJNbCm4sTXSi+lHgCXmbyZhOZlGiYZA8RpQ3/YeVN7D719lnJjZcU0Z3GwW6CNn7jS6t5X9FwZuBCwfgWTDEfOlBirpST1HNzpsHicAc14srpY3sDIxhsYOAYPWFMxKkROD7Xyhh5yFwvdYP8aqzZeDbAJuQYCkS2wWfmWp9O2DlUT7pPxYD0DJ1r0wk5B1BDZm4uVUxCjqA+7Nz8KJGE3MhvVEmzSSYiRp1pgQm55LjR3uwWWxl/Gio+Z9IYBAlCQdozdWyUUNrJvdEdPuouVnzSDs6qBhzhk3ZwTjXjMJ64ezN9L48j8rkZRuAf2SxfbVQyIadmmmyDdDIpNrM3wxCcjGuzqi7paHxjQG6a2Pn+8hk40wPSO1HZegiZojzUiSM3QM4G+FiSOEtSuLGTNIxRB2qCITMisvBoGfpojdH1E5+Sq+ftHouQ6WKx9mYWvtQolEJJz9NcY12T+zaIJTZ0t4V5n2qKoehxh0gwheG7Wbg1Aic+VjIXjzpMMj3ymbnSUKkMz83MqT6dysDqDJ0TW3rRGx+bmUc+19WP7dYn3oqHE3c+tR0NTMil9CWaUeXdhRIfKphfEg3SiTs3r8Sikumb0a7WeumZzrYbvYN3A2vL9YGQLRZ2J7Z53s5S1+gZvaIO1EBCXkw0ajLmh8BYCRms3LsxK1cIJiFfJpkAGHOEe8SfDDTxqOOYPxJDjt3gexBsPHeCRTksv0gwIa+mmwcdc0lsEpQMifEQ3OwyX4eLr/VRBQ4Ts+cPc3lDJRL3o7wANk6N3rAl61OXzuRGFqIdMl08VEcwt1iEStwGbt1ggobxiA8kk5Av0zTxxzzh74z1moVzcqMCEqsvJhu17boySCWUKvVcQ772b2qfRsnE0qkKPrlPg1Ri/aM9ArCLzfKTuzQGJubVDP2R9GSaYfVRT/jH1Gl9xTm5QjCJ91/n5EgFpNJ/nZM/ElMcg93EuflFgkn2X+fkUotKuv86J486XLIDzcLnNEUow+OqFXgKB5Y3IsYvgmpM5V1MqmWth/b2LRjLNoRv+bITmuWRc8J6zxs/uo8FPHCUHyuTdj+20wt3dYgE6/xKgI9l6G0MIR6EGD92LB+sYE3QrPM4ZI1HNNeP6y4aOD5aT1S+FVeeVxdxtwDkhl2GrnfvuFrZN3LHO8vb6j71LgTjJVE/jirrUH8/S5i6xtA241jcesjKzHgzL3jw6GncevLzxcXA8svWYx9vr4vbNn66+yv1+V6IIAzs/AVKg6AkHUY+YKEq4tGqIspC8WCRTloNv1Wh0RY6Uv0Aif5T9/W5UtJxe+MDJ9a/w16fPzWfUPrM15+Sju7N4CrVsQWswytbW0X2GvrXIKCdttmJwz0EkX02YRYvSa0GhHs9L/fUBM87EqShb+C0WWZElHhWY591xFMcItFAf/ePG5UkYMBuYBRDB+Bl5vkMhWtmrRQ3+jAP67CbJ7zsaIc8jybE71IwoH9EMrd3N+vpgEkCBqznJvo7RtyglXXWiZAwScAO2g/ZdmtgQJYbt8/BAo8yO4rDNDQz4ccP3uPgAP8Wgwh1ZSfGJikY0NXbbTpgkoAFG4e/ospwwtglCRiw/8hgNmG01uaZpxn60X7C+qGxzwJNgfNo5+3uCTNAl4IBvcaPX0/M3IFgxnMMgT/5e6OHwTojDyT6r8blpq2ssyDx/QgT5oTGPvcoHq09f/A+GGm5Rz7Y+D04Ztf6KGrejjs4YmV1FK14dxycrTY7CteuGA4O2TM/CovrhQkyYm12HK6oDQ5PV9vtzEhxzEn2Bk3K0yTHJ2M89yEG8QtlBJscVzFw1F4vLkpcqySyBi4raIMlwRKFNnDNKxuPNM2E/Ba76USQtelRyAeQwDcHGEPsEjZ2R/EMbf5h4g3v8KHg5b3LqRhr4+OtAGcCwNIoC8x2svhpipqmbZsT0w7gtylRK/ujuL7+w3iYiD71lJ12ZePuguwAc3K9yqa2y4FXX1A/BWNtnAfUDydI5rZtTkw7BhMUmr79UdzQnSA6S6OjYJGB2+OZYBHtFvku2KOTnE+AVpllVNmbRP/VTxw1dWmWEXNTRBsH1kStasIwA3CiFjVhmAE4UWuQMDxeLAxtQmcXjOGd5jRAOwo9/acX8FLW1seb/2lsb+IH/Rus2W1/wvIo4h7f4jQNY8v0+ABWfmHQ4Qkbu+PJHAbIGj65xPUnqHp65seHsKpbaA4P2jI9XkuWa54nqCYJy1yIZk5R4eYcPDOlDZuAqTgJy+OjYR5qjRs5row9GkaaHu+xBLafHWDBcq+nUtvlGKwzt4GVc9RudK/qYI/68LBt47w96qk4S9siPeoJUSv7Argo2zyBxH2aLivQQLjHdw8O3THOaKIYOQCXo20ydMLtcAvKdv1ogrqLziCCPVm1O8zBHLSuz0owcU8i17B1l4ADeapqomudv596eNSO9fGFFX6YHmCNTZexNsuYq8BnRh6errHLwpum+icMjwNGianzz9mMLdtMTDMn0HNRDp4434PEfdhDLPOjUTa2mZhpDJwJqsiW6fE+Kdh4oXOAFYi9LmljmDVUO90rvWOca0x5CkiuKqgaHJ8AsDbMNT4yZYLTEEahYxQCtZnsPfQOs4Gii0wBGB/GTTx7Ax+y3XTIVAQmtOO5kwxMtW0zMX03OcAmRxpkZZmJiAIuTydswdMZOLGNHHQngDx0nF0L9/n85P3hh3krq0w020FtmCkmaDvGeUAnGC4nLfMgHmA9/AAie0188WAywepM0jITcaIFAy3TfJDTpXfLPC/sVCnfMs9s9jnxS5SG1Wt2kvmxHgIn9JS0InFr5PJHkZgduuJxMDNMtKqcisALXd5FVO1ktgNkFB9xp/94OBE/Rql4XUu8MCWvjJrSIQoLtxvlZYyT4jcMvNiT7A3o2+fF3W43+m8eEeGtAHiBjVyVIgI8dDXKELCJm6VFeAfujx7CjcDjtGWuAuAFjqdYl9y3z12tTVyjseO2PGm8fLOaOTmcPUlLhxBqRE0yKtkBEGxETYk8fI75IPQkK5EoACLAk03vDXNINlondYHCot72npFDbSqR1t+UTpT2hVp/U/JWAEKtvymBhy4WGAKebvB+AIIbfIo1Ln37Iq3sKXEHLjUebWVPyVsBcLeyQbAJ/YkWGw5ziHQSJsXmWBvVbqVPSZsIZY3uGM+k5H0W0fGmSfEbBubRNb1DaIaO9+U60KYXM+AQZ2NWUVEfLll+BlwHYoLYPWCp7jGW1lmQSbCcELK0zgF54EVHNNLB5Uf0Y3YmY23ss0CLozKmA63ts0Az19ugduL2AGfiD8K2GBjADvrH86Ys/yQBC9aZMA+UxlmIe4D+tzqZkJMg4IKNQk//7XmCuBUDExiau1VSABmKXCF56CHJHi7vvYrOQTt/PcyBrl9/T1kE0uPV4nlC1BYDExjnExtsfs2S1PbgDjgv1XKGCT0YhZJxaRuj/uRB98xwOUViCbmFuxN54EJoLm5RsITdSjI8MgvTGbpGQRNyL0kOOK4y7k6JIoSfr2idCX/FwufAHEo+dzEnTi6ZkrmNwcKuDteZjpggYMDmux2mI63NMzEnHJzZcA3ObPYT9nlK4wxE6Bxy9qvLWFlnQ05ZQ9XmmZi/HnZOow/669CcRn9X/JSY1AnagWPlpsPkvgj+wLuQe6BDO5E7mLsph2B2XEMw+8fNdjrGyjoLsj6GbjpSEoGB6/kTvihL4wzE8unJR4f6HCzwA66w68FSF9f1AM+nJDznQYR+GL+Ut33YwPPCCUeyBmAYLgSo05uEziM85IKRLnoHgoXsom7i4wFP/OrhNgAMVHyi9WSYpXEGIj7bejLE0jgL8YAHZ/QIeS7uwodwLw9wI/0wZGWfA3TCWrU2z4H5dlrMtzyY1fmp05ESBJywE48/9TB4sfODVafHrjBY2KH3sjw9mbKsEQQs2Nh9Aim0gYN4JmwG9DlY4NWBu9MhEwQM2NhFb7vN8s2Ek/QtBBbulONWAwsuu6eV7MGEi7Mq62xIfK76lJilfTbo6aSYp3yQ51O2tBr7LNApZwT55gDzE5eAs5+wNdBC4MHNF5UkmZtOOYVCI+GBD8NHd+rIrhk4gFMXd8mnBW4YGMDNhQuT8bYQWLiuPy1rbZ8B+oRyuXvIDZddUgKAgTrtMgbOhQv1qVeTglYEfLAT1gEkARdsMuF0FknQ27gydIV6a3sH6k+wtnZMtYgPoVn8a/XKffTFmh6Q4c2tAXjw4ATjoQ34EI+QM+Wqn0f4guy5sZ2+RFM0RvpeDYLJuJe8BGHwMsX826BjJNKMN0RgfrHNEIWzOEzqBvhggShMpmnhEnFPxxFzJYpRKyN1n2biShtHzBVcqlA9jQeoJmkNUdzpI/G5VKwjn9SFBoEXeZrZ9RYy78aL1oEUhz6uhELeIxF0IEjhc5rMpV4ax5JzbUaFm40m5+JMquJxLD7XUMB4grFpwouagA94qv0OBDH/vobygInJpjYJaIHZzfrogzKHTcrdouAEd3+buGTWBJzAaZw50zYiGgQ+5Ok7aoI9sifgZRMTNwhc4xmUL2lftcc8Erjbwbgz7NF95iVJof/kwm+sAzLWl58/X96yBlGKp+zbuzvz5zkW/lmNC1Zh3WpDMPJE8bD5zMCg5ci9hFtT01YQ2jJv/4tOPnU9FzVubMzdyoP959AT+UmC4xm/XNVZHIrTLSTdORsPPZev+zSSp0nfLMIBq81otTBGz+9pnjSSUcSBe1lFIk+0PrY/kL6DKFo4USTudxSHTy6eEUHuBjAGaRhbtZjVN+QDN9BnqVGjmAqe/FOZvEczVGlRzGBvJTLMUMTtqUY+opD5CZJh8Gco1XiHz8CPPNgsHquyaQPSN8Ik+br+KJeawji1JQrT1yrMxR4EAZSazeIAopnhoMnfqIbiaNDWGNdl4ISSFymKIBFmeGiMR1HL0BjRdVETm44fwgwPjfH4aRkaI0J/pKETGi9mpB0enl+W5iOpa4uLa3VArhWbay1/OqEI0rp7AOEozWf8FpQ7tlkCirDGE1PGk69liIPoIkvS0D8YF2mOg+4T3ILMSw+G17JH4ftB+oy/JHbw2ZpWkkVRGKfWD53T+mhmpLweMDTo0QtqG92Ff4JSJ0D1jLXUxs3p8a6jRzF58aerr1/C3U5yXVrXoldIWR1ZiuE7+JzeBN5L9ZQWhyvzVHEKxDrP8hfSE43Yfrf8dDVZZqUdHzU85LFydHetsqIY54SLfFzx68cv0v0YbLkYnbToggP1xOm7ojkqm7gJTLPIjsCLF4KN1VVkGZV2dcQs6W0xPZ3FeeetIihraWQYpQ/0Wt/ggZOLfBrefXA9N33BQwVOtgCxv4Db+HS18Hers+JP/Bc4WZ5slyerM9c/e4fCp2HoOXvgBl0R9Fuyeex+66CXWZFNFujXBfLPQf/Fx259OFucLU4W9vHy3enZu9XZ+dl5Pcz1nY/89D5sYOLEboRd+8N3Vv+7YqSlFQXou+/w++tX6KTo76PXR+ur669fri6u7v5ir+9+/nR1Y1/ffPr5y+X66MPR3/55fxRDP3yCm/ujD1vgJfB1sYoodYPL53xYPEG//O3vzdfrItXwt9VxVNaPMYS3dzfrpsi/rn9rRiRt/HP7RxChNI5DFB2pC8ld/M0j0EfVW3H3SHGrM+0hPGqCfs4TmfpA6xqT/s/5ldFPrgNtN3BTe+NsHJ7noJ8NPdY/mID6c6k29FQxYDfwa+JBGOH9hDHxBE4plFMyD6IUuj/6rswLH66v8y9fPftekHwov/3+/v7+aJ+m0QfL+vbtW5VTUaa1kgQ3fvBDC5gP16InX5VZIA+WxlnxpbvJP2fOorC7yIvuInPK3FCPOi12jpOHiDZ+S+IP93lOzudhcGMrQaUUVweFocX/xf+18EN11q7c+UMeJSUWchgr/v5aNVfjdZKI2rq9u7RxgQ4DGBRb9F83P3pu8AhjXKctvE3rF5Q0KF97rpOXyHz2qxM235FQ6troMYDe2v1HyJzmeKHzmOCBwfYz8AlL7EGw8YqrJMd+7llwwySNIfDLJyj6vSeGNXJONz9mzaE86HaKhF3+xH4wjUGQ5LtQh4Pg8t0s3WPo55VB76EZlZw76ONGBvwfWHb6Q7uvB4dyX9MaC90v9/VXVS/lNdnPet3vM7zu9lpeD7XBXvda/6877cPX9DFfytf1gOjroSFH2g8drdbIGO2HzvPkwBXt+2rwiPrbqv9ba3SA9sOe+jXZa6f83Oo1U34nxkhe07oG3S+L56g9nNfEDNPresoE/VXPkdl5u6tpxF9cr6/W1gWeH7sqFwI4fuIWK+zcptoUF0BFVjossftHNHx+S5t/eioR1o8yGzWEn95JhE1/q5a38QXG7cVVx+swWckEDpN8v5xI0KLOwiHxHDoO7IwH/pS/jHEPE8+tfsH9vssfb09X159XZzVI3WuwgeMzc56oZP1S1aX4gDoNvn7NlJntBTUdP9OsiBv4uiXR4+XGB52qPrCrLl/IrIpExSPP0ywJtScVzBIQM+sUQdFtEju6k2oXObHuTLWLXP2YbmjjoULNuvut/ijdb591S7or3WnkAu11iVsfDqBR9BG+JA5gNikEVT1UQxmQfH4Aumu7ShaPE7mB9pepB/OhGN2qBkqVp79U+cD1HsJn3aoR2D/ob6L4ie7y6j/pbuxFTqC7go5i3VkeKdqJuwuAp1sZNXhA6uyj4jpyncLQTFZNtLd7UL/E2e70i+pvnpuo9zITFZ+JluTTBuiOz2+bUHO6tyax9Ej75fC0FrViHbtd8yqJll37JEWJnUWEqEoPvxLtgjI0H8IwxROYMO5MdXU+snSKTax4p0o+u5AkkJ2RO2HS0HeZObUdph6UITbRCoSWCAPLix/5Q+Bh5Yz5+mmFcQnH4HNqpyB5tGO4c1HaMuuxUimfoq5Tgi/1yjC4Ygeex5vmRahxZj6leuq6irud46A8CMv9T5ye91SKSUq10Lkr3JmlESmjtERgRAP0N/ETmRUgMcHACIUnmPMw+fp88SDV5LRM0F1reFwo6B4wB3gGgtlbDyR7mcB4plIgHF68hP4/D3pxfXqaD5sD7uA4B3Rm/3H62sX0LF+WaAs1zksEzp1nhsM+5xnQ56zLW0G4hlg7IZg1ZOt5Z+8yOzTtAOzmb/d54jAM/mBcr5ROkJijiHcC8Bc5IiDH0GfreZ6xvVYAjn5r93mZSOboJ7aeRx30bRaIoSViacLXF+oGSSGzLUEJIpX2XF2gVgjOCb4mDG4DiJQ0XCXVlQVXjVSGwBaEnsclTCQAFDSQFxSRALigiDyPc7zI84mYv2VWFAlS5CdGCNR3yE9jBV6eQZqPnN18VnjOyoOQyVfH9jAYbiTQyWIX72YlVkWhb+t9qlVLv/6GRSSuaINmhYoB9e4d1rrl6zundQubIuYcgpdQJu6A06y852gPS8i6AV6thUoLnp8xoN8/ScmAgXx1YLOESqd8PsCcZuxWnIR2fTCJZt3qagQNsi6pu4V5TOhCdmn5UFm7GuZoqmvVF0BPsVVd61cv61P9wlXlpF95z9FflZDtVB5mDNSVh375pvLQr50Xcj2yrYIYJtoKYU+3HEWMU46VdgM2mr0L1ZazekdCs3IbX2XjwVjllcNlJ/QDN0IVImTPlsqYwQvFjbiC618AUTyxRwSFNB3H16+Jby7VLVpefKxdd2cgAuoy5OvMZrnyHiz14+Y335iQPV+u9MvWX+uWbbZ8mVBuTojTqOyagq6v6NCafvlgRX6VtdxrkK5YfZdf4m7nF86bkEf5WasuqiUAeyJXTNKrWoyojRRvvgHpdsygflX2NMsS2VivsqtDGksUJ5UWA18JaG5k1yVYd+vTOEtSuLGT1g4aLTZwdJi2U0V57zRtA+KK3VuqfH0oLSqd7NVkcspK428MbS3DNxw2NLRwRm347GVSgsJhBP6RQRvPPeGo0Z1rSnmVIaIh6XxDa4Rq8yA1hk/aMOFDOX+F8mYEgw0MHFePCeIF4qaJXR2HqEeWVpWZEceboXVLp7H0GAlVs1M1GlEmLu2Qb2SMWlAarGNoaxn+4rBBVL5GbODK14iw6vjakHxZ+ZrLnm0DRvJQ24T+JCj1jeZR8i1iLjEoVoykCMWOwri5gBX9iU8aMZoDqO95pok4X0pNHGBSDZ1QzjRhNhq4xHh29HJKcSzk4lQKntjriGhSZSOodyRRspKJeooaPrhIm1juJadUdVpMFVvVZ95YGgpvxylHx6WjUfpSM4j60AmfM3BqNGc31dPH9Te8fvQ0XDWRaoqr0RB3pqeRL6flWoTGrca7rIhTTs1VtyPHHe/5orA68Yulh3mvQUXgEcbNCTW8Es2+hJqBNzZoAiUDXUL+RgV9YvTY6d2xsY0hxBOlxdkk9V6l6msbvTrDoagWFhuej2VJlTuTmgNURuKKKlOkl+XEIXomGGrJM8MXR43tkEo01EBgapQ7UerzBIWjpKOzxsctXgPk2lAzg1tqdP8Ut0odyYo6qDUWQwfg8d58KZ07NoTErdpKQkUtz02GqjJuDR8mCSq76PWx3Q6+0fjVogzvuEvDseEwIbVvMYgiOLhcnlsLz2/gS5mVdeLwV5QdlHlQtzJThkmgH+2VEy0/MBX3nzQU4bKVpic34Q14yhq40SDtVl7CJCvaKsdZny8urI+318XOq5/u/moF6FWadxGLbGncAEiG2t16bUhGMreNPIsqW7mG/jUIdtYegsge2l3MlCvKr2TgdimRFMGlQ7oVUBYLztDdW/yqz8PHpfFLxO7Q2Tb8Gkmw1KHxLXZT7jp5SOiBPDZTXsXdDW/T5ldp3RWvouQAH3qeelI5jrJTzh6g/61O9OhEoTe0kUFACbK2nvBrjc1z86sMTwXza+Q7k45Xi6FzXASUinOAN79mSVreFzy+AktNehuHPh5q1SuOx25z5ebCY73i+ZH1CUyNGEgS7n40n+Dz+cl7TYp6kgofip3mI03KYmlsb+IH5bKcn6WvLqL8jt3slSvdfImguoZ6QpeLmtVlhjbf80uM7oLjlxlZFcctslN/re4fN1tlDbyMXUfJ8XzlTE+9HEJebWhZm4DCubJEMdpadBtsoYHWIckAvW6S0HmEqXKEB6ivbD9+U5UJXeWojgYPJeCX4B54GFZwkiX3gPOYinK+wSJvlUXKiQldOlre1LXYyG5QfjHUFVienqhHd75MAOa7mhLlYoVbhjq8i11ULDbLN8r9plj9JY735GjQWJ2/0aAydNSUiMa5eknX0EBP8HKTkVONxYTyjmiSual6AzTXC8NHVwsZvkoAck98DCnV23WUhVxfg8r4WTXcMjp6ZVgDdc+Vs2Opo5xShU4i3UjG2zjUR8KwitqYRaNQ9aHxzUg2DPCwtHSjp69a9qrxkrUIuOMnMMjLJy9BGLzIt9MJ4eZSKT8KE5UKhyLKtblDRhi95n03dZ+00I4eAyQmpNLdIYR4NiNJ6eX31yR6E7srrj3Ruwa0Jj5SjKXbVISO2rghIaTa1iek6nXsOrTc37TEd5LGmaOjsOmqWUc3Og8Kee5DDOIXa3hLLDuo5+6CkQ1pbIEkWKJfBk9r4xIo5t0kBcr5NqXQrIN52DLFdJ0sRB567JA/IQklH2w/HNp6IKKgzGDH4Js6R66ihQVlkSeQoIpMWi6w/Uw6g5VTjCrv+FpqbNsHZ+h8WZYOCVkfpFolTWhyskjWC1LEdv1IPm3bSopVUT15JelXMUsjHVg6V6CgtpPFT/Ivs0bBDqB05YFV6g3/wyd5cOqo1RnjZ5nxBrej0BtdK86tIelGMy0kCeH5ofj8RBXYl36f+RuZqWkitEoV50eJZOeLVJCaXSQF8CCZeOOdVEB9m8ErTJgCeDZIMvUisBk5Jp8dfPA4YXbQR/mQyvU/klB5OVfBpa3j+Spp20pdiehRqSNRBZf1vOozy5pvTWXJirAO7hFQGNuFLCwjG6mNjFqztS+kgchzh08TEBJS6LV1ZBRfGYRaOeIluv+CX9EOwvxmocGNrXq01bkVGrUdFXWW7XbwqDZhGXWakZ39wjLqNKqtbVJKvrlJqAwfSSCqos6SL5pTh5FZe0eRiUGwCX2lXjOpJt9h7aiosyg0X1oiGki8MGWfeqOiqIGxPEhPA9vokXxcSqoNL/yQcvM9xsche6m9h57Ka7dcHSPpTb5eo2jdyNrPz694yHbKjuA95e7grUxc4dHn5any66FRGrlFgqnSLDuRTJt8hYdCWFRgIJDvJdWLQ9TCS1dH9WIQhfBK3cRaQNWLUoTflQTu8kMW8iMgnlz4zVpffv58eVv+w5sjGTL27d2dJqJcqqDygRssnCjKP1QHDv4d/xBuMg/eH324P/oO762GTvrh+jr/8tWz7wXJh/Lb7+/v74/2aRp9sKxv374timMoFqi/g0qm9bV4aAFxJwg/+apcppEHS+Os+NLd5J8zZ1HYXSQwzaJFfUbTOv+Y304G0AtlsXOcPFy08VtCf7i/D169+i6fysXLtpJXEb4uIy7MLf4v/q+FH/rO6jj1hzwGSjjkNlb8/fU/749i6IdPcIO+2gIvgc1Dl8/5fHGCfvnb35uvizMp6m9nEY8Xeeez1PgaJukPeNnV/8anbHyW922iXIlC7JL/jUjZiPwfnyXvj/BaQURtLZzYqWaA0Z8zqkUzpwp/8Nj9+9Hro/XV9dcvVxdXd3+x13c/f7q6sb9+ul4ffTj67o8oCu7vX5VLbL+/P1ouTu6P0DcwcELcFkNf/Xz34/G7+6M/IpvIaGkTPRIAH35P8/WiSIK2q1i0vHH8Ze2gf1HQSusol0YPoP99tw29DYwb/SJBW89UT7ooYevnkvo252YRAPq1pIAoYYooxl/m1/Pib/LcVsTnqHjz1raxd1q1O/cHa2dvjYNqV6cdFGjATHUm04URD4hD1Aywgyiyy6zvwuQAiQDz+421W8nvCC3G0osOrBlPyh6BYfUyvtSMfGcVtRX5VbmI8StI9+hjFrvYbppt3PBDVWFbVZ1WaNUVef7plZlK+Q76eB4N6qqWy5cuV72MCoDnOvmC43xhaBnd9Ru0iex+CjDT9PbuEtXJZS8l0SpNnlFrI37ghTvdBihH0jp6LRTnquXX3BWTe+bENZPXR5sW+toTt6dvij9PXTdfy+Zod8Pt1Jh2WTRNm8kPgc7n5U0ZzFsuZn3CJppNNsZM5S+RGH+38DZyurLvmrqiPtjLZnTMSPqF0xlyo754OkGqhSOUZwdCfMpr48HnB0Kti0Ngv6D+IiPogMDlj7enq+vPqzOu4EMUeZ9YQIFeKIpjdOE2Pl35u9VZWSe28xZKheY43iLOLCISrNqf6nxYmmq/0IiBpiBG2Us7KU2WH5VSSgWTsTycVzUda35wsjzZLk9WZ65/9q6s1GTjqDo4eFhcMVFrYRs4fqSbthbVR1m/zTVCFpraGB+yuDjhXitlraqTM3VMcBaq2jid4oIGrZSlpjbG6j4KrZCVqD5KZL/cd64XtNHVx+oDpJs4sRulYawfuSevjzwq9ifo5S1FtVFCA6UKai5VMEtAcWiDXsxaVhvpNokd/eWqVtXGuYucWH9FVatq5HT1R2clqpXSzs/NNoFaK2vj3W9N5NJaVSPnswnMZ72U7kp/SSo19TECA+2SSlQfZX1UlF7OWlYbKV5v54BAOyqhq43VQ20c7aCVqFbK5wegv4VHCmunLe5hMdAZpRnQRw/ry8L0Qje6+liNvKs87e8qz8S7ytP9rvKB6z2Ez9pBCV19rBHYP5gYQiGF9dEm+l+wpaY+xif9Q3ulpjbGyAn0N6MrUX2Usf4qv9TUyVjc1OwZYSW1tTEnTgxSZx8V+3O1Irel9RFDU3VqS1kfr4HxnkTzeE+CGsHbnX7MWlYnqYnB80ZWG6mZFp/+9l5mpsGX6W/xmRmU1D8m+bQB+nNoJaqN8tsm1F/iK1FVSr9cLqYLkNRTnY7vzvDrQaTJGpmN53qU8RDr5/7SdrwpSHS1iRs4nKtMeusw61tkx1Om4Koud25C8UQ8ZQlldTcwcQKHiH1aeCkSl5CCz6mNrwm0Y7hzURbjeCW1qJhacnEFy6vtROKnCiNnESQJ5Hl9dPJEHUoyT8jlBLX0z889yTg6IW2rTahxq3w1SLc0J7FkaR7Pgcy1UKWP1TXyDC2lvCXGQoSSTGU8dO15iahdMpyJlE7D0HP2wGVV+IoZZShais1R3JFS01bR04TnShaBN6/sC27c0zz/CtQxjcNlue/JCLQ7hmJfEUaVYuc4dhTDctmwPE1fR1ue0NzEegjD1AvxQY2izaz2ri2ZOrqtwC55DavVDmr1lZQqKNFIvLheX61F4+8Cb5SVqunEV8B2Ir54meFl6K7H07LL/bMwcN3v6EtIl7tCirjFRhKGUJBmSX+rLmqQAmkHV4wRVIkoxEYZWp4hPxLQPz2VYyBDSzP4UWaD2H/iWJZNYWiFPlT9O1Bk8dbc1aSlHTVdeWMxh+1kpzK4dFKGSX75lhwCEdh020p4k0jfTzwkg2mZ7zPS1XJ3RTv4XBsNqP8hsw2I2MopVxRUG76UvaTM/Fj6ahFhq1YeXU6l/dk9KUAbnvjSdOYOem1swquQx9GCJ54hPE62Ssx0taPaee0dMYGqa2b9Q4uDsjM7oKcx++gkbOS0ZiJdeJXYxG2Qarf1NMMrlXX+WK1CVFFKKqikc73rPE45xt5YNJXK3Ad96tjjrhprj8t6kVTQFv/KNJXKxIWrmIebpmiVc4C4ruHPz0UgYmyXlFBJ3lLqEcYB9BR5GpG5Fy8yArnzdOlzNY/SltCXBmo8jcjURaw+hGqiYlbbF8jVdZg6Z7dUlBK5OZRrD7xqLZIWtI6gRspiu7E+yFpPI6PGNG7rKTC6HVGBQt3Fo0nNvnptSo2M41WN1lLRlBoKQF2ZuY73QH8TP4mO9nAt3jGRXaAT8szd5U6RSxnqcBO/5/B5lxNEG3nMJm/s4TB53HUDS5et/IBvGevKVu38HCaesSOa70RgNQquup1GIFCTj1q3t155qY4UQxNcjaS6gE8cogo56zG7zqG2uBKyi0PQ2A0Pwmn8Rh+X0pUfpKhawTXkBymIKqTp/OC5D/L5AQXOYS+uT0/zuVsg4CoKbNEE5tueYMeVroWAqAfL2efIqfJsQ4aRWGOHguPzisQMliEkzZX3v4vYK4PIGYTC/kEV//KTQMTsVUHkDOIdnWL2yhBy5vCWQjFzZQg5c4loZkkUskoxjCRqsA4kZ5SzC9+yydlNl5x8kduUUNdEHC0h7Erej/HFDnWjVCd7l2MDcstiFUS+uhS0xz9pTDWXL10DXB2dtlUioOQmC3F/26HkfObdx9DyV2kbQ54POVZYdHIu9yIK2muQb9dEO01VNk2g8DHwt1kgWkKJUHJmuddLtswKrpEcaGmIRnDMP6xCNSjSNe8YFu2Y05s6YnYFFrQMNHXE7Imck0Zv6ojZEzjugmpOru5tB5RvZ4lZFTjsgdrOErOWKBQU3s34bYNCW+0HG3fCRstAKkYlK4V+YJUWpphtzoF+7as1HQSYvgh3uInbPSRnXJvrZJIswscjqpzfPnjRlVDSkRc1AMeBHj57n+tFWsWjRUaM1fexvL3GGrOkupmckA79wI1imECe83hUfOgY0uACvlzCfBrQrBjZIj9UCoTGiQciy0Z+xmH0Yu9BvPkGeIZ7uOOqnLWmWVA9kCGOQ66ZdAHSWlM5A1by+qOSEFalhPl595oJa1FFukocOhtUyNB/OeYJhEjpBjRRJ3tgCLdUVs+hzd2E+jNpW1tTdW4Itit+yPpbbFadVdfoe8VVO58bYcU0RHWCDXgm/4UQCVVFPsfxDfARqqp8PnBMABKy6m8THwRgB329rcU8K3a1VVn3YGkgNklZdcLV+RszjISwOuX5cmWGkhBWpNwZKds7bWUbOr9G4BEaYOwoK3Km+xgCfC2dAdKettb2jfYXY1tbb/tGN2xXXE/MPgEv4znzXCZmG+3pjpurfO8PO2HCTew+ce4VGBLS0XMvkjaNg51Cq7jxp9UgrlQ1Ze7CAvdGIjHWjrhe5HISzxR0I68BGzWUymk4ewMjGGxg4HCdfyMCP2JEjwtlzFR3w+ILMbR7MGBDqwNIlGy3GnKhb0W7E6jmcp+KAU6DTrStaHUCoMaFKfRKWysw6peZ4i2lteLm58CnmdLQ8Sh0y4BWdMeN9nxbsmS4G3U90PnF3xGI8c3vZh0YtWTMmT3XenB1V/bCC8cFHTFS44zYMeaIiZpo2Ix+Nwy/v8ZN6XEnjMA/MqOvBqoFrfCGk2HQiokUMFTjDhkx4YKR6oluQs94g5smdr4XTyN0T3j+Uz8dEeSAsAg1lt0ghXGAN5THWZLCjZ2kYYy6FAoDPER047Ed9NEasyKaTQbz+mFcYVnS4k6xHkErdy0pXCxFc/xgAvUHP7R5OCivd0hoC/O+iMrw5zg4acDgsJBW/BEj+vvyusk72iZ6mIaQG3WTwz6G4PtWTA7AGXRC7/R0b7zGELmv5QKndusPbz3Biam/dqEZ0IqevkQGKsWuuP4urrkoH7Si3wkzkU+1ML9ZxWp9y/RTinbDIt3VqhXqQ6tamuodwkbPzlKX6yRAUdBaWCut4sjBGK/G8QJStly5bQSZ0NbKrDSIPAasbdSYFNU0MjbGbWBYrCu/B8HGcxUWIrD4SQNa6dXnrsbQ9U5ckcogTSG+H81Upuno63njV+JhwnfOIBc1VVk/b3lRW5xy3Xchy961Mv8xyZEWhHosVccrtjS1lqMN3LqBQtNzhJXU1sqs1lgeI9bXTek1pkzgVsJ6S7vy+F4XeVBdayzX49H5aiYT0d2zoDfeK3kz8U6q623V76HzaBfbQo1Ee9eAXnqD3IaI1QZUR4n1jabSejomkAlt/b0oE8B74UM8OWg1jVePcRsYrB7s5JjiJw0Y6kWZQG+pG+tFmSDv6E839Kj55I8IFQRcVQLvQIdtNga5ryNoguSdLarCjA7k6/MxMyThYT7BT1OQOIOmp8N9rBILiPuYJdnsOvxDN5Vw12Ewmfo3iO/deGNHIBY/7GYbQ4i783IH3eTDASrTAsLXTlKzRHNV51Cxw/FpEdFkVX4X13xW11C2hA45uCF+2Sk1IiqvRm89ZcVFdftoV0zx1VbLoVdTONyLEqar5WY3c1bcIKRSPPDmQ/DgyeSLjtLniwvpxXgdqY+318Vp2z/d/VVSc0A5CAM7b8aIO8zMfjgqJaqHIg2tKiEsFI8WGQFWw2xVJuSW1zHYQTJ8lrA+/NKKEQ984MTDO4j1+VDbMZYO5n0orah4ILkmUimw9DrMTkVwDf1rEIifeUdNtT0EkX2mofCUVFYjaHydqpZpEx0tC5CG/siZk8xILDGsRkf10Jk4REaD4S49NxKppAi1gVEMHYCXW+ezK+74yiduxGFd1eM9nvDiox2KgUgDZldNEe5HZAbfEq8ORiopQnluMtzV5AaqVFRPdINJAnbQfsi225FBZ26svp4qYJTZURym4fikID9gT08D4LcYRBEcOSJZCI9UU4Sr3jzqYKSSKlQc/ooqIQ2xRSopQv0jg5mGaKpllM8b86O9hvLY6KgCpcB5tPOWsYaE66opwq2x3LUmto6Ycrzld23rql97cqqnXoFk+LI4bqpKRRUGn/KtIQUbHaMjcPTWpHQvgVQQ4B5pjkmjdFWkcfJWijRGFVrafFEbS9uvg0sDtIurNEhPRhoIl1SFjFEHlwcoyqY8QR2ea+bH3GWz5QlvctMenvsQg5g11zISjWNHW/fisES1SquW8BHWXeNJsETKI5ehsRFICSWQb7GbKoLUEtIgDyCBbwRGiLoUTXhpBMbmCyaCzA4LKkLef1HlqEXk34SOAkQZWMW47WTxk0opbWtoQLED+E0HTqUjjeQPH87BxPAFT93oFlR3F2QC8yC9glqHV0QYv3mUk0PoLtIxGD9USJK2hgYUOwYKGbWvI40UugrRUgaWNh6N3P3JNB7x3wFKN/7oJOcK5qvgCtXWJhm+ZIKjtiqDK8SAivuKphVbV4SAAoRiy4oQUIBQbE0QAvJZkbFJlJ0ZZXaC0iHsKPSGdwTzktQq8k29NLY38cPwhkh2O49QkMbY4xse1DhaEvId/fziAXmKJrx8koRBkgJ8EgDqIiukS1dGvqtfnXovD9OSkK9FyvWbCtUIoaCMMb6rn5tFYg9/FygBqiyEgnwv3kMtttEjbNi9eFJCvgUa2H4msLCw1/KswysOJLC3inGOKEjuChvpscgDtUV09FhUWUoNXT0WDTiVjiYklPxPIHGf1JONJqhlnEgarCOi8NocPaSP430pfgrf2Jvbdv1IodzTtXShKVdLw3pKA1z1zt2xu3u4hri6SopYqsWvq6Kn/yCP01GRnxj0w1RgprfLUQdXGIPEZ2rJEzThVRDUqkBCQB4iSljnlLI5WhpKKONnwnKRSJwBSwHB/Q6RBRs0kkZDCSWNgaNQhbQk5PsKYOOFjsBakV5XoRFQGQ5SfwV1RJTHplRAlItvNcimAFELKPcjdSQOTUoaLMYXmHupvYee2PLMLhZFSH6oKPHsDXzIdupYVCklMMdzlTribQ0lFN9NBLYL0EAqBSUMJLo81dCSo2tpQBs9DkcAS/zQmw7S8/nJe/mhpCq0knnbQe9clUmLjogqjMLQGqmgiiGwBm8AQ20dXiGSKKxOIRWUMBQntVoS6iDqadOS0QGkmkotGaUmRXGpa/U6URqj7klpANNBpCuORi+8EYkh8WttRhJOcUUaVUoHWHk+fLXPxg5COz/EZvjQFRHWUXUd+IkXpjw3b4pAUzS1oJZXxmhBbLR0oCmtD+zr6EDabjfDJzuLMFVCOqBGj5sWgRI/XnoYauyWOBEm4bvghpHwVfRamCohHVCxyhqqvo6WKkFTbaAWR+WZk+XbZfyMSPakBl1M24tdaTSlI6Txxa4DS+ZEyhEwpRlqipAuKOUh9mE9Aw0iLZgUTbNtNwPQbXVdrQ4doKWOtlaHDqZKSFurQweU+HGsw1Dqg4ADYlrgVOZT+zq6Wmk6kIQv8mK00nQwVUJaWmkg2IS+4sKPYT1dDUktaIpz5u0WoA6iRFsydvvQWuj6mjr75VoQxa6FVTkPUfwoKqVtuyNxCUTOoqkirz7kpfwMtBxAA2JXomz2WEoVVZgkWGqAKVU0wEhOj9OIJCbKqVjlZl9lpkZHFajY1KgOVOuoAmWut0EtpK3AaY+DUC0tRTAH/eN5OsobqaQK5WhIu1JEFWUP0P9WJxp4CCUtUFHoDd9QIYhVaSmDcdysIoCm9RoV2UGeHpauO0ccqe5ED0fPrcDFQe/Hq8WzBqSWljIYTn8bbH7NktT24A44L9VkowbSUXET6NsY9WSkVupywZPyWvFxAzkXLwzpxqfIa8dPMjwGBlODLlBMaHUjSSR61OPYpaRWzHztk2bOSlMPqM4SqK24EftUdbC15VTxqu3N6mSEkiJUvlZUnaiWUcbR0IneaOlEb/Ya2tCliCIKdGTG47sslYo6jI6SX8so4/wqN4LbB/pVfAR3AEliRqePIzidQ0dh3MPHj6Pt0j3J/Tw9IPE9PVScnY6u8k5LV3n/uNmqs1QqqjD1IRjqRKSUIpbna3hBlCKKKKWatt56X08VUGKdSA9KcInIAMi5DpJzHSjQD+OX8txUG3heqGEEYUBUETVAnaAkdB6hzDRuF7Ejpormou7Eo8T5Bj2sRkgRCZ+tpoxTiiii4JPWlFFKEVUUie2ZPRIdB3nj49+WAvfHDcNUOhqANNRKtYwGnLd6cN7qwKlON1InIpQ0QWnq3/fkdOHlxx/pw6vkVPFC72V5eqIjzxNKqlD57dLQBo4DEw2vub6eKmB1nJU6GqGkCBW76O2wWb7RMKnWklLF0jEuILwciIqS7IGGRQeVijoMPslPB06pow50qgXnVA/MuY6WQaOjCqRjhkDPnEC+dx44ew1vu5aUDqx8EjfJ3FTHkC1NUQdkGD66uiKv1tIAlrq4K6YHrNFSBGuO1lTmakmpYrm+HqZaRxHoCeVSV2brRpeIEFJE0jNdqGmCsD6nQAtQpaQHSkOZI5W0QCUahsNJJYHlwMpXwaJWrerCWdWlKAjB0rfipNwbV8yYgwxvrAnwLYcKo0sN4JCuVuhyTv0RviAeN7YFr8lk0w8aMOFG8hKEwYvKqP+gA6T0DJanYk69S1MLp7Fm6gZ4g2AUJmrtNSIu6bJ6kVF33ndT90kzcltWLzIuFaiGxMMQSm0ECnZfWg96sdpQC2ojpQtNbW6thaZrGWxrY6jsNl4KYU9RM2iQwuc00V0PjMubccFAIWObMOOK5ipuXF6PC0g4VhjpI2hrJT1gqqtSCTJ9q0/LzaHKEyoEnMY5lXrLY5lztPC11DQBur9pKiG1kiawNM4cPS/PRkoPmr6Gv+YW/hPwMk1kjZSRXq/B22ITuNuhz1KXxSYvSQr9Jxd+U9n4ur78/PnyVrX7XqjYt3d3/Kf4FL5bjRtWoWK1xRRzWyHGn80YVBryP+GeLqpKbLbZn/VzL2+7nouaL7YHHlh7smlhUaj8PBy5glWu5Cq21/MVTmpC46OGi3Vgo2WC9NUi4K02h9WSk97t36iMZj5xKM7sZzQ3UX6gfdXZDh5FCyeKiNjAv3+wfgm9zIeJdQuTMIsd9NfOTS0fpCn2OwXJ3ori8MnFo/fWDgYwBmkYW7WcNWzSB26g02ajN2I0ePJPWzlRyWSlNmIQx8ReZ7TuR819RAr5qUxh8Gf4ImcZPgM/8mCz8KbM+wRS3ww309f1R9lUFwarbY3Qfa3CXuxBEEDPWJzRDAlw5S0BY/E2aI2H8DJwwu6FIEbgCEMiXAeItpYpHrbr4t1hPs4IQyJcB4izlikeNvRHGjrhAYooaUmE7JflISKua02IcHVQwhU/4Zpy7pIRuDX9UCYG12f8Fu4cpWgSj7AnEnsHSNyWKQG2iyxJQ/+AhKRBAc5PcAsyLz0gaMviCOkP/dORuLGS2MEnlFlJFkVhnFo/UE9IohuUjIkBk0wvX1Ab7i78E3zW5GlLj8+wLo87iiPGL/509fVLuNt1V3hJ2/YKMasjPIJwB5/Tm8B7qZ7WFAkVCFV+BGedF5uL/sSnEEm3FHZVeQEUImMUgRULGhKja583AXDOuchHl79+/KLQb8MMxSi1RZdk1ECn74rmtXwmSGCaRXYEXrwQbKyuJq95BfdHAPoxUI7ufwXpXsu4gU756u1mQrObUXXaQImgWa6ahNEp+9tvv9llssENHu45LtZA6LRBDqkHHkgSiOroGEahCYfaxtxQ1BA5JlvnjocwTHE5gjw5u6VQdkusBy+DKVLZl+fuyJJ8gk+uA3GFjgeKv+DK9fLH29PV9efVmXWl5mWR8jiuZBUurtdXa+sCj1grshRKt3c365WqFMpueZxt8qiz3QBlGQUfob+JnzTEFfRxk0VcoDdLtY0hKlAhPk0vcRPJ2BpWfYRxAD3JEsSUxe01vBbX+nxxYX28vbYvrk9P7Z/u/moFYWAn0Mli5YR3wySNIfBVUisncfGcoFcsswpgkObzMOXREVXbU614C5pB9QtEL3rj1jTk0s5CAl1y5RHYaoUZz6qoZI4IxvkGKuAZyWJ4+2qxqMeAPKwnFU1q8zWGpCbjZXQ551kVX6Tofe85e+AqvSaqSqzwW4uSB2GEN97TktwHThx+glv8lkRN+nbXzL64+enHq8/2+k8fV+dv8Ke7y/++s9dXf73Muyn5iiD0LPpRSHb9l/Xd5XWl/vP60l7fXPz58m7dUl3yaq6vvnz8YW1/vb355Wp9dfMT/uvu5uLmi/3LUlLxC6F28aePP/10+cW+/OnjD18uP8kp1o22jyfLkx+XJ6uzq+uzdwI8H79+xUxfL2/vri7XvAGL18ZPP1/b609/tu/+dHv58ZP95ebi4xd7fXdz+/Hzpf315gol62077lcCZBc3119vfrr86c6++Hj38cvNZ8R5uUafeTWuf7j8dPdlXeWHH6++tLPX//HS/yBeUtXq8j+I6n9df7Qvbv/y9e5m1FTrtidBU7d3d3l+/ri+bumecMdnvdLIXl9e3KH81+b7Rxam/9E8VHzuiJfvyB/bK1LSjfPv/z76JN97E7di0f/jv4vG285xFoAHAT3I8xjXQz7PQ0GIatHOg7iSvonKSMcfrjboyebbReYs8CsNCcV51If51yNPLfATvvtbviaglVr5UZ3m7TvPz/iirGAD4k3LPkrw5VsJ+/WLbBygfmzhRFnbMGpEwudj//T0ENa3Hevb6On8OOmOeZkx7YUgtcGD2wLY5wlhOtlRX6BuXPfXaOFAX9wkrY1WcKgmRQXRbkL3BiitxqCoC3iwx39g+lA/NksnPDd4ZHlQPFP+s3ZQfyBtZYF/ieLwV+ikFt5/uoOBVTyJh7UW3gEyxzYLHPwl7lLjf5MWXRpn3R6ZAYYNSIFW+wIJE4AAvb5t9FqYwvPQd1N7G6O3kB2FbpCWAQ8LEeBNPw6Mpkp+ZD9OU/fACV8Nbl+DKG8MHN5vZ+Bl/P69edvfQBy4wS5ZAM+bwPXafL7bbEqACG5AkLpOu2EAvOSQECCGdn54ciKJ4YNHmL8yQOwvcDs7BfEOpl2Ogcd6zbJjH33zvWDjTJEh3Wf+Q4ei/M688W7L8NhH33xftg+PN8vusIUZCGobEaHg74/R999ztxd7JppKh0nTPDpUQx2jPuL3vNXUiH45ic0Hg9fxD/VfCiDuToxmJNr763gbhMfFt5MADbzQcyzyt8Plpaqys7tvnOP/yr85bDSZpZGKl/6r8Pi/yu8mihtjRCLxMzhocnzDN2SiMWZMsojEyXCH6XiLfztufjtsBB0MTCS2xru4x9vq98li7eCAQuVvvIt6vMUPHOcPHNcPHLhYHh5RJAbnMXSlMb5n6tDgmOLwk3N1hT6OMfDY8OCi/JiiIlE+qtYbUztGnSnoJN/jXxf5n4dgqQZ67OKz7YOoTfXfpd79q+NrEH3/L/968/Pd15/v7E9Xt/9m/cu/fr29+c/Li7ufPl5f/tsiD6xSdfSo3cQRynrH/4Wa8FmQJXBzHAFcp/WqM6aE7/jdgQVmGEbaMcN3G0hyBWQfJun84pOVr4V8RZ4tiqUnCxdVLIl7uio83qSLYv3JJj+SMJ9dWuyCbEFUz/i28W6UEIKdp4uHFjjOFmG6h7GH3Pv/IjdSz2wYeX7rwyRBUX/swWCX7r/vztObTmHcsxdJY/L5/03lSVK5WB+Rp3A5x9dNtHLhcRi1O5NwG5+u/N3qDJwsT7Z4CZDr95YA0W26IF7Ab1H+1hmqIa8ucRf+phoBEkqPfhqKxQnmw2z4i3JFQJVLn84XZ4tlF3YkQJmtwWaTL40B3s8JjKdxi7eFIv5u8l4fH++cgY4fX3BUdPGIznH87Rnp7HwYpErvJ4F2t7C/z+hDLXf8zU33x/1NScbfpmXDkVHPiso5buxkHog3MILBBgbOi9y813w8ClBjbNNr5fPPWsm9B+llS0Xrf0DSNM9W9Z632XpgJ1jTGakQlKKeszFDen5Av7+zyld3/unVd3989j2sW1wSgZSXi5OcHuIdlG6wQ1/9fPfjMXpb/7EQqF799VrDzFn44SZD9We+E3Nxka8J/1o89hWVqB/yuKq3TC7y1ZAoPFKKYJy+rB307/f4nq6yUUE6ECGBPLLXKYz+gPBbnw/k0qY4RGAN0zSf3hbwxTJHpRLFBrEypwSTTfLueXnloA+tgLR3FC+c2KlWLjtxYbE817yu9NBX9S5Qek1IHujWzltHr4/WV9dfv1xdXN39xV7f/fzp6oZYmn704eif90fE1uAP9+iLe9RwBk8Q5dfQefwFxC7eBJbgrz/g/+AH8P+hOiJy0VObxy+hU8zHlD98qP7YQecxtJPNo32GWpYn1devqz/w7pGbqB9uoPld/vp78R8kc1SelfE/DPx3lCwFA172lKBU+FuTDHCD9HAiv64zW75/BLtY7izDC+yLzTDN6e3o6TB2dy5qhJePB5nnva6v5UFfLF/nCimqdNGn9+/evzt9f37y5vfXQsaJVe/40AIp28fLNyfvUeS8PXsnaD1fyl5WbireH5+fn757f7p6dy4IQNlxIGH+9M3y9O3bN+9OBK2jlCe2ytobZ+MoxsL56Zv3b9+LZoIfYwjxBuALedvLN6erk3cn7968FY+DZpuUkvfL1cnZ8uzNuzeiyQCiyC5fDC5MVBjO3py9OX/7fnUqHgnVHpsyQyhFxPIU1QVvBfJBNa2BOFBkeG5Rj+YnYksivD999/b92+XZmSjC7d0lqonKjY2JXF5cnp2uTlfnb7mrI8J/skA6HnrzJHhuUiIGzlZvzs+W5ycrCQj4hFTsPQg2XnG8n3iVdH7+ZvX+zYq7OA5al/F+uXr37v1yef5OxvlqE3lJIJkHzs5W78+WJ1LR3yOQioOTs5Plcvn+hLsyoBHk+dAN8CI9Rzoq3r1/+w6VSAkQt7P3zy5/kq0V3r9dvV++PZVIlB5JGoMgye+OVmFavn3/frk6PeOvsgemgMVjY3X+brl6c3oqXER73Qxh26sTlAwnb9+9ffP7349+/3/evoXd=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA