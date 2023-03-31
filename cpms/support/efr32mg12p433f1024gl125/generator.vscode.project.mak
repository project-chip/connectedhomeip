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
 '-DEFR32MG12P433F1024GL125=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG12P433F1024GL125=1' \
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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA