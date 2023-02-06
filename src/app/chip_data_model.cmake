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

if (NOT CHIP_ROOT)
    # TODO: these are WORKAROUNDS and should be removed
    if(DEFINED ameba_matter_root)
        SET(CHIP_ROOT "${ameba_matter_root}")
    endif()
endif()

include("${CHIP_ROOT}/build/chip/chip_codegen.cmake")

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
#   SCOPE           Cmake scope keyword that defines the scope of included sources
#                   The default is PRIVATE scope.
#   INCLUDE_SERVER  Include source files from src/app/server directory
#   ZAP_FILE        Path to the ZAP file, used to determine the list of clusters
#                   supported by the application.
#   IDL             .matter IDL file to use for codegen. Inferred from ZAP_FILE
#                   if not provided
#
function(chip_configure_data_model APP_TARGET)
    set(SCOPE PRIVATE)
    cmake_parse_arguments(ARG "INCLUDE_SERVER" "SCOPE" "ZAP_FILE;IDL" "" ${ARGN})

    if (ARG_SCOPE)
        set(SCOPE ${ARG_SCOPE})
    endif()

    if (ARG_INCLUDE_SERVER)
        target_sources(${APP_TARGET} ${SCOPE}
            ${CHIP_APP_BASE_DIR}/server/EchoHandler.cpp
            ${CHIP_APP_BASE_DIR}/server/Dnssd.cpp
            ${CHIP_APP_BASE_DIR}/server/OnboardingCodesUtil.cpp
            ${CHIP_APP_BASE_DIR}/server/Server.cpp
            ${CHIP_APP_BASE_DIR}/server/CommissioningWindowManager.cpp
        )

        target_compile_options(${APP_TARGET} ${SCOPE}
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
        )
    endif()

    if (ARG_ZAP_FILE)
        chip_configure_zap_file(${APP_TARGET} ${ARG_ZAP_FILE})
        if (NOT ARG_IDL)
            string(REPLACE ".zap" ".matter" ARG_IDL ${ARG_ZAP_FILE})
        endif()
    endif()

    if (ARG_IDL)
        chip_codegen(${APP_TARGET}-codegen
          INPUT     "${ARG_IDL}"
          GENERATOR "cpp-app"
          OUTPUTS
                "app/PluginApplicationCallbacks.h"
                "app/callback-stub.cpp"
          OUTPUT_PATH   APP_GEN_DIR
          OUTPUT_FILES  APP_GEN_FILES
        )

        target_include_directories(${APP_TARGET} ${SCOPE} "${APP_GEN_DIR}")
        add_dependencies(${APP_TARGET} ${APP_TARGET}-codegen)
    else()
        set(APP_GEN_FILES)
    endif()

    chip_zapgen(${APP_TARGET}-zapgen
        INPUT     "${ARG_ZAP_FILE}"
        GENERATOR "app-templates"
        OUTPUTS
              "zap-generated/access.h"
              "zap-generated/CHIPClientCallbacks.h"
              "zap-generated/CHIPClusters.h"
              "zap-generated/endpoint_config.h"
              "zap-generated/gen_config.h"
              "zap-generated/IMClusterCommandHandler.cpp"
        OUTPUT_PATH   APP_TEMPLATES_GEN_DIR
        OUTPUT_FILES  APP_TEMPLATES_GEN_FILES
    )
    target_include_directories(${APP_TARGET} ${SCOPE} "${APP_TEMPLATES_GEN_DIR}")
    add_dependencies(${APP_TARGET} ${APP_TARGET}-zapgen)

    target_sources(${APP_TARGET} ${SCOPE}
        ${CHIP_APP_BASE_DIR}/../../zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
        ${CHIP_APP_BASE_DIR}/../../zzz_generated/app-common/app-common/zap-generated/cluster-objects.cpp
        ${CHIP_APP_BASE_DIR}/util/af-event.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-size-util.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-storage.cpp
        ${CHIP_APP_BASE_DIR}/util/attribute-table.cpp
        ${CHIP_APP_BASE_DIR}/util/binding-table.cpp
        ${CHIP_APP_BASE_DIR}/util/ClientMonitoringRegistrationTable.cpp
        ${CHIP_APP_BASE_DIR}/util/DataModelHandler.cpp
        ${CHIP_APP_BASE_DIR}/util/ember-compatibility-functions.cpp
        ${CHIP_APP_BASE_DIR}/util/ember-print.cpp
        ${CHIP_APP_BASE_DIR}/util/error-mapping.cpp
        ${CHIP_APP_BASE_DIR}/util/generic-callback-stubs.cpp
        ${CHIP_APP_BASE_DIR}/util/message.cpp
        ${CHIP_APP_BASE_DIR}/util/privilege-storage.cpp
        ${CHIP_APP_BASE_DIR}/util/util.cpp
        ${APP_GEN_FILES}
        ${APP_TEMPLATES_GEN_FILES}
    )
endfunction()
