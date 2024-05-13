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

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.generators.cluster_selection import server_side_clusters
from matter_idl.matter_idl_types import Idl


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
            vars={
                'clusters': server_side_clusters(self.idl)
            }
        )

        # Source for __attribute__(weak) implementations of all cluster
        # initialization methods
        self.internal_render_one_output(
            template_path="CallbackStubSource.jinja",
            output_file_name="app/callback-stub.cpp",
            vars={
                'clusters': server_side_clusters(self.idl)
            }
        )

        self.internal_render_one_output(
            template_path="ClusterInitCallbackSource.jinja",
            output_file_name="app/cluster-init-callback.cpp",
            vars={
                'clusters': server_side_clusters(self.idl)
            }
        )
