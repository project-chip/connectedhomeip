####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /res/git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.2.2

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

INCLUDES += \
 -Iconfig \
 -Iautogen \
 -I../../generator \
 -I../../../examples \
 -I../../../src \
 -I$(SDK_PATH)/platform/Device/SiliconLabs/EFR32MG12P/Include \
 -I$(SDK_PATH)/platform/CMSIS/Core/Include \
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

# $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# 	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
# 	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
# 	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
# CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
# OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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

$(OUTPUT_DIR)/project/_/_/generator/nvm3.o: ../../generator/nvm3.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/nvm3.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/nvm3.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/nvm3.o

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