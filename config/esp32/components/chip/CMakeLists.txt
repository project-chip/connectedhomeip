#
#   Copyright (c) 2021 Project CHIP Authors
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

# The compile flags written args.gn come from chip.c and chip.cpp.
# Any build specifications (include dirs, compile flags, definitions, etc)
# that is set in the component will be reflected in
# the output args.gn. By default, all components inherit build-level
# specifications stored as idf-build-properties as done in
# tools/cmake/build.cmake.
#
# Adding REQUIRES/PRIV_REQUIRES will also propagate the
# appropriate build specifications from the required component.
idf_build_get_property(idf_path IDF_PATH)

if(NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. REALPATH)
endif()

set(CHIP_REQURIE_COMPONENTS freertos lwip bt mdns mbedtls fatfs)
if (CONFIG_ENABLE_CHIP_SHELL)
    list(APPEND CHIP_REQURIE_COMPONENTS console)
endif()

idf_component_register(SRCS chip.c chip.cpp
                       PRIV_REQUIRES ${CHIP_REQURIE_COMPONENTS})

# Prepare initial args file (lacking compile flags)
# This will be saved as args.gn.in
set(chip_gn_args "import(\"//args.gni\")\n")

macro(chip_gn_arg_append arg val)
    string(APPEND chip_gn_args "${arg} = ${val}\n")
endmacro()

macro(chip_gn_arg_bool arg boolean)
    if (${boolean})
        string(APPEND chip_gn_args "${arg} = true\n")
    else()
        string(APPEND chip_gn_args "${arg} = false\n")
    endif()
endmacro()

chip_gn_arg_append("esp32_ar"              "\"${CMAKE_AR}\"")
chip_gn_arg_append("esp32_cc"              "\"${CMAKE_C_COMPILER}\"")
chip_gn_arg_append("esp32_cxx"             "\"${CMAKE_CXX_COMPILER}\"")
chip_gn_arg_append("esp32_cpu"             "\"esp32\"")
chip_gn_arg_bool("is_debug"                is_debug)

if(CONFIG_ENABLE_PW_RPC)
    string(APPEND chip_gn_args "import(\"//build_overrides/pigweed.gni\")\n")
    chip_gn_arg_append("chip_build_pw_rpc_lib"              "true")
    chip_gn_arg_append("pw_log_BACKEND"                     "\"//third_party/connectedhomeip/third_party/pigweed/repo/pw_log_basic\"")
    chip_gn_arg_append("pw_assert_BACKEND"                  "\"//third_party/connectedhomeip/third_party/pigweed/repo/pw_assert_log\"")
    chip_gn_arg_append("pw_sys_io_BACKEND"                  "\"//third_party/connectedhomeip/examples/platform/esp32/pw_sys_io:pw_sys_io_esp32\"")
    chip_gn_arg_append("dir_pw_third_party_nanopb"          "\"//third_party/connectedhomeip/third_party/nanopb/repo\"")
    chip_gn_arg_append("pw_build_LINK_DEPS"                 "[\"\$dir_pw_assert:impl\", \"\$dir_pw_log:impl\"]")
endif()

if (NOT CONFIG_USE_MINIMAL_MDNS)
    chip_gn_arg_append("chip_mdns"                          "\"platform\"")
endif()

if (CONFIG_ENABLE_CHIP_SHELL)
    chip_gn_arg_append("chip_build_libshell"                "true")
endif()


set(args_gn_input "${CMAKE_CURRENT_BINARY_DIR}/args.gn.in")
file(GENERATE OUTPUT "${args_gn_input}" CONTENT "${chip_gn_args}")

# This generates the final args.in file to be fed to
# CHIP build using GN build system. The necessary compile_commands.json
# and args.gn.in input file should exist by the time this is invoked,
# since these requirements ar generated at the parent project's
# GENERATE phase.
idf_build_get_property(idf_ver IDF_VER)
set(filter_out "-DHAVE_CONFIG_H" "-DIDF_VER=\\\"${idf_ver}\\\"" )

