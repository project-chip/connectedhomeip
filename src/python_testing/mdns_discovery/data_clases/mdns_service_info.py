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

from dataclasses import dataclass, field
from typing import Dict, List, Optional

from zeroconf.asyncio import AsyncServiceInfo

from .json_serializable import JsonSerializable


@dataclass
class MdnsServiceInfo(JsonSerializable):
    service_info: AsyncServiceInfo = field(repr=False, compare=False)

    service_name: Optional[str] = field(init=False)
    service_type: Optional[str] = field(init=False)
    instance_name: Optional[str] = field(init=False)
    server: Optional[str] = field(init=False)
    port: Optional[int] = field(init=False)
    addresses: Optional[List[str]] = field(init=False)
    txt_record: Optional[Dict[str, str]] = field(init=False)
    priority: Optional[int] = field(init=False)
    interface_index: Optional[int] = field(init=False)
    weight: Optional[int] = field(init=False)
    host_ttl: Optional[int] = field(init=False)
    other_ttl: Optional[int] = field(init=False)

    def __post_init__(self):
        si = self.service_info

        self.service_name = si.name
        self.service_type = si.type
        self.server = si.server
        self.port = si.port
        self.addresses = si.parsed_addresses()
        self.txt_record = si.decoded_properties
        self.priority = si.priority
        self.interface_index = si.interface_index
        self.weight = si.weight
        self.host_ttl = si.host_ttl
        self.other_ttl = si.other_ttl

        def _strip_dot(s: str | None) -> str:
            if not s:
                return ""
            return s[:-1] if s.endswith(".") else s

        # Normalize names (remove trailing dot for comparisons)
        name = _strip_dot(self.service_name)
        stype = _strip_dot(self.service_type)

        # If subtype, use the base service type after "._sub."
        if "._sub." in stype:
            stype = stype.split("._sub.", 1)[1]

        # Remove ".<base_service_type>" suffix from the full service name
        suffix = "." + stype if stype else ""
        if suffix and name.endswith(suffix):
            self.instance_name = name[: -len(suffix)]
        else:
            self.instance_name = name
