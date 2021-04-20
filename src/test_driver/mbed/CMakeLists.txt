# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-tests)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

if("wifi_ism43362" IN_LIST MBED_TARGET_LABELS)
    add_subdirectory(wifi-ism43362)
endif()

add_executable(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/main/include/
)

target_sources(${APP_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/arch.c
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
)

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

if("wifi_ism43362" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(${APP_TARGET}
            wifi-ism43362
    )
endif()

if("BlueNRG_MS" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(${APP_TARGET}
            mbed-ble-blue_nrg
    )
endif()

# As noted in the nrfconnect impl:
#   TODO: this list is hard to maintain manually (and currently incomplete).
#   An automated way that includes figuring out dependencies would be much better
set(CHIP_TESTS
# TODO: Enable all the tests.
    # -lAppTests
    -lASN1Tests
    -lBleLayerTests
    -lChipCryptoTests
    # -lCoreTests
    # -lInetLayerTests
    # -lMessagingLayerTests
    -lPlatformTests
    # # TODO: raw transport tests fail by running out of TCP/UDP endpoints
    # # -lRawTransportTests
    -lRetransmitTests
    # -lSupportTests
    -lSystemLayerTests
    # -lTransportLayerTests
    -lMinimalMdnsCoreTests
    -lMinimalMdnstests
)

# Link the *whole-archives* to keep the static test objects.
target_link_options(${APP_TARGET} PUBLIC -Wl,--whole-archive ${CHIP_TESTS} -Wl,--no-whole-archive)

# Add test libs.
target_link_libraries(${APP_TARGET}
# TODO: Enable all the tests.
        # InetTestHelpers
        # NetworkTestHelpers
        nlunit-test
)

target_link_libraries(${APP_TARGET} mbed-os mbed-ble mbed-events mbed-netsocket mbed-storage mbed-storage-kv-global-api mbed-mbedtls mbed-mbedtls-cryptocell310 chip)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
