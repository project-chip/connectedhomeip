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

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA