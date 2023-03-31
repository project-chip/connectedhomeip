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
# SIMPLICITY_STUDIO_METADATA=eJzlnQlv3Mi177+KIQQP975ITbW2sX3tDCayxlf3WWPDkpMXRAFBkdXdHHELyZasCea7vyqySBbX2g6X4AVJrO5m/c+virVv518HX75+/p+ryzvz9vO3r5dXtwdvD979+N33Xj2hOHHD4P39wXp1fH/wCgV26LjBFn/x7e7no9f3Bz/+6T54F8Xhr8hOX+EgQfLWDx3k4Sd2aRq9NYzn5+dV4nrWQ7KyQ99IEuM23TtuuEJ2GCMsikNHKE5fbm38Lw5H1e4PsPSrV+82oeeg+FVg+eRHOww27pb+Rn51PVT8lnimj/wwfjHzp1Y7rL6PXfwTeeyt8ZfQ2/soMT7GCAWGE1ubNDF8K01RbCSplewMO/ITI9lHURinBtrEpyf+dn0SnZ2ebtbHJ2db/+K1kYsbXdaMLqzbq48fr76aX+/uzEv85LhUHcY6oYIn/9R00Mbae+kkydVjsBMuSizTjl+iNJwErdNcJxh+5w56cm1kuoGbmo7t2FPltQG7IqjI389BWjfbCYp8z30wSV2AQz7st5NgDhjtS03/ATmpl0xWt7TM5WDvjLxC7KocrX0ablHQXTt+vbvCVYIfhQEK0mQkfkpgtI31JatdPGXaVmp54VgpW5D1mRQpRLYX2o+4GRsbsNtmHyF6IlHZWYHjoXj81GuZE+MaPdFa5vq43DBJY2T59Nmxi0KPRWG60ROuyyKXLsubboANBvakSdg23Mlarz5NqjQyZ79RsUbFTGMrSDZh7E8ELEgg1kmbiHnQbiep5waPKCbfrDxnVLaGpU6alR2XnTf856g8LVv8PsQW2Y+hmTiP5tnqhAz6yjjUHos8KyXZpPy99cSHrBFjfm89cet6Lob7hIeGtcdaD179/PX05Obj+uRL47m2ZLiPG0aL59iC95KkyDerxFOoZNOdGztmZOGBq5EPb40y6YwidYw8EQwmpkYVGSOnNTpxDE4UUmx5Hy0nDp08zUjUMl7vS7wObG/vcN9id95XqH0gkoEy9xRItmrixsa0bD9aTDRKGgl+x14Qfg4jQb8xI9y/WFAMKiC5WOAGYmHRKIjE42H7++VEgcJI0McvYer6SsOyseLAIknFBPe4FhUNyiMRh0SpczpWDJJ6l5XPT6YFgs2CXgIDJBEL38IBEzt28QtcUMFoc0nFKUb/XFRcKI94HNCSalokW9Oi1F8QfQ4jTr+J9mi3HP4SRzwG2wiPL5cTgxJHJgbugqrWgkaO31zQuKHiEY+De7KgPERhJOgda0n4lEac38PNxnL4Cxo5ftNeUD3KAEnEAuGOYYIWFIsKSDoW5sNeZcPDuDEpoORjs6jMVWOSj0uSLi8uOZNMXBY2umaAZGKxxxxLikTBIx4HP1lQ00dhxOkjO1hQ+hc0Evzxgib6KIwU/aJq1opHLg6Juw0sb1lvgoUSj028pLmAWHYuIA791HrwFtSFYokk4pHaC6pUCxo5fnNpUTCVYhGjBbUPLJF4PJIllelEtkwvrJ+n0MvLgiyqOLBEEvGIA5U9mmPFgdKI8++TRXW2SxzxGDwtap7pSXqeiYQww2hBc00skXg8nh2l3cojxaGgkePHg40F9bxZIl48fLpNem56loO3aau10Wte9k4eoT1bHV+2vmp/Ud/mdXlze307tD/wMjsfNrQxsG+nWC3VbT9xk2zTv6u2QV4sybPoGIS5TN+25VbitkHpmbvpORnDg5jpb2RLaYq+j9eKdjDWrQqk43bErlZvGm67e1N1PHLqyPbPJqVjbA7C+dHetGL/6Ycp6WpGDfCKJkExbysyc+JmuMZxA3u4tuk+vjNWYtKoGYxJAyP2HCLivfyOY4ZzkxcMMtxj7mQRxO7ZvzJEHeA/58YuGLhlsF3GYrlykR1hTdajbVrvijBm7DxJm2NI5rAFwFcU0vlsZvKCQb6ub1fI9GDYcLUtkj0LpdETpzBUpAxrmPcmy3NwcZrOBloYl68mRJrPMjnGrgzL6NGakDUs9R7mAi2MAxSjfCioX4jokJKU8tGzZ26ryJwNy7wXSB+P4tBGSWJadkrGPrMgtxnGKVhsCo2dZWlEaYZtWFZ7N7MgtxngxyfId+KnodGJHfp+GGgOTJAdOuNNlmVxMHLQLAlLcwCVE7k7RrNqalw/QzDN/LTuaGU+TxJiOCvvwwSDRSILurM8c+Nh85MC16zyIck4c1K+wuAINSb7wkYuN1mcSKlp2uSm+JRgQkAkx0yaWtSeTAGaGK+yKliApuQrDMoXIM99GC5A+IHMxOXNWTYxao0fLWzS6DA7RpvdjD63emkuX9k7d7wFoIwvb4grS8Ywz4jTZnWcrsmxDpps5cQas+6tQzH2BtgSzzUnS626seEUsxLck50otSpbw0wP+/HmvmtA1BAnT2W9ronyU2lrmCm2/M0+GG/mtwbFGBumGnsRskbVu/DYfn/xmCOY2tuLuwYvTZ5xD8XXiXoOvHczTVt1Nk0OE465PFPj6lyEadKMeuyzhtN9pLPJM+ZBihpO5yGJDppJs1Ld3jBbPkMzDVdlyxjqNQrMS3T1usYbMJMokLFyZUighp0Gh1oSqmGnISpt8euvSYCQyBvLapVJcApL/PprEhxqSKiOmASosjVYR/BGlh2TvXJDS3IXarAfr2JkJnkZU4MjprHHJnUkocFJfg/wmMOBOpTQeICsO6RWOuKgqQ5VGQNt1qp3MFbBo9Gg62iVMblopGHo2TvL5SytiKx75q4Axo5vCVzEvDI7zpIh9XAQo+2Yo8FW7Gj2bFnnLRzS5JiHUwRwa9tmFCM33/g4OWjb/Bi7LW0cpfRlqAVjL2HnlL3qhmJ65y3vCtiGh5Ced5E/dZRvSjqybBt5KLZGXmfOE6Z2Bb3RjmDhBGAYUe1q1a6qrKv7kMZh9GLurNh5tkYcEQgnCa3uusB450TiOBxvjVcqAiUK75hIGXwZ3HUeDjydDLKUbq4fAb7OIwaP7PFuU1aBpzwc+FJgZy2DvgHEzfWuWd3tvowYtJnUqt2urlZ/oZ+9ASq6NRUP593h0mVaI+6SkCJnYHjF3faXg83A8LB9y14QN0PDK+I7a70ccJaGD35yfrEodIaHD3++PlkUPMPDgd8uqYhuhYtoo+FYBH2bSfWob9cojSg7sfuEYt4ATWgMkqNmtztM3hOoolLrBBQwAq+e9ttICKq0iFh0Ywlm5TzA2Dv75V5Jg0kuJnRGdmFxqagEYpOgYvOE6aAIBQ4KbHeOkVdHnAbYxEZhvhVYW+RjC7PHp5NIph6wggSzEAd1xWt2o93SaoUBSK247sbctA4T0wJRK564I7zwaFJCrVhayHIWHs0CkT/PRmTdNDGz7e3zR6rFAzvdUPfgtn/w3K7nulPpyfL2So4e4dKIdGJzaqMF1fGqu5OqnQ6JxU8EN8ARCiwP57Z9kiLHTNIwxm3B/OmRWMYQXDtdOgr7omPHA+TGMJ92X0JUShKp3MofW8wdt34spR75EqLTSaXRI19CnAbYpHvkc8enk0itX0N2xJNV1WW8JB6gWhzTl2ghmXAIbqyr55of6x2Afep6/W5smRQY2tFAJ9yGNzN47kNsxS/Du25GWDklMaw9QWkNCmT0rI7WuZJgjUMn4LsB+HSsZS7jM86J8zCWlgcZH6wEXZxNDliZHb6cbpR5CS5d/0xDB535HFvgC/SCiKXtQc4RNhBw+Xo2CTS5TFxVP81QxdRNcyidBHwLgwAgtTp8PyL43AcXzO+cy6jXKe6WbMOevE4pzQ7She70iUZtDnJF8CcDuFxR+4RAm+vRTs6nJyuscthmABOgmqdnwtjl8M3TK2HscvjmaVMZu8MtQr7vc/o2obIrwmdGoefNBVkaH+5BpTEekT2Ar6rzu0+M4UHCHdlPNAtizfLwTv5s/83kgJVZzgXTAbZGBtauP32d07I+fJn4LkaW48Jv8+By1iwP1450nmP66pExLERoEsnZMEvrw33VwPT34FdE8fuqpVmBnnS54DA9Ztu+8Ih4ataGbU6dNMbUvkBl1Dd3319jzvfy+zGk6J+sxH0CX7yTpq8wuHMS5freCFs9hWYmmgACxDOlctO4eL90ctKGcc68yjx1GGN3mC9KRlqe5CPWTHMpR1nuFoLsXd5uMZKeF/y6tRBkZZpLmcYWvHspIcjS8nCvz3LGuC6S39+r7ArNKkzO9yhWpovZken5SruDfHFikfbI3CEvQvAenrmcHfaHR8WZl4eH/XY24k4CLrPtuSNsaRCCrUxzKX0X/mYwIcbCMJcQB1yfztcz6kYQpH5EL/Nk17r5Qdrv58dvJh/cF0a5ZKaNm9YZ5rgbtkU4p58iYQ2LEIIfeRAl5PvOyB9Mpt8jwBrmEmYt7GwJWbMuyjpTktasD/eXysODRbs1+UxjF4Eg84ywMik7xvEKmXTtOzvRmxPm2TDUSSDKTDf2kkFC1kmcEb4DRTQWiRemIx6Sk4lFB4pwLOhO/jnpKwRR6jk2eLXNi9KOcQBPBrfvdF0f7wgH6WRwe07J9dHGM2xWa5sXLm/zFjV+KbN3WLm6U5zcgTb9UkY3g1QbPvmsUtu+ZBs+I3FFIMw8gi8AKeJODwHifY4Z0TtQZNq9Gcmpeal2b0bcwr4o72xTZz0MwtzprPmZmpfpXcxIS80L9y6swAl90/Wj6Vdw+zFk+kZzUufmJfpGM8ImUmWuOeSaE7yNIjv6m5O+7yi54B3a5CZl3pVX3acDWwljwZ/R7E0JSl5+toQOAluxO12BbiFS4zzGJFjPx0iNCzBOu/jcBdq7DN19ZHEu1Mo8jzM/jzQbZ2mex7l3PQf3tDbgNwOKs9YQeMf/8T+eN2PJZwH4d7rOh2mLvH17Z+H/nhzPh8kACLFGoQfuVEGStkDg8mYTN9POGrWJ6xA85mlnX1q0vVMvTc4pB04typ5hU3ubeWSlRyer7/OR1hC4vCSTkHsyZuRlEfi8zOGeGZHrFDzq4uzZbMAMAIc12742G2hpnUs530jFERqpOLv5+gHUNocQ2c50s5NNxMI4n3HGqqm0zqX8NbIe56uPGPtc0unmz9uUnZPnPUetZ6Os7PNIJz3o0OIcuMuPpdzOOB7ZCo1Hdo/OZjbEwjiPsTyaPRsoS8ChpWHmHoe0MXjc061at1g7F6xbfOczAp6LEOYeCx/2mw25h9jzwvmGTD0snBgEKDWT0H5EE67pNckbDDxiN0nNx+f5aCv7HFJyKdFclNQ2hzCCd6guTBi1Hax3EU533q0F2HnkrePypvXJjIyFeQHO+WrT0jqPsrgBYzZQBkCQdd65kRaFKHV2ecfs1AUFjzr0XtanxzNmYAaAxxq7T1aKTOLBNZmvUW1j8LgTa+ZMwQBwWGMXtx7O+mK+RZUaAY92xjmVni0x7Quu5ltDL4zzGcltWDNSUvN8zvMZewSVeR5nMl9Rp7b5hKZt2bv5WtcagQhttriW7N10xgnzLhAR9jB8dGdO6hJBgDd1yYhxVt4KgcNbXT43F26NgEfr+rOiluY5nNS7xWygjH0OaXaqfi7MwrgAY3ZmfU7OAkCMdb7SzwIIsSbzLV2wAK2NukIOxrp9ZtXX3EfxfMuNI3Gz1O/gtpPRJHvhUjfIXZckszTVFXYPjVxM8FjPd1P3aRkxqdOIxSTfRTMnfUUgSjzLYk+NWHTHWe0c2MTX2nWAyzraagYLUvQ9TRZSfoep1GK2jPI8TCUWMxwwnn7yg4lECSDGO9sENIMsMQddHiUayTWgDHYNQpDb/W3eLF4CCPKm8d6etZWqCMSIR3EEJwPc6+ytm3cct7MywP0+ZpXd07X60wnablE87DwueUlS5D+56Jl32uz26uPHq6+8Dnr+lPn17m70CyHy6BlVDIzcuFFn4OSI/OHRswIHViDrMrGaGbZgAMu57S8amdT1XNw/MAn3kLvEPGbZJQrDuZ5uYslPlzZLSHMS0MPPZdtcxsjQbNQMht+oIxo1iuHbJ8snx8gm8rytjKLrarP+gY26FUUrO4qko21HPologGIrDWOjlDHaJuhoCMAKq9RlKEYOClLX8hIQa025zrj5+PU5MPZqWh3GUGCHuZ8UfWN1rQ5jvuUGEIYqnQ4jJNvIF7mOvLcbynnaFprj+d5cp2+pLjaU4wBiVSkN5TZtQ6xSh6E/qx5qRt8tP/JQdpmAkeyjKIxT48+NM8r1DkFW+16qZoskthlfy3ll35SkVvMss8cJ4IZBYf4D2lh7L8V2cRjk1b4hLchlNtPlPuAGI30hZdDeryw83ESb+PRk5W/XJ/mf5K/o7PR0sz4+Odv6F69x+DQMPXuHC1tTBP+Gm6Lmt/jlr/IYrPCvKxw/G///PkLx27PVyep4Za5/eH18fv7m9OSHsrF/54cO8t46KLFjNyIx+9M7o/1d3uLUUgB/986I4vBXZKf474PDg9vrmy+fri+v7/5m3t59+3D92bz5/OHbp6vbg7cHf/9XfjQ7dYOr79k4APdh3v79H4f3BzFuM5+Qgz9ucBlBh+WDt+E+tvPnioLP9L1M8oLI6zksf0Q+zjZZQ0xvKq9KefVQ8OSf4p+zl9T5QO0iqvbP2TXoT65N3LC4qenYji3yHPL3fY+195LXf877FNWvJNHwK9p7uH/39v7gHX0Jb29usi9fffe9IHlLv31/f39/sEvT6K1hPD8/FzkEZxYjSYwv+UMrlPUW8ZOvaNpnwfD4N//SdbLPe3uV210lKN1Hq71NC0lZMay2tp2FiBy/JvGn+ywLZWNAso6X4NJBimFuaPW/yf8b5KEyTxXR+VOWHBQLR5go/n6om52sfRpiauPr3ZV5WbhVT2jaFz96bvCIYlKVrDyn9kv9lZn060bwVkbqeY717G7aVmp54bb9CJuZbOIjIrvQtP4MeiISO9wEePmtrUM/tyy4YZLGyPLpEx36rSf6NTJONztLanc92EjBzGd9tkuRSaMFZfQ75JNmAv3bZfXFpCGtcm9RikNsk3+7hKxy78qOyyof/7mofFqE//dL3aqXd9ju1R0yw9rDcpBx2BytHdY704f1keohO3w47BhcHra6/ofNIdphvReNP5ZTA2bWy6p6k5c3t9e3xiWZFrimU5+2n7j5qpVb1cDyAviVKodl9tHIhif9Kts/UwjqR3sT93qfflAIm/5WrIrxAn/IWkjSfSeTM59IV/7q56+nJzcf1ydfSr3u7ja0tmnZfgQv6nBfvLzmxoxwOz2KLh69wQvb/h5eM34JyaEObplUUaYbZmBlE8Qtw9KipItJb62C1fUtsxhNhvBJjOVj9E9wWTRCPkOpD665ifZoB666jXDXZgRVFz57EVETvrJ1T+Dj7zoWvKiHC8AooqYNn6/wqBEF+XWsY+iSI9mjaY+XHGYC3rvB0uO0aB7aWzE8rp/Al4zIDuBBoxi+T4M1x8hdRDZxt0E1moHTjkdoHuPQT60HD752iFP+kElJ1BxLN0bwOTcZ4ZWNU8dkqmOkbRrzB+7SovtkjArxaYy+AhE1wwi+v/DshPAJS0RxJQ5VMfp0Eh1GLt8YZFa4erL5ZBhZlozTfcTK2hCyLViOar5hnGzaI7P1VpIgfhavh6m2nIuHQfSSZ/EQOL3SPbd1o2HIam8VG7EUKFdhC4tbXDtHMaLbGQUtt1TyVT290LiR2ArM5bVEaDJQBE4yIN+Jn9hER3bIj3Yeiqy1ZmHIHwpBinValaA7y1MMZm486lxGNnDhul0wHNnkgP+XBb28OcvmdS3h0OQtNhazyTsy81VLsddaF6rirhA4izs3HIlylol8wTqlFuSBX9Jrz9s7l1sr1wPwe0jN55nDTOLBhKrGRpBYoNCxAYQmHjuCqERIYK6s9rzIPFAtgMDosPm8Sjxiy9/sAzlTedMqFURwEaYKk3iuKZM9SamkzwsVyeJ5ksmkApQ+eIWDIEmmLKvIBCBZReZ5+v44QRJk72OX7AhmdmuwGxmK3Wp0407+z1He4TwiFwN5KN84xXuDwpZIxrBQYlr8BlJK07Z9eE1yxSe06HYEUHJBzhiiJ+cXY8ier0/gZcuvgWVdk/HbDSVNSnHpmzMBK8aMLPUIDCabX28MqVh+t7NAdbMRWXZnNO4bxs6zxW8eZPWLnAYrW8tpqtJEIj9lmVdhhY9B1hkbhDA5EFdeeQ0l6AZkLw/u0KfxHrdvDuuLDcoGSefR7ewfPNcufqgODEJoE/7i6CTZfxjh/B2kJukBkoZ6bDvlcU1wG8GWOR4MpV59JTh+kRFPUNEpxsPZCAUOCmwXJnWYmpwpuwCVAq3ByDdumpjZHAeYbGeOocmf+aCexBJ1YDSJrZ3AxAOMJeqEHtxQVewA1dvFbhxxsmEbWjpbbQHU7KsluCbibIqc2UJfdHQ7dtVzqxwhMXIuBEhKYHJFUCnAf6pI0Zqude4lWaskfYcaOR0DJpbFUlCqOK9QpFbxWTSV+sKbcSrQP2xo0LiUDLJxaITPGAQ18kmQclElXy3KqgTBSHQKRHFILkQ2LVts4qMmRWPDsshFpi7QYOmWUj+NDifWnUqyzoa1JKgzYC0N6qxXV6P0o6sj9MCeLVBXKX3QaqnUvMPqKLGeW7V0bO1IsW5P9XUKh6R6SqjmKFRLq3TjqaXi9+7nUHSAqafEuqbUVKp7jNQSY7w56uiUzhb1RLSrUuquUEeicCeoqaH/ohlXfHoyfWuyKo7stGQGJt1kHcHpSBSO2rQ0WB9qOkKdp9HV1Rx9hXNtiW5vXTqSDTdaWlKVjysdGeqESkci6l3GlvTjpKdQeFnSVdHON8XAAkoHpIUsxQr3LVpijKsdLZ2WAxwtNcY5jY5OzW+MlpB+41k4XtHUoG5RNFXO9QsY9SqiKVE5/NAWajjh0NYr3WPoKlWOK3SUaj4ltIRKjw86KsM7mKTdJehqFK4MAHS03xTrAkBFp37Nvp5Cz433QKL1S6o1RQd366ld1a4pJLLYCXd7ObA46LspL/bW1NHtUPXdgK2rVdxMratTXhitKTS0M0DtfmUZIc99iK34xejfTsUPmgRr/EvSt11ISCCf+lUUoFO+qqHzqV6t0OXWGNX0fwhMf68cmk6A6tSJpdTQLgDB0OZzbPVto5OSUI2DUi1ehWbnOFVjwYpo546m2pOV4OpQWa2Yd1WMWz7BqBxYOWfgoKa9j5/UqyqiUG5i699KK6ij9xaKeU3VqNC9mnhU74FoKEajmotUhPD7tjcJhNSqJfwoUezvsQpK88qsABnqyfdsWAXcX+s958oVIFOJiukfWc7AITh+8N6DGPygfafPBEJqNU5FcNUoZzOVyra1elnRo1YfqwiuGvNiNKBqvjYTqirC2ycuoTC0MVNaRjVRmUtntbptbSEAIs/t33srJdR/GlJGhg6aSfGNUKzchRhQ1GfU6Bw1VPRZBrboSsvo0+j2tlgp+WW8DpX+fcWyKvossRU4IR6D+pFyN45VU+/cN1T0WTSau5oIAIkXpvwzBTqKAIz0OA4A2+DBHq4SfvlkeGXukKdTM9I1MMXoZCs7eSOkaj/bbEycC+hGhOz4dHuPtQuFx5/Xp9r1YKX0iF6U41MtUCm+m2wtSCMszqfIUu/MlstIeuGVa4Fy2UgjfNYd14WgIuIkAl7dAGQKT3ZQUjlV5ctoMZe4l4dgbrOPk9/k/o9OVy5fPtwQPy7vfsTpcH//iq7/vr8/WK+O7w/wN8W16Pirb3c/H72+P/gR28RGqU38CPWZI+hGhIjisLiOTV9ubfwvcY5OtQ4yafwA/i/1alfp55PdtWeKJ138du9ZH3FN3yr4V0qB8NvJk5h8mV2qQ77JrrDP03NQvMNRDZh2jwsbMP1ODzhg6l0HicY3U/e/A2ZlwOUQZExoPa2pXvkMrL6iq4ZfrHSHPyo4BSsO3nffq28UhTHnYDxGkUprnNqkx1eLcn1C/X7wK5S2MyH8W1n1V++o/eJE8kCnjyBIA90ehiAttBwQjScOTN7l+WhU/bH4236ZwMzUKynWkRNkTBpWOh1HgdkbdOEFZqXhZ0xFd46qvawXJ6vbB7vGyvV7YzjRWc83ghRLDx3P9oTIb2ntfb4nFHOpKydoj0B1E6xQ+D6M7BJZCYXuwtt1B22ZM5N077jhWwO/B4i7bduFTo618xpeCNhOYXHajoIu+TKv82pB9232+1HSSaXr1iXQTXXNF9vhogmWt5QFBM2vkoDmdGxgzMqTFDRqpQyLWzioGoG3kIYDprd/ArNSVUBM1pcWNCyrDYpMt9vD81JhQNik7H6ColJZOFDGnRgwK6MMiNvyUgZN3TIACk99oMFDU2E4WDRKRYagKzLqrA0aM1eFwyz9vwGDlrpwqKVTOWDUUhcS1R2j5ipkYUHNMfqxlTAcLPWpB0xKVQExqesdaE4qCwdauP4DBi1kYUGph7MRWKkyIG7lpRAat1IGxy2cH46DXKjDY4+TL2ri8NC5x8aRoHNxSOixRmSMMiRu4WISnLYQhoOlrkmASakqHGbhCxOYs5AFBI3HmJKhqqCY41RclTAsLOMIdARiRh0OOx5loBhDDxRZt6XQrIw0IDB1hwoNS2VhQc3RWM1RcKnn1jF4qTQccDJKAUugC9hYnZcRui6s59oxaMHzbOERFxqWysKBll52gUlLXTjUp3HmD57A5w9Yd8AjwBbScMCFm2Fg2EIWFrRwXTwCbCGtC8y6RAbBZAV192B0OViGgOwUHmULhtCjnId4Pzf3d1ze3F7fym40uiQnFZR2GMlvJ2m8ZttP3PwyLNcTaeWz+BkEuHyhbQmJt9lFw1xvpwjDKCizpL8V93kpgdSDa6bIVqRL05saW9leS5OBbLC3/TM1BCawMoEf7U0r9p9+UEKohRZhmKPioO4zZKsOZtu4Wg3iBrZ67dHrw2f4HUm7BFLOOD1+gUDwCjEoOKGFeTmvRVBohesiELZCDKwk9pWoGC5nMw6X5NNg2IETYPaBJKzkQDMRFF4hNlV13pPHipMdanWvbg5lfVWJpeqQtyud99z0eqVHU6iMXUPotn2svzLJGLttj2dg6a9NU6jMXLjykeI8Ravh+EwsPQd9p+m83l4vahpcbbGlF7eGYzypuLudvvXg34keV1tsqcMV5Dvxk+xgJb/ccYZxCrJDR2CWLYsUvaw2ezNluJlrQnLnwAz1YOOqA5IsZn5ckl/35GlJFLJ6Z1hKuRxmsjvLMzcevR1NnqoWXI8ku31TCaIIueg6mH2JoqUpiyYpS83AWimtZF3bKskqavGmAaHyuSpDFRwgnytBFCHHzuee+6Cez3HgDPby5iybPLUkYorDGh3hl9uO81NKuRJpr3fZO1dgISmDyhvgKohQCrYNikyq1e2JT511mmN8qchZZQIqGCce2uXjWw+lFmcrwX1a2fhWgdSMPuwF5plrFmkIxfdaXksu807LQGpGY8vf7AOBideaVSaUmlnh5beaWcklt64kjoW66LUEjsV7522DgudO6yalzpT2GVWsI5ph1RCE1hlqhiVWE9rmxM5y1ezJnNNqGxTaEl6zJ7Hdu9Oc2uusB1Qzns8bSBquAg0bVZy1ExxxdrfVAoMnEhMybqpCaFQ4kvZoEK0KR9JkGUi9tMtZRDqJmhVdOXtFEPXSLmePhtAqcHIWq0BaBU62by803QbVuSd3wBGvXdyqiJleY8Io9XiFu551m1p9z/wuQKHOYN2qVm+QTBGnVirS6a1brUItqK6v3gG3HNHY0KWLKtQYsUnD0LN3lqs4Q6275JRfhyucJCVtkThV+EVPIlYX/8ZoKzTOaEWY5u2WjM5yDk09TRhdiq1tm1GM6OWB6jRtnaVOQCXIxvFLX2SbKfZOVsXyWl1nSe9a1Lmlr/fea6kskKsc5TtdjohDd49csii4spinpMEmjdGOJb0V2Bi2NeWleHI1Z1/ZKZzn7azYebZE+vbCKUZr2C4LumcAMm91sKSlpu4RgFIePikZYU3K0m0XNGVdGIaS+pOCp6TCmpSlgZ0FjNlQ1s6Zrln3HQeaO5viU9aFcn0rXrmFazSKbk4lrPkOcbkqPL4CIjKquiXW9kfgY1R1+Yif1xEAGVndUrqz1iMQsrL6hCfnF+MwMsL6lOfrk3EoGWFNyu0o5WULVl4a1Tp0zdgUn+9EYxH79nCHEOY+MnVGOiC99Ty5snPq6q13FZ9aw12oAmQY2rsiitQSLG63PlBOp+5QRfcwyyVyQxwWmc6VjgVdyQNgJ6hY+zYdFKHAQYHtag1GOuAHjMAMTOqONGHAO6UhC6UVJBEehgdp+WJLp8sjFNEBa6NGaie0sxUmSjvpbbAKEaKeeqeIDzU1anQKv+NTxKewpT+zQ8y6aWJmG2MB6VvCyx88N93L7B88V15nKKGfLG+PdEbjTDKT7maOaLTUpbOFbIr3JVliwaSXGxDXmZaH8/4+SZHD+j0GSrrEMoasyCZhz2t3zWmiwrMEEp18QhmUu5ScLMuK9O3BotivP0pHGZS7U37EjjIo/IAR8I4yGHin9Dh9FrI1mCzxASc7z9I4kUlfIuj8M2Tl3++Sqv4fmi048eIt7k+Pcfctu25PZ8/Uluypp3T1zSdDq4Rc/+zSK4FN40mwxspJ/8o0H4GV0ALJHM3rgZQSyiAPVoIuztQpqvDql1sNTxlwEVQmAToRzOfY6l8cFuQoRZRhhlaouRDSq9Bt4yauq590SmldQwPFSfoXwQUoaHD1O8/6JxW41n3JSYJmsXS3ZFuuerEswysjhK5G9GlgZePRwA5vrvFIZKf3kPFHOznXMF8E1wDQsa5pWrN9ZAQ0IDTbRkZAA0KzPWAE1OvAfL+bRi1YCehCmFHoedokpYp6Y53GeMzw0L9syW+pGQVljB3ZpqHHUZNQ37Sc7XRQp6jCa9xPGiSpRUZqrq9RbFsy6lfH7mJkESfr6jA1CfVahA6BNaoRRkEbwyTm9FlKGfUOTmD6+/6rPPgdnDK8Zh+rnLvVYGkLgYxElIEaIhrFenBuU6A8y09eDtUsAO+qXw8M8clK3Kf+RQZpxEpPa1RXrlEMbRETGts1lTSxdNOrqQLTz1HHaahoDD81qwFGQB0iSnhLHHyOmoYWyvA6mBCJwrpXBwjpGwwsVwmRVBpaKGlsDTiwECIpJdQ7GJYzeCUVv2tRCWgP2dQhHvVLTDH21IAoBZQh4sQitbO5Q16EBhwKcmE6hNQHKtkN0g/7rT5Wp5QWmO25QyuKQkSVhhaK7w7cnyIEUihoYWDR9SlA+9ytBYD2iF40s1BdRxnp+/nxG/VBVRFay7xp49ZEZ8qsIaILozHIZBV0Mfr30opi6N0cnYskGqtbrIIWRtao6CdJTQYCSDdxajLqrXZ5dKSoxdUnSbqkAMAgiKDSaHDHrUwKyW+nHXhxmgvJnVIQYHQvF+laZn0VCMIOTQjUxAtTkaMIMqgdmiCodI8lCGKlBYGmtYTf1oFAGjzDIMMkf0ChH2roHIIMk/Qhg36kWGfPQVsHJKMD5XG97G3vkP1Y3YBJbi7RmOPsFgNrmtQH320hwKYJAquSAgEbuvNVCkviJlgeVLNtg+Dr0ISq6SHwqA5YTQ/BtNO9qZ7R0p9G6BEDgUth8hjVgWoZIZB8WZfZAy2jFTihb7p+pLE60q8H1XiDoCWSfrsHG28IogQsszd74yB0bU3IHj4IotyJLM3LGaVdQmvs0R9IS2vgcEZv4rmF01762QI5vGPFrkLZbLFQFV2YJFgDwFAVABjFJaMuIoXFo04surNfm6nS0QXK9zTrA5U6ukB713NwZ2TTf5eNOFRNS/egHP7H8yDKG6ukf5sXAI8N8eLsnYX/e3IMwMMogUBFodd/Ba4kVqGlDZaN1RVH/G20upounOLAuoWlMKruBlLqubdwpPvt3TCZc4ujk9V3AKSaljYYef/k+CcEGKulD8ZsQYZgq8vp4hU71/XJGCVNqGwjhj5RKaONA9DpdUA6vc4OoM2jIpooyHYUZoWaLIWKPgxEyS9ltHF+jaxHgOLOCGkjKcwrtnEkJxW7UegRIn2cSkgXSW0baAtI+QqUOs4Womu7Bena7h6djT5LoaILU55v0idipTSxqCZYl7atpwuosIDVgpJcu+oBOYcgOYdAyV2NPOw3G3ILm+eFAN3sHlFN1AClZhLaj0hlWaGJ2BDTRXOT1Hx8BsCqhDSRyFFxbRwqookSDTjyE0aJRBz78VEUttK3SCR30/eA2Mn6BAKm0AEAAqiVShldnOJ4pj4RowQEBTSKbclB4WXHSOHwCjldvNB7WZ8eQ2QyRkkXKnafrBSZxEtQAtCutPV0ARML6n0ySppQsYurY2d9ATDVW5PSxYIY/UovUneiEF8O+iyFij4MuYoAAofq6AOdQ7R+lY4uUAJQwKiIPoppW/YOoIGpSUFgZbP5yd5NIeYCuxQhIMPw0YVKvFILACx1yXADBqzS0gSrruPQ5qpJ6WK5PgxTqaMJRO9e1SdihDSRsgNu2jyFCgBMdroMBKhQgoECKHOsEghUAjDPyipJ7AvTvoxe5VL05oLbsK8ibhKQm7ZVXBINwJhkr0jqBvlNuoleG1fx9cjCIuNRh++m7hMwcl0WBjlf7wbBrKSg0PQmrmtoUBsxatvsVa8M6SCEvgi9KRuk6HuaQBemYflxogBcuIblYaKAhWONoTBDWyrBgOlP6zFsgDN75XZwnsMEGb6aGhCg+xtQNiyVgMDSeG/DVOuVFAza8GX6MmQKF+YPgXGc58iQqXjKmfgu/yIlmk4X0XYr6LaxHfYlSZH/5KJnnUMBt1cfP1591e3R5irm17s78VOledyNKhpGrmLUxTRzWy4mns04VAD5n4keFFUhttjsz/u5lbddz8V9BNOzHnjnVbrC4lDZ2VG1gkUX8POjR3I+VevTXh7WyJb/B8sEG1eDgTfqHEZNTv2mo1JlMPPJQwlmv6kdo3R91TgqE0UrO4qY1CC/vzX+Enp7HyXGxxihwHBia5Mmhm+lKYl2aiU7w478xNiiAMVWGsZGKWT0G6NDIxB7rNaQyRg5KEhdy0uA7DYFB+Pr46ziQFmuqQ2YRYEd5tcsQ5itqw2Y9S03gDFZKQ2YIxluB5NvdyK5FsBW99TAQI6FsFmXE8mtIDGttERyKoBJVmvA5J/bh/BEDaLvlh95KDsMaxSe0v/ceRiv2evJ2pBL9YxE/JBWvr8yNaMpWrNPBwpfrHSnmbCAwkUCAkpmjtu5amwLXybjB/Tk2ogkI2nsP5Ekvfr56+nJzcf1yRfjujhyrCZ+eXN7fWtckk6DplKC4ozTyXAz5+RG5hNWTQ75TvxE7xHX0yGlQENAH6FIGTdM0hhZvp5Wjz+9YityUd7zOnx0M/JR4c0ZQMmV/up08h9xpDzK66q7PxxZ3ijcKuulbN/4VkVXcOiiWWjTMPTsHe6lQRTfPN4dSr5lx+EHtCFVnhsGVXN6+e327vONeff5/1z9Yv731U8fSFrhn7OpMNKT2Sdp6B+l4SNqu+rsl61q/7PT05/XxydnH28uXouGvv1kXn6++fL5l6tf7szLn+5++vT5o/nl69Ut/iyqcfPnqw93n26x0C8/X380f77+dFWL2P/y0v9iKouiP/knWf0vtz+Zl1//9uXu86Cp2mVUkqa+3t2Z326vzJ9ub2q6x8LpWU7umLdXl3fXn3+p8/1zH6b/VT2Uf26I07rq5/okQOrYf/zj4JNi9RdpAPH/yN/m5c2ZubXtlSVCgB8UeUzoIV/koSDEhazxICnDnyOa5uTDtUN235Tfrvb2qiznWdKH2fdDj+FB275eEnFbhr4f+WdTGN80jG+ip7OjpDmKHMe0F1qpaT24NYAk3KQbFfukzcEvMB42Xzy1imLUf0NOZvCTm6Sl0RLPI1nWrEK3qkqjMigbBStJkP/AjUP52CIj4bnBIy8G+TP0n1s7dqO0lgn+EMXhr8hODWufhnhUZeRPkgHOynPGzxyFz2oTd2oy39U1ujTeN3uGIzA4VmqB2pd4MYEV4ObLxPXiHDEPfTc1NzGuhs0ozNz8zAARkMV8G0VzvX5sP05Td+IXT2cw0I0VZa3h9PG2TXJVhWPFTr1RfPNmAtvfv/dY/+Mf1+vx7T9bceAG22Rled4MSV+az3axzAkQIccKUteud00sL5kSwopR7qYpmQODPOG7v1mZaK2D5P4mZt63HlHWYlqxvyL97NSKtyht2u95rNUtPfLxN+/lOqeaCOlu7z80IOh34xtv9oyPfPzNe9o/PnLWF5NAdPaRMQr5/gh//16iv9wyUlV8XJ7q0b4a+giPEd+LVtMD+nQxTAyGLKD2Vdo5kHDNDYzU1X4fbYLwKP92FqCeDk2Gxf42XV4qKluz2eId/TX7ZtpkGpdGKV3aTfHRX+l3M6XNaEQy6dPbOh591s6/sikzJotMmvQPGI825Lej6rdpE2gyMJnUGh7iH22K32dLtckBpcrf8BD9aEMeOMoeOCofmLhYTo8ok4LLmLoDTO+FRqh3TrX/yaVGpXsep+ex/slV9TlVTaJsVrE1p3iEh1PITt6TX1fZn1OwFBNdZv7Z9K2oTvV/qd79q6MbK3r/h//4/O3uy7c788P11/80/vAfX75+/p+ry7tffrq5+s9VFliAOV8cW7k4Z9AJ7iYu3bASRvWeBNrEpyf+dn0SnZ2ebsgq69ZvrbLqpZOb2FKZ/eiv3uHR0dbua35gqiF9TE7u4oZvbltRi9cuTNJpIyYHirFW+c6HLHcm7ulJjuukq3z7g5PdDZ8t3622wX7F1P7kbv1mfBjBxtP5Q6vMOXCY7lDs4ehN+047t8MPPL/xUZLgNDjyULBNd++b6/FjJzUZwcskNvv8/+/JLdEhkC6C3/GHUu7o2U13R+0d2KOXQ9qicTKGrJztxvbes2IHRShwUGC/qC0ILCdGAa6DnVb3Q3w6X63gdje3Oloyr+adQTsa2adX73787nvk0fxWFfzwenWcBS52guOvvt39fIT7Fj/mAkVHpdwXs7dXfujscYlKULrHo3u0sfZeeovSNFs2KTdBr7ItOzgglohQnL7c2vjf9+QSOdr3Mcaj2ttf8kdkeGqb0RtnvGivuauCqO9bX9mxXWz9suPcIr0honw5+KuiA+p0Fyb2EFL9JR4cHtxe33z5dH15ffc38/bu24frzybujn65+np3fXV78PbgX/cHzE70t/f4i3vc+bSekHObhvbjX4j/ogeyp/ye/HyfP0D+g/Nl5OKnnMdPoZ1PaNEf3hZ/bJH9GJqJ82ierU7wa6JfHxZ/kO2Zn6N2uJ4uLP319/z/sMzBhzw//ZuB/45fS85A1s0T/Bb+Xr0GhIdXb8lLPiwzW7bVk0QxzynZduZ8t2l14AU/Hcbu1g0sjz4e7D3vsLxdCX+xPswUUhSQBHvz+s3r0zfnxxe/H0oZZ7YNkqMRSraP1j+8Pn99/ObN+VrSerYXkNYiOrFfvzn+4ez89fnJiSRAx5ZNpQQ4Ozl7c3x+8eZM1H4xEscvnzmpYNoezuuZL235NMBv4Pj1m5MT4ZfAMKAnLGLurMChs1nyaXB2cfr6h4uLN8cK5osjCZQgUcsEZ2cnb87Wx8fCmWCIQOkVHJ8dr9c4M57qEGQZwc3cttjKSfH6zQ+v129OpUHqe6VN+rViqTg/Pl2fX1wc/6CQHA2QNLaCJLtjVwfpzdnZxfHJG+lUaVUTeglzdnxyfnx8fCFdUhvzdvL5Asf99BznDWnLra6NvO3T1yfrN68vLs5//8fB7/8POAZp7g===END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA