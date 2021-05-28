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

# The list of src and include dirs must be in sync with that in all-clusters-app/esp32/main/CMakeLists.txt
COMPONENT_SRCDIRS :=                                                                \
  .                                                                                 \
  ../third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/gen  \
  ../third_party/connectedhomeip/src/app/server                                     \
  ../third_party/connectedhomeip/src/app/util                                       \
  ../third_party/connectedhomeip/src/app/clusters/test-cluster-server               \
  ../third_party/connectedhomeip/src/app/clusters/descriptor                        \
  ../third_party/connectedhomeip/src/app/clusters/on-off-server                     \
  ../third_party/connectedhomeip/src/app/clusters/level-control                     \
  ../third_party/connectedhomeip/src/app/clusters/identify                          \
  ../third_party/connectedhomeip/src/app/clusters/account-login-server              \
  ../third_party/connectedhomeip/src/app/clusters/application-launcher-server       \
  ../third_party/connectedhomeip/src/app/clusters/audio-output-server               \
  ../third_party/connectedhomeip/src/app/clusters/barrier-control-server            \
  ../third_party/connectedhomeip/src/app/clusters/general-commissioning-server      \
  ../third_party/connectedhomeip/src/app/clusters/groups-server                     \
  ../third_party/connectedhomeip/src/app/clusters/color-control-server              \
  ../third_party/connectedhomeip/src/app/clusters/content-launch-server             \
  ../third_party/connectedhomeip/src/app/clusters/low-power-server                  \
  ../third_party/connectedhomeip/src/app/clusters/keypad-input-server               \
  ../third_party/connectedhomeip/src/app/clusters/media-playback-server             \
  ../third_party/connectedhomeip/src/app/clusters/operational-credentials-server    \
  ../third_party/connectedhomeip/src/app/clusters/trusted-root-certificates-server  \
  ../third_party/connectedhomeip/src/app/clusters/media-input-server                \
  ../third_party/connectedhomeip/src/app/clusters/network-commissioning             \
  ../third_party/connectedhomeip/src/app/clusters/ota-server                        \
  ../third_party/connectedhomeip/src/app/clusters/target-navigator-server           \
  ../third_party/connectedhomeip/src/app/clusters/temperature-measurement-server    \
  ../third_party/connectedhomeip/src/app/clusters/tv-channel-server                 \
  ../third_party/connectedhomeip/src/app/clusters/scenes                            \
  ../third_party/connectedhomeip/src/app/clusters/basic                             \
  ../third_party/connectedhomeip/src/app/clusters/bindings                          \
  ../third_party/connectedhomeip/src/app/reporting                                  \
  ../third_party/connectedhomeip/src/app/clusters/door-lock-server                  \
  ../third_party/connectedhomeip/src/app/clusters/occupancy-sensor-server           \
   ../third_party/connectedhomeip/src/app/clusters/ias-zone-server                  \
   ../third_party/connectedhomeip/src/app/clusters/relative-humidity-measurement-server \
#  ../third_party/connectedhomeip/src/app/clusters/ias-zone-client                  \


COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/third_party/connectedhomeip/src/app/util                                   \
                            $(PROJECT_PATH)/third_party/connectedhomeip/src/app/server                                 \
                            $(PROJECT_PATH)/third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common  \
                            $(PROJECT_PATH)/third_party/connectedhomeip/third_party/nlio/repo/include                  \
                            $(PROJECT_PATH)/third_party/connectedhomeip/src

# So "gen/*" files are found by the src/app bits.
COMPONENT_PRIV_INCLUDEDIRS := .
