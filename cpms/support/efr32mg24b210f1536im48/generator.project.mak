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