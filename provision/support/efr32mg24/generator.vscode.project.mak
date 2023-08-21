####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Resources/git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.3.1

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEFR32MG24A010F1024IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DEFR32MG24A010F1024IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../../generator \
 -I../../../examples \
 -I../../../src \
 -I$(SDK_PATH)/platform/bootloader \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/platform/security/sl_component/se_manager/inc \
 -I$(SDK_PATH)/platform/security/sl_component/se_manager/src \
 -I$(SDK_PATH)/util/third_party/segger/systemview/SEGGER \
 -I$(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager \
 -I$(SDK_PATH)/platform/common/toolchain/inc \
 -I$(SDK_PATH)/platform/service/system/inc

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
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 -mcmse \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m33 \
 -mthumb \
 -mfpu=fpv5-sp-d16 \
 -mfloat-abi=hard \
 -std=c++11 \
 -fno-rtti \
 -fno-exceptions \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
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
 -Wl,--gc-sections


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
$(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o: $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o

$(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o: $(SDK_PATH)/platform/common/src/sl_assert.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/common/src/sl_assert.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/common/src/sl_assert.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.d
OBJS += $(OUTPUT_DIR)/sdk/platform/common/src/sl_assert.o

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

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o: $(SDK_PATH)/platform/service/system/src/sl_system_init.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_init.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_init.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_init.o

$(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o: $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/system/src/sl_system_process_action.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/system/src/sl_system_process_action.o

$(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o: $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/silicon_labs/silabs_core/memory_manager/sl_malloc.c
CDEPS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.d
OBJS += $(OUTPUT_DIR)/sdk/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o

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

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_driver_wrappers.o

$(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o: $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/util/third_party/mbedtls/library/psa_crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/util/third_party/mbedtls/library/psa_crypto_ecp.o

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

$(OUTPUT_DIR)/project/_/_/generator/commands.o: ../../generator/commands.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/commands.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/commands.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/commands.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/commands.o

$(OUTPUT_DIR)/project/_/_/generator/config.o: ../../generator/config.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/config.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/config.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/config.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/config.o

$(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o: ../../generator/credentials_efr32.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/credentials_efr32.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/credentials_efr32.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o

$(OUTPUT_DIR)/project/_/_/generator/encoding.o: ../../generator/encoding.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/encoding.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/encoding.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/encoding.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/encoding.o

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

$(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o: ../../generator/platform_efr32.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/platform_efr32.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/platform_efr32.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA