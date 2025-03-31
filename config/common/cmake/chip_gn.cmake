#
#   Copyright (c) 2023 Project CHIP Authors
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

#
#   @file
#     CMake file defining to setup and build the Matter library
#     and other optional libraries like unit tests.
#     Matter headers and libraries are exposed to the application
#     as a specific interface target.
#     Since Matter doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system.
#

# ==============================================================================
# Validate paths
# ==============================================================================
if (NOT GN_ROOT_TARGET)
    message(FATAL_ERROR "GN_ROOT_TARGET not defined. Please provide the path to your Matter GN project.")
endif()

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../.. REALPATH)
endif()

if (NOT CHIP_APP_ZAP_DIR)
    get_filename_component(CHIP_APP_ZAP_DIR ${CHIP_ROOT}/zzz_generated/app-common REALPATH)
endif()

# ==============================================================================
# Find required programs
# ==============================================================================
find_package(Python3 REQUIRED)
find_program(GN_EXECUTABLE gn REQUIRED)

# Parse the 'gn --version' output to find the installed version.
set(MIN_GN_VERSION 1851)
execute_process(
    COMMAND ${GN_EXECUTABLE} --version
    OUTPUT_VARIABLE GN_VERSION
    COMMAND_ERROR_IS_FATAL ANY
)
if (GN_VERSION VERSION_LESS MIN_GN_VERSION)
    message(FATAL_ERROR "Found unsupported version of gn: ${MIN_GN_VERSION}+ is required")
endif()

# ==============================================================================
# Macros
# ==============================================================================
# Setup and build the Matter library and other optional libraries like unit tests.
# Expose Matter headers & libraries to the application as specific
# interface target.
# [Args]:
#   target - interface target name
# Available options are:
#   LIB_SHELL       Build and add Matter shell library
#   LIB_PW_RPC      Build and add Matter PW RPC library
#   LIB_TESTS       Build and add Matter unit tests library
#   LIB_MBEDTLS     Build and add Matter mbedtls library
#   DEVICE_INFO_EXAMPLE_PROVIDER Add example device info provider support
#
#   GN_DEPENDENCIES List of targets that should be built before Matter GN project
macro(matter_build target)
    set(options)
    set(oneValueArgs
        LIB_TESTS
        LIB_SHELL
        LIB_PW_RPC
        LIB_MBEDTLS
        DEVICE_INFO_EXAMPLE_PROVIDER
        FORCE_LOGGING_STDIO
    )
    set(multiValueArgs GN_DEPENDENCIES)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(MATTER_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)

    # Prepare Matter libraries that the application should be linked with
    if (FORCE_LOGGING_STDIO)
        set(MATTER_LIBRARIES -lCHIPWithStdioLogging)
    else()
        set(MATTER_LIBRARIES -lCHIP)
    endif()

    if (ARG_LIB_MBEDTLS)
        list(APPEND MATTER_LIBRARIES -lmbedtls)
    endif()

    if (ARG_LIB_SHELL)
        list(APPEND MATTER_LIBRARIES -lCHIPShell)
    endif()

    if (ARG_LIB_PW_RPC)
        list(APPEND MATTER_LIBRARIES -lPwRpc)
    endif()

    if (ARG_DEVICE_INFO_EXAMPLE_PROVIDER)
        list(APPEND MATTER_LIBRARIES -lMatterDeviceInfoProviderExample)
    endif()

    list(TRANSFORM MATTER_LIBRARIES REPLACE
        "-l(.*)"
        "${MATTER_LIB_DIR}/lib\\1.a"
    )

    # ==============================================================================
    # Define 'chip-gn' target that builds CHIP library(ies) with GN build system
    # ==============================================================================
    ExternalProject_Add(
        chip-gn
        PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
        SOURCE_DIR              ${CHIP_ROOT}
        BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
        CONFIGURE_COMMAND       ""
        CONFIGURE_HANDLED_BY_BUILD TRUE
        BUILD_COMMAND           ${CMAKE_COMMAND} -E echo "Starting Matter library build in ${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND                 ${Python3_EXECUTABLE} ${CHIP_ROOT}/config/common/cmake/make_gn_args.py @args.tmp > args.gn.tmp
        # Replace the config only if it has changed to avoid triggering unnecessary rebuilds
        COMMAND                 bash -c "(! diff -q args.gn.tmp args.gn && mv args.gn.tmp args.gn) || true"
        # Regenerate the ninja build system
        COMMAND                 ${GN_EXECUTABLE}
                                    --root=${CHIP_ROOT}
                                    --root-target=${GN_ROOT_TARGET}
                                    --dotfile=${GN_ROOT_TARGET}/.gn
                                    --script-executable=${Python3_EXECUTABLE}
                                    gen --check --fail-on-unused-args --add-export-compile-commands=*
                                    ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND                 ninja
        COMMAND                 ${CMAKE_COMMAND} -E echo "Matter library build complete"
        INSTALL_COMMAND         ""
        # Byproducts are removed by the clean target removing config and .ninja_deps
        # allows a rebuild of the external project after the clean target has been run.
        BUILD_BYPRODUCTS        ${CMAKE_CURRENT_BINARY_DIR}/args.gn
                                ${CMAKE_CURRENT_BINARY_DIR}/build.ninja
                                ${CMAKE_CURRENT_BINARY_DIR}/.ninja_deps
                                ${CMAKE_CURRENT_BINARY_DIR}/build.ninja.stamp
                                ${MATTER_LIBRARIES}
        BUILD_ALWAYS            TRUE
        USES_TERMINAL_CONFIGURE TRUE
        USES_TERMINAL_BUILD     TRUE
    )

    if(ARG_GN_DEPENDENCIES)
        add_dependencies(chip-gn ${ARG_GN_DEPENDENCIES})
    endif()

    # ==============================================================================
    # Define target that exposes Matter headers & libraries to the application
    # ==============================================================================
    add_library(${target} INTERFACE)
    target_compile_definitions(${target} INTERFACE CHIP_HAVE_CONFIG_H)
    target_include_directories(${target} INTERFACE
        ${CHIP_ROOT}/src
        ${CHIP_ROOT}/src/include
        ${CHIP_ROOT}/third_party/nlassert/repo/include
        ${CHIP_ROOT}/third_party/nlio/repo/include
        ${CHIP_ROOT}/third_party/nlfaultinjection/include
        ${CHIP_APP_ZAP_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/gen/include
    )

    if (ARG_LIB_MBEDTLS)
        target_include_directories(${target} INTERFACE
            ${CHIP_ROOT}/third_party/mbedtls/repo/include
        )
    endif()

    # ==============================================================================
    # Link required libraries
    # ==============================================================================
    target_link_libraries(${target} INTERFACE -Wl,--start-group ${MATTER_LIBRARIES} -Wl,--end-group)
    add_dependencies(${target} chip-gn)

    if (ARG_LIB_SHELL)
        target_link_options(${target} INTERFACE -Wl,--whole-archive ${MATTER_LIB_DIR}/libCHIPShell.a -Wl,--no-whole-archive)
    endif()

    if (ARG_LIB_TESTS)
        target_link_options(${target} INTERFACE -Wl,--whole-archive ${MATTER_LIB_DIR}/libCHIP_tests.a -Wl,--no-whole-archive)
    endif()

    if (ARG_DEVICE_INFO_EXAMPLE_PROVIDER)
        target_include_directories(${target} INTERFACE ${CHIP_ROOT}/examples/providers)
    endif()
endmacro()
