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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA