#
#    Copyright (c) 2026 Project CHIP Authors
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

# ****************************************************************************
# Pigweed RPC server (HDLC over TCP / lwIP) for NXP FreeRTOS applications.
# ****************************************************************************

if(NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../ REALPATH)
endif()

get_filename_component(NXP_PLATFORM_DIR ${CHIP_ROOT}/examples/platform/nxp REALPATH)

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")
include(${PIGWEED_ROOT}/pw_build/pigweed.cmake)
include(${PIGWEED_ROOT}/pw_protobuf_compiler/proto.cmake)

include($ENV{PW_ROOT}/pw_assert/backend.cmake)
include($ENV{PW_ROOT}/pw_log/backend.cmake)

pw_set_module_config(pw_rpc_CONFIG pw_rpc.disable_global_mutex_config)
# The lwIP transport does not use pw_sys_io, so avoid any backend that pulls it
# in. pw_log_basic routes through the pw_sys_io facade, and the pw_assert_log
# check backend requires a FATAL-capable log backend (which pw_log_null refuses
# to provide). Use pw_log_null together with the print_and_abort assert
# backends: they emit failures via printf and call abort() directly, so no
# pw_sys_io backend is required. Application logging still goes through ChipLog*.
pw_set_backend(pw_log pw_log_null)
pw_set_backend(pw_assert.check pw_assert.print_and_abort_check_backend)
pw_set_backend(pw_assert.assert pw_assert.print_and_abort_assert_backend)

set(dir_pw_third_party_nanopb "${CHIP_ROOT}/third_party/nanopb/repo" CACHE STRING "" FORCE)

# Collect all generated python proto modules into a single directory that is
# put on PYTHONPATH for the protoc plugins.
set(pw_protobuf_compiler_PYTHON_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/python_protos"
    CACHE STRING "" FORCE)

add_subdirectory(${PIGWEED_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/pigweed)
add_subdirectory(${CHIP_ROOT}/third_party/nanopb/repo ${CMAKE_CURRENT_BINARY_DIR}/nanopb)

if(CONFIG_CHIP_APP_JF_ADMIN)
  pw_proto_library(joint_fabric_service
    SOURCES
      ${CHIP_ROOT}/examples/common/pigweed/protos/joint_fabric_service.proto
    INPUTS
      ${CHIP_ROOT}/examples/common/pigweed/protos/joint_fabric_service.options
    PREFIX
      joint_fabric_service
    STRIP_PREFIX
      ${CHIP_ROOT}/examples/common/pigweed/protos
    DEPS
      pw_protobuf.common_proto
  )
endif()

target_sources(app PRIVATE
  ${CHIP_ROOT}/examples/common/pigweed/RpcServiceLwip.cpp
  ${NXP_PLATFORM_DIR}/RpcFreeRTOSLwip.cpp
  ${NXP_PLATFORM_DIR}/common/rpc/source/AppRpc.cpp
)

target_include_directories(app PRIVATE
  ${CHIP_ROOT}/examples/common
  ${CHIP_ROOT}/examples/common/pigweed
  ${NXP_PLATFORM_DIR}
  ${NXP_PLATFORM_DIR}/common/rpc/include

  ${CHIP_ROOT}/src/lib/support
  ${CHIP_ROOT}/src/system
)

if(CONFIG_CHIP_APP_JF_ADMIN)
  get_filename_component(JFA_DIR ${NXP_PLATFORM_DIR}/common/jfa REALPATH)
  target_sources(app PRIVATE
    ${JFA_DIR}/source/JFAManager.cpp
    ${JFA_DIR}/source/JFADatastoreSync.cpp
    ${JFA_DIR}/source/JFAPlatform.cpp
    ${JFA_DIR}/source/JFAInit.cpp
    ${JFA_DIR}/source/JFARpcServer.cpp
  )
  target_include_directories(app PRIVATE
    ${JFA_DIR}/include
    ${CHIP_ROOT}/examples/common/pigweed/rpc_services
  )
endif()

target_compile_options(app PRIVATE
  # AppTaskBase.cpp guards the Rpc::Init() call on CONFIG_ENABLE_PW_RPC.
  # CONFIG_CHIP_APP_RPC is the Kconfig option that pulls this file in, so bridge
  # the two here to keep the app task init call enabled.
  "-DCONFIG_ENABLE_PW_RPC=1"
)

if(CONFIG_CHIP_APP_JF_ADMIN)
  target_compile_options(app PRIVATE "-DPW_RPC_JF_ADMIN_SERVICE=1")
endif()


target_link_libraries(app PRIVATE
  pw_checksum
  pw_hdlc
  pw_log
  pw_rpc.server
)

if(CONFIG_CHIP_APP_JF_ADMIN)
  target_link_libraries(app PRIVATE joint_fabric_service.nanopb_rpc)
endif()
