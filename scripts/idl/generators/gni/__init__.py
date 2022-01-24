#!/usr/bin/env python
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

from idl.generators import CodeGenerator, GeneratorStorage
from idl.matter_idl_types import Idl, Cluster, ClusterSide
from typing import List

import logging


def clientSide(clusters: List[Cluster]):
    for cluster in clusters:
        if cluster.side == ClusterSide.CLIENT:
            yield cluster


def serverSide(clusters: List[Cluster]):
    for cluster in clusters:
        if cluster.side == ClusterSide.SERVER:
            yield cluster


class GniGenerator(CodeGenerator):
    """
    Generation of ni data based on IDL files.
    """

    def __init__(self, storage: GeneratorStorage, idl: Idl):
        super().__init__(storage, idl)

        self.jinja_env.filters['clientSide'] = clientSide
        self.jinja_env.filters['serverSide'] = serverSide

    def internal_render_all(self):
        self.internal_render_one_output(
            template_path="gni/idl_content.jinja",
            output_file_name="idl_content.gni",
            vars={
                'clusters': self.idl.clusters,
            }
        )
