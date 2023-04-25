####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /res/git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.2.2

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEFR32MG24A010F1024IM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DEFR32MG24A010F1024IM48=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../../generator \
 -I../../../examples \
 -I../../../src \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG24/Include \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
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

# $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# 	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
# 	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
# 	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
# OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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

$(OUTPUT_DIR)/project/_/_/generator/nvm3.o: ../../generator/nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/nvm3.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.o

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