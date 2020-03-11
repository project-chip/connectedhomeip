#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the CMake template for the CHIP SDK LWIP portability layer code.
#

if (CHIP_WITH_LWIP_INTERNAL)

# add_subdirectory(tests)

add_library(lwipTarget)

if(CHIP_LWIP_TARGET_STANDALONE)

set(COMMON_SOURCES
    standalone/sys_arch.c
    standalone/TapInterface.c
)

set(COMMON_INCLUDES
    standalone/arch
    standalone
    standalone
)

endif(CHIP_LWIP_TARGET_STANDALONE)

if(CHIP_LWIP_TARGET_NRF5)

set(COMMON_SOURCES
    freertos/sys_arch.c
)

set(COMMON_INCLUDES
    freertos/arch
    nrf5/arch
    nrf5
)

# Add lwip and lwipTarget to the PlatformBsp library
#target_link_libraries(PlatformBsp PRIVATE lwip lwipTarget)

endif(CHIP_LWIP_TARGET_NRF5)

if(CHIP_LWIP_TARGET_EFR32)

set(COMMON_SOURCES
    freertos/sys_arch.c
)

set(COMMON_INCLUDES
    freertos/arch
    efr32/arch
    efr32
)

endif(CHIP_LWIP_TARGET_EFR32)

target_include_directories(lwipTarget PRIVATE ${COMMON_INCLUDES})

target_sources(lwipTarget PRIVATE ${COMMON_SOURCES})

target_link_libraries(lwipTarget PRIVATE chip-config)

endif (CHIP_WITH_LWIP_INTERNAL)
