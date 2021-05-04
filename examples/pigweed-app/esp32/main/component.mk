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
  ../../../common/pigweed															\
  ../../../common/pigweed/esp32                                                     \
  ../../../platform/esp32                                                           \

COMPONENT_ADD_INCLUDEDIRS += ../third_party/connectedhomeip/third_party/pigweed/repo/pw_sys_io/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_assert/public    			\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log/public            \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log/public_overrides  \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_bytes/public    			    \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_checksum/public    			\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_containers/public    		\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_hdlc/public    				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_log/public    			    \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic/public    			\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic/public_overrides   \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public_overrides 		\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_span/public 					\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/standard_library_public \
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_polyfill/public_overrides 	\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/public 					\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/nanopb/public 			\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/raw/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_protobuf/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_status/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_stream/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_result/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_varint/public 				\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_preprocessor/public 			\
							../third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/system_server/public 	\
							../third_party/connectedhomeip/third_party/nanopb/repo                                  \
							../../../platform/esp32/pw_sys_io/public 				\
							../../../platform/esp32                                 \
							../../../common/pigweed									\
							../../../common/pigweed/esp32                           \
							../build/chip/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb \
							../build/chip/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc \
							../build/chip/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb \
							../../../../src/lib/support    \

COMPONENT_EXTRA_INCLUDES := ${IDF_PATH}/components/freertos/include/freertos/      \

# So "gen/*" files are found by the src/app bits.
COMPONENT_PRIV_INCLUDEDIRS := .

WRAP_FUNCTIONS = esp_log_write
WRAP_ARGUMENT := -Wl,--wrap=

COMPONENT_ADD_LDFLAGS = -l$(COMPONENT_NAME) $(addprefix $(WRAP_ARGUMENT),$(WRAP_FUNCTIONS))
