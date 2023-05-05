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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA