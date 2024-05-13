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

from typing import List

from matter_idl.matter_idl_types import Cluster, Idl


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
