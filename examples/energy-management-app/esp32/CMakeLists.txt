#
#    Copyright (c) 2021-2023 Project CHIP Authors
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

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "v1.0")
set(PROJECT_VER_NUMBER 1)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/examples/common/cmake/idf_flashing.cmake)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/config/esp32/components"
    "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/examples/common"
)

if(${IDF_TARGET} STREQUAL "esp32")
    list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/examples/common/m5stack-tft/repo/components")
endif()


project(chip-energy-management-app)

# WARNING: This is just an example for using key for decrypting the encrypted OTA image
# Please do not use it as is.
if(CONFIG_ENABLE_ENCRYPTED_OTA)
    target_add_binary_data(chip-energy-management-app.elf "esp_image_encryption_key.pem" TEXT)
endif()

# C++17 is required for RPC build.
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For the C3, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=maybe-uninitialized" APPEND)

#For the H2, -Werror=uninitialized will cause an error in "src/lib/support/LambdaBridge.h"
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=uninitialized" APPEND)

flashing_script()

if (CONFIG_ENABLE_PW_RPC)
get_filename_component(CHIP_ROOT ./third_party/connectedhomeip REALPATH)
include(third_party/connectedhomeip/third_party/pigweed/repo/pw_build/pigweed.cmake)
include($ENV{PW_ROOT}/pw_assert/backend.cmake)
include($ENV{PW_ROOT}/pw_log/backend.cmake)
include($ENV{PW_ROOT}/pw_sys_io/backend.cmake)
include($ENV{PW_ROOT}/pw_trace/backend.cmake)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)

pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.esp32)
pw_set_backend(pw_trace pw_trace_tokenized)

add_subdirectory(third_party/connectedhomeip/third_party/pigweed/repo)
add_subdirectory(third_party/connectedhomeip/third_party/nanopb/repo)
add_subdirectory(third_party/connectedhomeip/examples/platform/esp32/pw_sys_io)

endif(CONFIG_ENABLE_PW_RPC)
