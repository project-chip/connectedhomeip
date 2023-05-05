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