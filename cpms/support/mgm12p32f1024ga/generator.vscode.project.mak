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
 '-DMGM12P32F1024GA=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DCUSTOM_TOKEN_HEADER=custom-tokens.h' \
 '-DMGM12P32F1024GA=1' \
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
 -I$(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Include \
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

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/startup_mgm12.o

$(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.o: $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.d
OBJS += $(OUTPUT_DIR)/sdk/platform/Device/SiliconLabs/MGM12/Source/system_mgm12.o

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

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_hfxo_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_hfxo_s1.o

$(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.o: $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/service/device_init/src/sl_device_init_lfxo_s1.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.d
OBJS += $(OUTPUT_DIR)/sdk/platform/service/device_init/src/sl_device_init_lfxo_s1.o

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