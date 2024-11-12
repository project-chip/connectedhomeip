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
include(${pigweed_dir}/pw_assert/backend.cmake)
include(${pigweed_dir}/pw_log/backend.cmake)
include(${pigweed_dir}/pw_sys_io/backend.cmake)
include(${pigweed_dir}/pw_trace/backend.cmake)

set(dir_pw_third_party_nanopb "${chip_dir}/third_party/nanopb/repo" CACHE STRING "" FORCE)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
pw_set_backend(pw_log pw_log_basic)
pw_set_backend(pw_assert.check pw_assert_log.check_backend)
pw_set_backend(pw_assert.assert pw_assert.assert_compatibility_backend)
pw_set_backend(pw_sys_io pw_sys_io.ameba)
pw_set_backend(pw_trace pw_trace_tokenized)

add_subdirectory(${chip_dir}/third_party/pigweed/repo ${chip_dir}/examples/all-clusters-app/ameba/out/pigweed)
add_subdirectory(${chip_dir}/third_party/nanopb/repo ${chip_dir}/examples/all-clusters-app/ameba/out/nanopb)
add_subdirectory(${chip_dir}/examples/platform/ameba/pw_sys_io ${chip_dir}/examples/all-clusters-app/ameba/out/pw_sys_io)

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

pw_proto_library(descriptor_service
  SOURCES
    ${chip_dir}/examples/common/pigweed/protos/descriptor_service.proto
  PREFIX
    descriptor_service
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

if (matter_enable_shell)
list(
    APPEND ${list_chip_main_sources}
    #shell
    ${chip_dir}/examples/platform/ameba/shell/launch_shell.cpp
)
endif (matter_enable_shell)

if (matter_enable_ota_requestor)
list(
    APPEND ${list_chip_main_sources}
    #OTARequestor
    ${chip_dir}/src/app/clusters/ota-requestor/BDXDownloader.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/ota-requestor-server.cpp
    ${chip_dir}/examples/platform/ameba/ota/OTAInitializer.cpp
)
endif (matter_enable_ota_requestor)

list(
    APPEND ${list_chip_main_sources}

    ${chip_dir}/src/app/clusters/microwave-oven-control-server/microwave-oven-control-server.cpp

    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/bridged-actions-stub.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/air-quality-instance.cpp
	${chip_dir}/examples/all-clusters-app/all-clusters-common/src/chime-instance.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/concentration-measurement-instances.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/device-energy-management-stub.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/energy-evse-stub.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/energy-preference-delegate.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/fan-stub.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/oven-modes.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/laundry-dryer-controls-delegate-impl.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/laundry-washer-controls-delegate-impl.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/resource-monitoring-delegates.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/rvc-modes.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/smco-stub.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/static-supported-modes-manager.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/static-supported-temperature-levels.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/operational-state-delegate-impl.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/rvc-operational-state-delegate-impl.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/microwave-oven-mode.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/oven-modes.cpp
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/src/oven-operational-state-delegate.cpp

    ${chip_dir}/examples/all-clusters-app/ameba/main/chipinterface.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/BindingHandler.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/DeviceCallbacks.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/CHIPDeviceManager.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/Globals.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/LEDWidget.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/ManualOperationCommand.cpp
    ${chip_dir}/examples/all-clusters-app/ameba/main/SmokeCOAlarmManager.cpp

    ${chip_dir}/examples/microwave-oven-app/microwave-oven-common/src/microwave-oven-device.cpp

    ${chip_dir}/examples/energy-management-app/energy-management-common/common/src/EnergyTimeUtils.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/device-energy-management/src/DeviceEnergyManagementDelegateImpl.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/device-energy-management/src/DeviceEnergyManagementManager.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/src/ChargingTargetsMemMgr.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/src/EVSEManufacturerImpl.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/src/EnergyEvseDelegateImpl.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/src/EnergyEvseManager.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/src/EnergyEvseTargetsStore.cpp
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-reporting/src/ElectricalPowerMeasurementDelegate.cpp

    ${chip_dir}/examples/platform/ameba/route_hook/ameba_route_hook.c
    ${chip_dir}/examples/platform/ameba/route_hook/ameba_route_table.c

    ${chip_dir}/examples/platform/ameba/test_event_trigger/AmebaTestEventTriggerDelegate.cpp

    ${chip_dir}/examples/providers/DeviceInfoProviderImpl.cpp
)

add_library(
    ${chip_main}
    STATIC
    ${chip_main_sources}
)

chip_configure_data_model(chip_main
    ZAP_FILE ${matter_example_path}/../all-clusters-common/all-clusters-app.zap
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
    ${chip_dir}/zzz_generated/all-clusters-app
    ${chip_dir}/zzz_generated/all-clusters-app/zap-generated
    ${chip_dir}/zzz_generated/app-common
    ${chip_dir}/examples/all-clusters-app/all-clusters-common
    ${chip_dir}/examples/all-clusters-app/all-clusters-common/include
    ${chip_dir}/examples/microwave-oven-app/microwave-oven-common
    ${chip_dir}/examples/microwave-oven-app/microwave-oven-common/include
    ${chip_dir}/examples/energy-management-app/energy-management-common/common/include
    ${chip_dir}/examples/energy-management-app/energy-management-common/device-energy-management/include
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-evse/include
    ${chip_dir}/examples/energy-management-app/energy-management-common/energy-reporting/include
    ${chip_dir}/examples/all-clusters-app/ameba/main/include
    ${chip_dir}/examples/platform/ameba
    ${chip_dir}/examples/platform/ameba/observer
    ${chip_dir}/examples/platform/ameba/route_hook
    ${chip_dir}/examples/providers
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
)

if (matter_enable_rpc)
target_link_libraries(${chip_main} PUBLIC
    attributes_service.nanopb_rpc
    button_service.nanopb_rpc
    descriptor_service.nanopb_rpc
    device_service.nanopb_rpc
    lighting_service.nanopb_rpc
    locking_service.nanopb_rpc
    wifi_service.nanopb_rpc
    pw_checksum
    pw_hdlc
    pw_log
    pw_rpc.server
    pw_sys_io
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
    -DCHIP_HAVE_CONFIG_H
    -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
    -DCHIP_SHELL_MAX_TOKENS=11
    -DCONFIG_ENABLE_AMEBA_FACTORY_DATA=0
    -DCONFIG_ENABLE_AMEBA_TEST_EVENT_TRIGGER=0
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
    -DPW_RPC_DESCRIPTOR_SERVICE=1
    -DPW_RPC_DEVICE_SERVICE=1
    -DPW_RPC_LIGHTING_SERVICE=1
    -DPW_RPC_LOCKING_SERVICE=1
    -DCONFIG_ENABLE_PW_RPC=1
)
endif (matter_enable_rpc)

if (matter_enable_shell)
list(
    APPEND chip_main_flags

    -DCONFIG_ENABLE_CHIP_SHELL=1
)
endif (matter_enable_shell)

list(
    APPEND chip_main_cpp_flags

	-Wno-unused-parameter
	-std=c++17
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
