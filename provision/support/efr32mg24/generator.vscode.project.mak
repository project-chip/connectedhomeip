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

#$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
#	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
#	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
#OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA