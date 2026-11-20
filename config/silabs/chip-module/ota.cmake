#
#   Copyright (c) 2026 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# Silabs OTA backend hook. Must be included while the GN arguments are being
# collected (before matter_generate_args_tmp_file).

if(CONFIG_CHIP_OTA_BACKEND_SIWX91X)
    # Compiled into the Zephyr platform library (not a separate GN source_set) so
    # CHIPDeviceLayer.h include checks pass.
    matter_add_gn_arg_list("chip_zephyr_ota_image_processor_sources"
        "\"//src/platform/silabs/zephyr/OTAImageProcessorImplSiWx.cpp\""
        "\"//src/platform/silabs/zephyr/OTAImageProcessorImplSiWx.h\"")
    zephyr_compile_definitions(CHIP_ZEPHYR_OTA_IMAGE_PROCESSOR_HEADER="platform/silabs/zephyr/OTAImageProcessorImplSiWx.h")

    # The Matter OTA image wraps the SoC-generated RPS. The SoC post-build commands
    # write it alongside zephyr.bin without a CMake rule of its own, so depend on
    # zephyr.bin instead.
    if(CONFIG_SIWX91X_SIGN_KEY OR CONFIG_SIWX91X_MIC_KEY)
        set(_chip_siwx_rps ${CMAKE_BINARY_DIR}/zephyr/zephyr.signed.rps)
    else()
        set(_chip_siwx_rps ${CMAKE_BINARY_DIR}/zephyr/zephyr.rps)
    endif()
    set_property(GLOBAL PROPERTY CHIP_OTA_IMAGE_INPUT_FILES ${_chip_siwx_rps})
    set_property(GLOBAL PROPERTY CHIP_OTA_IMAGE_DEPENDS ${CMAKE_BINARY_DIR}/zephyr/zephyr.bin)
endif()
