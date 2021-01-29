#
#    Copyright (c) 2020 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := chip-temperature-measurement-app

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/config/esp32/components \
                        $(PROJECT_PATH)/../../common/m5stack-tft/repo/components \
                        $(PROJECT_PATH)/../../common/QRCode \
                        $(PROJECT_PATH)/../../common/screen-framework \

CXXFLAGS += -std=c++14 -Os -DLWIP_IPV6_SCOPES=0
CPPFLAGS += -Os -DLWIP_IPV6_SCOPES=0 -DCHIP_HAVE_CONFIG_H
CFLAGS += -Os -DLWIP_IPV6_SCOPES=0

top: all flashing_script

include $(IDF_PATH)/make/project.mk

FLASHING_SCRIPT=$(BUILD_DIR_BASE)/$(PROJECT_NAME).flash.py

$(FLASHING_SCRIPT): $(APP_BIN) $(BOOTLOADER_BIN) $(PARTITION_TABLE_BIN) $(PROJECT_PATH)/sdkconfig
	@third_party/connectedhomeip/scripts/flashing/gen_flashing_script.py esp32 \
	  --output $(BUILD_DIR_BASE)/$(PROJECT_NAME).flash.py \
	  --port $(ESPPORT) --baud $(ESPBAUD) --before $(CONFIG_ESPTOOLPY_BEFORE) --after $(CONFIG_ESPTOOLPY_AFTER) \
	  --application $(subst $(BUILD_DIR_BASE)/,,$(APP_BIN)) \
	  --bootloader $(subst $(BUILD_DIR_BASE)/,,$(BOOTLOADER_BIN)) \
	  --partition $(subst $(BUILD_DIR_BASE)/,,$(PARTITION_TABLE_BIN)) \
	  --use-partition-file $(PARTITION_TABLE_BIN) \
	  --use-parttool $(IDF_PATH)/components/partition_table/parttool.py \
	  --use-sdkconfig $(PROJECT_PATH)/sdkconfig

flashing_script: $(FLASHING_SCRIPT) $(BUILD_DIR_BASE)/esp32_firmware_utils.py $(BUILD_DIR_BASE)/firmware_utils.py
	@echo To flash $(subst $(CURDIR)/,,$(APP_BIN)), run $(subst $(CURDIR)/,,$(FLASHING_SCRIPT))

$(BUILD_DIR_BASE)/esp32_firmware_utils.py: third_party/connectedhomeip/scripts/flashing/esp32_firmware_utils.py
	@cp $< $@

$(BUILD_DIR_BASE)/firmware_utils.py: third_party/connectedhomeip/scripts/flashing/firmware_utils.py
	@cp $< $@

.PHONY: flashing_script
