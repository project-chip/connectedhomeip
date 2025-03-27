# Copyright (c) 2025 Project CHIP Authors
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
#
import os

from matter.idl.generators import CodeGenerator, GeneratorStorage
from matter.idl.matter_idl_types import Idl


class SdkGenerator(CodeGenerator):
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
        Renders the cpp and header files required for sdk cluster definitions
        """

        for cluster in self.idl.clusters:
            self.internal_render_one_output(
                template_path="ClusterMetadataHeader.jinja",
                output_file_name=f"{cluster.name}.h",
                vars={
                    "cluster": cluster,
                    "input_name" : self.idl.parse_file_name,
                },
            )

        self.internal_render_one_output(
            template_path="BuildForMetadata.jinja",
            output_file_name="BUILD.gn",
            vars={
                "cluster_names": [x.name for x in self.idl.clusters],
                "input_name" : self.idl.parse_file_name,
            },
        )



