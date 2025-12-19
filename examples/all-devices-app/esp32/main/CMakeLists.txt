#
#    Copyright (c) 2025 Project CHIP Authors
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

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(ALL_DEVICES_COMMON_DIR ${CHIP_ROOT}/examples/all-devices-app/all-devices-common REALPATH)

set(PRIV_INCLUDE_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${ALL_DEVICES_COMMON_DIR}"
                      "${CHIP_ROOT}/examples/providers"
)

# Application sources
set(APP_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DeviceShellCommands.cpp"
    # All-devices common sources
    "${ALL_DEVICES_COMMON_DIR}/devices/interface/DeviceInterface.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/interface/SingleEndpointDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/boolean-state-sensor/BooleanStateSensorDevice.cpp"
    "${ALL_DEVICES_COMMON_DIR}/devices/root-node/RootNodeDevice.cpp"
    # Providers
    "${CHIP_ROOT}/examples/providers/DeviceInfoProviderImpl.cpp"
    # Shell extension
    "${CHIP_ROOT}/examples/platform/esp32/shell_extension/launch.cpp"
)

# Code-driven data model provider sources
set(CODE_DRIVEN_SRCS
    "${CHIP_ROOT}/src/data-model-providers/codedriven/CodeDrivenDataModelProvider.cpp"
    "${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.cpp"
    "${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/SpanEndpoint.cpp"
)

# Server and core app sources
set(SERVER_SRCS
    "${CHIP_ROOT}/src/app/SafeAttributePersistenceProvider.cpp"
    "${CHIP_ROOT}/src/app/StorageDelegateWrapper.cpp"
    "${CHIP_ROOT}/src/app/server/AclStorage.cpp"
    "${CHIP_ROOT}/src/app/server/CommissioningWindowManager.cpp"
    "${CHIP_ROOT}/src/app/server/DefaultAclStorage.cpp"
    "${CHIP_ROOT}/src/app/server/DefaultTermsAndConditionsProvider.cpp"
    "${CHIP_ROOT}/src/app/server/Dnssd.cpp"
    "${CHIP_ROOT}/src/app/server/EchoHandler.cpp"
    "${CHIP_ROOT}/src/app/server/Server.cpp"
    "${CHIP_ROOT}/src/app/server/TermsAndConditionsManager.cpp"
    "${CHIP_ROOT}/src/app/server/JointFabricAdministrator.cpp"
    "${CHIP_ROOT}/src/app/server/JointFabricDatastore.cpp"
)

# Persistence sources
set(PERSISTENCE_SRCS
    "${CHIP_ROOT}/src/app/persistence/AttributePersistence.cpp"
    "${CHIP_ROOT}/src/app/persistence/AttributePersistenceProviderInstance.cpp"
    "${CHIP_ROOT}/src/app/persistence/DefaultAttributePersistenceProvider.cpp"
    "${CHIP_ROOT}/src/app/persistence/DeferredAttributePersistenceProvider.cpp"
    "${CHIP_ROOT}/src/app/persistence/String.cpp"
)

# Server cluster infrastructure
set(SERVER_CLUSTER_SRCS
    "${CHIP_ROOT}/src/app/server-cluster/AttributeListBuilder.cpp"
    "${CHIP_ROOT}/src/app/server-cluster/DefaultServerCluster.cpp"
    "${CHIP_ROOT}/src/app/server-cluster/ServerClusterInterface.cpp"
    "${CHIP_ROOT}/src/app/server-cluster/ServerClusterInterfaceRegistry.cpp"
    "${CHIP_ROOT}/src/app/server-cluster/SingleEndpointServerClusterRegistry.cpp"
)

# Cluster implementations (new-style, excluding CodegenIntegration.cpp files)
set(CLUSTER_SRCS
    # Access Control
    "${CHIP_ROOT}/src/app/clusters/access-control-server/access-control-cluster.cpp"
    "${CHIP_ROOT}/src/app/clusters/access-control-server/ArlEncoder.cpp"
    # Administrator Commissioning
    "${CHIP_ROOT}/src/app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.cpp"
    "${CHIP_ROOT}/src/app/clusters/administrator-commissioning-server/AdministratorCommissioningLogic.cpp"
    # Basic Information
    "${CHIP_ROOT}/src/app/clusters/basic-information/BasicInformationCluster.cpp"
    # Boolean State
    "${CHIP_ROOT}/src/app/clusters/boolean-state-server/BooleanStateCluster.cpp"
    # Descriptor
    "${CHIP_ROOT}/src/app/clusters/descriptor/DescriptorCluster.cpp"
    # General Commissioning
    "${CHIP_ROOT}/src/app/clusters/general-commissioning-server/GeneralCommissioningCluster.cpp"
    # General Diagnostics
    "${CHIP_ROOT}/src/app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.cpp"
    # Group Key Management
    "${CHIP_ROOT}/src/app/clusters/group-key-mgmt-server/GroupKeyManagementCluster.cpp"
    # Identify
    "${CHIP_ROOT}/src/app/clusters/identify-server/IdentifyCluster.cpp"
    # Network Commissioning
    "${CHIP_ROOT}/src/app/clusters/network-commissioning/NetworkCommissioningCluster.cpp"
    "${CHIP_ROOT}/src/app/clusters/network-commissioning/WifiScanResponse.cpp"
    # Operational Credentials
    "${CHIP_ROOT}/src/app/clusters/operational-credentials-server/OperationalCredentialsCluster.cpp"
    # Software Diagnostics (header-only cluster, uses SoftwareDiagnosticsLogic)
    "${CHIP_ROOT}/src/app/clusters/software-diagnostics-server/SoftwareDiagnosticsLogic.cpp"
    "${CHIP_ROOT}/src/app/clusters/software-diagnostics-server/software-fault-listener.cpp"
    # WiFi Network Diagnostics
    "${CHIP_ROOT}/src/app/clusters/wifi-network-diagnostics-server/WiFiNetworkDiagnosticsCluster.cpp"
)

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRCS ${APP_SRCS} ${CODE_DRIVEN_SRCS} ${SERVER_SRCS} ${PERSISTENCE_SRCS} ${SERVER_CLUSTER_SRCS} ${CLUSTER_SRCS})

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)
