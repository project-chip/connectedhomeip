#
#    Copyright (c) 2025 Project CHIP Authors
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

from typing import Generator

from matter.idl.generators.cluster_selection import server_side_clusters
from matter.idl.matter_idl_types import Idl


def expand_path_for_idl(idl: Idl, path: str) -> Generator[str, None, None]:
    """
    Generates the expanded list of items in a path, where `path` contains
    placeholders such as `{{defined_cluster_name}}` or `{{server_cluster_name}}`
    """
    if '{{defined_cluster_name}}' in path:
        # Expands the path for every "defined" server cluster
        for cluster in idl.clusters:
            yield path.replace('{{defined_cluster_name}}', cluster.name)

    elif '{{server_cluster_name}}' in path:
        # Expands the path for every "used" server cluster (i.e. part of some "endpoint" config)
        # List is deduplicated across endpoints
        for cluster in server_side_clusters(idl):
            yield path.replace('{{server_cluster_name}}', cluster.name)
    else:
        # single item, just return as is
        yield path
