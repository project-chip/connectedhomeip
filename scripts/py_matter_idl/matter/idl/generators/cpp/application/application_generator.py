# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from dataclasses import dataclass
from typing import List, Mapping, Optional

from matter.idl.generators import CodeGenerator
from matter.idl.generators.cluster_selection import server_side_clusters
from matter.idl.generators.storage import GeneratorStorage
from matter.idl.generators.type_definitions import TypeLookupContext
from matter.idl.matter_idl_types import Bitmap, Idl, ServerClusterInstantiation


@dataclass
class Feature:
    name: str
    value: int


@dataclass
class ServerClusterConfig:
    endpoint_number: int
    cluster_name: str
    feature_map: int
    instance: ServerClusterInstantiation

    # Set if a `Feature` enumeration is available in the underlying
    # cluster type
    feature_bitmap_type: Optional[Bitmap]

    @property
    def features(self) -> list[Feature]:
        """Returns a list of names of features based on the `feature_map` value"""

        if not self.feature_map:
            return []

        if not self.feature_bitmap_type:
            raise Exception(
                f"No feature enumeration for cluster {self.cluster_name}")

        result = []
        returned_values = 0

        for entry in self.feature_bitmap_type.entries:
            if self.feature_map & entry.code == 0:
                continue
            result.append(Feature(name=entry.name, value=entry.code))
            returned_values = returned_values | entry.code

        if self.feature_map != returned_values:
            raise Exception(
                f"Not all bits set in the feature map for {self.cluster_name} are defined: {self.feature_map}")

        return result


@dataclass
class ClusterConfiguration:
    endpoint_configs: List[ServerClusterConfig]
    feature_bitmap_type: Optional[Bitmap]


def find_feature_bitmap(idl: Idl, cluster_name: str) -> Optional[Bitmap]:
    """
    Searches for an enumeration named `Feature` within the given cluster
    and returns it.
    """
    cluster = [c for c in idl.clusters if c.name == cluster_name][0]
    if not cluster:
        raise Exception(f"Cluster {cluster_name} not found in IDL definition.")
    lookup = TypeLookupContext(idl, cluster)

    for name in ["Feature", f"{cluster_name}Feature"]:
        value = lookup.find_bitmap(name)
        if value:
            return value

    return None


def cluster_instances(idl: Idl) -> Mapping[str, ClusterConfiguration]:
    """
    Returns a map with all configured clusters in the application.

    The map contains a list of all server cluster configs for which
    a cluster is enabled within the IDL file.
    """
    # Map of cluster names to actual cluster data
    endpoint_infos = {}

    # Generating metadata for every cluster
    for endpoint in idl.endpoints:
        for server_cluster in endpoint.server_clusters:

            # Defaults as per spec, however ZAP should generally
            # contain valid values here as they are required
            feature_map = 0

            for attribute in server_cluster.attributes:
                if attribute.default is None:
                    continue

                match attribute.name:
                    case "featureMap":
                        assert isinstance(attribute.default, int)
                        feature_map = attribute.default
                    case _:
                        # no other attributes are interesting at this point
                        # although we may want to pull in some defaults
                        pass

            name = server_cluster.name
            feature_bitmap_type = find_feature_bitmap(idl, name)
            if name not in endpoint_infos:
                endpoint_infos[name] = ClusterConfiguration(
                    endpoint_configs=[],
                    feature_bitmap_type=feature_bitmap_type,
                )

            endpoint_infos[name].endpoint_configs.append(
                ServerClusterConfig(
                    endpoint_number=endpoint.number,
                    cluster_name=name,
                    feature_map=feature_map,
                    instance=server_cluster,
                    feature_bitmap_type=feature_bitmap_type
                )
            )

    return endpoint_infos


class CppApplicationGenerator(CodeGenerator):
    """
    Generation of cpp code for application implementation for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl, **kargs):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl, fs_loader_searchpath=os.path.dirname(__file__))

    def internal_render_all(self):
        """
        Renders the cpp and header files required for applications
        """

        # Header containing a macro to initialize all cluster plugins
        self.internal_render_one_output(
            template_path="PluginApplicationCallbacksHeader.jinja",
            output_file_name="app/PluginApplicationCallbacks.h",
            vars={"clusters": server_side_clusters(self.idl)},
        )

        # Source for __attribute__(weak) implementations of all cluster
        # initialization methods
        self.internal_render_one_output(
            template_path="CallbackStubSource.jinja",
            output_file_name="app/callback-stub.cpp",
            vars={"clusters": server_side_clusters(self.idl)},
        )

        self.internal_render_one_output(
            template_path="ClusterCallbacksSource.jinja",
            output_file_name="app/cluster-callbacks.cpp",
            vars={
                'clusters': server_side_clusters(self.idl)
            }
        )

        for name, config in cluster_instances(self.idl).items():
            all_enabled_attributes = set()
            all_enabled_commands = set()
            for cfg in config.endpoint_configs:
                for attr in cfg.instance.attributes:
                    all_enabled_attributes.add(attr.name)
                for cmd in cfg.instance.commands:
                    all_enabled_commands.add(cmd.name)

            all_enabled_attributes = list(all_enabled_attributes)
            all_enabled_commands = list(all_enabled_commands)

            all_enabled_attributes.sort()
            all_enabled_commands.sort()

            self.internal_render_one_output(
                template_path="ServerClusterConfig.jinja",
                output_file_name=f"app/static-cluster-config/{name}.h",
                vars={
                    "cluster_name": name,
                    "config": config,
                    "input_name": self.idl.parse_file_name,
                    "all_enabled_attributes": all_enabled_attributes,
                    "all_enabled_commands": all_enabled_commands,
                },
            )
