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