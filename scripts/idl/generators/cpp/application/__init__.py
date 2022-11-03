# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from idl.generators import CodeGenerator, GeneratorStorage
from idl.matter_idl_types import Idl, ClusterSide, Field, Attribute, Cluster, FieldQuality, Command, DataType
from idl import matter_idl_types
from idl.generators.types import ParseDataType, BasicString, BasicInteger, FundamentalType, IdlType, IdlEnumType, IdlBitmapType, TypeLookupContext
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
