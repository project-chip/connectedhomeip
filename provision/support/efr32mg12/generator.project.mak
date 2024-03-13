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