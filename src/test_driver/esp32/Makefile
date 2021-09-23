#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := chip-tests

CXXFLAGS += -DCHIP_SUPPORT_FOREIGN_TEST_DRIVERS -DCHIP_TARGET_STYLE_EMBEDDED -Wno-deprecated-declarations

CXXFLAGS += -DLWIP_IPV6_SCOPES=0 -std=gnu++14
CPPFLAGS += -DLWIP_IPV6_SCOPES=0 -DCHIP_HAVE_CONFIG_H
CFLAGS += -DLWIP_IPV6_SCOPES=0 -std=gnu11

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/config/esp32/components

include $(IDF_PATH)/make/project.mk

esp32_elf_builder: all
	mkdir -p build/chip/
	echo "#!/bin/sh" > build/chip/esp32_elf_builder.sh
	echo set -e >> build/chip/esp32_elf_builder.sh
	echo set -x >> build/chip/esp32_elf_builder.sh
	echo $(CXX) $(CXXFLAGS) $(CPPFLAGS) -L$(PROJECT_PATH)/build/chip/lib -Wl,--whole-archive '$$1' -Wl,--no-whole-archive \
		 -lnlunit-test $(LDFLAGS) -lnlfaultinjection '$$2' -o $(PROJECT_PATH)/build/chip-tests.elf -Wl,-Map=$(APP_MAP) >> build/chip/esp32_elf_builder.sh
	echo $(ESPTOOLPY) elf2image $(ESPTOOL_FLASH_OPTIONS) $(ESPTOOL_ELF2IMAGE_OPTIONS) \
		 -o $(PROJECT_PATH)/build/chip/chip-tests.bin $(PROJECT_PATH)/build/chip-tests.elf >> build/chip/esp32_elf_builder.sh
	ln -sf $(PROJECT_PATH)/build/partitions.bin $(PROJECT_PATH)/build/chip/partitions.bin
	mkdir -p build/chip/bootloader
	ln -sf $(PROJECT_PATH)/build/bootloader/bootloader.bin $(PROJECT_PATH)/build/chip/bootloader.bin
	ln -sf $(PROJECT_PATH)/idf.sh $(PROJECT_PATH)/build/chip/env.sh
