#
#   Copyright (c) 2020-2023 Project CHIP Authors
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
#     CMake sub-project to configure and build the chip library.
#

include(ExternalProject)

# ==============================================================================
# Prepare CHIP configuration based on the project configuration
# ==============================================================================
# Set paths
if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. REALPATH)
endif()
get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/mbed/chip-gn REALPATH)
get_filename_component(COMMON_CMAKE_SOURCE_DIR ${CHIP_ROOT}/config/common/cmake REALPATH)

# Get common Cmake sources
include(${COMMON_CMAKE_SOURCE_DIR}/util.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn_args.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn.cmake)

# Mbed targets passed to CHIP build
set(CONFIG_CHIP_EXTERNAL_TARGETS)

# Read configuration file and parse it content to create cmake variable
file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/config ConfigContents)
foreach(NameAndValue ${ConfigContents})
  # Strip leading spaces
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
  # Find variable name
  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
  # Find the value
  string(REPLACE "${Name}=" "" Value ${NameAndValue})
  # Set the variable
  set(${Name} "${Value}")
endforeach()
if (CONFIG_CHIP_PW_RPC)
    if (${APP_TARGET} MATCHES "pigweed-app")
        set(CONFIG_CHIP_PW_RPC_ECHO_PROTO "y")
    elseif (${APP_TARGET} MATCHES "lighting-app")
        set(CONFIG_CHIP_PW_RPC_COMMON_PROTO "y")
        set(CONFIG_CHIP_PW_RPC_LIGHTING_PROTO "y")
    elseif (${APP_TARGET} MATCHES "lock-app")
        set(CONFIG_CHIP_PW_RPC_COMMON_PROTO "y")
        set(CONFIG_CHIP_PW_RPC_LOCKING_PROTO "y")
    endif()
endif(CONFIG_CHIP_PW_RPC)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CONFIG_CHIP_DEBUG YES)
else()
    set(CONFIG_CHIP_DEBUG NO)
endif()

# Prepare compiler flags
matter_add_cflags(${CMAKE_C_FLAGS_INIT})
matter_add_cxxflags(${CMAKE_CXX_FLAGS_INIT})

# Create a list of external targets passed to CHIP build
list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
    mbed-core
    mbed-cmsis-cortex-m

    mbed-rtos
    mbed-events

    mbed-os-posix-socket
    mbed-netsocket
    mbed-ble

    mbed-mbedtls

    mbed-storage-kv-global-api
)

if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W")
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        mbed-cy8cproto-062-4343w
        mbed-psoc6
        mbed-cat1a
        mbed-cy8cproto-062-4343w-bsp-design-modus
    )
endif()

# Get compiler flags from external targets
matter_get_compiler_flags_from_targets("${CONFIG_CHIP_EXTERNAL_TARGETS}")

# Additional settings
matter_add_flags(-D__LINUX_ERRNO_EXTENSIONS__=1)
matter_add_flags(-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>)

matter_add_gnu_cpp_standard("17")

if (CONFIG_MBED_BSD_SOCKET_TRACE)
    matter_add_flags(-DMBED_BSD_SOCKET_TRACE=1)
endif()

# Option can be set with `-DCHIP_DATA_MODEL_INTERFACE=enabled` or similar on the command line
set(CHIP_DATA_MODEL_INTERFACE "disabled" CACHE STRING "Data model interface option to use: enabled or disabled")

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
matter_common_gn_args(
    DEBUG           CONFIG_CHIP_DEBUG
    LIB_SHELL       CONFIG_CHIP_LIB_SHELL
    LIB_TESTS       CONFIG_CHIP_BUILD_TESTS
    LIB_PW_RPC      CONFIG_CHIP_PW_RPC
    PROJECT_CONFIG  ${CONFIG_CHIP_PROJECT_CONFIG}
    DATA_MODEL_INTERFACE ${CHIP_DATA_MODEL_INTERFACE}
)
if (CONFIG_CHIP_PW_RPC)
    matter_add_gn_arg_import("${GN_ROOT_TARGET}/lib/pw_rpc/pw_rpc.gni")
endif()
matter_add_gn_arg_string("mbed_ar"                              ${CMAKE_AR})
matter_add_gn_arg_string("mbed_cc"                              ${CMAKE_C_COMPILER})
matter_add_gn_arg_string("mbed_cxx"                             ${CMAKE_CXX_COMPILER})
matter_add_gn_arg_bool  ("chip_monolithic_tests"                CONFIG_CHIP_BUILD_TESTS)
matter_add_gn_arg_bool  ("chip_with_platform_mbedtls"           CONFIG_CHIP_WITH_EXTERNAL_MBEDTLS)
matter_add_gn_arg_bool  ("chip_enable_data_model"               CONFIG_CHIP_DATA_MODEL)
if (CONFIG_CHIP_PW_RPC)
    matter_add_gn_arg_bool  ("chip_build_pw_rpc_echo_proto"         CONFIG_CHIP_PW_RPC_ECHO_PROTO)
    matter_add_gn_arg_bool  ("chip_build_pw_rpc_common_proto"       CONFIG_CHIP_PW_RPC_COMMON_PROTO)
    matter_add_gn_arg_bool  ("chip_build_pw_rpc_lighting_proto"     CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)
    matter_add_gn_arg_bool  ("chip_build_pw_rpc_locking_proto"      CONFIG_CHIP_PW_RPC_LOCKING_PROTO)