# CMake adds config include dir relative to build directory.
# Make sure full path is given to CHIP build.
list(APPEND filter_out "-Iconfig")
idf_build_get_property(config_dir CONFIG_DIR)
target_compile_options(${COMPONENT_LIB} PRIVATE "-I${config_dir}")

if(filter_out)
    set(filter_arg "--filter-out=${filter_out}")
endif()

set(args_gn "${CMAKE_CURRENT_BINARY_DIR}/args.gn")
add_custom_command(OUTPUT "${args_gn}"
                   COMMAND ${python} 
                           ${CMAKE_CURRENT_LIST_DIR}/create_args_gn.py 
                           "${CMAKE_BINARY_DIR}"
                           "${idf_path}"
                           "${CMAKE_CURRENT_LIST_DIR}/chip.c"
                           "${CMAKE_CURRENT_LIST_DIR}/chip.cpp"
                           "${args_gn_input}"
                           "${args_gn}"
                           "${filter_arg}"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
                   VERBATIM)

add_custom_target(args_gn DEPENDS "${args_gn}")

# CHIP build as an external project.
find_program(GN_EXECUTABLE gn)
if (${GN_EXECUTABLE} STREQUAL GN_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "The 'gn' command was not found. Make sure you have GN installed."
                        "Or have followed necessary build preparations stated in BUILDING.md.")
endif()

set(GN_ROOT_TARGET ${CHIP_ROOT}/config/esp32)

set(chip_libraries "${CMAKE_CURRENT_BINARY_DIR}/lib/libCHIP.a")

if(CONFIG_ENABLE_PW_RPC)
    list(APPEND chip_libraries "${CMAKE_CURRENT_BINARY_DIR}/lib/libPwRpc.a")
endif()

externalproject_add(
    chip_gn
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       ${GN_EXECUTABLE} --root=${GN_ROOT_TARGET} gen --check --fail-on-unused-args ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND           ninja "esp32"
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        ${chip_libraries}
    WORKING_DIRECTORY       ${CMAKE_CURRENT_LIST_DIR}
    DEPENDS                 args_gn
    BUILD_ALWAYS            1
)

idf_component_get_property(freertos_dir freertos COMPONENT_DIR)

# ESP-IDF components usually need 'freertos/<header.h>', while
# CHIP might include do 'header.h'.
target_include_directories(${COMPONENT_LIB} PRIVATE
    "${freertos_dir}/include/freertos")

target_include_directories(${COMPONENT_LIB} INTERFACE
    "${CHIP_ROOT}/src/platform/ESP32"
    "${CHIP_ROOT}/src/include"
    "${CHIP_ROOT}/src/lib"
    "${CHIP_ROOT}/src"
    "${CHIP_ROOT}/third_party/nlassert/repo/include"
    "${CHIP_ROOT}/third_party/nlio/repo/include"
    "${CMAKE_CURRENT_BINARY_DIR}/src/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/include"
)

idf_component_get_property(esp32_mbedtls_lib esp32_mbedtls COMPONENT_LIB)

if(CONFIG_BT_ENABLED)
    idf_component_get_property(bt_lib bt COMPONENT_LIB)
    list(APPEND chip_libraries $<TARGET_FILE:${bt_lib}> -lbtdm_app)
endif()

if (CONFIG_ENABLE_CHIP_SHELL)
    idf_component_get_property(console_lib console COMPONENT_LIB)
    list(APPEND chip_libraries $<TARGET_FILE:${console_lib}>)
endif()

if(NOT CONFIG_USE_MINIMAL_MDNS)
    idf_component_get_property(mdns_lib mdns COMPONENT_LIB)
    list(APPEND chip_libraries $<TARGET_FILE:${mdns_lib}>)
endif()

idf_component_get_property(main_lib main COMPONENT_LIB)
list(APPEND chip_libraries $<TARGET_FILE:${main_lib}>)

target_link_libraries(${COMPONENT_LIB} INTERFACE -Wl,--start-group 
                                                ${chip_libraries}
                                                $<TARGET_FILE:mbedcrypto> $<TARGET_FILE:${esp32_mbedtls_lib}> 
                                                -Wl,--end-group)

# Make the component dependent on our CHIP build
add_dependencies(${COMPONENT_LIB} chip_gn)
