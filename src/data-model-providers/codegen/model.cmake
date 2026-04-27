# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set(BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

# If you change this list, please ALSO CHANGE model.gni
SET(CODEGEN_DATA_MODEL_SOURCES
  "${BASE_DIR}/ClusterIntegration.cpp"
  "${BASE_DIR}/ClusterIntegration.h"
  "${BASE_DIR}/CodegenDataModelProvider.cpp"
  "${BASE_DIR}/CodegenDataModelProvider.h"
  "${BASE_DIR}/CodegenDataModelProvider_Read.cpp"
  "${BASE_DIR}/CodegenDataModelProvider_Write.cpp"
  "${BASE_DIR}/EmberAttributeDataBuffer.cpp"
  "${BASE_DIR}/EmberAttributeDataBuffer.h"
  "${BASE_DIR}/Instance.cpp"

  # These are dependencies from model.gni that are not included directly in cmake

  # "${chip_root}/src/app/server-cluster",
  "${BASE_DIR}/../../app/server-cluster/AttributeListBuilder.cpp"
  "${BASE_DIR}/../../app/server-cluster/AttributeListBuilder.h"
  "${BASE_DIR}/../../app/server-cluster/DefaultServerCluster.cpp"
  "${BASE_DIR}/../../app/server-cluster/DefaultServerCluster.h"
  "${BASE_DIR}/../../app/server-cluster/ServerClusterContext.h"
  "${BASE_DIR}/../../app/server-cluster/ServerClusterInterface.cpp"
  "${BASE_DIR}/../../app/server-cluster/ServerClusterInterface.h"

  # "${chip_root}/src/app/persistence",
  "${BASE_DIR}/../../app/persistence/AttributePersistence.cpp"
  "${BASE_DIR}/../../app/persistence/AttributePersistence.h"
  "${BASE_DIR}/../../app/persistence/AttributePersistenceProvider.h"
  "${BASE_DIR}/../../app/persistence/PascalString.h"
  "${BASE_DIR}/../../app/persistence/String.cpp"
  "${BASE_DIR}/../../app/persistence/String.h"

  # "${chip_root}/src/app/server-cluster:registry",
  "${BASE_DIR}/../../app/server-cluster/ServerClusterInterfaceRegistry.cpp"
  "${BASE_DIR}/../../app/server-cluster/ServerClusterInterfaceRegistry.h"
  "${BASE_DIR}/../../app/server-cluster/SingleEndpointServerClusterRegistry.cpp"
  "${BASE_DIR}/../../app/server-cluster/SingleEndpointServerClusterRegistry.h"
)
