#
#    Copyright (c) 2025 Project CHIP Authors
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
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(CAMERA_COMMON_DIR ${CHIP_ROOT}/examples/camera-app/camera-common REALPATH)
get_filename_component(APP_COMMON_GEN_DIR ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated REALPATH)
get_filename_component(KVS_SDK_PATH ${CHIP_ROOT}/third_party/amazon-kinesis-video-streams-webrtc-sdk-c REALPATH)

set(PRIV_INCLUDE_DIRS_LIST
                      "${CHIP_ROOT}/examples/providers"
                      "${CHIP_ROOT}/examples/platform/esp32"
                      "${CHIP_ROOT}/src/app/clusters/camera-av-stream-management-server"
                      "${CHIP_ROOT}/src/app/clusters/webrtc-transport-provider-server"
                      "${CAMERA_COMMON_DIR}/include/camera-avstream-controller"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      "${CMAKE_CURRENT_LIST_DIR}/common"
                      "${CMAKE_CURRENT_LIST_DIR}/clusters"
                      "${CMAKE_CURRENT_LIST_DIR}/webrtc"
)

set(SRC_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_CURRENT_LIST_DIR}/common"
                      "${CMAKE_CURRENT_LIST_DIR}/clusters"
                      "${CMAKE_CURRENT_LIST_DIR}/webrtc"
                      "${APP_COMMON_GEN_DIR}/attributes"
                      "${APP_COMMON_GEN_DIR}"
                      "${CHIP_ROOT}/src/app/clusters/camera-av-stream-management-server"
                      "${CHIP_ROOT}/examples/providers"
                      "${CHIP_ROOT}/examples/platform/esp32/ota"
                      "${CHIP_ROOT}/examples/platform/esp32/common"
                      "${CHIP_ROOT}/examples/platform/esp32/shell_extension"
)


idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST})

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
chip_configure_data_model(${COMPONENT_LIB}
    ZAP_FILE ${CMAKE_SOURCE_DIR}/data_model/camera-app.zap
)

if (CONFIG_ENABLE_ESP_INSIGHTS_TRACE)
    target_add_binary_data(${COMPONENT_TARGET} "insights_auth_key.txt" TEXT)
endif()

if (CONFIG_ENABLE_SET_CERT_DECLARATION_API)
    target_add_binary_data(${COMPONENT_TARGET} "certification_declaration.der" BINARY)
endif()

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

if (CONFIG_ENABLE_ENCRYPTED_OTA)
	create_esp_enc_img(${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
			   ${project_dir}/esp_image_encryption_public_key.pem
			   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-encrypted.bin
			   app)
endif()
