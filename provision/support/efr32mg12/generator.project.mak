####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 14                                              #
####################################################################

BASE_SDK_PATH = /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.3.0

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
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset \
 -I$(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc \
 -I$(SDK_PATH)/util/third_party/mbedtls/include \
 -I$(SDK_PATH)/util/third_party/mbedtls/library \
 -I$(SDK_PATH)/platform/emdrv/nvm3/inc \
 -I$(SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc \
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

$(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o: $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.d
OBJS += $(OUTPUT_DIR)/sdk/platform/security/sl_component/sl_mbedtls_support/src/sl_psa_crypto.o

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

$(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o: ../../generator/credentials_efr32.cpp
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/credentials_efr32.cpp'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ ../../generator/credentials_efr32.cpp
CXXDEPS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/credentials_efr32.o

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

$(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o: ../../generator/platform_efr32.c
	@$(POSIX_TOOL_PATH)echo 'Building ../../generator/platform_efr32.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../../generator/platform_efr32.c
CDEPS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.d
OBJS += $(OUTPUT_DIR)/project/_/_/generator/platform_efr32.o

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
# SIMPLICITY_STUDIO_METADATA=eJzlnQtz3Ma157+KipXaundDDkRKVGxdya6YolW8V7RUJJVsKkyhQKBnBiZeQWNI0al89+0GGkADaKBfBxh6N+XYnBn0//y60e/X+dfBl6vP/31+duNef/56dXZ+ffD24N2P3+LoxQPKcZgm728Pjlcvbw9eoMRPgzDZkC++3vx89N3twY8/3Cbvsjz9FfnFCxIkwW/jNEAReWJbFNlbx3l8fFzhMPLu8MpPYwdj57rYBWG6Qn6aIyJKQmcoL56uffJfEo6p3R4Q6Rcv3q3TKED5i8SL6Y9+mqzDDfuN/hpGqP4NR26M4jR/cqunVluivstD8hN97K3zlzTaxQg7Vwinu9wnf+HCw1sn9ooC5Y6fxeSbXZaleeGgdf7qJN4cnziVmCNSd0QY1+cfP55fuVc3N+4ZeRKWQiAuhEge4ldugNbeLipmSY4RA0KYDHuunz9lRToLilBeCILiKLxzab4j6He7zSw4E0aEUDRj3aGgiPBs+XYgLwMJ0EPoo7l5BlYqrHdOVeZF5d/bFekGJeIK4OrmnJSKOEsTlBQYCJtZdIbiY4no10+5vld4UQqVgDXJmIkxHvRAH9x6SRChHJ5lIK/G4c/L4Y9zhCkucuTF7FnojDJiQZkGPGFEFqQ0YRIW5F/EVOLPmkRDQyNsYa8ec5kaONuUIVW2IvcSvE7zeDlKoclR3rLRnDsdxUYmme52YVSEyTJs48aEjFGY3KOcfrOKAlCgnrLQ+srP/bqtJH+C2h9oy1viDfLvUxcH9+7r1Ss6OmiYO49lkVfQTNn8PnjiQ9kL4H4fPHEdRiGB+0TGEJ3HBg+e/3z16uTy4/HJl95zQ8kykQZP9TNl4eXFLnObBMsGtfNXTMZITh76HvaiyLkO44zAhmQ4U45ynOsP/4OdJq2cOjmcKtYOFzWnpXcqPEds35FAP+ECxftjFpnvI3cy1uhLukj8aBdI31Jr6uTVyfr0+GQTv/luUGPME2mG6IwySF5WG871/DjbG3VjXYM38PeIWxnXoF27Gell7JG4BdCjJjX2nrFrAnVuP97tD5kZ16DNn9IijAWDo+WYeQQt8qxI94rN7GswY0F/bjli3O3gyXnpLEWy3mMicwAa1LFHAmI/D8kL2mPGHnJoxSFH/9wrO7Ovzoz2WfMh3ZoPFfEeaSvj6rTrbIe2++NtzKsTbzIymtofcWNehzjcY1VXW9fjdffYb27tqzOHJ3vME8y4Bm3g7ROXWVfnjUi1vT/e2roer+vvsV7jADSoEelIYbRH6hZAm9q92w3XZZcmryH06feaWToM+uy42D97xaDDvufRIQegQ73z8r2mdm1fnTnGe2xqmHF12sxP9pi+tXUN3nyPE0nMuBbtXmu61r4eMw43iRftN6V5CHX6fJ9j11x37JqnceHdRXvsgvAEGtyFv8dKrraux+vuG9k1os7RHutnnkCdG++zDGLdMrjnfpFBr6gMstfszBNocOfJcB/ZcszMujrvDu+189mYVyd+2Os8x4P2PAcN4abZHuc6eAJ17sdAsCNyMebauh4v6WzvsSfKE8i4Y7ardmla3q72BpVlWYX2lXaoCL4cfDX8orup5ezy+uJ6arfTWXksYmqb09i+mE4q+zEOcblROBRt+zVN4hLfoYxNeg4tDRJzCMaOlszPxRmaxCp+oxvgCvQNrtUSMHWtKKTTBrCrMppGG3FvpItDDzT48etZaTgbkzBxtnO9PH7405w0HSMOeEWA4iB/mKoISImK02S6KggTf7oaQH4awDUGJbNTgTnEttPI66fPIC70SNF0bHEuiW3vVBLFdKu9tWCbIKskoIYcgtM/B9WzOJmJy6BbL3LXkYe3swJ2rMihotS/n5WnNiDNNfoZnn8hwPm+jAPN9X0b0hSdE0QJwN2gBBFzs6YIZ0Mp488Kw/R1SuDMOK0VxRI4J09tQL8ERuHddAkkD5Qmzi5flz0LDz4axIQjMDNHI92PrrQ+6o/H/G0IN8IpeaqWt1V2pu0Dzih2zYtmCwXWy6GBB1kZdyE4/QkWeuZnttToik+niIeJuZlSo9WeZrjbwa1PdQCYsCRPlN2kmfJDoz3NkHvxepdAtoccBCc+TQE9Cu5QjI58h+8jhxwidN5GLhodCOzP0EHpcYx3TwY8sLvquyQjO+bFDPNWnX0T00SQO5s7HMJdy33roPtSO+bFe0779iF3pnTMC3edCKzPmhW6+tMs1fTpPByttgrDvHXG0IYz1ZNUmKwQ9czgRtkUnQ6wW2GFWn8e80xZqZadh6DRltdpswAglTdQ1jyzmK+V5XXaLOaZsFI9MgtAqz1ZZmWjP8EMrN7wj15LkuzgKktu5pWTnhz1QI83ughKA47qwhrILn8XQqnPT57DhVcADny6EK04aDPRpjFUQWHYtKR0xPWwizSN/K0XStYnpDP2zbVq0PFrAOuYtmZmmHVub4fL0QZyRDeIDctuA2uTc4pt9JfhUgHa+L6b5SisVttmBxuag58tJMiyu0zqe4/si02tBFZwGL1TC9cFhzcke6nNtU55AVdjycBqY/OU6yb6UDl0EB2WQ3lDWum8FFhtDGCtt+qg2RcCNkijl4iBZ7dKu85sPUuyF8Qez/LURxi7nl/QenoRxKHNeQoGnyLQWZBFrO5bdS2Zpf0iiEObc7QyPolH8TTVzPB3REpKWXsfJ7sfTHZ3Vu8O3tH30N5Q5/k+ilDuAb+HKhk692E6w+i0F4+OA8l2LHJB0zgJST8CI8CzXzZR6fEoxITefPds3ogIxuy2MFFzIRqOF3maPblbLw8ePcAZMOXIs8paBCLb3JvnKdxGJi3gxrQ0f9XB95awnH0JLFsA8QSXri4C27WvBot8uPv8TGCZfQlsI7D19kPbA5Dn2vYS8b1l3C6CYkW+X+Y+g1nNLepsjtcri7dW9cxGa1/ybkiBdj3A3YdapJxxWY3ix/vD5IzLMGPP3yMnZ11WJLfe8f5Aeety0JPTN3tF5ezLYU+PT/YKy9mXwG72WaQ2ykWq29jsq0LtIui1d3ti7jOopfODF+0Ed+gvmc4tgulBN9HEAVUO8vAB5bI5A6XBWZW65Vnj2fszLXqnK1MbV8iNrENMQzClvVCLMRRLUxUAevpUL8l7DHrkbOl3z+wthQI9RvXOTDdAGUoClPjhEkNQQRwmWNSGo7GXeBsU08mnpfmFBDrl1ktw5uXUaUz9GsNsu+9SPAFlFbct5BEzmJhtR86jacWLdM+fWbQYkVWsPOQFzyxaNZJalycssFseTls+EgP7sJMSXe8m2BM81EuSMClQnngRSdUdLlDg4iLNSZ01f4eQSxvaFyQfnSmY4asVZNpnFRsZkDRG1bz2PtAby4ICJc6gan3QpeMyjmHUk9sHvpBCj36NvGKXLzHIm+bnOSz6ovuIxQSLdl90aX4hgVkPgB4cowuh+3kJMiCzOBVP2Z4y1RTMXFf+9D92uwy7IozGnaEV2zAPXEI6vb2DzfRM7+yIwrvcy5+m9/EALH7SGDkceL345jAAZ2SBs8uBk2MSGlsvyMtpeEtSpkeS05ZhaixNMt15GL15PTtQa2b6UiCQEbOUZnwMLKBxH3PPeo1cEamxNckFsGYv5RlZl+9zuKTqfFigyHdNSagCbL1LQAGIWZm+V8p61C0FiYWj6G4ZDzf0ZM/sZbwxM0mThvMnCrMxyZHZHxaTcmTDQ2NDjnsfn85PUluRsCwAokCxTMvN2ZHwLNNqc3YkPMu0SZyd6Rq32no4f53b2lHhcbM0ipaCaoxN9yiKnIwo7qzXI+XdCc7QJNGW7kdZBKljaXrLfbmfY3ag1ozkosrSmXxRXrQ9/4vrW5u+ZHSbIy8I7Re4pVwdS9O1ExtXz189cYaUiFwquRhWY226b5a48c765jp536wxo9BTbKbA58ca2lMekc3N1rMlqSMgJpsVKoex2eTxGmu5lzluVov2wcPhg/XyjzZta1Y65m1WiAC25imNfPsGFQgXSsW+MfV+2exkPWOScfoydQpnZ5onw0ALWHKkjikpFcgCpxLU6ILmgIn2TOxXKpWgWlNSqiL37N0mKEE1lqZ7QV4AcQusvP/T2lEaxc7Oc69W5urR9/w8jZ1JnpyeZIwKd4uiDNl72pNyCexNj9Jw5AbobrdZjFBoUcroRyHAEq4SXGtKShWH9hcAKjHVhqREJODxq+V6DmKTipT36GmZ7NY1N0n37fTl97MPLmsjUhLXJ03VAnOWPVsqXPMPwXlDKkTWm41VieRXfVQP4vnXQHlDUqKyxVosoTrWVNkWSrKOten+RHOyp24XZp95EllUZFwQTiflIDYu66Tb2K7k0Te7zIYFoUVVRraVj3aCy07TgrAC06rUOEoLwOMgOtQC08rUbK/wkrStSVXKJTaQDM2p0kEcLdHBGzs3MsYHcEREB2/k/McYXb7A5pehOeXysWzRkJcKf0uU26vr6T1C8089i21qtYmzz1IM7Wm2iQsSthaVGQFcVGgRCh1XqLfZC6IKTOu0KwuSMnNa7cqCeFsV71zc84tNvYzYVOYsFs2PzJxO67wgXSx2sz3aOntJkMYuBViSsmdWpy+xJCUWuv6e6EssCIe1ykh/CLEk6NC07uhlSdqxw5WKt8qOOK5WON0ySAjP/kzRaMzD2gcw++wpHUTz8nC+AjhAYsZkTDg5Xo6JGVNgmnfxTgQ2uownPmKzFFprTsZV7ddfjKsxJ+PahVFAeiZr62uZ1Nk6JmXHQ8l/omjBkskblN/BtxyWr/I2/a1H/jl5uRwWZ1CJLUsjaz8WmnS1SSlfOVEw76zEkLBrVMY472h/QDc61O9zzTkQGFCNDAOG20Qzrzg6WX1bjqxjUspHXzo997wgH29Szsdtjl8QsWtVRlmfvVgMkDMoYSu3zywG1liTUi3X8w6Uet7Bdrl2lNmSECE/mG/2qo9UG5MzLVhVNNakVL9m3v1y9QNnT0o23/zokEo4OTpydG8xqtaejGzWjcYDronbk3iqzYL9641S/3p7H6wXQ6qNyZiao36LgfEWJXQszNL96qFZGed8q3wDNuEC34DndEGgUxWiymnh3W69pjcKRlG6XJd/xLaEOEGFi1P/Hs24htIn7dmUEYa4cO8fl6Nr7UnI6KUOS1ExWxKizN5vuTJRJvJjPiSa73zHAEh4xENwucXxyYJMtTkFruVqs8aajKo+0bwYGGdQkW3ZsffAqiplefh6ccraqowyjZ6OX71cMANyBmVsefjgFah0ooaXa6SGZmWc2Fv4JXMGJWx5SGrr4PjNcpPeHYsyugXH7CNL+sMLP5ZbU6yNyZno7SALUjFzcq7TBVvU1pyMCy9XFJktOZHre/52udaqY1GFrlzMwLuwWHACVGRYhTVN78OFk7IxqcBXhHSEsyhfa1LC116esxRex6KMLowXRWvMSbjY7c2LgXH2JGTlKcylsGpjCkzlmccluWqDamzLlU7eoBIbXm6qmTc42Jin5EpC7CWiuwYJ4tZMGifqeEDZexnb8Uf32hRhUl21jRdp+lrMEet65GSsEodF+LAf8q51NfJqF8CStK1FVcJFJuM7hKo7XDrnIma+lkcAqutKoh8sKdC3Au+pvE1TmMVkP+VvmkItJiRgPv/gm4NuDKrxLTYBySFqzEE2W/GBnNPoYHaMKnKGvy2bRRuDinxFvvMXbRVai2qEII4+dABHnXmI+WA8seoA2jteFX3Vc66KNpuBC9b+M0+4QPFDiB5lpy+uzz9+PL+SdVirp9yrmxvwA8JVdJyW2KmMOV2bkjdePQz+qiVwClmRi8XCcLVNsJw4/KKX6UKCnyZu5N3hKXc25DnyRHnodjoXs0X86rSUyOkwP8kUkefKZX6IDMpHxeF4nS6S07E6fTtW8yRENtDnG2QEW1dH3Q98VL0sW/lZ1ovmX9JoFyPsXCGc7nKf/IULD2+d2CsKQupnMY1lgnKvSHOn0XCG+rSr7SUBtjbSFRJaKgch9nZaGZGVHAUoKUIvwi5a569O7A2KFQW2UeKn1b3hlia7QgJLsRcm1lZaEYEFoQdSPf2uE1Hx2jVLUCs7QzGBRVoG+pWFfinaTpchOwO8zHj5sbXRnQkYKzu2ZrpKU2XFzhAvM5HPLK0Mx20dKz+JTz1KbKBvHmmcUHlU2MG7LEvzwvmpd6Kx2/0p26Yzo3xAHfy2bgBLIaevx0xWeWRHoh6mSW37A1p7u6ggRkkYFHW+oT2os3ISLLwjbWnxRGsof7fySIpVZTLeHJ9Uf9K/spNXJ+vT45NN/OY7ErxI08jfksqor0F+I410/1tSTFZVBFbk1xWJpU/+vctQ/vb16tXq5co9fnPy3fev/nTy3XHTzXkXpwGK3gYI+3mY0Yj98M4Zfle1xZ0EIN+9c7I8/RX5Bfn74PDg+uLyy6eLs4ubv7nXN18/XHx2Lz9/+Prp/Prg7cHf/1Ud6izC5PxbOaIhvbe3f//H4e1BTnoTDyggH9ekWKDD5sHr6n3VzxGqXUQ6c29vD94xu28vL8svX3yLowS/Zd++v729PdgWRfbWcR4fH+tEIenjYOx8qR5aobJrSJ58wcyVwcjgtPoyDMrPO39V2V1hVOyy1Vk5A8c0vqS4+IkeG101RWK18f0yfBbEHcEfbss0LIdzdIkJk9xBM2BldvW/6b8d+lCTqHXkfrg9aNOERJ8q/vvw/5H0/N2nZN3+cOMhl9YctN44bH5EManMys4zu724bW/ah2i/gPxc1h7CBzqXCQ1/pv3wwQZz4c8Begh9NPpU1b1vf302uWXns0r595dLvF2REmrn6ubcPas9zGKW9vWPUZjco5y2W6so6PzC+6V1fa/wonTTC0weQQ/05y3pLEXV/ZBTPw9Ch6QQ5siL2RN4KDB4YlwjTMKC/IuemvQFD4a9rOqyn+QPln53y27teBBaUNpVPol+WaoGDz2jPH+DYtpDQb/DXN922w6H3bRDbhR/2IxEDvtj78PuQOKwO2I+5AcAh4O++uHoyPewPy497Ha+ycdmPsQtO1BtP/Hs8vri2jmjcyEXbH7Wj3FYLW2FbeHSFyAv1jgst3lFNzxtmPz4tUHQONu5pD/78CeDsMVv9VKaLPCHssGiHXM6I/WJdtLPf756dXL58fjkS6Mn7EhDS7ueH2fwooH0vetrrt0sTPAsumRMBi/sxzt4zfwppQcHpEXSRJlteoGVxUhahLVFaYeP3WwDqxt7bj1OTOGTmMjn6J/gsmiGfIaKGFxzne3QFlx1k/k5fGWzyUL47EVFXfjKNjyBj38YePCiESkAs4i6Pny+IuMBlFRXJM6hS4/ZzqY9X3K4GLxzQ6TnadEitPNyeNwYw5eMzE/gQbMcvk9DNOfIXVQWh5ukHebAaeczNI95GhfeXQRfO+SFfMRkJOrOpZsj+JyLZ3hl89QxpeocaVvk8nG7tugOz1EhPszRV6CibprB9xcegxQ+YakoqcShKsaYTWnDyFWbndwW1062mhGjy455sct4WR9CdgArUa02kdONhXTC1sMYybN4N0y7DV09DGIXv6qHIOlV7KStGwtDl3Hb2KilQLO+WlvckNo5yxHbcqloeaBSLZ/YhSaNxEZhKm8gwpKBIUiSAcVB/sAnOvJTebSrUHStqgxTbmbRD1Kvc5kELafBQ2kNKgq6rZyRGwRz1xHz5aAbuPaEqxiO7nog/y+Dnl2+LmeEPeXQNAP01hHp63WrlSW1HNEVauNuELiMuzQcjXKZ/2LF6qgT5E5eSXSe97ehtELvBpB3rvrPc2ef1IMp1aq9ILlCee0FUC8/fEClyU5BEJOUUJif6zyvMvfUCaAwIu0/bxKP3IvXu0TPVNWcGwQxeq2KS0ZtGLpgqlMkaE3AnleqBurnacbWCtC4lVQOgjSZylymE4DmMp3n2auXBMHI3+Uh3ZnPbQngN3fUW+aaDR3tEjq9WSdC1f4t2fuzsZPGSUg6UhjJJ2lMzNAV+1miQnO4hzD1gQ6q6fsxvCa97hJadDMDKL1sZw7Rk9M3c8ieHp/AyzZfQ8tyLnVnUG5PmgEqh3NA0yq08VWIwepQTpZ5MAWTra4bhlRsvtt6oLrl6L28s5kMBvLg0ZO3zbr6dekAlu06nAZUDiGkqUR1nriq0GtvcrzbLghhenS0udwaSjBM6LYyMjYs8h3ptgS8Fy8oGzSd57ZDbdQHf+nOwozk7qRwab+ddi3MKz81O81hY3AbyYY7rA6l3n6lOFw1FF8jr9jlsCmDUT3ccQOUoSRAiR/CmOCaia7PcAhh9k1YYLc+OQYjK8yQ7AWUDn8XsdQ6s5/f1lZhGgvGEvPYDW6oLdWA6sNSPY843RMOLV0u+wFqjtUSUhN5uVZT73+ve9H1Z3ktMx3ezQuF/kBPg9UeDYNuHHrhSwZFjWouo1nKqWaoyvevGAmhQJan9GpgMvhXG/Z3pFhseBa9yHQFeixiKfNz/nBi4lTSdVNqJcHcilppMDegthqNx04boTv+WIO5SuMN00ql47fSRon3MWml41tHinfgaK9Tu1q0U0Idl4hWWo0DQyuVeHQXiaFrQDsl3omfpVLX156VGOcXz0ancWNnJ2JdlTJHcTYStWM3Sw37F805TbOTGVvONXFJZiUzMcmi6/LLRqJ20WWlwXvTshESHkk2VwvsFU6tJcR+n2wkew6arKRab0o2Msz9kY1ENrqQrelhyE6h9gdkq2Kdb+qBBZQOSAvZiNWOTazEOCczVjoDVzBWapzbFhudjocVKyH7xrN2YWKpwRyOWKqc2hcw5s/DUqJ1vWEt1HOPYa3XOLKwVWpdTtgodbxDWAk1vhxsVKY3Imk7SrDVqJ0aAOhYvyneWYCJTveCfjuFkbvzgUS712lbik7u1zO7JN5SSGXhDO5edWBx0HfTXEluqWPboRq729tWq76D21anuSrbUmhqldns5mkdoSi8y738yRnfmCMPipNj8gse23iiJFBN/RoKsClf09DVVK9V6GYrhGn63yVuvDMOzSZAberERmpqyVcxtPuYe2MbsrQkTONgVIu3ofk5TtNY8CLWuaOv9uBhUh0aq9XzroZxqyYYjQMb5wwS1PV3+YN5VUUVmk1L4xtJFXXs3kI9r2kaFbbrj4zqIxANw2i0c5GGEPHYXhaFkFa1RJxhw/4er2A0r8wL0KGefs+GVyD9tdHTtVIBOpVomP6ZF0ycn5MHHz1PIQ86dnBNIaRV41QHN41yOVNpbNuql5XdW/Wx6uCmMa9HA6bmOzOhpiKyfcEaClO78LRlTBOVu3PWqts2FAIgisLxjZZaQuMHKXVk2KCZFt8M5cZdiAlFe0aLzlFPxZ5lYj+mtow9jW1vi5fSX8YTqIxvItVVsWfJvSRIyRg0zoy7cbyaeee+p2LPYtHcdUQASKK0kG8gt1EEYGTHLwDYJg9ySJVyegg0KtwtimxqRrYGZhidcmWnaoRM7ROF6kp424jQHZ/h6MF2pfDk8/Er63qwVbpHT8bxaReoDN9NuRZkEZbkU+SZd2abZSS78Ma1QLNsZBG+7I7bQjARdRIFf3gAMrXPPyipiqr1HXXY+oo67DrcORT5eXo29803BySuy4+/tzvnn006sgt4SCqSEBv8u0vI1knCys/9esaf/PmM8urOr8Mvnrr/EDo8+vLhkno7evcjSYLb2xdsV8P724Pj1cvbA/JN7WGAfPX15uej724PfiQ2iVFmkzzCHEsp+j6hoiQs6TkUT9c++S/17sW0Dkpp8gD5h3m9bPWrF9p5pn4yJC/2lvcp2XcIQ35lFIi8mCqJ6ZflLVP0mzJbVek5KS5wmgOmPeJOB0xf6I0HTH3CYxCYDe54+7zqA39DJkZaX6DtV2yN+otXbMlHXUd29Zn+5u5Epy4UlV3OvxmtPOYp1SPeXYzLNau15QV76ImI/NbUvu07Gb4olVcv9FYEaWDgzGg+cR9WXORFaVb9ufiHPp4AzUx5iJrTjNC/FKhBscMpcBPjDrDATPX8lZnoLlGrN1XiYtX65CDKuGrvjVOFVXwvSD3QFDw7EqK6dHj0+ZFQ3B3FkqAjAu3FxkrhxzDKXKGhIC5Bwpuam6yIS5fpbx3yIkCugB4WMz1a0QXQELAiXXVWQcHWfJkXVTVg+zZHvYLZpNHF4ErznrjlSxX4G4PFbWQBQSsfZtCclSogZusWDRq1VYbFrb2tzcBbS8MBs7thgVmZKiAm7xgOGpbXBkVmpzjgeZkwICxuOpugqEwWDpTzjQfMyikD4g5c7kFTDwyAwjOHfvDQTBgOFs1SkSHoiox5HoTGrFThMBtnhsCgjS4cauMhERi10YVEDeeouWpZWFB3jn5sKwwHyxxEApMyVUBM5kcKmpPJwoHWfiyBQWtZWFDmrm8GVqYMiNu63ITGbZXBcWtPnvMg1+rw2PPki444PHTlfnQm6EocEnquERmnDIlb+0sFp62F4WCZzxtgUqYKh1k7dgXmrGUBQfM5pmSYKijmPBVXKwwLy3m1nYGYU4fDzmcZKObQA0XeBy80KycNCMx8+0LDMllYUHc2VncWXOaGeA5eJg0HjGcpYBi6gM3VeZmh68K7YZ6DFjzP1u6doWGZLBxo4zIamLTRhUN9mGf+4AF8/oD3bT0DbC0NB1z7zAaGrWVhQWs/3DPA1tK2wLx/bxBMXhB8/wUQpFB4lg0YSo9KHpL93N/dcXZ5fXGtu83ojJ4SMNpfpL+ZpPea/RiH1R1rYaTSypfxcyhw80KHEhpvU0TD3ZpoCMMpGLMUv9XXxBmBdINbpshGpUszmhob3V5Ln4Fu1/fj12YIXGBjgjjbuV4eP/zJCKETWoVhHxVH6Wdct+KoLuExqzrCxDevNpCfBgrNVuU8vaKsnDPX4ZZ6DSORp6dozJIN5xbJNu3DXiUtu47oR6WMS1opu/Uidx2xWyz0qTrB7UjKW5KMIOqQYBltjmLEv0TV0lRGk5alfmCrlDaybm2V9+6uH3cusHV+NyNgAaFKmylDGxygtBlB1CHnLm1ReGde2kjgEvbs8nXZJ/I0Ykod2gvCP9/ehDyljKuy4TDW34YK48MSquoGtEGUUnBoUGXqtGtPfVpUaI67eVvPKhfQwDg99qQf324oszh7GCOVKb9OfNtAZkbvdgpLZB2LLIThe20usdR5p00gM6O5F1M/zJpWuVBmZpVH1R2zmiNpURLnSgOFTgLn6mMEoUGdfkXPsG6vQgCguJ+9a1prr/qYUcNKqh/WDEFpY3HHsMam4aE5tT2iHXs6+z+HBpW2mnTsaWwjEZoze53dgGbGq0laTcNtIBujhmV3GHgawmzeQHX0L+6xKAxkaYzoGLYNYVHtatpjQaxqPU2TTSDzKkfPIrJJ1LL+0LNXBzGvcvTssRBWpV7PYhvIqsDpjnCUpj6hhjj07gzm1Hg6ZbipTi6MUb9fuQPetWnVA69uOFHqEnetWvWJqffxwitUuv5dq22oZ1TXt+9AWo5YbJgH9jbUHLEp0jTyt15ouFpgNe3dXLalnCQNbZ04bfhnPaHbXiuWo43SaGsQYZa3BzLGU3tt6lnC2FJsfN/NcsTuQTGnGeo812k4UqBNrh2pryLaT1GtrcsLK4ueU4eoCyuvYJNhmiuZ8kKhNpXR1CrPvQJpUk9aQAYxZgWEVwBLf2uaWmXPS79Vf3U/RYsNUukNY+r5uQpU5+aehM3rZVJZnlJ3xK7nF7SpseMaij334sYnqHIeZ3Gv+6FdCfh3Ysc1FHu+7aVPIlk86TaY/M2MhiW7vdaT3bRmc2HX6C20mtmhvcaPugyP6G1ritmhSkeHTxhnGEd2M6gzZcl2HyknncZJSDpuGKkchrOJQ88QQBTo1YPzvwORlSXvJtNr30YSy62d4229PHj0VOYhldOKNTYiC7absUtvdLCkjaZ1Bqzl4ZOSE7akbNxyQVN2hWEomb8oeEombEnZGNh6wJg9Zfuc2XUNB5s5u9pA1fhMsH3xJettvT6wrI6Ba9rq6aNW2PIdkjqg9j4LiMip2tYufjwDH6dqy0d9zs4AyMnaltKtdzwDIS9rT3hy+mYeRk7YnvL0+GQeSk7YknIzS3nZgJWXbhsEXjF2tWHbN2jYvjhMyj540Q7N0OR0tfd3vLCO+3C6gRJWflBtZhpARmzVqy0PjZv3itr4dDpEtSpA5mY9bKrILMHiivWBSiVzeas69auXyD1xWGS2tj8XdCsPgI1RvWPVDVCGkgAlfmg1IBXATxiBGZx2naXCgAulIQull+DMy6mDmfrFNo61ZyiiE9ZmjdRW6VQcTJS22kfoDCLEvDEvER9matbo1L7ll4hPbQumJxQW2K1da0LRD4Sf/6RE39ML9rRFhKkcJtQroheRF7nDBQp4R832yU17neSjM2VFN5uM5v5loiKzBBKdaoYclLuR1C6Wujl+9AUJOpJgURzXn6XXB8otlIfFXiOv2OVWI8ppcN7AjB1WUPwJI+AdVjBwofQ8fQd6sI4utwInu8zSPJEpnjLo/DNl5fd3c9P4D/3OB/WYru5ijnOtrrt/hU1imW1dYV7pzTdBTa3YDpzGM1SHWXW0V2X7xnFyTJTx+C4BOQIvYQXySEqEJUgjYQxy52H05rU5RRve/Man6aG7FMFkMC5EcB9zb3yhXpGjETGGmdotIIXQ3hEwNO6SuvrBppR2NSxQAjy+IUGBggU3vwhsfHAvtR5rDtb7xTLc0ONc5sWyCW+MkIYW0WeBjY1nEycDpcYzlROCU8bvfXxqYb4ObgFgY93StGX7yAlYQFi2jZyABYRle8AJmNeB1d5Di1qwFbCFcLM0iqxJGhXzxrrIyZjhbnz5UN5ScwrGGFu6DcWOoyNhvpG/3MlhTtGGt7i0s/RgX5QXl1u8l76M+X2q2xx51O+4OUxHwrwWYUNgi2qEU7DGcKk5e5ZGxryDk7jxbvwiPHkHpwlv2cdqpp0tWIZCICMRY6CeiEWxnpyWVSjP+vOuUzULwLsa1wNDfPBw+DC+PqKN2OpZjeqa5ZWpLXBKY7u+kiWWbXr1VWD6OeY4PRWL4adlNcAJmENkWLY6I+foaFihTC/hKZEYLNkJQGjfYGKlTYmk1bBCKXJvwquDEkkjYd7B8ILJC13lXYtWwHrIZg5xb19i6rGnBUQjYAyR07N9UeFuUZShCS97UhiBkPlABUdugO52G3ssoZQVmB+FUyuKSkSthhVKHE5c/qcEUitYYRDR41cA7bNYCwDtHj1ZZqGujjHSt9OX35sPqurQVuZdn7QmNlNmPRFbGItBJq9gizG+p1UVw+7ii0oEW6xu8QpWGGWjYp8kHRkIINvE6ciYt9rNcZO6FjefJBFJAYBBEEGl0eTOV50U0t/WOvHiLBeShVIQYGw3F+1aln0VCEKBJgQqjtJC5UiADqpAEwSVbQ8FQWy1INCslvCHOhBIk2cJdJj0DwqMQ02dB9Bh0t7sP46U2+w5GOqAZHSgPG6Xvf0t8u/b69vpNTIWc5xiMbCmyXzwPRQCbJogsFopELApjwlaWBp+FGRQ/bYNgk+gCVXTQ+AxHbCaHoJpa+vnidOyn0YYEQOBK2DyGNOBahkhkGJdP9ITLaOXBGnshnFmsToyrgfVeIOgYU1n1pONNwQRBsvs/d44CN1QE7KHD4Kod5jM8pJQbT/JFnv0J9LSmzicMZp4Ye3Jln32QA7veHloUDYHLEzFFgYnxwAwTAUAxnDJSERksHgkxGI7+62ZWh1boGpPsz1Qo2MLtAujgHRG1uP336hDdbRsD8qR/0QRRHnjlexvKwPg8SFenL/1yD8nLwF4OCUQqCyNxl0naGLVWtZg5VjdcMQ/ROuq2cIZDqwHWAajajGQUc99gKPdbxfDlJ7Zjk5W3wCQOlrWYPT90+OfEGC8lj0YtwUZgq0rZ4tX71y3J+OULKHKjRj2RI2MNQ5ApzcA6fQGW4A2j4lYoiA/MJgV6rPUKvYwECW/kbHG+TXz7gGKOydkjWQwrzjE0ZxUFKOwI0T2OK2QLZLZNtABkPHtLV2cDUTXdgPStd3eB2t7llrFFqY532RPxEtZYjFNsC7tUM8W0GABawCluXY1AnIKQXIKgVK5qLvbrdf0NrQoSgG62SOilqgJKlyc+vfIZFmhj9gTs0ULceHePwJgtUKWSPSouDUOE7FEySa8UCujZOpeqadQDLbSD0g0d9OPgPj4+AQCptYBAAKolRoZW5z6eKY9EacEBAU0ih3IQeGVx0jh8Go5W7w0ejp+9RIik3FKtlB5+OAVqPTYhAHalaGeLSD2oN4np2QJlYekOg6O3wBM9XakbLEgRr/ai9RCFOqrwp6lVrGHoVcRQOAwHXugU4jWr9WxBcIABYyJ2KO4vudvARqYjhQEVjmbj3dhATEXKFKEgEzT+xAq8RotALAipMMNGLBWyxKsvY7DmqsjZYsVxjBMjY4lELt71Z6IE7JEKg+4WfPUKgAw5ekyEKBaCQYKoMzxSiBQGGCelVfS2BdmfSm8yX3u/QW3af9G0iSgl4SDuTFiW9noXpEiTKqbdLFdG9fyjcjCIpNRRxwW4QMwclcWBrla7wbBbKWg0OwmrjtoUBsxOtvsTa8MERBC3+Hel00K9K3A0IVpWn6eKAAXrml5mCgQ4dxiKMzRNkowYPbTehwb4Mxesx1c5utBh6+jBgQY/gaUDRslILAi3/kw1XorBYM2fZm+DpnBhflTYBK/iTpkz8FNotpDA+eHaLNRdJ84DPuECxQ/hOjR5lDA9fnHj+dXtj3aSsW9urlRP1Vaxd1po+FUKk5XzDK3VWLq2UxCBZD/uehBUdVizzb7y34e5O0wCkkfwY28O9l5FVFYEqo8O2pWsNgCfnX0SM+3aXfaKyIa5fL/ZJng4+pw8E6Xw+nImd901KhMZj59KMXst7RjFNFXvaMyWbbys4xLDfr7W+cvabSLEXauEE53uU/+woWHt07sFQWJuZ/F2NmgBOVekeZOo+KMW6LDDS8JMIC5rtSkzXIsBmGxFZqyl6MAJUXoRdhF6/zVCYRpseYEBUr8tLpm2dp4V2rCZuyFCYC9VmbC1sDNooklkUvFsUV1luyWFodyE7ZpidoClMqtWpm0NcULycujvTXRFMt4WbQ32NVSKXu2JnkhhTxqbW9sRN2z99PwDKOSNfTNi7MIlQeJndrZ+0/Cg4z9HmPZ/p4Z5hzqSLV1mlZKOX3FjnE2wvriFVub9IRSrdMNSq90OC+V4ntETep9QA+hj2jq0c7RJ5qS5z9fvTq5/Hh88sW5qI9om4mfXV5fXDtntJNlqYTiIH9gF6U7pRNbUx2aVS0E7BEwysskD1Nc5MiL7bRGHAbWe63rQllVrguZcbIcYVTMbk0/4WRTMFByjfs/m9xOOzOzZI6uN0nbOI8N5E10FcdoloW3SNPI35JOKUQxruItUIo9P08/oHWYhNS7aNv2tfXryauTn0+PTz5evvlONfD1J/fs8+WXz7+c/3Ljnv355s+fPn90v1ydX5PPqhqXP51/uPl0TYR++fnio/vzxafzsgUuJx3J7/8rKv6LK251x+wHXf0v1392z67+9uXm86Spzu1Ymqaubm7cr9fn7p+vLzu6L5XTs5ltcq/Pz24uPv/S5fvnLi3+q32o+twTZ6X95+6sRBH4f/zj5JNqNQBtsMj/6d/u2eVrd+P7K0+FgDyo8pjSQ7HKQ0lKCkPvQVrWPmcszemHi4BuB2q+Xe38VVMey6RPy++nHiNjyV3nLZEqokDfjuLXSxhf94yvs4fXR7g/uJ3HdJR6hevdhR0AnK6LtYl92iiQF5hPm6+fWpFWffzKntLgpxAXjdEGL6JZ1m1DD3rnTmtQNwoexii+k8aheexZRiIKk3tZDKpn2H+u/TzMik4m+EOWp78iv3C8XZGSAYtTPUmHD6somD9z1E60XdLrKJ1pd+iKfNfvW83AEHiFB2pf48UkXkKaL5fUi/uIeRqHhbvOSTXsZmnpd2gPEAndXeCjbF+vn9jPiyJc+MWzyQF06WVla7h8vH2X3p0ReHnQbRS//34B29++jVj/4x+Pj+e3/+jlSZhs8MqLoj0kfWO+3FazT4AMBV5ShH63a+JFeEkIL0eV3yi8Dwz6RBz+5pWinQ5S+Jua+di7R2WL6eXxivazCy/foKJvf+SxQbf0KCbfvNfrnFoiFNtdfNeDYN/Nb7zfMz6KyTfvWf/4KDh+swiEsI9MUOj3R+T79xr95YGRtuKT8rSPjtXQR2SM+F61mp7QZwt0ajB0UXes0q6AlGtuYCRR+320TtKj6tu9AI10aEos/rfl8lJd2br9Fu/or+U3yybTvDRG6TJsio/+yr7bU9rMRqSTPqOt49Fn6/yrmzJzsuikyfiA8WhNfztqf1s2gRYD00mt6SH+0br+fW+ptjigVvmbHqIfrekDR+UDR80DCxfL5RF1UvB5TN0BpvczjdDonOr4k881KuJ5nJHHxidXzedULYnKWcXBnOIRGU4hH7+nv67KP5dgqSe63OqzG3tZl+r/ML3bF0eXXvb+D//x+evNl6837oeLq/90/vAfX64+//f52c0vf748/89VGViBuVocW4UkZ7AJ7j5uUG5gcdOs25Mot+PFm+OT7OTVyfr0+GQTDxZZ7ZIpxL5WXj/6a3R4dLTxx1ofmFrIHlOSuaTh+7tMzOK1TXGxbMT0QAnWqtqgUGZOHL46qXCDYlXtUgjKu+rL1bvVJtmtuMqf3vXfjw8n2Hu6emhVOitOiy3KIxK9Zd+pcHv+xPPrGGFM0uAoQsmm2L7vL8fPndR0AK+T2Pzz/78nt0Z/QLsIfiMfGrmjx7DYHg33hc9eDlmDJskYunJ+mPu7yMsDlKEkQIn/ZLYe8HxilJA6OBj0PtRn880Krri5tdHSeTXvHNbPKD+9ePfjtziij1a3vJCHj1cvy8D1Vm7y1debn49I3+LHSqDupzTbYnb+Kk6DHSlRGBU7MrhHa28XFdeoKMpVk2Z78arcsUMCEokM5cXTtU/++55ease6Ps58VDv/S/WIDk9ng3fvzBnrNIsqiO5e8JWf+/XOLz+vLLIbK5qXQ76q+5+BuDDxh6IWeIln5VZClmRfSEH5qcx9hmmXEYEy+14XKPuB4Hc+LxQlm8hUr+Lg8OD64vLLp4uzi5u/udc3Xz9cfHZJv//L+dXNxfn1wduDf90ecLvp396SL25JL997QCSqqX//F+q56o5ujb+lP99WD9D/kRogC8lTwf2n1K9mDtkPb+s/Nsi/T10c3LuvV69IcrCvD+s/6H7Vz9kwnHiswH78d/UvonLwoSq4mtxGVhtm41jX3P8mL6USodsTMHkHf//Hwb//L06fODM==END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA