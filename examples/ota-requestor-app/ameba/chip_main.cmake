cmake_minimum_required(VERSION 3.6)

project(chip_main)

set(chip_dir "${ameba_matter_root}")
set(chip_dir_output "${matter_output_path}/chip")
set(dir "${sdk_root}/component/common/api")
set(chip_main chip_main)
set(list_chip_main_sources chip_main_sources)

include(${prj_root}/GCC-RELEASE/project_hp/asdk/includepath.cmake)

list(
    APPEND ${list_chip_main_sources}

    ${chip_dir}/zzz_generated/ota-requestor-app/zap-generated/callback-stub.cpp
    ${chip_dir}/zzz_generated/ota-requestor-app/zap-generated/IMClusterCommandHandler.cpp

    ${chip_dir}/examples/ota-requestor-app/ameba/main/chipinterface.cpp
    ${chip_dir}/examples/ota-requestor-app/ameba/main/Globals.cpp
    ${chip_dir}/examples/ota-requestor-app/ameba/main/LEDWidget.cpp
    ${chip_dir}/examples/ota-requestor-app/ameba/main/CHIPDeviceManager.cpp
    ${chip_dir}/examples/ota-requestor-app/ameba/main/DeviceCallbacks.cpp

    ${chip_dir}/src/app/clusters/ota-requestor/BDXDownloader.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
    ${chip_dir}/src/app/clusters/ota-requestor/ota-requestor-server.cpp
    ${chip_dir}/examples/platform/ameba/ota/OTAInitializer.cpp

    ${chip_dir}/examples/providers/DeviceInfoProviderImpl.cpp
)

add_library(
    ${chip_main}
    STATIC
    ${chip_main_sources}
)

chip_configure_data_model(chip_main
    INCLUDE_SERVER
    ZAP_FILE ${matter_example_path}/../ota-requestor-common/ota-requestor-app.zap
)

target_include_directories(
    ${chip_main}
    PUBLIC
	${inc_path}
    ${chip_dir}/zzz_generated/ota-requestor-app
    ${chip_dir}/zzz_generated/app-common
    ${chip_dir}/zzz_generated
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
    ${chip_dir}/src/app/clusters/ota-requestor
    ${chip_dir}/examples/ota-requestor-app/ameba/main/include
    ${chip_dir}/examples/platform/ameba
    ${chip_dir}/examples/providers
    ${sdk_root}/component/soc/realtek/amebad/fwlib/include
)

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
