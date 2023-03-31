####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Green/drafts/matter/stash/third_party/silabs/gecko_sdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.2.0

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24B010F1536IM40=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24B010F1536IM40=1' \
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
 -I$(SDK_PATH)/../../../src \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
 -I$(SDK_PATH)/platform/service/device_init/inc \
 -I$(SDK_PATH)/platform/emdrv/common/inc \
 -I$(SDK_PATH)/platform/emlib/inc \
 -I$(SDK_PATH)/platform/common/inc \
 -I$(SDK_PATH)/platform/service/iostream/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc/public \
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

$(OUTPUT_DIR)/project/_/_/generator/credentials.o: ../../generator/credentials.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/credentials.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/credentials.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/credentials.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/credentials.o

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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA