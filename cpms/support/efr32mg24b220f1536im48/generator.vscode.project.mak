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
 '-DEFR32MG24B220F1536IM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24B220F1536IM48=1' \
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