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