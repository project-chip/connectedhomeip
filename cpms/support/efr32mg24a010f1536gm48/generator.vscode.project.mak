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
 '-DEFR32MG24A010F1536GM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG24A010F1536GM48=1' \
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