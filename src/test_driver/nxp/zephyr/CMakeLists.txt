#
# Copyright (c) 2023-2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/zephyr/chip-module)
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/third_party/pigweed/repo)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)
target_compile_options(app PRIVATE -Werror -Wno-error=format)

pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.zephyr)

project(chip-nxp-test-driver-app)

include(${CHIP_ROOT}/config/nxp/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_include_directories(app
    PRIVATE
    main/include
)


target_sources(app
    PRIVATE
    main/main.cpp
)


if(NOT "${CONFIG_NXP_CUSTOM_UNIT_TESTS_DIR}" STREQUAL "")
# read test_components.txt file which contain the list of NXP test modules to be built
file(STRINGS ${CONFIG_NXP_CUSTOM_UNIT_TESTS_DIR}/test_components.txt TEST_NAMES_FROM_FILE)
else()
# read test_components.txt file which contain the list of Matter test modules to be built
file(STRINGS ../test_components.txt TEST_NAMES_FROM_FILE)
endif()

foreach(TEST_NAME IN LISTS TEST_NAMES_FROM_FILE)
# add the the path of the library to be linked
list(APPEND tests_list "${CMAKE_CURRENT_BINARY_DIR}/modules/connectedhomeip/lib/${TEST_NAME}.a")
endforeach()

# Link the *whole-archives* to keep the static test objects for each tests library previously selected.
target_link_options(app
PUBLIC
    -Wl,--whole-archive
    "${CMAKE_CURRENT_BINARY_DIR}/modules/connectedhomeip/lib/libPWTestsWrapper.a"
    -Wl,--no-whole-archive
    -Wl,--start-group,--whole-archive
    ${tests_list}
    -Wl,--end-group,--no-whole-archive
    )

target_link_libraries(app PUBLIC
  chip
  $<TARGET_FILE:kernel>
  pw_unit_test
)

target_compile_definitions(app PUBLIC CHIP_HAVE_CONFIG_H)