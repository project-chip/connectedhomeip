#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := chip-crypto-tests

CXXFLAGS += -DCHIP_SUPPORT_FOREIGN_TEST_DRIVERS -DCHIP_CONFIG_ENABLE_ARG_PARSER

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/config/esp32/components
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/src/crypto
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/third_party/nlunit-test

include $(IDF_PATH)/make/project.mk
