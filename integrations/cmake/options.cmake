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
#      This file is the CMake template for defining global build options for the CHIP SDK.
#

#
# This option chooses whether to optimize builds of third_party packages:
#
#   ON: If available, use optimized cmake scripts specified by the CHIP project.
#       This option will result in faster builds, but require updates when upstream
#       diverges. The optimization is applied for libs that don't change often and
#       can provide a measurable benefit from recasting the build using cmake.
#
#   OFF: Use the native build system provided the third_party project (autotools or other).
#
option(BUILD_EXTERNALS_WITH_CMAKE
    "Builds third_party packages with optimized cmake. Otherwise, use the native build system provided by the package."
    ON
)

if (NOT BUILD_EXTERNALS_WITH_CMAKE)
include(ExternalProject)
endif ()

#
# Provides selection of the cryptographic implementation.
#
set(with_crypto "mbedtls" CACHE STRING "Specify the crypto implementation from one of: auto, mbedtls, or openssl (default=auto)")
set_property(CACHE with_crypto PROPERTY STRINGS mbedtls openssl)

#
# Provides selection of the network layer technology.
#
set(with_network "all" CACHE STRING "Specify the target network layers from one of: ble, inet, or all (default=all)")
set_property(CACHE with_network PROPERTY STRINGS all inet ble)

#
# Provides selection of the system layer technology.
#
set(with_system "sockets" CACHE STRING "Specify the target network stack from one of: sockets, lwip or Network.framework (default=sockets)")
set_property(CACHE with_system PROPERTY STRINGS sockets lwip Network.framework)

#
# Map CMake target descriptions to same names as autotools and gn for convenience.
#
set(target_cpu ${CMAKE_SYSTEM_PROCESSOR})
set(target_os ${CMAKE_SYSTEM_NAME})

#
# Update the version variable with the final toolchain the platform set.
#
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpfullversion -dumpversion
    OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION
)

message(STATUS "
  Configuration Summary
  ---------------------
  Package                                          : ${PROJECT_NAME}
  Version                                          : ${PROJECT_VERSION}

  Host system                                      : ${CMAKE_HOST_SYSTEM_NAME}
  Host build system                                : ${CMAKE_MAKE_PROGRAM}
  Host name                                        : ${SITE}
  Host compiler                                    : ${CMAKE_C_COMPILER}
  Host compiler version                            : ${CMAKE_C_COMPILER_VERSION}

  Target platform                                  : ${CHIP_PLATFORM}
  Target build type                                : ${CMAKE_BUILD_TYPE}
  Target architecture                              : ${target_cpu}
  Target OS                                        : ${target_os}
  Target style                                     : ${CHIP_TARGET_STYLE}

  Target device layer                              : ${CHIP_DEVICE_LAYER_TARGET}
  Target crypto layer                              : ${with_crypto}
  Target network layer                             : ${with_network}
  Target system layer                              : ${with_system}
")