endif(CONFIG_CHIP_PW_RPC)
if (CONFIG_CHIP_OTA_REQUESTOR)
    matter_add_gn_arg_bool  ("chip_enable_ota_requestor"            CONFIG_CHIP_OTA_REQUESTOR)
endif(CONFIG_CHIP_OTA_REQUESTOR)

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================
matter_build(chip
    LIB_SHELL   ${CONFIG_CHIP_LIB_SHELL}
    LIB_TESTS   ${CONFIG_CHIP_BUILD_TESTS}
    LIB_PW_RPC  ${CONFIG_CHIP_PW_RPC}
)

# ==============================================================================
# Define mbed target configuration according to CHIP component usage
# ==============================================================================
# CHIP includes path
list(APPEND CHIP_INCLUDES)

# CHIP defines
list(APPEND CHIP_DEFINES)

# Target specific configuration
if("capsense" IN_LIST MBED_TARGET_LABELS)
    add_subdirectory(${CHIP_ROOT}/third_party/mbed-os-cypress-capsense-button/repo ${CMAKE_BINARY_DIR}/capsense_build)
    target_link_libraries(${APP_TARGET} capsense)
endif()

list(APPEND CHIP_INCLUDES
    ${CHIP_ROOT}/config/mbed/mbedtls
)

list(APPEND CHIP_DEFINES
    __LINUX_ERRNO_EXTENSIONS__=1
)

list(APPEND CHIP_DEFINES
    CHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>
)

if (CONFIG_MBED_BSD_SOCKET_TRACE)
    list(APPEND CHIP_DEFINES
        MBED_BSD_SOCKET_TRACE=1
    )
endif()

if (CONFIG_CHIP_PW_RPC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17" PARENT_SCOPE)
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC=1
    )
endif()

if (CONFIG_CHIP_PW_RPC)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")

target_sources(${APP_TARGET} PRIVATE
    ${CHIP_ROOT}/examples/common/pigweed/RpcService.cpp
    ${CHIP_ROOT}/examples/common/pigweed/mbed/PigweedLoggerMutex.cpp
    ${CHIP_ROOT}/examples/common/pigweed/mbed/Rpc.cpp
    ${MBED_COMMON}/util/PigweedLogger.cpp
)

list(APPEND CHIP_DEFINES
    PW_RPC_USE_GLOBAL_MUTEX=0
)

# TODO: Update this to use target_link_libraries instead of
# target_include_directories. target_include_directories never should be used to
# access other libraries since it does not add source files to the build graph
# and does not support transitive dependencies.
target_include_directories(${APP_TARGET} PRIVATE
                    ${PIGWEED_ROOT}/pw_sys_io/public
                    ${PIGWEED_ROOT}/pw_assert/public
                    ${PIGWEED_ROOT}/pw_assert/assert_lite_public_overrides
                    ${PIGWEED_ROOT}/pw_assert_log/assert_backend_public_overrides
                    ${PIGWEED_ROOT}/pw_assert_log/check_backend_public_overrides
                    ${PIGWEED_ROOT}/pw_assert_log/public
                    ${PIGWEED_ROOT}/pw_bytes/public
                    ${PIGWEED_ROOT}/pw_checksum/public
                    ${PIGWEED_ROOT}/pw_containers/public
                    ${PIGWEED_ROOT}/pw_hdlc/public
                    ${PIGWEED_ROOT}/pw_log/public
                    ${PIGWEED_ROOT}/pw_log_basic/public
                    ${PIGWEED_ROOT}/pw_log_basic/public_overrides
                    ${PIGWEED_ROOT}/pw_span/public_overrides
                    ${PIGWEED_ROOT}/pw_span/public
                    ${PIGWEED_ROOT}/pw_string/public
                    ${PIGWEED_ROOT}/pw_sync/public
                    ${PIGWEED_ROOT}/pw_polyfill/public
                    ${PIGWEED_ROOT}/pw_polyfill/standard_library_public
                    ${PIGWEED_ROOT}/pw_polyfill/public_overrides
                    ${PIGWEED_ROOT}/pw_rpc/public
                    ${PIGWEED_ROOT}/pw_rpc/nanopb/public
                    ${PIGWEED_ROOT}/pw_rpc/raw/public
                    ${PIGWEED_ROOT}/pw_protobuf/public
                    ${PIGWEED_ROOT}/pw_status/public
                    ${PIGWEED_ROOT}/pw_stream/public
                    ${PIGWEED_ROOT}/pw_result/public
                    ${PIGWEED_ROOT}/pw_varint/public
                    ${PIGWEED_ROOT}/pw_function/public
                    ${PIGWEED_ROOT}/pw_preprocessor/public
                    ${PIGWEED_ROOT}/pw_rpc/system_server/public
                    ${PIGWEED_ROOT}/pw_toolchain/public
                    ${PIGWEED_ROOT}/third_party/fuchsia/repo/sdk/lib/fit/include
                    ${PIGWEED_ROOT}/third_party/fuchsia/repo/sdk/lib/stdcompat/include
                    ${CHIP_ROOT}/third_party/nanopb/repo

                    ${CHIP_ROOT}/examples/common
                    ${CHIP_ROOT}/examples/common/pigweed
                    ${CHIP_ROOT}/examples/common/pigweed/mbed
                    ${MBED_COMMON}/pw_sys_io/public

                    ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb
                    ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/pigweed/repo/pw_protobuf/common_protos.proto_library/nanopb
)

