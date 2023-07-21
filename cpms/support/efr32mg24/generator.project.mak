####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.3.0

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
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o

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

$(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o: ../../generator/credentials_efr32.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/credentials_efr32.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/credentials_efr32.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o

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

$(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o: ../../generator/platform_efr32.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/platform_efr32.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/platform_efr32.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o

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