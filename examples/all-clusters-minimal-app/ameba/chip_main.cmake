cmake_minimum_required(VERSION 3.6)

project(chip_main)

set(chip_dir "${ameba_matter_root}")
set(chip_dir_output "${matter_output_path}/chip")
set(dir "${sdk_root}/component/common/api")
set(chip_main chip_main)
set(list_chip_main_sources chip_main_sources)

if (matter_enable_rpc)
set(pigweed_dir "${chip_dir}/third_party/pigweed/repo")

include(${pigweed_dir}/pw_build/pigweed.cmake)
include(${pigweed_dir}/pw_protobuf_compiler/proto.cmake)

set(dir_pw_third_party_nanopb "${chip_dir}/third_party/nanopb/repo" CACHE STRING "" FORCE)

pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.ameba)
pw_set_backend(pw_trace pw_trace_tokenized)

add_subdirectory(${chip_dir}/third_party/pigweed/repo ${chip_dir}/examples/all-clusters-minimal-app/ameba/out/pigweed)
add_subdirectory(${chip_dir}/third_party/nanopb/repo ${chip_dir}/examples/all-clusters-minimal-app/ameba/out/nanopb)
add_subdirectory(${chip_dir}/examples/platform/ameba/pw_sys_io ${chip_dir}/examples/all-clusters-minimal-app/ameba/out/pw_sys_io)

