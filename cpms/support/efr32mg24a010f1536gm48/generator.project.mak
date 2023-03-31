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