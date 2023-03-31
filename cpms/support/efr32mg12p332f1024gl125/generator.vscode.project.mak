####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Green/drafts/matter/stash/third_party/silabs/gecko_sdk
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
 '-DEFR32MG12P332F1024GL125=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG12P332F1024GL125=1' \
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
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Include \
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
 $(SDK_PATH)/platform/emdrv/nvm3/lib/libnvm3_CM4_gcc.a

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -fdata-sections \
 -ffunction-sections \
 -fomit-frame-pointer \
 -imacros sl_gcc_preinclude.h \
 --specs=nano.specs \
 -g

CXX_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
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
 --specs=nano.specs \
 -g

ASM_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
 -imacros sl_gcc_preinclude.h \
 -x assembler-with-cpp

LD_FLAGS += \
 -mcpu=cortex-m4 \
 -mthumb \
 -mfpu=fpv4-sp-d16 \
 -mfloat-abi=softfp \
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

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/startup_efr32mg12p.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o: $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/EFR32MG12P/Source/system_efr32mg12p.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o: $(SDK_PATH)/platform/emlib/src/em_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_crypto.o

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

$(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o: $(SDK_PATH)/platform/emlib/src/em_system.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emlib/src/em_system.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emlib/src/em_system.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.d
OBJS += $(OUTPUT_DIR)/sdk/platform/emlib/src/em_system.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_aes.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/crypto_ecp.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/error.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/error.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/mbedtls_sha.o

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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/crypto_management.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/crypto_management.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sl_psa_its_nvm3.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_aead.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_cipher.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_hash.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_transparent_driver_mac.o

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_crypto_trng_driver.o

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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o: $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_dcdc_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_dcdc_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_emu_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_emu_s1.o

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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA