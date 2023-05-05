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
# SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA