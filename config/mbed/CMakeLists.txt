#
#   Copyright (c) 2020 Project CHIP Authors
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
#     CMake sub-project defining 'chip' target which represents CHIP library
#     and other optional libraries like unit tests, built with 'mbed'
#     platform.
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system.
#

include(ExternalProject)
include(mbed-util.cmake)

# ==============================================================================
# Declare configuration variables and define constants
# ==============================================================================
# C/C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS)

# C compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_C)

# C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_CC)

# CHIP libraries that the application should be linked with
list(APPEND CHIP_LIBRARIES)

# GN meta-build system arguments in the form of 'key1 = value1\nkey2 = value2...' string
string(APPEND CHIP_GN_ARGS)

# C/C++ compiler flags which should not be forwarded to CHIP
# build system (e.g. because CHIP configures them on its own)
set(CHIP_CFLAG_EXCLUDES
    "-fno-asynchronous-unwind-tables"
    "-fno-common"
    "-fno-defer-pop"
    "-fno-reorder-functions"
    "-ffunction-sections"
    "-fdata-sections"
    "-g*"
    "-O*"
    "-W*"
)

# ==============================================================================
# Helper macros
# ==============================================================================

macro(chip_gn_arg_import FILE)
    string(APPEND CHIP_GN_ARGS "import(\"${FILE}\")\n")
endmacro()

macro(chip_gn_arg_string ARG STRING)
    string(APPEND CHIP_GN_ARGS "${ARG} = \"${STRING}\"\n")
endmacro()

macro(chip_gn_arg_bool ARG BOOLEAN)
    if (${BOOLEAN})
        string(APPEND CHIP_GN_ARGS "${ARG} = true\n")
    else()
        string(APPEND CHIP_GN_ARGS "${ARG} = false\n")
    endif()
endmacro()

macro(chip_gn_arg_flags ARG CFLAGS)
    list(SORT CFLAGS)
    string(APPEND CHIP_GN_ARGS "${ARG} = [${CFLAGS}]\n")
endmacro()

macro(chip_gn_arg_lang_flags ARG CFLAGS)
    list(SORT CFLAGS)
    list(SORT CHIP_CFLAG_EXCLUDES)
    set(CFLAG_EXCLUDES "[")
    foreach(cflag ${CHIP_CFLAG_EXCLUDES})
        string(APPEND CFLAG_EXCLUDES "\"${cflag}\", ")
    endforeach()
    string(APPEND CFLAG_EXCLUDES "]")
    string(APPEND CHIP_GN_ARGS "${ARG} = filter_exclude(string_split(\"${CFLAGS}\"), ${CFLAG_EXCLUDES})\n")
endmacro()

macro(mbed_interface_library_named name)
  add_library(${name} INTERFACE)
endmacro()

# Select gnu++<YY> standard based on project configuration
macro(mbed_get_gnu_cpp_standard VAR)
    if (CONFIG_STD_CPP11)
        list(APPEND ${VAR} -std=gnu++11)
    elseif (CONFIG_STD_CPP14)
        list(APPEND ${VAR} -std=gnu++14)
    elseif (CONFIG_STD_CPP17)
        list(APPEND ${VAR} -std=gnu++17)
    elseif (CONFIG_STD_CPP2A)
        list(APPEND ${VAR} -std=gnu++20)
    endif()
endmacro()

# ==============================================================================
# Prepare CHIP configuration based on the project configuration
# ==============================================================================
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
    set(CONFIG_STD_CPP17 y)
endif()

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. REALPATH)
endif()

set(GN_ROOT_TARGET ${CHIP_ROOT}/config/mbed/chip-gn)

# Prepare compiler flags

mbed_get_target_common_compile_flags(CHIP_CFLAGS mbed-core)
mbed_get_lang_compile_flags(CHIP_CFLAGS_C mbed-core C)
list(APPEND CHIP_CFLAGS_C ${CMAKE_C_FLAGS_INIT})
mbed_get_lang_compile_flags(CHIP_CFLAGS_CC mbed-core CXX)
list(APPEND CHIP_CFLAGS_CC ${CMAKE_CXX_FLAGS_INIT})

mbed_get_target_common_compile_flags(CHIP_MBEDCMSISCM_CFLAGS mbed-cmsis-cortex-m)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDCMSISCM_CFLAGS})
if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W")
    mbed_get_target_common_compile_flags(CHIP_MBEDCMSISCY8_CFLAGS mbed-cy8cproto-062-4343w)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDCMSISCY8_CFLAGS})
    mbed_get_target_common_compile_flags(CHIP_MBEDPSOC6_CFLAGS mbed-psoc6)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDPSOC6_CFLAGS})
    mbed_get_target_common_compile_flags(CHIP_MBEDCAT1_CFLAGS mbed-cat1a)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDCAT1_CFLAGS})
    mbed_get_target_common_compile_flags(CHIP_MBEDCMSISCY8MODUS_CFLAGS  mbed-cy8cproto-062-4343w-bsp-design-modus)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDCMSISCY8MODUS_CFLAGS})
endif()

# Add support for Mbed Posix Socket
mbed_get_target_common_compile_flags(CHIP_MBEDPOSIXSOCKET_CFLAGS mbed-os-posix-socket)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDPOSIXSOCKET_CFLAGS})

# Add support for Mbed BLE
mbed_get_target_common_compile_flags(CHIP_MBEDBLE_CFLAGS mbed-ble)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDBLE_CFLAGS})

# Add support for Mbed event
mbed_get_target_common_compile_flags(CHIP_MBEDEVENTS_CFLAGS mbed-events)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDEVENTS_CFLAGS})

# Add support for Mbed rtos
mbed_get_target_common_compile_flags(CHIP_MBEDRTOS_CFLAGS mbed-rtos)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDRTOS_CFLAGS})

# Add support for Mbed storage
mbed_get_target_common_compile_flags(CHIP_MBEDSTORAGE_CFLAGS mbed-storage-kv-global-api)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDSTORAGE_CFLAGS})

# Add support for Mbed Socket
mbed_get_target_common_compile_flags(CHIP_MBEDNETSOCKET_CFLAGS mbed-netsocket)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDNETSOCKET_CFLAGS})

if (CONFIG_CHIP_WITH_EXTERNAL_MBEDTLS)
    mbed_get_target_common_compile_flags(CHIP_MBEDTLS_CFLAGS mbed-mbedtls)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDTLS_CFLAGS})
endif()

mbed_get_gnu_cpp_standard(CHIP_CFLAGS_CC)

list(APPEND CHIP_CFLAGS
    \"-D__LINUX_ERRNO_EXTENSIONS__=1\"
)

list(APPEND CHIP_CFLAGS
    \"-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>\"
)

if (CONFIG_MBED_BSD_SOCKET_TRACE)
    list(APPEND CHIP_CFLAGS
        \"-DMBED_BSD_SOCKET_TRACE=1\"
    )
endif()

# CFLAGS are put in random order, sort them before converting them to a string
list(SORT CHIP_CFLAGS)
list(SORT CHIP_CFLAGS_C)
list(SORT CHIP_CFLAGS_CC)

set(SEPARATOR ",")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS ${SEPARATOR})
set(SEPARATOR " ")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_C CHIP_CFLAGS_C ${SEPARATOR})
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_CC CHIP_CFLAGS_CC ${SEPARATOR})

# Prepare CHIP libraries that the application should be linked with

if (NOT CHIP_LIBRARIES)
    set(CHIP_LIBRARIES -lCHIP)
endif()

if (CONFIG_CHIP_LIB_SHELL)
    list(APPEND CHIP_LIBRARIES -lCHIPShell)
endif()

if (CONFIG_CHIP_PW_RPC)
    list(APPEND CHIP_LIBRARIES -lPwRpc)
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

# Set up CHIP project configuration file

set(CHIP_DEFAULT_CONFIG_FILE "<${CHIP_ROOT}/config/mbed/CHIPProjectConfig.h>")
set(CHIP_PROJECT_CONFIG         ${CHIP_DEFAULT_CONFIG_FILE})

if (CONFIG_CHIP_PROJECT_CONFIG)
    get_filename_component(CHIP_PROJECT_CONFIG
        ${CONFIG_CHIP_PROJECT_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    set(CHIP_PROJECT_CONFIG "<${CHIP_PROJECT_CONFIG}>")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "debug")
    set(CONFIG_DEBUG "y")
endif()

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
if (CONFIG_CHIP_PW_RPC)
    chip_gn_arg_import("${GN_ROOT_TARGET}/lib/pw_rpc/pw_rpc.gni")
endif()

chip_gn_arg_flags("target_cflags"                         ${CHIP_CFLAGS})
chip_gn_arg_lang_flags("target_cflags_c"                  ${CHIP_CFLAGS_C})
chip_gn_arg_lang_flags("target_cflags_cc"                 ${CHIP_CFLAGS_CC})
chip_gn_arg_string("mbed_ar"                              ${CMAKE_AR})
chip_gn_arg_string("mbed_cc"                              ${CMAKE_C_COMPILER})
chip_gn_arg_string("mbed_cxx"                             ${CMAKE_CXX_COMPILER})
chip_gn_arg_string("chip_project_config_include"          "${CHIP_PROJECT_CONFIG}")
chip_gn_arg_bool  ("is_debug"                             CONFIG_DEBUG)
chip_gn_arg_bool  ("chip_build_tests"                     CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_monolithic_tests"                CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_build_libshell"                  CONFIG_CHIP_LIB_SHELL)
chip_gn_arg_bool  ("chip_with_platform_mbedtls"           CONFIG_CHIP_WITH_EXTERNAL_MBEDTLS)
chip_gn_arg_bool  ("chip_build_pw_rpc_lib"                CONFIG_CHIP_PW_RPC)
chip_gn_arg_bool  ("chip_enable_data_model"               CONFIG_CHIP_DATA_MODEL)
if (CONFIG_CHIP_PW_RPC)
    chip_gn_arg_bool  ("chip_build_pw_rpc_echo_proto"         CONFIG_CHIP_PW_RPC_ECHO_PROTO)
    chip_gn_arg_bool  ("chip_build_pw_rpc_common_proto"       CONFIG_CHIP_PW_RPC_COMMON_PROTO)
    chip_gn_arg_bool  ("chip_build_pw_rpc_lighting_proto"     CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)
    chip_gn_arg_bool  ("chip_build_pw_rpc_locking_proto"      CONFIG_CHIP_PW_RPC_LOCKING_PROTO)
endif(CONFIG_CHIP_PW_RPC)
if (CONFIG_CHIP_OTA_REQUESTOR)
    chip_gn_arg_bool  ("chip_enable_ota_requestor"            CONFIG_CHIP_OTA_REQUESTOR)
endif(CONFIG_CHIP_OTA_REQUESTOR)


file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/args.gn CONTENT ${CHIP_GN_ARGS})

# ==============================================================================
# Define 'chip-gn' target that builds CHIP library(ies) with GN build system
# ==============================================================================
ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       gn --root=${GN_ROOT_TARGET} gen --export-compile-commands --check --fail-on-unused-args ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND           ninja
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        ${CHIP_LIBRARIES}
    CONFIGURE_ALWAYS        TRUE
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)

# ==============================================================================
# Define 'chip' target that exposes CHIP headers & libraries to the application
# ==============================================================================
mbed_interface_library_named(chip)
target_compile_definitions(chip INTERFACE CHIP_HAVE_CONFIG_H)
target_include_directories(chip INTERFACE
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
)
target_link_directories(chip INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/lib)
target_link_libraries(chip INTERFACE -Wl,--start-group ${CHIP_LIBRARIES} -Wl,--end-group)
add_dependencies(chip chip-gn)

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
                    ${CHIP_ROOT}/third_party/nanopb/repo

                    ${CHIP_ROOT}/examples/common
                    ${CHIP_ROOT}/examples/common/pigweed
                    ${CHIP_ROOT}/examples/common/pigweed/mbed
                    ${MBED_COMMON}/pw_sys_io/public

                    ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/pwpb
                    ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_protobuf/common_protos.proto_library/nanopb
)

if (CONFIG_CHIP_PW_RPC_ECHO_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb_rpc
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/third_party/pigweed/repo/pw_rpc/protos.proto_library/nanopb
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_ECHO_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_ECHO_PROTO)

if (CONFIG_CHIP_PW_RPC_COMMON_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/button_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/button_service.proto_library/nanopb_rpc

        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/device_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/device_service.proto_library/nanopb_rpc
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_COMMON_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_COMMON_PROTO)

if (CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/lighting_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/lighting_service.proto_library/nanopb_rpc
    )
    list(APPEND CHIP_DEFINES
        CHIP_PW_RPC_LIGHTING_PROTO=1
    )
endif(CONFIG_CHIP_PW_RPC_LIGHTING_PROTO)

if (CONFIG_CHIP_PW_RPC_LOCKING_PROTO)
    target_include_directories(${APP_TARGET} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/locking_service.proto_library/nanopb
        ${CMAKE_CURRENT_BINARY_DIR}/protocol_buffer/gen/third_party/connectedhomeip/examples/common/pigweed/locking_service.proto_library/nanopb_rpc
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
    include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
    chip_configure_data_model(${APP_TARGET}
        ZAP_FILE ${CHIP_ROOT}/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap
        GEN_DIR ${CHIP_ROOT}/zzz_generated/ota-requestor-app/zap-generated
    )
    target_include_directories(${APP_TARGET} PRIVATE
                           ${GEN_DIR}/ota-requestor-app
    )
    target_sources(${APP_TARGET} PRIVATE
               ${GEN_DIR}//ota-requestor-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/ota-requestor-app/zap-generated/IMClusterCommandHandler.cpp
    )
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
