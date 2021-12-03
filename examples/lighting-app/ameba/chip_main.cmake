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

    #chip app
    ${chip_dir}/src/app/Command.cpp
    ${chip_dir}/src/app/CommandHandler.cpp
    ${chip_dir}/src/app/InteractionModelEngine.cpp
    ${chip_dir}/src/app/CommandSender.cpp
    ${chip_dir}/src/app/decoder.cpp
    ${chip_dir}/src/app/encoder-common.cpp
    ${chip_dir}/src/app/EventManagement.cpp
    ${chip_dir}/src/app/ReadClient.cpp
    ${chip_dir}/src/app/ReadHandler.cpp
    ${chip_dir}/src/app/TimedHandler.cpp
    ${chip_dir}/src/app/WriteClient.cpp
    ${chip_dir}/src/app/WriteHandler.cpp
    ${chip_dir}/src/app/util/CHIPDeviceCallbacksMgr.cpp
    ${chip_dir}/src/app/util/esi-management.cpp
    ${chip_dir}/src/app/reporting/Engine.cpp

    ${chip_dir}/zzz_generated/lighting-app/zap-generated/attribute-size.cpp
    ${chip_dir}/zzz_generated/lighting-app/zap-generated/CHIPClientCallbacks.cpp
    ${chip_dir}/zzz_generated/lighting-app/zap-generated/callback-stub.cpp
    ${chip_dir}/zzz_generated/lighting-app/zap-generated/IMClusterCommandHandler.cpp
    ${chip_dir}/zzz_generated/lighting-app/zap-generated/CHIPClusters.cpp

    ${chip_dir}/examples/lighting-app/lighting-common/color_format/color_format.cpp

    ${chip_dir}/examples/lighting-app/ameba/main/chipinterface.cpp
    ${chip_dir}/examples/lighting-app/ameba/main/DeviceCallbacks.cpp
    ${chip_dir}/examples/lighting-app/ameba/main/CHIPDeviceManager.cpp
    ${chip_dir}/examples/lighting-app/ameba/main/Globals.cpp
    ${chip_dir}/examples/lighting-app/ameba/main/LEDWidget.cpp
)

add_library(
    ${chip_main}
    STATIC
    ${chip_main_sources}
)

chip_configure_data_model(chip_main
    INCLUDE_SERVER
    ZAP_FILE ${matter_example_path}/../lighting-common/lighting-app.zap
)

target_include_directories(
    ${chip_main}
    PUBLIC
	${inc_path}
    ${chip_dir}/zzz_generated/lighting-app
    ${chip_dir}/zzz_generated/lighting-app/zap-generated
    ${chip_dir}/zzz_generated/app-common
    ${chip_dir}/examples/lighting-app/lighting-common
    ${chip_dir}/examples/lighting-app/lighting-common/color_format
    ${chip_dir}/examples/lighting-app/ameba/main/include
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

list(
    APPEND chip_main_flags

    -DINET_CONFIG_ENABLE_IPV4=1
    -DCHIP_PROJECT=1
    -DCHIP_DEVICE_LAYER_TARGET=Ameba
    -DUSE_ZAP_CONFIG
    -DCHIP_HAVE_CONFIG_H
    -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
)

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
