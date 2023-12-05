# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.
project("chiptest")

message(STATUS "MATTER_SDK_SOURCE_BUILD=${MATTER_SDK_SOURCE_BUILD}")
message(STATUS "MATTER_SDK_BUILD_ROOT=${MATTER_SDK_BUILD_ROOT}")
message(STATUS "MATTER_SDK_UTEST_LIB=${MATTER_SDK_UTEST_LIB}")

set(MATTER_SDK_ROOT "${PROJECT_SOURCE_DIR}/../../../../../../..")

if(IS_ABSOLUTE ${MATTER_SDK_BUILD_ROOT})
set(MATTER_SDK_BUILD ${MATTER_SDK_BUILD_ROOT})
else()
set(MATTER_SDK_BUILD "${MATTER_SDK_ROOT}/${MATTER_SDK_BUILD_ROOT}")
endif()

message(STATUS "MATTER_SDK_ROOT=${MATTER_SDK_ROOT}")
message(STATUS "MATTER_SDK_BUILD=${MATTER_SDK_BUILD}")

set(CHIPTest_srcs
        CHIPTest-JNI.cpp)
add_library(CHIPTest SHARED ${CHIPTest_srcs})
set_property(TARGET CHIPTest APPEND PROPERTY INCLUDE_DIRECTORIES
        ${MATTER_SDK_ROOT}/src/include/
        ${MATTER_SDK_ROOT}/src/
        ${MATTER_SDK_BUILD}/gen/include/
        ${MATTER_SDK_ROOT}/src/lib/
        ${MATTER_SDK_ROOT}/config/standalone/
        ${MATTER_SDK_ROOT}/third_party/nlassert/repo/include/
        ${MATTER_SDK_ROOT}/third_party/nlio/repo/include/
        ${MATTER_SDK_ROOT}/third_party/nlfaultinjection/include/
        ${MATTER_SDK_ROOT}/third_party/nlunit-test/repo/src/)
set_target_properties(CHIPTest PROPERTIES COMPILE_DEFINITIONS "CHIP_HAVE_CONFIG_H=1")

set(CHIPTest_Libs
        -Wl,--start-group
        ${MATTER_SDK_BUILD}/obj/src/lib/core/lib/libChipCore.a
        ${MATTER_SDK_BUILD}/obj/src/lib/support/lib/libSupportLayer.a
        ${MATTER_SDK_BUILD}/lib/libnlunit-test.a
        ${MATTER_SDK_BUILD}/obj/src/ble/lib/libBleLayer.a
        ${MATTER_SDK_BUILD}/obj/src/inet/lib/libInetLayer.a
        ${MATTER_SDK_BUILD}/obj/src/platform/logging/lib/android.a
        ${MATTER_SDK_BUILD}/lib/libnlfaultinjection.a
        ${MATTER_SDK_BUILD}/obj/src/system/lib/libSystemLayer.a
        ${MATTER_SDK_BUILD}/obj/src/platform/lib/libDeviceLayer.a
        ${MATTER_SDK_BUILD}/obj/src/platform/android/lib/libAndroidPlatform.a
        ${MATTER_SDK_BUILD}/obj/src/crypto/lib/libChipCrypto.a
        ${MATTER_SDK_BUILD}/lib/libmbedtls.a
        -llog

        -Wl,-whole-archive
        ${MATTER_SDK_BUILD}/lib/${MATTER_SDK_UTEST_LIB}
        -Wl,-no-whole-archive

        -Wl,--end-group
        )
target_link_libraries(CHIPTest ${CHIPTest_Libs})

#if( MATTER_SDK_SRC_DIR )
#    include(../../../../${MATTER_SDK_SRC_DIR}/CMakeLists.txt)
#endif()
