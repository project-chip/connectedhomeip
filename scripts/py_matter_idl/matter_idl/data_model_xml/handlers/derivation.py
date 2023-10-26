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
import logging

from matter_idl.matter_idl_types import Cluster, Idl

from .context import Context, IdlPostProcessor
from .parsing import NormalizeName

LOGGER = logging.getLogger('data-model-xml-data-parsing')


class AddBaseInfoPostProcessor(IdlPostProcessor):
    def __init__(self, destination_cluster: Cluster, source_cluster_name: str, context: Context):
        self.destination = destination_cluster
        self.source_name = NormalizeName(source_cluster_name)
        self.context = context

    def FinalizeProcessing(self, idl: Idl):
        # FIXME implement
        LOGGER.error(
            f"NOT YET IMPLEMENTED: attach {self.source_name} to {self.destination.name}")
