####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = ../../../third_party/silabs/gecko_sdk
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

$(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o: autogen/sl_device_init_clocks.c
	@$(POSIX_TOOL_PATH)echo 'Building autogen/sl_device_init_clocks.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ autogen/sl_device_init_clocks.c
CDEPS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.d
OBJS += $(OUTPUT_DIR)/project/autogen/sl_device_init_clocks.o

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