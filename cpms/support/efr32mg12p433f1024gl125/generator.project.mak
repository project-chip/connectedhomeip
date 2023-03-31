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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA