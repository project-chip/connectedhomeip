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
import logging
from typing import Callable, Dict, List, Optional, Protocol, TypeVar

from matter_idl.matter_idl_types import ApiMaturity, Attribute, Bitmap, Cluster, Command, Enum, Event, Field, Idl, Struct


class Compatibility(enum.Enum):
    UNKNOWN = enum.auto()
    COMPATIBLE = enum.auto()
    INCOMPATIBLE = enum.auto()


T = TypeVar('T')


class HasName(Protocol):
    name: str


NAMED = TypeVar('NAMED', bound=HasName)


def group_list(items: List[T], get_id: Callable[[T], str]) -> Dict[str, T]:
    result = {}
    for item in items:
        result[get_id(item)] = item
    return result


def group_list_by_name(items: List[NAMED]) -> Dict[str, NAMED]:
    return group_list(items, lambda x: x.name)


def attribute_name(attribute: Attribute) -> str:
    """Get the name of an attribute."""
    return attribute.definition.name


def not_stable(maturity: ApiMaturity):
    """Determine if the given api maturity allows binary/api changes or not."""
    # TODO: internal and deprecated not currently widely used,
    #       so we enforce stability on them for now.
    return maturity == ApiMaturity.PROVISIONAL


class CompatibilityChecker:
    def __init__(self, original: Idl, updated: Idl):
        self._original_idl = original
        self._updated_idl = updated
        self.compatible = Compatibility.UNKNOWN
        self.errors: List[str] = []
        self.logger = logging.getLogger(__name__)

    def _mark_incompatible(self, reason: str):
        self.logger.error(reason)
        self.errors.append(reason)
        self.compatible = Compatibility.INCOMPATIBLE

    def _check_field_lists_are_the_same(self, location: str, original: List[Field], updated: List[Field]):
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
            self._mark_incompatible(f"{location} has field changes")

    def _check_enum_compatible(self, cluster_name: str, original: Enum, updated: Optional[Enum]):
        if not updated:
            self._mark_incompatible(
                f"Enumeration {cluster_name}::{original.name} was deleted")
            return

        if original.base_type != updated.base_type:
            self._mark_incompatible(
                f"Enumeration {cluster_name}::{original.name} switched base type from {original.base_type} to {updated.base_type}")

        # Validate that all old entries exist
        for entry in original.entries:
            # old entry must exist and have identical code
            existing = [
                item for item in updated.entries if item.name == entry.name]
            if len(existing) == 0:
                self._mark_incompatible(
                    f"Enumeration {cluster_name}::{original.name} removed entry {entry.name}")
            elif existing[0].code != entry.code:
                self._mark_incompatible(
                    f"Enumeration {cluster_name}::{original.name} changed code for entry {entry.name} from {entry.code} to {existing[0].code}")

    def _check_bitmap_compatible(self, cluster_name: str, original: Bitmap, updated: Optional[Bitmap]):
        if not updated:
            self._mark_incompatible(
                f"Bitmap {cluster_name}::{original.name} was deleted")
            return

        if original.base_type != updated.base_type:
            self._mark_incompatible(
                f"Bitmap {cluster_name}::{original.name} switched base type from {original.base_type} to {updated.base_type}")

        # Validate that all old entries exist
        for entry in original.entries:
            # old entry must exist and have identical code
            existing = [
                item for item in updated.entries if item.name == entry.name]
            if len(existing) == 0:
                self._mark_incompatible(
                    f"Bitmap {original.name} removed entry {entry.name}")
            elif existing[0].code != entry.code:
                self._mark_incompatible(
                    f"Bitmap {original.name} changed code for entry {entry.name} from {entry.code} to {existing[0].code}")

    def _check_event_compatible(self, cluster_name: str, event: Event, updated_event: Optional[Event]):
        if not updated_event:
            self._mark_incompatible(
                f"Event {cluster_name}::{event.name} was removed")
            return

        if event.code != updated_event.code:
            self._mark_incompatible(
                f"Event {cluster_name}::{event.name} code changed from {event.code} to {updated_event.code}")

        self._check_field_lists_are_the_same(
            f"Event {cluster_name}::{event.name}", event.fields, updated_event.fields)

    def _check_command_compatible(self, cluster_name: str, command: Command, updated_command: Optional[Command]):
        self.logger.debug(f"  Checking command {cluster_name}::{command.name}")
        if not updated_command:
            self._mark_incompatible(
                f"Command {cluster_name}::{command.name} was removed")
            return

        if command.code != updated_command.code:
            self._mark_incompatible(
                f"Command {cluster_name}::{command.name} code changed from {command.code} to {updated_command.code}")

        if command.input_param != updated_command.input_param:
            self._mark_incompatible(
                f"Command {cluster_name}::{command.name} input changed from {command.input_param} to {updated_command.input_param}")

        if command.output_param != updated_command.output_param:
            self._mark_incompatible(
                f"Command {cluster_name}::{command.name} output changed from {command.output_param} to {updated_command.output_param}")

        if command.qualities != updated_command.qualities:
            self._mark_incompatible(
                f"Command {cluster_name}::{command.name} qualities changed from {command.qualities} to {updated_command.qualities}")

    def _check_struct_compatible(self, cluster_name: str, original: Struct, updated: Optional[Struct]):
        self.logger.debug(f"  Checking struct {original.name}")
        if not updated:
            self._mark_incompatible(
                f"Struct {cluster_name}::{original.name} has been deleted.")
            return

        self._check_field_lists_are_the_same(
            f"Struct {cluster_name}::{original.name}", original.fields, updated.fields)

        if original.tag != updated.tag:
            self._mark_incompatible(
                f"Struct {cluster_name}::{original.name} has modified tags")

        if original.code != updated.code:
            self._mark_incompatible(
                f"Struct {cluster_name}::{original.name} has modified code (likely resnopse difference)")

        if original.qualities != updated.qualities:
            self._mark_incompatible(
                f"Struct {cluster_name}::{original.name} has modified qualities")

    def _check_attribute_compatible(self, cluster_name: str, original: Attribute, updated: Optional[Attribute]):
        self.logger.debug(
            f"  Checking attribute {cluster_name}::{original.definition.name}")
        if not updated:
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} has been deleted.")
            return

        if original.definition.code != updated.definition.code:
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} changed its code.")

        if original.definition.data_type != updated.definition.data_type:
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} changed its data type.")

        if original.definition.is_list != updated.definition.is_list:
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} changed its list status.")

        if original.definition.qualities != updated.definition.qualities:
            # optional/nullable
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} changed its data type qualities.")

        if original.qualities != updated.qualities:
            # read/write/subscribe/timed status
            self._mark_incompatible(
                f"Attribute {cluster_name}::{original.definition.name} changed its qualities.")

    def _check_enum_list_compatible(self, cluster_name: str, original: List[Enum], updated: List[Enum]):
        updated_enums = group_list_by_name(updated)

        for original_enum in original:
            updated_enum = updated_enums.get(original_enum.name)
            self._check_enum_compatible(
                cluster_name, original_enum, updated_enum)

    def _check_bitmap_list_compatible(self, cluster_name: str, original: List[Bitmap], updated: List[Bitmap]):
        updated_bitmaps = {}
        for item in updated:
            updated_bitmaps[item.name] = item

        for original_bitmap in original:
            updated_bitmap = updated_bitmaps.get(original_bitmap.name)
            self._check_bitmap_compatible(
                cluster_name, original_bitmap, updated_bitmap)

    def _check_struct_list_compatible(self, cluster_name: str, original: List[Struct], updated: List[Struct]):
        updated_structs = group_list_by_name(updated)

        for struct in original:
            self._check_struct_compatible(
                cluster_name, struct, updated_structs.get(struct.name))

    def _check_command_list_compatible(self, cluster_name: str, original: List[Command], updated: List[Command]):
        updated_commands = group_list_by_name(updated)

        for command in original:
            updated_command = updated_commands.get(command.name)
            self._check_command_compatible(
                cluster_name, command, updated_command)

    def _check_event_list_compatible(self, cluster_name: str, original: List[Event], updated: List[Event]):
        updated_events = group_list_by_name(updated)

        for event in original:
            updated_event = updated_events.get(event.name)
            self._check_event_compatible(cluster_name, event, updated_event)

    def _check_attribute_list_compatible(self, cluster_name: str, original: List[Attribute], updated: List[Attribute]):
        updated_attributes = group_list(updated, attribute_name)

        for attribute in original:
            self._check_attribute_compatible(
                cluster_name, attribute, updated_attributes.get(attribute_name(attribute)))

    def _check_cluster_list_compatible(self, original: List[Cluster], updated: List[Cluster]):
        updated_clusters = group_list(updated, lambda c: c.name)

        for original_cluster in original:
            updated_cluster = updated_clusters.get(original_cluster.name)

            if not_stable(original_cluster.api_maturity):
                continue

            if updated_cluster and not_stable(updated_cluster.api_maturity):
                continue

            self._check_cluster_compatible(original_cluster, updated_cluster)

    def _check_cluster_compatible(self, original_cluster: Cluster, updated_cluster: Optional[Cluster]):
        self.logger.debug(
            f"Checking cluster {original_cluster.name}")
        if not updated_cluster:
            self._mark_incompatible(
                f"Cluster {original_cluster.name} was deleted")
            return

        if original_cluster.code != updated_cluster.code:
            self._mark_incompatible(
                f"Cluster {original_cluster.name} has different codes {original_cluster.code} != {updated_cluster.code}")

        self._check_enum_list_compatible(
            original_cluster.name, original_cluster.enums, updated_cluster.enums)
        self._check_struct_list_compatible(
            original_cluster.name, original_cluster.structs, updated_cluster.structs)
        self._check_bitmap_list_compatible(
            original_cluster.name, original_cluster.bitmaps, updated_cluster.bitmaps)
        self._check_command_list_compatible(
            original_cluster.name, original_cluster.commands, updated_cluster.commands)
        self._check_event_list_compatible(
            original_cluster.name, original_cluster.events, updated_cluster.events)
        self._check_attribute_list_compatible(
            original_cluster.name, original_cluster.attributes, updated_cluster.attributes)

    def check(self):
        # assume ok, and then validate
        self.compatible = Compatibility.COMPATIBLE

        self._check_cluster_list_compatible(
            self._original_idl.clusters, self._updated_idl.clusters)

        return self.compatible


def is_backwards_compatible(original: Idl, updated: Idl):
    """
    Validate that 'updated' IDL contains only
    incremental changes from 'original'
    """
    checker = CompatibilityChecker(original, updated)
    return checker.check() == Compatibility.COMPATIBLE
