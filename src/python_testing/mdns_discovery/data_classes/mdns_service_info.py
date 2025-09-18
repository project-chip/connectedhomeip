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
    service_info: "AsyncServiceInfo" = field(repr=False, compare=False)

    @staticmethod
    def _strip_dot(s: Optional[str]) -> str:
        if not s:
            return ""
        return s[:-1] if s.endswith(".") else s

    @property
    def service_name(self) -> Optional[str]:
        return getattr(self.service_info, "name", None)

    @property
    def service_type(self) -> Optional[str]:
        return getattr(self.service_info, "type", None)

    @property
    def instance_name(self) -> Optional[str]:
        # Normalize names (remove trailing dot for comparisons)
        name = self._strip_dot(self.service_name)
        stype = self._strip_dot(self.service_type)

        if not name:
            return None

        # If subtype, use the base service type after "._sub."
        if "._sub." in stype:
            stype = stype.split("._sub.", 1)[1]

        # Remove ".<base_service_type>" suffix from the full service name
        suffix = "." + stype if stype else ""
        if suffix and name.endswith(suffix):
            return name[: -len(suffix)]
        return name

    @property
    def hostname(self) -> Optional[str]:
        return getattr(self.service_info, "server", None)

    @property
    def addresses(self) -> Optional[List[str]]:
        si = self.service_info
        if si and hasattr(si, "parsed_addresses"):
            try:
                return si.parsed_addresses()
            except Exception:
                return None
        return None

    @property
    def port(self) -> Optional[int]:
        return getattr(self.service_info, "port", None)

    @property
    def txt(self) -> Optional[Dict[str, str]]:
        return getattr(self.service_info, "decoded_properties", None)

    @property
    def priority(self) -> Optional[int]:
        return getattr(self.service_info, "priority", None)

    @property
    def interface_index(self) -> Optional[int]:
        return getattr(self.service_info, "interface_index", None)

    @property
    def weight(self) -> Optional[int]:
        return getattr(self.service_info, "weight", None)

    @property
    def host_ttl(self) -> Optional[int]:
        return getattr(self.service_info, "host_ttl", None)

    @property
    def other_ttl(self) -> Optional[int]:
        return getattr(self.service_info, "other_ttl", None)
