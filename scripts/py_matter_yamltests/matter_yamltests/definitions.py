#
#    Copyright (c) 2022 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import enum
import glob
import io
from typing import List, Optional

from matter_idl.matter_idl_types import (Attribute, Bitmap, Cluster, Command, Enum, Event, FieldQuality, Struct, StructQuality,
                                         StructTag)
from matter_idl.zapxml import ParseSource, ParseXmls

from .pseudo_clusters.pseudo_clusters import PseudoClusters


class _ItemType(enum.Enum):
    Cluster = 0
    Request = 1
    Response = 2
    Attribute = 3
    Event = 4
    Bitmap = 5
    Enum = 6
    Struct = 7


class SpecDefinitions:

    def __init__(self, sources: List[ParseSource]):
        self.__clusters_by_id: dict[int, Cluster] = {}
        self.__commands_by_id: dict[int, dict[int, Command]] = {}
        self.__responses_by_id: dict[int, dict[int, Struct]] = {}
        self.__attributes_by_id: dict[int, dict[int, Attribute]] = {}
        self.__events_by_id: dict[int, dict[int, Event]] = {}

        self.__clusters_by_name: dict[str, int] = {}
        self.__commands_by_name: dict[str, int] = {}
        self.__responses_by_name: dict[str, int] = {}
        self.__attributes_by_name: dict[str, int] = {}
        self.__events_by_name: dict[str, int] = {}

        self.__bitmaps_by_name: dict[str, dict[str, Bitmap]] = {}
        self.__enums_by_name: dict[str, dict[str, Enum]] = {}
        self.__structs_by_name: dict[str, dict[str, Struct]] = {}

        idl = ParseXmls(sources)

        for cluster in idl.clusters:
            code: int = cluster.code
            name: str = cluster.name
            self.__clusters_by_id[code] = cluster
            self.__commands_by_id[code] = {c.code: c for c in cluster.commands}
            self.__responses_by_id[code] = {}
            self.__attributes_by_id[code] = {
                a.definition.code: a for a in cluster.attributes}
            self.__events_by_id[code] = {e.code: e for e in cluster.events}

            self.__clusters_by_name[name] = cluster.code
            self.__commands_by_name[name] = {
                c.name: c.code for c in cluster.commands}
            self.__responses_by_name[name] = {}
            self.__attributes_by_name[name] = {
                a.definition.name: a.definition.code for a in cluster.attributes}
            self.__events_by_name[name] = {
                e.name: e.code for e in cluster.events}

            self.__bitmaps_by_name[name] = {
                b.name: b for b in cluster.bitmaps}
            self.__enums_by_name[name] = {
                e.name: e for e in cluster.enums}
            self.__structs_by_name[name] = {
                s.name: s for s in cluster.structs}

            for struct in cluster.structs:
                if struct.tag == StructTag.RESPONSE:
                    self.__responses_by_id[code][struct.code] = struct
                    self.__responses_by_name[name][struct.name] = struct.code

    def get_cluster_names(self) -> List[str]:
        return [name for name, _ in self.__clusters_by_name.items()]

    def get_cluster_name(self, cluster_id: int) -> str:
        cluster = self.__clusters_by_id.get(cluster_id)
        return cluster.name if cluster else None

    def get_command_name(self, cluster_id: int, command_id: int) -> str:
        command = self.__get_by_id(cluster_id, command_id, _ItemType.Request)
        return command.name if command else None

    def get_response_name(self, cluster_id: int, response_id: int) -> str:
        response = self.__get_by_id(
            cluster_id, response_id, _ItemType.Response)
        return response.name if response else None

    def get_attribute_name(self, cluster_id: int, attribute_id: int) -> str:
        attribute = self.__get_by_id(
            cluster_id, attribute_id, _ItemType.Attribute)
        return attribute.definition.name if attribute else None

    def get_event_name(self, cluster_id: int, event_id: int) -> str:
        event = self.__get_by_id(cluster_id, event_id, _ItemType.Event)
        return event.name if event else None

    def get_cluster_id_by_name(self, cluster_name: str) -> int:
        if cluster_name is None:
            return None

        # The idl parser remove spaces
        cluster_name = cluster_name.replace(' ', '')
        return self.__clusters_by_name.get(cluster_name)

    def has_cluster_by_name(self, cluster_name: str) -> bool:
        cluster_id = self.get_cluster_id_by_name(cluster_name)
        return cluster_id is not None

    def get_command_by_name(self, cluster_name: str, command_name: str) -> Command:
        return self.__get_by_name(cluster_name, command_name, _ItemType.Request)

    def get_response_by_name(self, cluster_name: str, response_name: str) -> Struct:
        return self.__get_by_name(cluster_name, response_name, _ItemType.Response)

    def get_attribute_by_name(self, cluster_name: str, attribute_name: str) -> Attribute:
        return self.__get_by_name(cluster_name, attribute_name, _ItemType.Attribute)

    def get_event_by_name(self, cluster_name: str, event_name: str) -> Event:
        return self.__get_by_name(cluster_name, event_name, _ItemType.Event)

    def get_bitmap_by_name(self, cluster_name: str, bitmap_name: str) -> Bitmap:
        return self.__get_by_name(cluster_name, bitmap_name, _ItemType.Bitmap)

    def get_enum_by_name(self, cluster_name: str, enum_name: str) -> Bitmap:
        return self.__get_by_name(cluster_name, enum_name, _ItemType.Enum)

    def get_struct_by_name(self, cluster_name: str, struct_name: str) -> Struct:
        return self.__get_by_name(cluster_name, struct_name, _ItemType.Struct)

    def get_type_by_name(self, cluster_name: str, target_name: str):
        bitmap = self.get_bitmap_by_name(cluster_name, target_name)
        if bitmap:
            return bitmap

        enum = self.get_enum_by_name(cluster_name, target_name)
        if enum:
            return enum

        struct = self.get_struct_by_name(cluster_name, target_name)
        if struct:
            return struct

        event = self.get_event_by_name(cluster_name, target_name)
        if event:
            return event

        return None

    def get_command_names(self, cluster_name: str) -> List[str]:
        targets = self.__get_targets_by_cluster_name(
            cluster_name, _ItemType.Request)
        return [] if targets is None else [name for name in targets]

    def get_event_names(self, cluster_name: str) -> List[str]:
        targets = self.__get_targets_by_cluster_name(
            cluster_name, _ItemType.Event)
        return [] if targets is None else [name for name in targets]

    def get_attribute_names(self, cluster_name: str) -> List[str]:
        targets = self.__get_targets_by_cluster_name(
            cluster_name, _ItemType.Attribute)
        return [] if targets is None else [name for name in targets]

    def is_fabric_scoped(self, target) -> bool:
        if isinstance(target, Event):
            return bool(target.is_fabric_sensitive)

        if isinstance(target, Struct) and hasattr(target, 'qualities'):
            return bool(target.qualities & StructQuality.FABRIC_SCOPED)

        return False

    def is_nullable(self, target) -> bool:
        if hasattr(target, 'qualities'):
            return bool(target.qualities & FieldQuality.NULLABLE)
        return False

    def __get_by_name(self, cluster_name: str, target_name: str, target_type: _ItemType):
        if target_name is None:
            return None

        cluster_id = self.get_cluster_id_by_name(cluster_name)
        if cluster_id is None:
            return None

        targets = self.__get_targets_by_cluster_name(cluster_name, target_type)
        if targets is None:
            return None

        target = None

        if target_type == _ItemType.Request:
            target_id = targets.get(target_name)
            target = self.__get_by_id(cluster_id, target_id, target_type)
        elif target_type == _ItemType.Response:
            target_id = targets.get(target_name)
            target = self.__get_by_id(cluster_id, target_id, target_type)
        elif target_type == _ItemType.Event:
            target_id = targets.get(target_name)
            target = self.__get_by_id(cluster_id, target_id, target_type)
        elif target_type == _ItemType.Attribute:
            target_id = targets.get(target_name)
            target = self.__get_by_id(cluster_id, target_id, target_type)
        elif target_type == _ItemType.Bitmap:
            target = targets.get(target_name)
        elif target_type == _ItemType.Enum:
            target = targets.get(target_name)
        elif target_type == _ItemType.Struct:
            target = targets.get(target_name)

        return target

    def __get_by_id(self, cluster_id: int, target_id: int, target_type: str):
        target_mapping = {
            _ItemType.Request: self.__commands_by_id,
            _ItemType.Response: self.__responses_by_id,
            _ItemType.Attribute: self.__attributes_by_id,
            _ItemType.Event: self.__events_by_id,
        }

        targets = target_mapping[target_type].get(cluster_id)
        if targets is None:
            return None

        return targets.get(target_id)

    def __get_targets_by_cluster_name(self, cluster_name: str, target_type: _ItemType):
        if not cluster_name:
            return None

        target_mapping = {
            _ItemType.Request: self.__commands_by_name,
            _ItemType.Response: self.__responses_by_name,
            _ItemType.Attribute: self.__attributes_by_name,
            _ItemType.Event: self.__events_by_name,
            _ItemType.Bitmap: self.__bitmaps_by_name,
            _ItemType.Enum: self.__enums_by_name,
            _ItemType.Struct: self.__structs_by_name,
        }

        # The idl parser remove spaces
        cluster_name = cluster_name.replace(' ', '')
        return target_mapping[target_type].get(cluster_name)


def SpecDefinitionsFromPaths(paths: str, pseudo_clusters: Optional[PseudoClusters] = PseudoClusters([])):
    filenames = []
    for path in paths:
        if '*' in path or '?' in path:
            filenames.extend(glob.glob(path, recursive=False))
        else:
            filenames.append(path)

    sources = [ParseSource(source=name) for name in filenames]

    for pseudo_cluster in pseudo_clusters.clusters:
        if pseudo_cluster.definition is not None:
            name = pseudo_cluster.name
            definition = pseudo_cluster.definition
            sources = (
                sources + [ParseSource(source=io.StringIO(definition), name=name)])

    return SpecDefinitions(sources)
