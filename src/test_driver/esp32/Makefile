#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := chip-tests

CXXFLAGS += -DCHIP_SUPPORT_FOREIGN_TEST_DRIVERS

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/config/esp32/components

include $(IDF_PATH)/make/project.mk

esp32_elf_builder:
	mkdir -p build/chip/
	echo $(CC) -L$(PROJECT_PATH)/build/chip/lib -Wl,--whole-archive '$$1' -Wl,--no-whole-archive \
		 -lnlunit-test $(LDFLAGS) -o $(PROJECT_PATH)/build/chip-tests.elf -Wl,-Map=$(APP_MAP) > build/chip/esp32_elf_builder.sh
	echo $(ESPTOOLPY) elf2image $(ESPTOOL_FLASH_OPTIONS) $(ESPTOOL_ELF2IMAGE_OPTIONS) \
		 -o $(PROJECT_PATH)/build/chip-tests.bin $(PROJECT_PATH)/build/chip-tests.elf >> build/chip/esp32_elf_builder.sh
