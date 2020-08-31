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
#    Description:
#      Component makefile for the ESP32 demo application.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_DEPENDS := chip QRCode tft spidriver

COMPONENT_SRCDIRS :=                                                           \
  .                                                                            \
  gen                                                                          \
  ../third_party/connectedhomeip/src/app/util                                  \
  ../third_party/connectedhomeip/src/app/clusters/on-off-server                \


COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/third_party/connectedhomeip/src/app/util

# So "gen/*" files are found by the src/app bits.
COMPONENT_PRIV_INCLUDEDIRS := .
