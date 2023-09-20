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

import dataclasses
import enum
import itertools
import logging
from typing import Callable, Dict, List, Optional, Protocol, TypeVar

from matter_idl.matter_idl_types import Attribute, Bitmap, Cluster, ClusterSide, Command, Enum, Event, Field, Idl, Struct


class Compatibility(enum.Enum):
    UNKNOWN = enum.auto()
    COMPATIBLE = enum.auto()
    INCOMPATIBLE = enum.auto()


T = TypeVar('T')


class HasName(Protocol):
    name: str


NAMED = TypeVar('NAMED', bound=HasName)


def GroupList(items: List[T], get_id: Callable[[T], str]) -> Dict[str, T]:
    result = {}
    for item in items:
        result[get_id(item)] = item
    return result


def GroupListByName(items: List[NAMED]) -> Dict[str, NAMED]:
    return GroupList(items, lambda x: x.name)


def FullClusterName(cluster: Cluster) -> str:
    "Builds a unique cluster name considering the side as well"
    if cluster.side == ClusterSide.CLIENT:
        return f"{cluster.name}/client"
    else:
        return f"{cluster.name}/server"


def AttributeName(attribute: Attribute) -> str:
    """Get the name of an attribute."""
    return attribute.definition.name


class CompatibilityChecker:
    def __init__(self, original: Idl, updated: Idl):
        self._original_idl = original
        self._updated_idl = updated
        self.compatible = Compatibility.UNKNOWN
        self.errors: List[str] = []
        self.logger = logging.getLogger(__name__)

    def _MarkIncompatible(self, reason: str):
        self.logger.error(reason)
        self.errors.append(reason)
        self.compatible = Compatibility.INCOMPATIBLE

    def CheckFieldListsAreTheSame(self, location: str, original: List[Field], updated: List[Field]):
        """Validates no compatibility changes in a list of fields.

        Specifically no changes are allowed EXCEPT names of fields.
        """

        # Every field MUST be the same except that
        # name does not matter and order does not matter
        #
        # Comparison is done on a dict (so order does not matter)
        # and replacing names with a fixed name based on code.
        a = {}
        for item in original:
            a[item.code] = dataclasses.replace(item, name=f"entry{item.code}")

        b = {}
        for item in updated:
            b[item.code] = dataclasses.replace(item, name=f"entry{item.code}")

        if a != b:
            self._MarkIncompatible(f"{location} has field changes")

    def CheckEnumCompatible(self, cluster_name: str, original: Enum, updated: Optional[Enum]):
        if not updated:
            self._MarkIncompatible(f"Enumeration {cluster_name}::{original.name} was deleted")
            return

        if original.base_type != updated.base_type:
            self._MarkIncompatible(
                f"Enumeration {cluster_name}::{original.name} switched base type from {original.base_type} to {updated.base_type}")

        # Validate that all old entries exist
        for entry in original.entries:
            # old entry must exist and have identical code
            existing = [item for item in updated.entries if item.name == entry.name]
            if len(existing) == 0:
                self._MarkIncompatible(f"Enumeration {cluster_name}::{original.name} removed entry {entry.name}")
            elif existing[0].code != entry.code:
                self._MarkIncompatible(
                    f"Enumeration {cluster_name}::{original.name} changed code for entry {entry.name} from {entry.code} to {existing[0].code}")

    def CheckBitmapCompatible(self, cluster_name: str, original: Bitmap, updated: Optional[Bitmap]):
        if not updated:
            self._MarkIncompatible(f"Bitmap {cluster_name}::{original.name} was deleted")
            return

        if original.base_type != updated.base_type:
            self._MarkIncompatible(
                f"Bitmap {cluster_name}::{original.name} switched base type from {original.base_type} to {updated.base_type}")

        # Validate that all old entries exist
        for entry in original.entries:
            # old entry must exist and have identical code
            existing = [item for item in updated.entries if item.name == entry.name]
            if len(existing) == 0:
                self._MarkIncompatible(f"Bitmap {original.name} removed entry {entry.name}")
            elif existing[0].code != entry.code:
                self._MarkIncompatible(
                    f"Bitmap {original.name} changed code for entry {entry.name} from {entry.code} to {existing[0].code}")

    def CheckEventsCompatible(self, cluster_name: str, event: Event, updated_event: Optional[Event]):
        if not updated_event:
            self._MarkIncompatible(f"Event {cluster_name}::{event.name} was removed")
            return

        if event.code != updated_event.code:
            self._MarkIncompatible(f"Event {cluster_name}::{event.name} code changed from {event.code} to {updated_event.code}")

        self.CheckFieldListsAreTheSame(f"Event {cluster_name}::{event.name}", event.fields, updated_event.fields)

    def CheckCommandsCompatible(self, cluster_name: str, command: Command, updated_command: Optional[Command]):
        self.logger.debug(f"  Checking command {cluster_name}::{command.name}")
        if not updated_command:
            self._MarkIncompatible(f"Command {cluster_name}::{command.name} was removed")
            return

        if command.code != updated_command.code:
            self._MarkIncompatible(
                f"Command {cluster_name}::{command.name} code changed from {command.code} to {updated_command.code}")

        if command.input_param != updated_command.input_param:
            self._MarkIncompatible(
                f"Command {cluster_name}::{command.name} input changed from {command.input_param} to {updated_command.input_param}")

        if command.output_param != updated_command.output_param:
            self._MarkIncompatible(
                f"Command {cluster_name}::{command.name} output changed from {command.output_param} to {updated_command.output_param}")

        if command.qualities != updated_command.qualities:
            self._MarkIncompatible(
                f"Command {cluster_name}::{command.name} qualities changed from {command.qualities} to {updated_command.qualities}")

    def CheckStructsCompatible(self, cluster_name: str, original: Struct, updated: Optional[Struct]):
        self.logger.debug(f"  Checking struct {original.name}")
        if not updated:
            self._MarkIncompatible(f"Struct {cluster_name}::{original.name} has been deleted.")
            return

        self.CheckFieldListsAreTheSame(f"Struct {cluster_name}::{original.name}", original.fields, updated.fields)

        if original.tag != updated.tag:
            self._MarkIncompatible(f"Struct {cluster_name}::{original.name} has modified tags")

        if original.code != updated.code:
            self._MarkIncompatible(f"Struct {cluster_name}::{original.name} has modified code (likely resnopse difference)")

        if original.qualities != updated.qualities:
            self._MarkIncompatible(f"Struct {cluster_name}::{original.name} has modified qualities")

    def CheckAttributeCompatible(self, cluster_name: str, original: Attribute, updated: Optional[Attribute]):
        self.logger.debug(f"  Checking attribute {cluster_name}::{original.definition.name}")
        if not updated:
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} has been deleted.")
            return

        if original.definition.code != updated.definition.code:
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} changed its code.")

        if original.definition.data_type != updated.definition.data_type:
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} changed its data type.")

        if original.definition.is_list != updated.definition.is_list:
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} changed its list status.")

        if original.definition.qualities != updated.definition.qualities:
            # optional/nullable
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} changed its data type qualities.")

        if original.qualities != updated.qualities:
            # read/write/subscribe/timed status
            self._MarkIncompatible(f"Attribute {cluster_name}::{original.definition.name} changed its qualities.")

    def CheckEnumListCompatible(self, cluster_name: str, original: List[Enum], updated: List[Enum]):
        updated_enums = GroupListByName(updated)

        for original_enum in original:
            updated_enum = updated_enums.get(original_enum.name)
            self.CheckEnumCompatible(cluster_name, original_enum, updated_enum)

    def CheckBitmapListCompatible(self, cluster_name: str, original: List[Bitmap], updated: List[Bitmap]):
        updated_bitmaps = {}
        for item in updated:
            updated_bitmaps[item.name] = item

        for original_bitmap in original:
            updated_bitmap = updated_bitmaps.get(original_bitmap.name)
            self.CheckBitmapCompatible(cluster_name, original_bitmap, updated_bitmap)

    def CheckStructListCompatible(self, cluster_name: str, original: List[Struct], updated: List[Struct]):
        updated_structs = GroupListByName(updated)

        for struct in original:
            self.CheckStructsCompatible(cluster_name, struct, updated_structs.get(struct.name))

    def CheckCommandListCompatible(self, cluster_name: str, original: List[Command], updated: List[Command]):
        updated_commands = GroupListByName(updated)

        for command in original:
            updated_command = updated_commands.get(command.name)
            self.CheckCommandsCompatible(cluster_name, command, updated_command)

    def CheckEventListCompatible(self, cluster_name: str, original: List[Event], updated: List[Event]):
        updated_events = GroupListByName(updated)

        for event in original:
            updated_event = updated_events.get(event.name)
            self.CheckEventsCompatible(cluster_name, event, updated_event)

    def CheckAttributeListCompatible(self, cluster_name: str, original: List[Attribute], updated: List[Attribute]):
        updated_attributes = GroupList(updated, AttributeName)

        for attribute in original:
            self.CheckAttributeCompatible(cluster_name, attribute, updated_attributes.get(AttributeName(attribute)))

    def CheckClusterListCompatible(self, original: List[Cluster], updated: List[Cluster]):
        updated_clusters = GroupList(updated, FullClusterName)

        for original_cluster in original:
            updated_cluster = updated_clusters.get(FullClusterName(original_cluster))
            self.CheckClusterCompatible(original_cluster, updated_cluster)

    def CheckClusterCompatible(self, original_cluster: Cluster, updated_cluster: Optional[Cluster]):
        self.logger.debug(f"Checking cluster {FullClusterName(original_cluster)}")
        if not updated_cluster:
            self._MarkIncompatible(f"Cluster {FullClusterName(original_cluster)} was deleted")
            return

        if original_cluster.code != updated_cluster.code:
            self._MarkIncompatible(
                f"Cluster {FullClusterName(original_cluster)} has different codes {original_cluster.code} != {updated_cluster.code}")

        self.CheckEnumListCompatible(original_cluster.name, original_cluster.enums, updated_cluster.enums)
        self.CheckStructListCompatible(original_cluster.name, original_cluster.structs, updated_cluster.structs)
        self.CheckBitmapListCompatible(original_cluster.name, original_cluster.bitmaps, updated_cluster.bitmaps)
        self.CheckCommandListCompatible(original_cluster.name, original_cluster.commands, updated_cluster.commands)
        self.CheckEventListCompatible(original_cluster.name, original_cluster.events, updated_cluster.events)
        self.CheckAttributeListCompatible(original_cluster.name, original_cluster.attributes, updated_cluster.attributes)

    def Check(self):
        # assume ok, and then validate
        self.compatible = Compatibility.COMPATIBLE

        self.CheckEnumListCompatible("", self._original_idl.enums, self._updated_idl.enums)
        self.CheckStructListCompatible("", self._original_idl.structs, self._updated_idl.structs)
        self.CheckClusterListCompatible(self._original_idl.clusters, self._updated_idl.clusters)

        return self.compatible


def IsBackwardsCompatible(original: Idl, updated: Idl):
    """
    Validate that 'updated' IDL contains only
    incremental changes from 'original'
    """
    checker = CompatibilityChecker(original, updated)
    return checker.Check() == Compatibility.COMPATIBLE
