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