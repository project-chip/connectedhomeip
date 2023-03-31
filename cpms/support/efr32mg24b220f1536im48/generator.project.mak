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