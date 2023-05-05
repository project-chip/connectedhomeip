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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA