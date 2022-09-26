#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
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

# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
# The list of src and include dirs must be in sync with that in all-clusters-app/esp32/main/component.mk
set(PRIV_INCLUDE_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}/include"
)

set(SRC_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../third_party/connectedhomeip/config/esp32/components"
    "${IDF_PATH}/examples/common_components"
)

if (CONFIG_OPENTHREAD_ENABLED)
    list(APPEND PRIV_REQUIRES_LIST openthread)
endif()

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST}
                       PRIV_REQUIRES ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

target_link_directories(${COMPONENT_LIB} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../chip/lib)
target_link_libraries(${COMPONENT_LIB} PUBLIC -lSupportTesting)
