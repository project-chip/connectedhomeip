#
#    Copyright (c) 2023 Project CHIP Authors
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

from typing import List, Set

from matter.idl.matter_idl_types import Cluster, Idl

# Clusters implemented via mode-base-server use Matter*ClusterInitCallback in
# CodeDrivenInitShutdown.cpp and do not declare emberAf*ClusterInitCallback in
# callback.h. Keep in sync with mode-base entries in src/app/zap_cluster_list.json.
_MODE_BASE_SERVER_CLUSTER_NAMES: Set[str] = {
    "DeviceEnergyManagementMode",
    "DishwasherMode",
    "EnergyEvseMode",
    "LaundryWasherMode",
    "MicrowaveOvenMode",
    "OvenMode",
    "RefrigeratorAndTemperatureControlledCabinetMode",
    "RvcCleanMode",
    "RvcRunMode",
    "WaterHeaterMode",
}


def server_side_clusters(idl: Idl) -> List[Cluster]:
    """
    Return a list of clusters that are instantiated in at least one endpoint
    within the given IDL.
    """
    cluster_names = set()

    for e in idl.endpoints:
        for item in e.server_clusters:
            cluster_names.add(item.name)

    return [c for c in idl.clusters if c.name in cluster_names]


def ember_af_cluster_init_callback_clusters(idl: Idl) -> List[Cluster]:
    """
    Return server-side clusters that use emberAf*ClusterInitCallback dispatch
    in cluster-callbacks.cpp.

    Mode-base-server clusters are excluded; they use Matter*ClusterInitCallback
    via CodeDrivenInitShutdown.cpp instead.
    """
    return [
        c for c in server_side_clusters(idl)
        if c.name not in _MODE_BASE_SERVER_CLUSTER_NAMES
    ]


def binding_clusters(idl: Idl) -> List[Cluster]:
    """
    Return a list of clusters that show up as bindings on some endpoints
    within the given IDL.
    """
    cluster_names = set()

    for e in idl.endpoints:
        for item in e.client_bindings:
            cluster_names.add(item)

    return [c for c in idl.clusters if c.name in cluster_names]
