####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH ?= /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.3.1

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
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DEFR32MG24A010F1024IM48=1' \
 '-DSL_APP_PROPERTIES=1' \
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
 -I$(SDK_PATH)/platform/bootloader \
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
$(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o: $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o

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