if (CONFIG_CHIP_PW_RPC_ECHO_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_ECHO_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_ECHO_PROTO)

if (CONFIG_CHIP_PW_RPC_COMMON_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/button_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/button_service.proto_library/nanopb_rpc

        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/device_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/device_service.proto_library/nanopb_rpc
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_COMMON_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_COMMON_PROTO)

if (CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/lighting_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/lighting_service.proto_library/nanopb_rpc
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_LIGHTING_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)

if (CONFIG_CHIP_PW_RPC_LOCKING_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/locking_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/examples/common/pigweed/locking_service.proto_library/nanopb_rpc
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_LOCKING_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_LOCKING_PROTO)

endif(CONFIG_CHIP_PW_RPC)

if (CONFIG_CHIP_OTA_REQUESTOR)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CHIP_ROOT}/examples/platform/mbed/ota/
        ${CHIP_ROOT}/src/app/clusters/ota-requestor
        ${CHIP_ROOT}/src/platform
        ${CHIP_ROOT}/src/platform/mbed
        ${CHIP_ROOT}/src/include/platform
    )

    target_sources(${APP_TARGET} PRIVATE
        ${CHIP_ROOT}/examples/platform/mbed/ota/OTARequestorDriverImpl.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/BDXDownloader.cpp
        ${CHIP_ROOT}/src/platform/mbed/OTAImageProcessorImpl.cpp
    )

if (NOT ${APP_TARGET} MATCHES "shell")
    target_sources(${APP_TARGET} PRIVATE
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/ota-requestor-server.cpp
    )
else()
    target_include_directories(${APP_TARGET} PRIVATE ${GEN_DIR}/ota-requestor-app)
endif()

    list(APPEND CHIP_DEFINES
        CHIP_OTA_REQUESTOR=1
    )
endif(CONFIG_CHIP_OTA_REQUESTOR)

if(BOOT_ENABLED)
    add_subdirectory(${MCUBOOT_PATH}/boot/bootutil/ ${CMAKE_BINARY_DIR}/mbed_mcu_boot_util_build)
    add_subdirectory(${MCUBOOT_PATH}/boot/mbed/ ${CMAKE_BINARY_DIR}/mbed_mcu_boot_build)

    target_include_directories(${APP_TARGET} PRIVATE
        ${MCUBOOT_PATH}/boot/mbed/include
    )

    target_sources(${APP_TARGET} PRIVATE
        ${CHIP_ROOT}/examples/platform/mbed/bootloader/default_bd.cpp
    )

    target_include_directories(bootutil PUBLIC
                           ${CHIP_ROOT}/config/mbed/mbedtls
    )

    target_link_libraries(${APP_TARGET} mbed-mcuboot bootutil)

    file(READ ${APP_PATH}/mbed_app.json mbedAppJson)
    string(JSON PRIMARY_SLOT_ADDRESS GET "${mbedAppJson}" target_overrides ${MBED_TARGET} mcuboot.primary-slot-address)
    string(JSON HEADER_SIZE GET "${mbedAppJson}" target_overrides ${MBED_TARGET} mcuboot.header-size)
    string(JSON SLOT_SIZE GET "${mbedAppJson}" target_overrides ${MBED_TARGET} mcuboot.slot-size)
    math(EXPR APP_START "${PRIMARY_SLOT_ADDRESS} + ${HEADER_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR APP_SIZE "${SLOT_SIZE} - 2 * ${HEADER_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    target_compile_definitions(mbed-core
        INTERFACE
            "-DMBED_APP_START=${APP_START}"
            "-DMBED_APP_SIZE=${APP_SIZE}"
    )

    list(APPEND CHIP_DEFINES
        BOOT_ENABLED=1
    )
endif()


target_include_directories(${APP_TARGET} PRIVATE
                           ${CHIP_INCLUDES}
)

target_compile_definitions(${APP_TARGET} PRIVATE
                           ${CHIP_DEFINES}
)
