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

PROJECT_NAME := chip-wifi-echo

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/third_party/connectedhomeip/config/esp32/components \
                        $(PROJECT_PATH)/../../../common/m5stack-tft/repo/components \
                        $(PROJECT_PATH)/../../../common/QRCode \
                        $(PROJECT_PATH)/../../../common/screen-framework \

CXXFLAGS += -std=c++11 -Os
CPPFLAGS += -Os
CFLAGS += -Os

include $(IDF_PATH)/make/project.mk
