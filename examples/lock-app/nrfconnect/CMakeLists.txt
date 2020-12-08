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
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(NRFCONNECT_COMMON ${CHIP_ROOT}/examples/platform/nrfconnect REALPATH)
get_filename_component(LOCK_COMMON ${CHIP_ROOT}/examples/lock-app/lock-common REALPATH)

set(CONF_FILE ${CHIP_ROOT}/config/nrfconnect/sample-defaults.conf prj.conf)
set(CHIP_OPENTHREAD_CONFIG ${NRFCONNECT_COMMON}/project_include/OpenThreadConfig.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrfconnect/)
include(nrfconnect-app)

project(chip-nrf52840-lock-example)
target_include_directories(app PRIVATE main/include ${LOCK_COMMON} ${NRFCONNECT_COMMON}/util/include ${NRFCONNECT_COMMON}/app/include)
target_sources(app PRIVATE
               main/AppTask.cpp
               main/BoltLockManager.cpp
               main/main.cpp
               main/ZclCallbacks.cpp
               ${LOCK_COMMON}/gen/call-command-handler.cpp
               ${LOCK_COMMON}/gen/callback-stub.cpp
               ${NRFCONNECT_COMMON}/util/LEDWidget.cpp
               ${NRFCONNECT_COMMON}/util/NFCWidget.cpp
               ${NRFCONNECT_COMMON}/util/ThreadUtil.cpp
               ${NRFCONNECT_COMMON}/app/Service.cpp
               ${CHIP_ROOT}/src/accessory/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/af-main-common.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/chip-message-send.cpp
               ${CHIP_ROOT}/src/app/util/client-api.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/process-cluster-message.cpp
               ${CHIP_ROOT}/src/app/util/process-global-message.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off.cpp)
