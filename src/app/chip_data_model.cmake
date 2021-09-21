#
#   Copyright (c) 2020 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

set(CHIP_APP_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

#
# Configure ${APP_TARGET} with source files associated with ${CLUSTER} cluster
#
function(chip_configure_cluster APP_TARGET CLUSTER)
    file(GLOB CLUSTER_SOURCES "${CHIP_APP_BASE_DIR}/clusters/${CLUSTER}/*.cpp")
    target_sources(${APP_TARGET} PRIVATE ${CLUSTER_SOURCES})
endfunction()

#
# Configure ${APP_TARGET} with source files associated with clusters enabled in the ${ZAP_FILE}
#
function(chip_configure_zap_file APP_TARGET ZAP_FILE)
    find_package(Python3 REQUIRED)

    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${CHIP_APP_BASE_DIR}/zap_cluster_list.py --zap_file ${ZAP_FILE}
        OUTPUT_VARIABLE CLUSTER_LIST
        ERROR_VARIABLE ERROR_MESSAGE
        RESULT_VARIABLE RC
    )
    if (NOT RC EQUAL 0)
        message(FATAL_ERROR "Failed to execute zap_cluster_list.py: ${ERROR_MESSAGE}")
    endif()

    string(REPLACE "\n" ";" CLUSTER_LIST "${CLUSTER_LIST}")
    foreach(CLUSTER ${CLUSTER_LIST})
        chip_configure_cluster(${APP_TARGET} ${CLUSTER})
    endforeach()
endfunction()

#
# Configure ${APP_TARGET} based on the selected data model configuration.
# Available options are:
#   INCLUDE_SERVER  Include source files from src/app/server directory
#   ZAP_FILE        Path to the ZAP file, used to determine the list of clusters
#                   supported by the application.
#
function(chip_configure_data_model APP_TARGET)
    cmake_parse_arguments(ARG "INCLUDE_SERVER" "ZAP_FILE" "" ${ARGN})

    if (ARG_INCLUDE_SERVER)
        target_sources(${APP_TARGET} PRIVATE
            ${CHIP_APP_BASE_DIR}/server/EchoHandler.cpp
            ${CHIP_APP_BASE_DIR}/server/Mdns.cpp
            ${CHIP_APP_BASE_DIR}/server/OnboardingCodesUtil.cpp
            ${CHIP_APP_BASE_DIR}/server/Server.cpp
            ${CHIP_APP_BASE_DIR}/server/CommissioningWindowManager.cpp
        )
    endif()

    if (ARG_ZAP_FILE)
        chip_configure_zap_file(${APP_TARGET} ${ARG_ZAP_FILE})
    endif()

    target_sources(${APP_TARGET} PRIVATE
        ${CHIP_APP_BASE_DIR}/../../zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
        ${CHIP_APP_BASE_DIR}/reporting/reporting-default-configuration.cpp
        ${CHIP_APP_BASE_DIR}/reporting/reporting.cpp
        ${CHIP_APP_BASE_DIR}/util/af-event.cpp
        ${CHIP_APP_BASE_DIR}/util/af-main-common.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-list-byte-span.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-size-util.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-storage.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-table.cpp
        ${CHIP_APP_BASE_DIR}/util/binding-table.cpp
        ${CHIP_APP_BASE_DIR}/util/chip-message-send.cpp
        ${CHIP_APP_BASE_DIR}/util/client-api.cpp
        ${CHIP_APP_BASE_DIR}/util/DataModelHandler.cpp
        ${CHIP_APP_BASE_DIR}/util/ember-compatibility-functions.cpp
        ${CHIP_APP_BASE_DIR}/util/ember-print.cpp
        ${CHIP_APP_BASE_DIR}/util/error-mapping.cpp
        ${CHIP_APP_BASE_DIR}/util/message.cpp
        ${CHIP_APP_BASE_DIR}/util/process-cluster-message.cpp
        ${CHIP_APP_BASE_DIR}/util/process-global-message.cpp
        ${CHIP_APP_BASE_DIR}/util/util.cpp
    )
endfunction()
