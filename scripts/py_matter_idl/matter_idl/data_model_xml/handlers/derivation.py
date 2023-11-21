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
import dataclasses
import logging
from typing import Iterable, Optional, Protocol, TypeVar

from matter_idl.matter_idl_types import Attribute, AttributeQuality, Bitmap, Cluster, Command, Enum, Event, Idl, Struct

from .context import Context, IdlPostProcessor
from .parsing import NormalizeName

LOGGER = logging.getLogger('data-model-xml-data-parsing')

T = TypeVar("T")


class HasName(Protocol):
    name: str


NAMED = TypeVar('NAMED', bound=HasName)


def get_item_with_name(items: Iterable[NAMED], name: str) -> Optional[NAMED]:
    """Find an item with the given name.

    Returns none if that item does not exist
    """
    for item in items:
        if item.name == name:
            return item
    return None


def merge_enum_into(e: Enum, cluster: Cluster):
    existing = get_item_with_name(cluster.enums, e.name)

    if existing:
        # Remove existing but merge constants into e
        cluster.enums.remove(existing)
        for value in existing.entries:
            if not get_item_with_name(e.entries, value.name):
                e.entries.append(value)

    cluster.enums.append(e)


def merge_bitmap_into(b: Bitmap, cluster: Cluster):
    existing = get_item_with_name(cluster.bitmaps, b.name)

    if existing:
        # Remove existing but merge constants into e
        cluster.bitmaps.remove(existing)
        for value in existing.entries:
            if not get_item_with_name(b.entries, value.name):
                b.entries.append(value)

    cluster.bitmaps.append(b)


def merge_event_into(e: Event, cluster: Cluster):
    existing = get_item_with_name(cluster.events, e.name)
    if existing:
        LOGGER.error("TODO: Do not know how to merge event for %s::%s",
                     cluster.name, existing.name)
        cluster.events.remove(existing)

    cluster.events.append(e)


def merge_attribute_into(a: Attribute, cluster: Cluster):
    """Pushes an attribute from a base cluster into an already
       parsed cluster.
    """
    existing: Optional[Attribute] = None
    for existing_a in cluster.attributes:
        if existing_a.definition.name == a.definition.name:
            existing = existing_a
            break

    if existing:
        # Merge examples:
        #   UserLabelCluster::LabelList changes qualities (access, mandatory)
        #   ModeDishwasher:
        #      - changes the type of a list from sint32 to list[ModeOptionStruct]
        #      - Sets the field as read-only

        # Carry over data type and definitions, except quality and maturity
        existing.definition = dataclasses.replace(
            a.definition,
            qualities=existing.definition.qualities,
            api_maturity=existing.definition.api_maturity,
        )

        # Inherit attribute quality
        if existing.qualities == AttributeQuality.NONE:
            existing.qualities = a.qualities

        return

    cluster.attributes.append(a)


def merge_struct_into(s: Struct, cluster: Cluster):
    existing = get_item_with_name(cluster.structs, s.name)
    if existing:
        # Do not provide merging as it seems XML only adds
        # constraints and conformance to struct elements
        #
        # TODO: at  some point we may be able to merge some things,
        #       if we find that derived clusters actually add useful things here
        #
        # LOGGER.error("TODO: Do not know how to merge structs for %s::%s", cluster.name, existing.name)
        cluster.structs.remove(existing)

    cluster.structs.append(s)


def merge_command_into(c: Command, cluster: Cluster):
    existing = get_item_with_name(cluster.commands, c.name)

    if existing:
        LOGGER.error("TODO: Do not know how to merge command for %s::%s",
                     cluster.name, existing.name)
        cluster.commands.remove(existing)

    cluster.commands.append(c)


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
        if self.source_name in self.context.abstract_base_clusters:
            base = self.context.abstract_base_clusters[self.source_name]
        else:
            for c in idl.clusters:
                if c.name == self.source_name:
                    base = c
                    break

        if not base:
            LOGGER.error(
                "Could not find the base cluster named '%s'", self.source_name)
            return

        LOGGER.info("Copying base data from '%s' into '%s'",
                    base.name, self.destination.name)
        inherit_cluster_data(from_cluster=base, into_cluster=self.destination)
