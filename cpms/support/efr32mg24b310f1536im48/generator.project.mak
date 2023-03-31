####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = ../../../third_party/silabs/gecko_sdk
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