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
from typing import Optional

from matter_idl.matter_idl_types import Cluster, Idl, Enum, Bitmap, Event, Attribute, Struct, Command

from .context import Context, IdlPostProcessor
from .parsing import NormalizeName

LOGGER = logging.getLogger('data-model-xml-data-parsing')

def merge_enum_into(e: Enum, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", e)

def merge_bitmap_into(b: Bitmap, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", b)

def merge_event_into(e: Event, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", e)

def merge_attribute_into(a: Attribute, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", a)

def merge_struct_into(s: Struct, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", s)

def merge_command_into(c: Command, cluster: Cluster):
    LOGGER.error("TODO: need to implement merge of %r", c)


def inherit_cluster_data(from_cluster: Cluster, into_cluster: Cluster):
    for e in from_cluster.enums:
        merge_enum_into(e, into_cluster)

    for b in from_cluster.bitmaps:
        merge_bitmap_into(b, into_cluster)

    for ev in from_cluster.events:
        merge_event_into(ev, into_cluster)

    for a in from_cluster.attributes:
        merge_attribute_into(a, into_cluster)

    for s in from_cluster.structs:
        merge_struct_into(s, into_cluster)

    for c in from_cluster.commands:
        merge_command_into(c, into_cluster)


class AddBaseInfoPostProcessor(IdlPostProcessor):
    def __init__(self, destination_cluster: Cluster, source_cluster_name: str, context: Context):
        self.destination = destination_cluster
        self.source_name = NormalizeName(source_cluster_name)
        self.context = context


    def FinalizeProcessing(self, idl: Idl):
        # attempt to find the base. It may be in the "names without ID" however it may also be inside
        # existing clusters (e.g. Basic Information)
        base: Optional[Cluster] = None
        if self.source_name in self.context.base_clusters:
            base = self.context.base_clusters[self.source_name]
        else:
            for c in idl.clusters:
                if c.name == self.source_name:
                    base = c
                    break

        if not base:
            LOGGER.error("Could not find the base cluster named '%s'", self.source_name)
            return

        LOGGER.info("Copying base data from '%s' into '%s'", base.name, self.destination.name)
        inherit_cluster_data(from_cluster=base, into_cluster=self.destination)


