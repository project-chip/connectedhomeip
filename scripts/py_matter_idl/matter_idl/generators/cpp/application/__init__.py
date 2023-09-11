# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
from typing import List

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Cluster, ClusterSide, Idl


def serverClustersOnly(clusters: List[Cluster]) -> List[Cluster]:
    return [c for c in clusters if c.side == ClusterSide.SERVER]


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

        self.jinja_env.filters['serverClustersOnly'] = serverClustersOnly

    def internal_render_all(self):
        """
        Renders the cpp and header files required for applications
        """

        # Header containing a macro to initialize all cluster plugins
        self.internal_render_one_output(
            template_path="PluginApplicationCallbacksHeader.jinja",
            output_file_name="app/PluginApplicationCallbacks.h",
            vars={
                'clusters': self.idl.clusters,
            }
        )

        # Source for __attribute__(weak) implementations of all cluster
        # initialization methods
        self.internal_render_one_output(
            template_path="CallbackStubSource.jinja",
            output_file_name="app/callback-stub.cpp",
            vars={
                'clusters': self.idl.clusters,
            }
        )
