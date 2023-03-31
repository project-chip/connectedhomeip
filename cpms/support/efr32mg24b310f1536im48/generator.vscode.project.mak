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
 '-DEFR32MG24B310F1536IM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24B310F1536IM48=1' \
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