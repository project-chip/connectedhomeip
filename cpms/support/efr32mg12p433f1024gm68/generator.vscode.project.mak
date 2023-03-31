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
 '-DEFR32MG12P433F1024GM68=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DEFR32MG12P433F1024GM68=1' \
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