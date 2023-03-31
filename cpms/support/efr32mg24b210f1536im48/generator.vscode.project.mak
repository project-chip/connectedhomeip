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
 '-DEFR32MG24B210F1536IM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24B210F1536IM48=1' \
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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA