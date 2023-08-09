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
# SIMPLICITY_STUDIO_METADATA=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END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA