#
#    Copyright (c) 2021 Project CHIP Authors
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

COMPONENT_DEPENDS := chip

COMPONENT_SRCDIRS :=                                                                \
  .                                                                                 \

COMPONENT_ADD_INCLUDEDIRS += ../third_party/connectedhomeip/third_party/pigweed/repo/pw_sys_io/public       \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public_overrides            \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public                      \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public                  \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/standard_library_public \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public_overrides        \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_status/public                    \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_preprocessor/public              \
                ../third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/system_server/public         \
                ../third_party/connectedhomeip/examples/platform/esp32/pw_sys_io/public                     \

COMPONENT_PRIV_INCLUDEDIRS := .
