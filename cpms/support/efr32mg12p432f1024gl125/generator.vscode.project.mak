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
 '-DEFR32MG12P432F1024GL125=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG12P432F1024GL125=1' \
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