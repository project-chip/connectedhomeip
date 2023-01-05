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

from matter_idl.generators import CodeGenerator, GeneratorStorage
from matter_idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldQuality, Command, DataType
from matter_idl import matter_idl_types
from matter_idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlEnumType, IdlBitmapType, TypeLookupContext
from typing import Union, List, Set
from stringcase import capitalcase

import enum
import logging


def serverClustersOnly(clusters: List[Cluster]) -> List[Cluster]:
    return [c for c in clusters if c.side == ClusterSide.SERVER]


class CppApplicationGenerator(CodeGenerator):
    """
    Generation of cpp code for application implementation for matter.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        """
        Inintialization is specific for java generation and will add
        filters as required by the java .jinja templates to function.
        """
        super().__init__(storage, idl)

        self.jinja_env.filters['serverClustersOnly'] = serverClustersOnly

    def internal_render_all(self):
        """
        Renders the cpp and header files required for applications
        """

        # Header containing a macro to initialize all cluster plugins
        self.internal_render_one_output(
            template_path="cpp/application/PluginApplicationCallbacksHeader.jinja",
            output_file_name="app/PluginApplicationCallbacks.h",
            vars={
                'clusters': self.idl.clusters,
            }
        )

        # Source for __attribute__(weak) implementations of all cluster
        # initialization methods
        self.internal_render_one_output(
            template_path="cpp/application/CallbackStubSource.jinja",
            output_file_name="app/callback-stub.cpp",
            vars={
                'clusters': self.idl.clusters,
            }
        )
