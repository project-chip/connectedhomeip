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

#$(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o: $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#	@$(POSIX_TOOL_PATH)echo 'Building $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c'
#	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
#	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $(SDK_PATH)/platform/emdrv/nvm3/src/nvm3_default_common_linker.c
#CDEPS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.d
#OBJS += $(OUTPUT_DIR)/sdk/platform/emdrv/nvm3/src/nvm3_default_common_linker.o

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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztfQtz3LiV7l9xqVJbuzdS05Isj+31zNSMrHFpy4pVkia5qSjFgkh0NyO+QrJb1qTmv1+ABEnwidcB7dTd1MR2dxPf9wHE4wA4wPnXwfXN5/+5OL9zbz5/vjt4d/Cv+4Obi08/3V3++cLlf7o/eHd/cH/w+8Fhk+L286835xe3JNH7H79E4Ys9zvIgib+/Pzhevbw/eIFjL/GDeEO++PXul6M39wc//nAfv0+z5B/YK16QJHH+Lkp8HJIntkWRvnOcp6enVR6E6CFfeUnk5LlzW+z8IFlhL8kwASWpU5wVz7ce+ZukY2j3BwT6xYv36yT0cfYiRhH90UvidbBhv9FfgxDXv+WhG+EoyZ7d6qnVlqDvsoD8RB975/w5CXcRzp0bnCe7zCP/yguUb50IFQXOHEK8D2h+nXyXpklWOHidnZ5Em+MTp0J0xiicMS23Fx8/Xty4N3d37jl50oKUEYZRJfE+OnV9vEa7sLBXMBMso4rSHLle9pwWiT09oxyjalCauqwKBji3p2iSZ1QVjsLgwaUthJTqw25jT9cM06gy2gQesF+EFstqlEOkxsf7wMOLiBpQVdreO1VnNdZxoV2RbHA82XOR6hEGHiqIBLd4TjFkBhi3M0EzWrA3dxekX4nSJMZxkdtQM2SYesNe/ZRLlKMwAX27XOGM8kyJwnv64BbFfogzS4IGHHJivAXEeNNigiQvMowi9qyVyjNBIy3JThGN0QglBXFQkD8IX+zZL6wh24TAoNcFuwzNjsA5NlmBRYbifJ1k0cJSR3knRZeWySIlOs40K+xhF4RFEC8ocJpxVGgYxI84o9+sQh9eVQ9+VMLKy7x66Cf/hBcxIBBbFxvsPSZu7j+6r1anq2POyug8loaooBW1+X3wxIfSsuF+HzxxGxADIok/kQld57HBgxe/3JyeXH08PrnuPTeELEtq8FS/jhYoK3ap2xRYKtGLb4LC2ZByceqsO1UOHS4bTqvUqaQ441yOQOBzXuBoGX1jVH15nQozWfiXsRfufGHpt1Qnpyfrs+OTTfT6jUTHoJpBJseZ5BO8hDadi7woXURhw6SgzfcWklYRKShbuymxDxZS15KpKSQ954ISazZ5jV60W0YeI1JQlj0nRRBJzVkg9PF0SirTIllMIuNS0JdLmUUQ6vKuOSTWRlcn4vVChceRKSiMEEmYe1lACn6hijjkVNKb4X8uppNxyevDS/U2WLW3wUW0kLKKSF7ZOt3h7TLaGip5dZuUWP7LqGuoVNQFC3UvNZOaNnchu6/lktcXnCz0XhmRgjIfLSWNMclrC0m3uIy2mklNm+st1JdwZAoKMTEicpk9BQiFLZmyQvdhJ7NXCqeyJlRXutgL7/Cp68yLZXVWfCo6F5yNcGQqCncoW6wUay55fVG+ULfNiOSVpV68ULnVTArasoUWEBiRkrLFepeWS01fHmxiFC5XgjyhvNJsqXlRpjovypKoQA/hQkMyz6agsfAW6lhqJjVt7pLyXC2FGV6o/+PZ5DXmS7WPXLV9LGgTaFgEZZLFqh/PpqAxi2X8ZCD0MSZ5bbt8MYOqoZJXt19sHrxXngfTFG6SLjQX5tnkNT75Uh5aAPpqJjVtxFhcyLri2UQaI+ZVaFMZz6G8mW1P1yiX1G72yJeDr4ZfdDfAH5KkCBNEvplze+i67w48H7iS6z6p5QPQSuq5DTtDcMck7+dXt5e3c9k+L33059w8pvwHOmXiRXmQl/6VgZy35LBMSqkO1dPUmyHqoNIMRbAzDbAaONBZCcVv1LGnwF/0Rr8R/i6iRP43mmbLZN4345ZJl5q6l3vRKzBmDm+WOEp3Lsqi/XdQzB1AB7wzwpGf7ecaJKntURLPN8kg9uabI/YSX2+gKfU5lQiH8DgNlHpZDHTTQyTzOcszQc5651CoTLdy6tPqiKvsUlCHUPdPufTQZytimXSLQncdonwLJqaDKBYQJt4jGHcNJnzz6hWUL2iDelrqpbW0jycsKShSKTJ3g2OcBXr98lhOOTypSglGzLBUWgIgdYso2RKguGsw9ZYQBg/zLYE8UFKcX70qR1pkJpnAOSOQNgayftaEfUB/PuRtA71ZR8ldjU4tijPPpbkS1qUaW+UaYSpNVqTbsXUJOawZXuptD5LLLtB8TlGeY81lnk4uW5x5voed3n5Eh4yBCN5hOfQDvL8GZ54vQ9F6F+uODxwhBzTPaDJL6jBOzoyGZZrpmqSdEs3GrNERLsNBt8c5PeQOuPU9W7usE16r43xwXU4fbp5d1zOxwznqddhn0vZJ61CN+5v1uXR33DtUo7vpI0xgr66LNc9bLcmZc7Y4MnxwbXKIN7tOJjHRHLMY9GZSVCadRLUgEj2jORVDkeqdzNkaHHH/YEyGZUqxbNnGVDWKuH8wpmIgUu3UmKzFMVpPHlmdUjP76TnzeKfX8XCrUhzMrFVsYqN26aSM1OqOAV3TsUsoZTuS5/ICFZqGcZewBQLtStuy06nETCKtxR0gNYlFkoTeFgWCdVXh6mNzPZBJXhoxda5aSAsrbe2NRhne6Fr8A+WsygyQZ9dl2qzCa5Ah33iem2Y4qFb6QUUMoeFXXEjVF53drm+BMK/mNZJWRWdKnRqkrug8qOhlNRdaZIVezyESUQPbaXNNVnVq2UA6q2U8qFL52RBRAwPsC1XGiXmlZZMCegWKUZWpcOoK00MVFTx7PM0SQpK7yKN3McHLGeLbqch87k2qEctEbWt0UfXKFFzOEN9GL+4RzcXzXDfOX1UlaBXtnWXsvhHRXRy9CxYny7y9BQd5Hg5xhgzKvMpy5wouZyi9vYhtmlzk1cQlTaI4IGNyjjXPKJjI7nFLqKZ35XyVkh4j1rtpZKyLHpv+FVmSPrtblPlPSHPlQzqjrNMcIxU57GVZoudUoCSuoRHWkTr5IgXGcQmEsQVjpOkKpySsyyUnDHt69/ToCGNcAmENwBbZV9YjE9ey9mbVRSpal06yo1xOX59Pr2ccM7Sm27fVnr+eNbdcgjInjc1Fml46Sqo4IlHL9qJlJHFEIkkR8hbSxDGJmssWHS8jimcSizo5e72YLI5LLOzs+GQxYRyXQNhmqeq+ka7u3Y57iQ6rS6c2Tiygr88nV357FO6kroSFKb+WTvfwxdgElCL7WbDvna8Ye1hmglCVZHnmDHR8b2V2hvaaSKJGMSOPpmBI1hWOU0rW/iqByXKYWlH2+NRUsu2uBXW2jBJKc1x7Ork+TnHs49jTPf2joneGV276E6EYbXBEFyJsah1lU2lTKM5TMjGPi+b1BOl2yRY2I8AoH1vd4wAwudhOnB1QygMxOb9iFhi7UQ4QRv5XzEJNL2cWBEXulocL7AoecMFObrs3U+do5KFe9oO4wFmMQlKCu7zAvpsXSUb6E1gDiSsHahuRj84c8fCVjVS8r6ZcRC5UX60/2pbZsIw0gPFKJmd/2dQ9Tall2diWOsqopnSNUbHLoCck81p5TgM7zLbiGV5lO8ym1lE2vZGTHiSgG0L2C1dErqefhj/6KtobYltXHPQ/dofaXRGE0wEgim2Q+S5ROr8FzVYR5nefw+AhQ9nzvA+B4sYQVe9wIusNDIeRORObP13OPD4mqXOlzUUxM48q5H8iNQiev0Gd5X9AOX79CpS8hZy/CEB51iZknp6HjTC7TxlS2u+TpG9wZzUo7jUKuSf2E/ucLumu9sDNrAsrUODnSjuZEuQMcf7uB6UZnZA0Gp2hddtVsKFe56DtqoGcZU4C2MwyvFnOVO3ggZAzHR5AGHI+evkZLGuNKOAFJpVghB+hOEwBN/zoxGEKuOH7aA5zvqeq3IBg+6oWU4bbTZMwtCGgAZ4fJYuMWLAPSnsr4iGSA51l39K9a3D6Duq8K2m5HwxK3kIKLkgqQzIW5WWPsIXfR56/oGqbYURDfINq6KDOt342x4Jt/hyoFLtLIa1IaJDn7YjYjXZKt7KI7YgGUsKCaZYMYSUMsaUtdkgdPVxBu1RdnJNokFOrb9M9gp0XMk2hpGyP8mCvtJStrKylEM51mtVuRZcYqRlPH1xCjYXS6QPL2xWgKnrAgrkYfDvmMOe501xjkV1M34EVKlDeSJESMLlxMuCnI7DajoiUgBZWqKDIkNr1tlICGtT5kR35qjd+icf0FlNqRgPK/ShX9+tZFyx3gznLndGTI2HhbnGYYrVoG0INI9jzln0euj5+2G2sqBlFF+rxwkBxu0dKSAsrVBAFahfZSPHXoEJ2kvD41M4IOQ4vqegRP8NXjy70rJIvZy/fgk4+akAhq+uR7hx43aeHK6MBdurFg8qwKznCybKLjwZXD+aweyM8qJC97NWtFEAHWVaHhaLoIM+Pm423dt2fgq4GjKFL6rEkRKVEVJ3lVMpjyhNu8u3AbyiOosvqYW4r1DgrDQFLwkZoZBXmYVJouv2qKByhkVbI/NNsKWvhZRVBb9wOoWWVqLoLq0iZ8gWe0qLo9qsiZcKnd0pJBrzBPISWrrv2qq24xnpb7D22V3TSM/ywy3Lj+ErjBuhMdIitOG5YUtOiS+tRvP5WSc3opbjyY5glWSM0Kv2xJVUMWqk/tiRlK3ObPfe8lan0BL60psJa/WHQKqOVJSXReBi3ydEKxX4SuUGUwq7IT1OojKO2FOWjYeRmxlFLQsZDk8tay7ZEDWlUrWVbyqYOi0jeIDYRHEzCm3iQaaTmrz2Zy6CO5cQ+IylHfZQFMI1jQM+ARfx5fGyHnwFL8MNtJoyJmNxWGHdftiGjhRZpqPwyrWhooEUadkHok1F5rXSFgbyODrzoGAv5KwwttRAeXHz3ix0Jnswb8baI/Hfy0o4EDlxKR5qESnfpKiqp4YVaykkj3Gx0qKZLINIDN/MbKJmc9vU1QBmjAwUTpujQrSlFxdHJ6osdFR14oRb64ujZKUtaeHixFs5Z0pKcLoNIUe07a0UMBy7QUW5vWxHRIAsV2LH+fCnrz9/aGVcYroAdez7MKkOfvgYW81tqng2yUME/UvRop01y2EIVMOtOQwWji04TxxOsKGixRSrAHNkGGmZuAeAVbCzZeBspG2/76K+t0NfAIv7mOIMVETy6QAlLY9O2G1KINMHsMAx0jG4uDLjPLJGfybBXwTYedus1vVEmDBM7JuYEj0BdjAs3T7xHDLQe3FfVwxepCfLCfXyyo6TFFqighyxtKGC4AvZULb6bNHs6Fu9tyA7jizsgH3XHHTlEenxiib+GltBgp7dokEUK6lNRVkRw4JI67M25BgyyisrDWlYV1QwiRUn4fHz60lKF4cBFOrJgjwpcBgTI7XTkQwqRphxZfFEcuEBHFpCezz9+bWfxr4MuUmJprjaxhTc8RGtnn6IGFvPTE7eWFDBosYYzSyNMCy3SkNtpEgxXzO56yNva6dE76DJKyoXZfBcUlhaRxkhkdCXJY2CxiBp4CS1FQK1na1paeIGW9rC4DSkddJGSILImo4EWaNAIxC4tYjI2+0BFeYLEhoQaWIK/PMNhS0MNLqfDTivhwaV05HaW4XjwgROK1JWv47e5dvc1lK/eF+qnF4lK37DPvFvoHncRxNU1fjn48NBKmmBSU0ls4ygogr19lV0mOZXqcepVlE1Frp9WA74o2VEju9vc8XUFPIY+Ikr1etd+srjAX4p8gbYwz6in2n7bmGeUU00SZrCTM05gAy6nxcoCECdHYQ2occ3UuLhZRVKHQFJT8Ju9KtWAS2opsp1nrYdt0eXUKF+gqyJm8pLccS3qkXJUxJgHxpEIcow3m0GInNFAyPsAP4k8am8vPn68uBEZYdVT7s3dndGhpUq606pzKmCniy94k9XDRq9QIESiOnGKLQqp8cFqkzCqcBAGZNxyQ/SQz0YWDugT5YGh+ZrINgErj/WxwE784kJIniu3CVUrGS/b4bQ5XXqnwzB/y0PzpOrrVdcyeMGmV3l3P/DZQmm68tJUmKW8QPnWiVBREKVpluwDugDgbHBMo/smmdMAOUMSalei2M9hmLpoo3SliQ1E1mKNUWXYx3ERoDB38To7PQFiHYcdEYBjL6nuR4Tg7aKN0EUoiGGoWqQRGskYNUKSbvyZ8a06Vr7mZEPEEVraTMQdiFRr2863NQAWHmu6nYEQdSfFU20MhKsLN9emANh4rJlaCEE1nDV1qH6WPfvSIcJfUJSGuDzg5dSRH3/unXXpGkLlCHeuVTdo+Kg2WEYJ5PTxGGVVZXZVaPma+wNeo11YEFKSBoedb6h9dV4uBQUPZEQunml35u1WiJRY1WKjzfFJ9U/6r/Tk9GR9dnyyiV6/IcmLJAm9Lem0+hjkNzLU978lTWdVZWBFfl2RXHrkz12Ks3evVqer45X79vS7l29fn7192dhA76PEx+E7H+deFqQ0Xz+8d4bfVQN6J//ku/e0TvwDewX598Hhwe3l1fWny/PLu7+6t3e/frj87F59/vDrp4vbg3cHf/vX/UFGbI899u8P3q1JQ8CH1ZmgIogvvpSTGGLsvfvb39uvb6vXRb+tGz5nkrr09dCXc9j8SHoolygiGS4CzLvhtY/giFSq0hRid6CNPUS7cPJz+RZHH+gcVx/+zAUuFfzs433g4cmnKmOt/ZUWDnk5u5AYvO/uD96z4n93dVV++eJLFMb5O/bt9/f39wfbokjfOc7T01NdNUgtcfLcua4eWuHSfCZPvmClXiYjk+vqy8AvP++8VcW7ynGxS1c7jzWOpitYbTyvTJH6UQfih/uy8pQTVdp75KRV0IZXEa3+D/3ToQ81tanOzg9lcTBZJMMU8fdD04qEdkVCVDs3dxfueR0PJ2dlX/8YBvEjzmj/sQr9zi/ktZBqRqzrshGUM/9eWj7OjkseQ2GyGT6C9/TnLRnqwurOnrmfB6mDJC8yjCL2RD4EGDwxjUFjnJI/6EkPb+TBoFeZXfaT+MEyjlBpnkwnoU2p3W0Q4JftbvDQN9Qq7nBExxL8b9gu2gH2cDigHnKztsPGojzsT7MOu7bgYXdedMibb4cDI+twcmpz2J9zHHYNJvJxg73HxCXDnlsOde2Ifn51e3nrnNO57yVbK/OiPKiW8YO2cakDkBernZbbzFZNT4cuL3qlkTRKdy6xPPbfaaQtfqu3EkSJP5RDGjWh6ArEJ2pOXfxyc3py9fH45LrBGzV5oKFd5EUpPKgvfO/qmGs3DeLcCi6xnuGBvWgHj5k9J9TpVdgkdZDZRjssbI6FTVgZlJqE7BQ7LG6E3NqkT+CLmMBn+J/gsNhCPcNFBI65Tnd4C466Sb0MvrPZpAF89aKgLnxnG5zA5z/wETxoSBqAFVDXg69XZD6A4xzbwqXHraxh2ysONwc3bgi0nREtxDuUwcuNcviWkXoxvNA0g7dpCKaN2kVh82ATt9McOOzMwvCYJVGBHkL43iErxDMmLVDXFm6G4WtubuGV2eljSlQbZUsj0IOD7nIbHeLehq1AQd0khbcXnvwEvmApKOnEoTrGiC16w8BVTituK9cMtloRoxtEWbFLeVgPAnYgVoD6kCRFmNCYRb29jd5HEU7lZMvCvLsoz7G4qXTTtG668mkwuxhOPgUp92InHCVZGrpx1+ZGrgSaHbWacUN6+TTDzI1OknmAUm3UmKUmg81GYklwAMKKgUkQFAOO/GzPFzr2EnG2q1R0V6xMU3o4qCepd9R0kpbL6YGwJx5Luq2CBGokc9chu9NYNXEdaUsyHd3nJv8vk55fvSpXlpF0aloBejuW9PW61R6WXI3oArV510hc5l2Yjma5rH+RZHfUSfIg7iQ6z3vbQDgwdBOIjbT+89yZD/lkUr1qL0km0V57CeTbD59QatF0JIlOSUis83Wel1nD6iSQmNn2n9fJR4ai9S5Wo6rMAo0kWq9VcuupTUM3XlWaBO0J2PNS3UD9PK3YSgmaUEDSSbCiprKWqSSgtUzlefbqBUly7O2ygHpqc64FvBtJ7STVuI60W/H0ZooQVy47ovdnwpNEcUAMqRyLF3t0aOjOv5Ws0BqOcF7HyAXD9LwIHpNedwYNurEglF5+YQP05Oy1Ddiz4xN42OZraFguDJoF5Pb0ECByYEM07UKbeDrCKbAOLIt2BQZbXRkJidh8t0WguOXsvYqYvUWZ/4TEY7Mqft06gGG7QQIBkQMIaApRnemsOvQ6IgsfSgMCmB4HbC4ohQJs4skX2Y6YLT4fbQOKg5azbR7KUR/cpB6KKandceFSu52aFvqdnxxPc1gUnCPecAeGodDbrySnq5rga4yKXQZbMjmupzuuj1Mc+zj2AhgKbpjoxo2EAGbfBEXu1seJYGBHKyR7AXUwU/tMbaBS+1xbiWUsGCYWBRKcqG3VgOjDVm0HnPqWQ0OX24eAmFO9hJAiK/d8aj/62oquP4t7mfn0blZI2AM9DNZ7NBpU89BLX2qQxKjWMpqtnGqFqnz/kpkYBUizhF6tSSb/ctP+DhTLDa9FLTNdgJ6WcSj9s+BwYOOlpBpizAiChQkzwmChvkwxmkhdJkAP/PEIfZQmWpYRSifWlQkSH6vKCMczzhQfMMocpw73ZIaEO6GajLCaYEtGKNGkN4pmuCMzJD5YkSFSN86QERgXJ8gEpwn1YwZi3JWyoDsmEHXgHEMM8xfNBa8xg5naztUJIWMEM7PIohrKxQSiDsdihMFHUzEBGj38rI/mmyOcGUOMxxExgewFATGCaqN4mMCwUBwmEOnkRrZiVAwzhDquhSmKcb2pJxZQOCAjZANWBwswAuMCMhjhDEIpGKFxoRBMcDqRDIyAzAfPOqSAIQYLCmCIcmbewNjd/IYQ7dX6xkC9K/GN8ZoL7U2R2uvoTZA6t8kbATX3wZugzDsiKV/KbopRX6oOgGP8pvhLzXVwupeLmyFM3AUOBNq9ltgQdNZfT+/CbEMgmY0zuLuogcFB301z3bMhjqlBNXW/silWfTeyKU5zrbEh0Nwus95twipAYfCQoezZmXbMESfN42PySz7leCIFUC39agKwJV/d1NVSr1HqxhVCt/wfYjfaaadmC6AmfWIDNbflK5nafcrQlEOWEoRuHrR68TY1v8apmwsexLh29NH2KCfdoTZave6qmbdqgVE7sXbNIEldb5ft9bsqitA4LU07kkrimL2Fel1TNyvM64/M6kMQDM1stGuRmiKiKV8WiZRGvUSU5pr2Ho+gta7MA9CpnrplwyMQe23ylK4QgC4lapZ/ivyZ83Pi5JPnKcRJpw6uSaQ0Gpzq5LpZLlcqtbmNrKz00cjGqpPr5ryeDejSd1ZCdUFEfsEKCHNeeMowuoXK3W5rZLYNgQAUhcG0o6US0PRBShUYNmmmzTfFmbYJMYNortHAOOqhmGuZ8cdUhjFXY2pt8VDq23gjKNNOpKoo5loyFPsJmYNGqbYZx6PpG/c9FHMtBsNdBwRASZgUYgdyE0QAjez4BYC22YMcQqSMHgINC3eLQ5Oeke2BaWan3NmpBiFdfoJQXT5vmhHq8RlMHmyXSk8+H58a94Mt0iN+1s5Pu0Gl+W7KvSCDtKSeYqRvzDbbSGbptXuBZtvIIH1pjpuKYCDySiRioQHA1LHdoKAqVW1UocM2itBhN87K4Vjwn2/m3vrmgMRt+fHf7e76b6Ycz8tlH4ZxneTFz9RR+3/LU7c82YVGpFaSFJv8fwtStyD/7atkGwZk5WVevRdF/vkN9aI7r06/eOn+fTT60vWHKxp66f2PpAju718wf5vv7w+OVy/vD8g3dQwN8tWvd78cvbk/+JFwElLGSR5hQa4k4/9QUHYr4fOtR/6m4cYY1kEJTR4g/7H4nC1+9UI7z9RPBuTF3vMRMftBkcivTAUmL6YqYvplef8Z/aasbVV5zoKPxJYCw54IKQWGPxqRCgx9MrAWGMNMXC4wDu5qB7vog6heOiRt/NT2K+afcY2KLflYBbnLi50fJO/qfsKpm1KFxYVoo12Onb5gIuqRdm/A+nqp7mAsGBd5oOm428IevgHhOx3GCAOD5s8l81HCIAkGQcTsgXuw4GPRy6zi29I/jK0GSDMXmc0mzWhcN1DC8UBv4BTTgefAqHqRBHVwdUeCphtdbCiYXUjQHg56azWjw0IvSb3YMvLsRIrqAu/J5ydScfd9C5JOALSXhEuln5JRzpQUEMZbxeit54PqRV4EyHXqw6ajpnbsMnUIsWO48lpHGqviy7ysmrbp25yMsGdSRpeD8AA9cMOXOhK7D1ZuAwsotIoHCK2zQgWU2YYYhJbaIsPKrSMXWtBbQ8MJZvcjA2tlqIAy+SCL0GJ5bFDJ7CQTvF4GDCg2bwxIUKkMFk4oF2cSWCuHDCh3EL4SWvWAAFQ8C44JL5oBw4nFVjoyDN2RsSie0DIrVDiZTWBQYKENLpzUJtoosNQGF1JqYKPnqmFhhbo27NgWGE4sC7YKrJShAspkMdmgdTJYOKF1TFhgoTUsrFAW+tKCVoYMKLcNXwstt0UGl1tHxbUjuUaHl22nXnTA4UVXoXwtia7AIUXbmpFxyJBy69jD4GprYDixLO4TsFKGCiezDpIMrLOGBRSa2ViSYaigMu10XC0wrFguQrQFxRw6nOzMykQxg54o8vGsobVy0ICCWZxsaLEMFlaoa02ra0UuC+ltQy+DhhOcW2lgOXQDs2W8WDBd+JDmNtSC19k6VDq0WAYLJ7QJvw6stMGFk7q3s36wB18/4OPEWxBbQ8MJruPPA4utYWGF1jHtLYitoU0FR8yVFEwmDwjufwEkchTYigOG1KOCh0Q/9707HpKkCBN6N4uqr1HXCVvS3WjGjVvsL9NqdbpJnSHS/AsCLsTzq9vLW9XyO6eHOrSctNQ9cnoF70V5UF3WGIQyplKZP4cKblrFEEKhSYyp4a5f1RTDIWhrKX6r75vUEtJNblgiGxm7cLI0NqqmX18DPfvgRa/0JHCJtRVE6c5FWbT/TktCJ7WMhq/R++LIz/aqHUd1m5de1xHEnn63gb3Elxj7y0yx2wmrKO91uqVew0Tm6aEnvWLLM4Ni6521osXiVm7Y4iGvKkuKUMb+mofSbmkl7BaF7jpk1+Goq+okN1NSXremJaJOCVbRbDQj/iXKtqYym7Qt9RMblbQWuzGrW/rhBxKD21jeucTG9V1PAUsI1dp0NbTJAVqblog6pe3WFgYP+q2NJC7Fnl+9Km0ipJBTktYZSf/tWhPiktLuyoZrAd42kJhkl6IqM6BNIlWCQ0KZ9ecun/za8igdd4W/GiuXUIOcngdTz283lV6eUZ5jmXXTTn7bRHqkDzuJfcYOI0uh+V6b23BV3mmTSI80QxEN6K7IyqXSo5WeVXdoFWfSY0WcSU0UOgWcyc8RRglV7IoesapVMSJA8lBAl1rJ4X+KVLOT6qfVkyDlnd0hVvC8HtLJOdp2+FScaIeEUv46HT4FX5xROr3X2U2oR16tdCsSt4lMSDXb7jCx0bpvrVFz9j9usUhMZGmO6By2TWHQ7SrysSRGvZ4iZZNIv8tRY8QmhVr2H2p8dRL9LkeNj6UwavVqjG2iRTdapJY+oaY49FIRFh19vmS4pU4ujZbdL22AdzmNLPDq6hcpk7jLamQTk/R5gQoZ07/L2qb6hvr69h0I2xHLDW1InVQ2clMkSehtUaC5W2C07N3cjSZdJI3aunDa9N/0gm57C1yGN1KzrUGGWd0ewGgv7bWlZyjGVMXG89w0w+yCGH01Q5xvdRmONGidu1vqO5q+TlOt2cWNlWXPqVPUjZVHMKkwzV1VWSHRm4rU1CjfegfSlJ6wgQxyzBoIjwBW/sZqapSvvPVb2atfp2mxSSq9ek2+PleJ6trcgzB5vQwqzRIa19xFHr2q0FDXEOxbb258gUrXcZb32g7tQsC/EzNdQ7Bvd7z0SCaLZ9UBk7+yUrNlt3ek5ruUXsJjcuvZ5KXBitWhvd8QeR4O6ZV1ktWhKkeHLxhnmEd2Jaszx2TqjMtBJ1EcEMMtxzInCk3y0CMCyAK9k9H+OxhjWfKCN7XxbaKw3DrK5hZl/hOSWYeULis22IwxmHq0l2EtYZU2mMYVsIaHL0oO2FBlE98PWmUXGEYlCzwHr5IBG6psCLYIWGYP2bxmdmNMwlbOLjZQN25JbB98yX5bzQYW9TFwQ1u9fNQCG75D0gfUYawBJXKopr2LF1nQx6Ga6qPBqy0I5GBNW+kWHVtQyMOaKzw5e21HIwdsrvLs+MSOSg7YUOXGSnvZgLWX7hgE3jF2sWHHN2ixfXCYkt2jcIctDDld7K93RrPO+3C5gSqsAiqbrDSAzNiqV1uevNe3itr8dAyiGhWgcjMLmyIyJli54/hArZLFzpZd+lUr5B44rGS2t29LdAsPIDvHtceq6+MUxz6OPakDxSriZ0hgJqfdqMswwkehIRslivMUZTTyTv1ig3RrrYnOsFnN1FbqVBxMlrbKR+g0MsTCui+RH0ZlNTsII3+p/NRcMJZQUORuHaMXSv0A+NtflOiHy8mRMshoKQcxDWKJQvIid3mBfT7iu3lxU6uTfHTmWFSryWTtXyYrIiaQ7FQr5KC6G0jlZqla4ydf0IghCZbFaXwrVh+o7lF4WNlrjIpdZjSjnBfOE1g0WEHlz5CAG6xgwkeh7dgO9GAd3W4FLnYRk53M0FCj9jPSsPz7XX81/UPf+NgVQSgfp6/YBpnvkvJR919hi1h6rith8JChTMQ5U5HmdmxpEThczur9Q4exOsq7sn3yPD4myPm0l4BYAg9hJOSJtAhDIQ2EtpAHlOPXr/RVtOn1b3yan7oLJehMxkcluE8Zmt6ol9TRgGiLmfMWEIpQ9ggYkrukr96btNIuhoEUP592SJBQwZLrXwQ2PbkXskeKk/V+sww29DiXfrNs0mtLSAKD7LPE2uTpzMlAIXkqc0JwjvzRy88M6OvkBgJM2A2pDcdHDsBAhOHYyAEYiDAcDzgA/T6w8j006AVbAFMRbpqEobGSBkV/sC4yMmd4mN4+FI/UHIK2jC11QzHT0YHQd+QvPTn0VbTpDS7tjPMC0ZlaEBk02wGM/n2q2wwjGrxdX0wHQr8XYVNgg26EQzCW4VI6cy0NjL6BE7vRbvoiPLGB06Q3tLGaZWcDLUMgkJmItqAeiEGznl2WlWjP6uuucz0LwLuaxgOTuEd5sJ/eH1GW2OIZzeqa7ZU5FzipuV0fyVCWaXn1UWDsHH05PRSD6adhN8AB6ItIc9HujFhHB8NIyvwWnpQSjS27ESHUNpjZaZNS0mIYSSkyNBMaQ0pJA6FvYCB/9kJXsWnRAhhP2fRFPJq3mHruaSCiAdAWkdGzfWHhbnGY4plQhUIxI0D6E5U8dH38sNuYyxqFMhLmhcHcjqKUohbDSEoUzFz+JyWkRjCSQUCPTwHG53EsAGmP+NmwCnVxtCV9OXv5Vn9SVac2onc9MpqYLJn1QEzFGEwyeQRTGdM+rbIyzC6+qEByg90tHsFIRjmomBdJBwZCkGnhdGD0R+3muEndi+svkoxBAQiDUARVRrOeryolpO7WOvPiDDeSR6EghDFvLmpalrYKhMIRTAipeZgUMkcCVKSOYIJIZe6hIBJbLAhpRlv4QxwISbNnCVQ0qR8UmBY1dx5ARZOys/+0pMzE52CIA1LRgeq4WfX2tth7bK9vp9fIGKxxjoOBDU36k+8hEODQBCGrhQIRNhcxQUmWQhwFkaj+2AahbwQTqqeHkMdwwHp6CE1b0zhPHJb5MsIEGIi4AqaOMRyokRFCUqQajHtmZESxn0RuEKUGuyPTeFCDN4i0XDEi+OzgDaEoB6vsfWscRN0QE9LCB5GodpjM8JJQ5TjJBj76M2WJZg5nTBZeUEeyZZ8RyOEdlAUabXOghaGYisnjYwAxDAVAjOaW0Zgijc2jUVnMs99YU4tjKqjyaTYX1OCYCtoFoU+MkfX0/TfyojpYpgflyF9hCNHeeCTz28oA9HgQL87bIvLfyUsAPRwSiKg0CadDJyjKqrGMhZVzdc0Z/1BaF81UnObEeiBLY1Y9LkjLch/IUbbbx8WUkdmOTlZfACR1sIyF0fdPj39CCOOxzIVxLsgQ2rpwpvJqz3VzZRySoajSEcNcUQNjLAfA6PVBjF5/CzDmMRBDKdjzNVaF+lpqFHMxEC2/gTGW848UPQI0dw7IWJLGuuJQjuKi4rgUdoTIXE4LZCpJzw10IEj79paunA2EabsBMW23j/7aXEuNYiqmOd9kroiHMpTFMMFM2iGeqUCNDayBKMW9qwkhZxBKziCkVCHqHnbrNb0NLQwTADN7AtRQaowLN0+8R6yzrdCX2AMzlRbkhfv4BCCrBTKURI+KG8thIIZS0pko1NJSUvmo1HNSNFzpB0oUveknhHj58QmEmBoHQBBAr9TAmMqpj2eaK+KQgEQBzWIHcFDyymOkcPJqOFN5Sfh8fPoSopJxSKaismCPClxGbMoBxpUhnqnAHEG9Tw7JUFQWkO7YP34NsNTbgTKVBTH7Vd6kHpVCY1WYa6lRzMXQqwgg5DAcc0FnEKNfi2MqKAdoYAzEXIrrIW8LMMB0oCBklav5+S4oINYCxxAhRCbJYwBVeA0WgLAioNMNGGEtlqGw9joOY10dKFNZQQSjqcExFMTuXjVXxAEZSioPuBnrqVEAxJSny0AE1UgwogDaHI8EIioHWGflkRT8wowvhde5z72/4TYf30hYBPSScLAwRsyVjfqKFEFc3aSbm41xrb4JWFjJZNYRBUWwB5bchYWRXO13g8hsoaCkmS1cd6RBOWJ03Ox1rwwZUQh9h3sfNi7wlyKHbkzz8HayANy45uFhskCAM4OpMKe2QYIRZr6sx2kDXNlr3MFFsR5U9HXQgAQGvwFVwwYJSFiR7TyYbr2FgpE2f5m+ijKNC/PnhAniJqoo+xbCJMo9NAh+iDcbyfCJw7TPeYGjfYCfTA4F3F58/HhxY2rRVijuzd2d/KnSKu9Omw2nQnG6YIa1rQKTr2YCVQD1n8selKoa7Jut/qKfB3U7CANiI7ghehCdVxlLS1KVZ0f1GhbbwK+OHqnFNu0ue4UEo9z+n20TfF4dTrzT1eF04PRvOmpQZiufuijJ6rd0YJSxr3pHZdJ05aUpVxr093fOn5NwF+HcucF5sss88q+8QPnWiVBRkJynWbIP6GKWs8ExzlCRZE4D5UzT0TkHiv0cirOLN0tczsrAaFu0OdIM+zguAhTmLl5npydg/OPAM1Jw7CXV1cswCrp4M8QRCmIo0hZrhnAQhFGbbizq4tS+O3sLELRDzBkBtNFtoVrvVq7tgvDxaOJ2C0Q5tjIz3WaBWLuAMm0UhJdHk6jBMKRT8/Ie6c/Dk5BSlPgLitIQl8eRnTpk/M+jxyH7dmc5ip9r1iYajrUNvVZCOX3EDjmbp12jYmtcqFDQdeFB4ZWx64VQvHHVFOEHvA88TIuQ2lmfaHFe/HJzenL18fjk2rmsT3vrgZ9f3V7eOufUXjNEwpGf7dmd604ZD1cXh9ZXAwBzCTnOyiIPkrzIMIrMsCZiD9Zu23XLrDrchWhI08E5LqyzqRecaDUHCq6JJGhS26nRY6VydANTmuZ5ak1AB1dyumfYeIskCb0tsWAhmnGV7xGkCHlZ8gGvgziggUrbAbDtX09OT345Oz75ePX6jWzi20/uT9fX7vXN5+uLm7vLi1uFhOefr64//+niT3fu+U93P336/JHAXNySz7IYVz9ffLj7dEuA/vTL5Uf3l8tPF+X4XS58kt//Iyz+m2untZX3gyr+9e1P7vnNX6/vPs9SdW7oUqS6ubtzf729cH+6vergvpQuz2bFy729OL+7/Pynrr5/7pLiv9uHqs89cNZN/NJdGSl8749/nH1SruugIx35P/23e371yt143grJKCAPyjwm9VAk81CckFbUe5A20s8pK3P64dKnLknNt6udt2oacln0Sfn93GNkxrrrvCXStxT4y1H0agnydY98ne5fHeX9KbQd6jBBhYsego6APFkXax1+OpqQF5jN09dPrYg5MH1tUEn4KciLhrSRF9Iq67apB7a90xKqZgHlOY4ehHloHvsmMxEG8aMoB9Uz7K9bLwvSolMJ/kDmOf/AXuGgXZGQmY5TPUnnHavQt1856kDeLjFXyoDeHXVFtusbZRY0+KhAoPwKLyZGMRm+XNIvfo2cJ1FQuOuMdMNumpSxj76CiJh6OHg4/Vqvn/BnRREs/OLZqgK+Qmk5Gi6fb8+l93f4KPO7g+Lbtwtwf/kywf7HPx4f2+d/QlkcxJt8hcLwKxR9Q1+69nxNASn2UVwEXtc0QWG+pAiU4Sp2Vf41ZNAnouA3VIJ2DKTgNzn6CD3icsREWbSidnaBsg0u+vwTjw3M0qOIfPO9mnFqKKHY7qKHngj2nX3yvmV8FJFvvmf28ZF//HoREaM2MpFCvz8i33+vYC8PSNqOT6infXSqhz4ic8TvZbvpGXy2Fygnhu4pT3XalSDpnhtY0tj4fbSOk6Pq268iaMKgKWXxvy1Xl+rO1u2PeEd/Kb9ZtpjsqtEql+FQfPQX9t1XKhtrilTKZ3J0PPpsXH9VS8amFpUymZ4wHq3pb0ftb8sW0GLCVEprfop/tK5//2qltrhApfY3P0U/WtMHjsoHjpoHFm6Wy0tUKcFvY+kOsLy/0QxNrqlOP/mtZmV8HWfisenFVf01VUNF5ariYE3xiEynsJd/T39dlf9cQku90OVWn90IpV1V/5fh3b84ukLp93/4z8+/3l3/eud+uLz5L+cP/3l98/l/Ls7v/vTT1cV/rcrEEpqrzbFVQGoGW+Duy/VLzxc3SbuWROnqF22OT9KT05P12fHJJhrszpoVU5B7SnX96C/h4dHRxpsafWB6IXOZgsolTN93T9HL1zbJi2UzpiaUyFpVng1l5cyD05NKrl+sKvcGv7wvv9y9W23i3Yrr/Gm8gX5+OMDe09VDqzJgclJscRaS7C37TkePCMw8v45wnpMyOApxvCm23/e3420XNZ3AqxQ2//z/78WtYA8oN8Ev5EMDd/QUFNujofe59XbIBjRBxVCF84LM24Uo83GKYx/H3rPefsC3k6OY9MH+wPqQX83Xa7jjw60Jlsqree8wO6P89OL9j1+ikD5a3TRDHj5evSwT197g5Ktf7345IrbFjxVAbac0bjE7bxUl/o60qBwXu3R1XrqvXVePXZMy/rkU3rgnr0rHHZKeIKU4K55vPfL39/R+PWYB8UWfEoAy57cFTn8g8jufF8qSj9doFxa3uCjKjSCFvDj2VJkUsUVZO48J033l/ROGbHoy1hV3ffZXXubVPnZeVjGy+0maZkC+qi19f7zb4o/AdevWweHB7eXV9afL88u7v7q3d79+uPzM+TgevDv41/0B54b/7p58cU+sfLTHpL4m3uOfafSsB+pTf09/vq8eoP8jPUAakKf8x0+JV60csh/e1f/YYO8xcXP/0X21Ol0d118f1v+gjq6f02G68bkC+/H36g+CcvChquX/Xrp/Jy+lkkDdE3LyDv7WvgRMZrHv6Cs+bKpa6TJLc8icwKkjaOW12572IU8nWbAJYhSyx+NdGB42N3uRL44PS4QCx7S83r55++b07dnL178fKpFz3pn0XIgW99Hxd2/O3rx8+/bsWJa9nu+TvKM0DYPqvZVXFmhKeHv65ru33x2/eqUhAe8JirtFsR9W119o8L988/rs5dnbl2+N+T0NfpL90zdvTl+fnPz+94Pf/x9ud51u=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA