cmake_minimum_required(VERSION 3.6)

project(chip_main)

set(chip_dir "${ameba_matter_root}")
set(chip_dir_output "${matter_output_path}/chip")
set(dir "${sdk_root}/component/common/api")
set(chip_main chip_main)
set(list_chip_main_sources chip_main_sources)
set(pigweed_dir "${chip_dir}/third_party/pigweed/repo")

include(${prj_root}/GCC-RELEASE/project_hp/asdk/includepath.cmake)

include(${pigweed_dir}/pw_build/pigweed.cmake)
include(${pigweed_dir}/pw_protobuf_compiler/proto.cmake)

set(dir_pw_third_party_nanopb "${chip_dir}/third_party/nanopb/repo" CACHE STRING "" FORCE)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.ameba)

add_subdirectory(${chip_dir}/third_party/pigweed/repo ${chip_dir}/examples/pigweed-app/ameba/out/pigweed)
add_subdirectory(${chip_dir}/third_party/nanopb/repo ${chip_dir}/examples/pigweed-app/ameba/out/nanopb)
add_subdirectory(${chip_dir}/examples/platform/ameba/pw_sys_io ${chip_dir}/examples/pigweed-app/ameba/out/pw_sys_io)

list(
    APPEND ${list_chip_main_sources}

    #chip app
    ${chip_dir}/examples/pigweed-app/ameba/main/main.cpp
    ${chip_dir}/examples/platform/ameba/PigweedLogger.cpp
    ${chip_dir}/examples/common/pigweed/RpcService.cpp
    ${chip_dir}/examples/common/pigweed/ameba/PigweedLoggerMutex.cpp
)

add_library(
    ${chip_main}
    STATIC
    ${chip_main_sources}
)

target_include_directories(
    ${chip_main}
    PUBLIC
	${inc_path}
    ${chip_dir}/examples/platform/ameba
    ${chip_dir}/examples/platform/ameba/pw_sys_io/public
    ${chip_dir}/examples/common
    ${chip_dir}/examples/common/pigweed
    ${chip_dir}/examples/common/pigweed/ameba
    ${chip_dir}/src
    ${chip_dir}/src/lib/support
    ${pigweed_dir}/pw_rpc/nanopb/public

    ${chip_dir_output}/gen/include
)

target_link_libraries(${chip_main} PUBLIC
    pw_checksum
    pw_hdlc
    pw_log
    pw_rpc.nanopb.echo_service
    pw_rpc.server
    PwRpc
)

link_directories(
    ${chip_dir_output}/lib
)

list(
    APPEND chip_main_flags

    -DINET_CONFIG_ENABLE_IPV4=0
    -DCHIP_PROJECT=1
    -DCHIP_DEVICE_LAYER_TARGET=Ameba
    -DUSE_ZAP_CONFIG
    -DCHIP_HAVE_CONFIG_H
    -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
    -DCONFIG_ENABLE_PW_RPC=1
)

if (matter_enable_persistentstorage_audit)
list(
    APPEND chip_main_flags

    -DCHIP_SUPPORT_ENABLE_STORAGE_API_AUDIT
)
endif (matter_enable_persistentstorage_audit)

list(
    APPEND chip_main_cpp_flags

	-Wno-unused-parameter
	-std=gnu++17
	-fno-rtti
  -fno-use-cxa-atexit
)
target_compile_definitions(${chip_main} PRIVATE ${chip_main_flags} )
target_compile_options(${chip_main} PRIVATE ${chip_main_cpp_flags})

# move static library post build command
add_custom_command(
    TARGET ${chip_main}
    POST_BUILD
    COMMAND cp lib${chip_main}.a ${CMAKE_CURRENT_SOURCE_DIR}/lib/application
)
