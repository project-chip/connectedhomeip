####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 15                                              #
####################################################################

BASE_SDK_PATH ?= /git/gsdk
UNAME:=$(shell $(POSIX_TOOL_PATH)uname -s | $(POSIX_TOOL_PATH)sed -e 's/^\(CYGWIN\).*/\1/' | $(POSIX_TOOL_PATH)sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell $(POSIX_TOOL_PATH)echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.3.1

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
 '-DSL_APP_PROPERTIES=1' \
 '-DSL_COMPONENT_CATALOG_PRESENT=1' \
 '-DMBEDTLS_CONFIG_FILE=<sl_mbedtls_config.h>' \
 '-DMBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>' \
 '-DRTT_USE_ASM=0' \
 '-DSEGGER_RTT_SECTION="SEGGER_RTT"'

ASM_DEFS += \
 '-DEFR32MG12P232F512GM68=1' \
 '-DSL_APP_PROPERTIES=1' \
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
 -I$(SDK_PATH)/platform/bootloader \
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
$(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o: $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/bootloader/app_properties/app_properties.c
CDEPS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.d
OBJS += $(OUTPUT_DIR)/sdk/platform/bootloader/app_properties/app_properties.o

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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA