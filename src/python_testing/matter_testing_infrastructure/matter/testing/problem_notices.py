#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
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

from dataclasses import dataclass
from enum import Enum
from typing import Optional, Union

from matter.testing.conversions import cluster_id_with_name, format_decimal_and_hex
from matter.testing.global_attribute_ids import GlobalAttributeIds


class ClusterMapper:
    """Describe clusters/attributes using schema names."""

    def __init__(self, legacy_cluster_mapping) -> None:
        self._mapping = legacy_cluster_mapping

    def get_cluster_string(self, cluster_id: int) -> str:
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Cluster Unknown ({cluster_id}, 0x{cluster_id:08X})"
        else:
            name = mapping["clusterName"]
            return f"Cluster {name} ({cluster_id}, 0x{cluster_id:04X})"

    def get_attribute_string(self, cluster_id: int, attribute_id) -> str:
        global_attrs = [item.value for item in GlobalAttributeIds]
        if attribute_id in global_attrs:
            return f"Attribute {GlobalAttributeIds(attribute_id).to_name()} {attribute_id}, 0x{attribute_id:04X}"
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
        else:
            attribute_mapping = mapping["attributes"].get(attribute_id, None)

            if not attribute_mapping:
                return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
            else:
                attribute_name = attribute_mapping["attributeName"]
                return f"Attribute {attribute_name} ({attribute_id}, 0x{attribute_id:04X})"


@dataclass
class ClusterPathLocation:
    endpoint_id: int
    cluster_id: int

    def __str__(self):
        return (f'\n       Endpoint: {self.endpoint_id},'
                f'\n       Cluster:  {cluster_id_with_name(self.cluster_id)}')


@dataclass
class AttributePathLocation(ClusterPathLocation):
    cluster_id: Optional[int] = None
    attribute_id: Optional[int] = None

    def as_cluster_string(self, mapper: ClusterMapper):
        desc = f"Endpoint {self.endpoint_id}"
        if self.cluster_id is not None:
            desc += f", {mapper.get_cluster_string(self.cluster_id)}"
        return desc

    def as_string(self, mapper: ClusterMapper):
        desc = self.as_cluster_string(mapper)
        if self.cluster_id is not None and self.attribute_id is not None:
            desc += f", {mapper.get_attribute_string(self.cluster_id, self.attribute_id)}"

        return desc

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n      Attribute:{format_decimal_and_hex(self.attribute_id)}')


@dataclass
class EventPathLocation(ClusterPathLocation):
    event_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Event:    {format_decimal_and_hex(self.event_id)}')


@dataclass
class CommandPathLocation(ClusterPathLocation):
    command_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Command:  {format_decimal_and_hex(self.command_id)}')


@dataclass
class FeaturePathLocation(ClusterPathLocation):
    feature_code: str

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Feature:  {self.feature_code}')


@dataclass
class DeviceTypePathLocation:
    device_type_id: int
    cluster_id: Optional[int] = None
    endpoint_id: Optional[int] = None

    def __str__(self):
        msg = ""
        if self.endpoint_id is not None:
            msg += f'\n       Endpoint ID: {self.endpoint_id}'
        msg += f'\n       DeviceType: 0x{self.device_type_id:04X}'
        if self.cluster_id:
            msg += f'\n       ClusterID: 0x{self.cluster_id:04X}'
        return msg


@dataclass
class NamespacePathLocation:
    """Location in a namespace definition"""
    namespace_id: Optional[int] = None
    tag_id: Optional[int] = None

    def __str__(self) -> str:
        result = "Namespace"
        if self.namespace_id is not None:
            result += f" 0x{self.namespace_id:04X}"
        if self.tag_id is not None:
            result += f" Tag 0x{self.tag_id:04X}"
        return result


class UnknownProblemLocation:
    def __str__(self):
        return '\n      Unknown Locations - see message for more details'


ProblemLocation = Union[ClusterPathLocation, DeviceTypePathLocation, UnknownProblemLocation, NamespacePathLocation]

# ProblemSeverity is not using StrEnum, but rather Enum, since StrEnum only
# appeared in 3.11. To make it JSON serializable easily, multiple inheritance
# from `str` is used. See https://stackoverflow.com/a/51976841.


class ProblemSeverity(str, Enum):
    NOTE = "NOTE"
    WARNING = "WARNING"
    ERROR = "ERROR"


@dataclass
class ProblemNotice:
    test_name: str
    location: ProblemLocation
    severity: ProblemSeverity
    problem: str
    spec_location: str = ""

    def __str__(self):
        return (f'\nProblem: {str(self.severity)}'
                f'\n    test_name: {self.test_name}'
                f'\n    location: {str(self.location)}'
                f'\n    problem: {self.problem}'
                f'\n    spec_location: {self.spec_location}\n')