pw_proto_library(attributes_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/attributes_service.proto
  INPUTS
    ${chip_dir}/examples/common/pigweed/protos/attributes_service.options
  PREFIX
    attributes_service
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(button_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/button_service.proto
  PREFIX
    button_service
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(device_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/device_service.proto
  INPUTS
    ${chip_dir}/examples/common/pigweed/protos/device_service.options
  PREFIX
    device_service
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(lighting_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/lighting_service.proto
  PREFIX
    lighting_service
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(locking_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/locking_service.proto
  PREFIX
    locking_service
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
  DEPS
    pw_protobuf.common_proto
)

pw_proto_library(wifi_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/wifi_service.proto
  INPUTS
    ${chip_dir}/examples/common/pigweed/protos/wifi_service.options
  PREFIX
    wifi_service
  DEPS
    pw_protobuf.common_proto
  STRIP_PREFIX
    ${chip_dir}/examples/common/pigweed/protos
)

endif(matter_enable_rpc)

include(${prj_root}/GCC-RELEASE/project_hp/asdk/includepath.cmake)

if (matter_enable_rpc)
list(
    APPEND ${list_chip_main_sources}
    #rpc
    ${chip_dir}/examples/platform/ameba/PigweedLogger.cpp
    ${chip_dir}/examples/platform/ameba/Rpc.cpp
    ${chip_dir}/examples/common/pigweed/RpcService.cpp
    ${chip_dir}/examples/common/pigweed/ameba/PigweedLoggerMutex.cpp
)
endif (matter_enable_rpc)

if (matter_enable_ota_requestor)
list(
    APPEND ${list_chip_main_sources}
    #OTARequestor
    ${chip_dir}/src/app/clusters/ota-requestor/BDXDownloader.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/ota-requestor-server.cpp
)
endif (matter_enable_ota_requestor)

list(
    APPEND ${list_chip_main_sources}

    ${chip_dir}/zzz_generated/all-clusters-minimal-app/zap-generated/callback-stub.cpp
    ${chip_dir}/zzz_generated/all-clusters-minimal-app/zap-generated/IMClusterCommandHandler.cpp

    ${chip_dir}/examples/all-clusters-minimal-app/all-clusters-common/src/bridged-actions-stub.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/all-clusters-common/src/static-supported-modes-manager.cpp

    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/chipinterface.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/DeviceCallbacks.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/CHIPDeviceManager.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/Globals.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/LEDWidget.cpp
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/DsoHack.cpp
)

add_library(
    ${chip_main}
    STATIC
    ${chip_main_sources}
)

chip_configure_data_model(chip_main
    INCLUDE_SERVER
    ZAP_FILE ${matter_example_path}/../all-clusters-common/all-clusters-minimal-app.zap
)

if (matter_enable_rpc)
target_include_directories(
    ${chip_main}
    PUBLIC
    #rpc
    ${chip_dir}/examples/platform/ameba
    ${chip_dir}/examples/platform/ameba/pw_sys_io/public
    ${chip_dir}/examples/common
    ${chip_dir}/examples/common/pigweed
    ${chip_dir}/examples/common/pigweed/ameba
    ${chip_dir}/src
    ${chip_dir}/src/lib/support
    ${pigweed_dir}/pw_rpc/nanopb/public
)
endif (matter_enable_rpc)

target_include_directories(
    ${chip_main}
    PUBLIC
    ${inc_path}
    ${chip_dir}/zzz_generated/all-clusters-minimal-app
    ${chip_dir}/zzz_generated/all-clusters-minimal-app/zap-generated
    ${chip_dir}/zzz_generated/app-common
    ${chip_dir}/examples/all-clusters-app/all-clusters-common
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/include
    ${chip_dir}/examples/all-clusters-minimal-app/ameba/main/include
    ${chip_dir_output}/gen/include
    ${chip_dir}/src/include/
    ${chip_dir}/src/lib/
    ${chip_dir}/src/
    ${chip_dir}/third_party/nlassert/repo/include/
    ${chip_dir}/src/app/
    ${chip_dir}/src/app/util/
    ${chip_dir}/src/app/server/
    ${chip_dir}/src/controller/data_model
    ${chip_dir}/third_party/nlio/repo/include/
    ${chip_dir}/third_party/nlunit-test/repo/src
)

if (matter_enable_rpc)
target_link_libraries(${chip_main} PUBLIC
    attributes_service.nanopb_rpc
    button_service.nanopb_rpc
    device_service.nanopb_rpc
    lighting_service.nanopb_rpc
    locking_service.nanopb_rpc
    wifi_service.nanopb_rpc
    pw_checksum
    pw_hdlc
    pw_log
    pw_rpc.server
    pw_trace_tokenized
    pw_trace_tokenized.trace_buffer
    pw_trace_tokenized.rpc_service
    pw_trace_tokenized.protos.nanopb_rpc
    PwRpc
)

link_directories(
    ${chip_dir_output}/lib
)
endif (matter_enable_rpc)

list(
    APPEND chip_main_flags

    -DINET_CONFIG_ENABLE_IPV4=0
    -DCHIP_PROJECT=1
    -DCHIP_DEVICE_LAYER_TARGET=Ameba
    -DUSE_ZAP_CONFIG
    -DCHIP_HAVE_CONFIG_H
    -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
)

if (matter_enable_persistentstorage_audit)
list(
    APPEND chip_main_flags

    -DCHIP_SUPPORT_ENABLE_STORAGE_API_AUDIT
)
endif (matter_enable_persistentstorage_audit)

if (matter_enable_rpc)
list(
    APPEND chip_main_flags

    -DPW_RPC_ATTRIBUTE_SERVICE=1
    -DPW_RPC_BUTTON_SERVICE=1
    -DPW_RPC_DEVICE_SERVICE=1
    -DPW_RPC_LIGHTING_SERVICE=1
    -DPW_RPC_LOCKING_SERVICE=1
    -DCONFIG_ENABLE_PW_RPC=1
)
endif (matter_enable_rpc)

if (matter_enable_ota_requestor)
list(
    APPEND chip_main_flags

    -DCONFIG_ENABLE_OTA_REQUESTOR=1
)
endif (matter_enable_ota_requestor)

list(
    APPEND chip_main_cpp_flags

	-Wno-unused-parameter
	-std=gnu++11
	-std=c++14
	-fno-rtti
)
target_compile_definitions(${chip_main} PRIVATE ${chip_main_flags} )
target_compile_options(${chip_main} PRIVATE ${chip_main_cpp_flags})

# move static library post build command
add_custom_command(
    TARGET ${chip_main}
    POST_BUILD
    COMMAND cp lib${chip_main}.a ${CMAKE_CURRENT_SOURCE_DIR}/lib/application
)
