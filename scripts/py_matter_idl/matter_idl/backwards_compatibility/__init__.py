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

import logging
import itertools

from enum import Enum, auto
from typing import List, Optional

from matter_idl.matter_idl_types import Idl, Struct, Enum, Cluster, Bitmap, Command, Event


class Compatibility(Enum):
    UNKNOWN = auto()
    COMPATIBLE = auto()
    INCOMPATIBLE = auto()


class CompatibilityChecker:
    def __init__(self, original: Idl, updated: Idl):
        self._original_idl = original
        self._updated_idl = updated
        self.compatible = Compatibility.UNKNOWN

    def _MarkIncompatible(self, reason: str):
        logging.error(reason)
        self.compatible = Compatibility.INCOMPATIBLE

    def CheckEnumCompatible(self, original: Enum, updated: Optional[Enum]):
        if not updated:
            self._MarkIncompatible(f"Enumeration {original.name} was deleted")
            return

        if original.base_type != updated.base_type:
            self._MarkIncompatible(f"Enumeration {original.name} switched base type from {original.base_type} to {updated.base_type}")

        # Validate that all old entries exist
        for entry in original.entries:
            # old entry must exist and have identical code
            existing = [item for item in updated.entries if item.name == entry.name ]
            if len(existing) == 0:
              self._MarkIncompatible(f"Enumeration {original.name} removed entry {entry.name}")
            elif existing[0].code != entry.code:
              self._MarkIncompatible(f"Enumeration {original.name} changed code for entry {entry.name}")

            

    def CheckEnumListCompatible(self, original: List[Enum], updated: List[Enum]):
        updated_enums = {}
        for item in updated:
            updated_enums[item.name] = item

        for original_enum in original:
            updated_enum = updated_enums.get(original_enum.name)
            self.CheckEnumCompatible(original_enum, updated_enum)

        # self._MarkIncompatible("NOT YET IMPLEMENTED")

    def CheckStructListCompatible(self, original: List[Struct], updated: List[Struct]):
        updated_structs = {}
        for item in updated:
            updated_structs[item.name] = item
        # self._MarkIncompatible("NOT YET IMPLEMENTED")

    def CheckBitmapListCompatible(self, original: List[Bitmap], updated: List[Bitmap]):
        updated_bitmaps = {}
        for item in updated:
            updated_bitmaps[item.name] = item
        # self._MarkIncompatible("NOT YET IMPLEMENTED")
        

    def CheckCommandListCompatible(self, original: List[Command], updated: List[Command]):
        updated_commands = {}
        for item in updated:
            updated_commands[item.name] = item
        # self._MarkIncompatible("NOT YET IMPLEMENTED")

    def CheckEventListCompatible(self, original: List[Event], updated: List[Event]):
        updated_events = {}
        for item in updated:
            updated_events[item.name] = item
        # self._MarkIncompatible("NOT YET IMPLEMENTED")

    def CheckClusterListCompatible(self, original: List[Cluster], updated: List[Cluster]):
        updated_clusters = {}
        for item in updated:
            updated_clusters[item.name] = item

        for original_cluster in original:
            updated_cluster = updated_clusters.get(original_cluster.name)
            self.CheckClusterCompatible(original_cluster, updated_cluster)


    def CheckClusterCompatible(self, original_cluster: Cluster, updated_cluster: Optional[Cluster]):
        if not updated_cluster:
            self._MarkIncompatible(f"Cluster {original_cluster.name} not found in updated list")
            return

        if original_cluster.code != updated_cluster.code:
            self._MarkIncompatible(f"Cluster {original_cluster.name} has different codes {original_cluster.code} != {updated_cluster.code}")

        self.CheckEnumListCompatible(original_cluster.enums, updated_cluster.enums)
        self.CheckStructListCompatible(original_cluster.structs, updated_cluster.structs)
        self.CheckBitmapListCompatible(original_cluster.bitmaps, updated_cluster.bitmaps)
        self.CheckCommandListCompatible(original_cluster.commands, updated_cluster.commands)
        self.CheckEventListCompatible(original_cluster.events, updated_cluster.events)

    def Check(self):
        # assume ok, and then validate
        self.compatible = Compatibility.COMPATIBLE

        self.CheckEnumListCompatible(self._original_idl.enums, self._updated_idl.enums)
        self.CheckStructListCompatible(self._original_idl.structs, self._updated_idl.structs)
        self.CheckClusterListCompatible(self._original_idl.clusters, self._updated_idl.clusters)

        return self.compatible

def IsBackwardsCompatible(original: Idl, updated: Idl):
    """
    Validate that 'updated' IDL contains only
    incremental changes from 'original'
    """
    checker = CompatibilityChecker(original, updated)
    return checker.Check() == Compatibility.COMPATIBLE
