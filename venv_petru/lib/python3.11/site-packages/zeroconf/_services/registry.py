"""Multicast DNS Service Discovery for Python, v0.14-wmcbrine
Copyright 2003 Paul Scott-Murphy, 2014 William McBrine

This module provides a framework for the use of DNS Service Discovery
using IP multicast.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
"""

from __future__ import annotations

from .._exceptions import ServiceNameAlreadyRegistered
from .info import ServiceInfo

_str = str


class ServiceRegistry:
    """A registry to keep track of services.

    The registry must only be accessed from
    the event loop as it is not thread safe.
    """

    __slots__ = ("_services", "has_entries", "servers", "types")

    def __init__(
        self,
    ) -> None:
        """Create the ServiceRegistry class."""
        self._services: dict[str, ServiceInfo] = {}
        self.types: dict[str, list] = {}
        self.servers: dict[str, list] = {}
        self.has_entries: bool = False

    def async_add(self, info: ServiceInfo) -> None:
        """Add a new service to the registry."""
        self._add(info)

    def async_remove(self, info: list[ServiceInfo] | ServiceInfo) -> None:
        """Remove a new service from the registry."""
        self._remove(info if isinstance(info, list) else [info])

    def async_update(self, info: ServiceInfo) -> None:
        """Update new service in the registry."""
        self._remove([info])
        self._add(info)

    def async_get_service_infos(self) -> list[ServiceInfo]:
        """Return all ServiceInfo."""
        return list(self._services.values())

    def async_get_info_name(self, name: str) -> ServiceInfo | None:
        """Return all ServiceInfo for the name."""
        return self._services.get(name)

    def async_get_types(self) -> list[str]:
        """Return all types."""
        return list(self.types)

    def async_get_infos_type(self, type_: str) -> list[ServiceInfo]:
        """Return all ServiceInfo matching type."""
        return self._async_get_by_index(self.types, type_)

    def async_get_infos_server(self, server: str) -> list[ServiceInfo]:
        """Return all ServiceInfo matching server."""
        return self._async_get_by_index(self.servers, server)

    def _async_get_by_index(self, records: dict[str, list], key: _str) -> list[ServiceInfo]:
        """Return all ServiceInfo matching the index."""
        record_list = records.get(key)
        if record_list is None:
            return []
        return [self._services[name] for name in record_list]

    def _add(self, info: ServiceInfo) -> None:
        """Add a new service under the lock."""
        assert info.server_key is not None, "ServiceInfo must have a server"
        if info.key in self._services:
            raise ServiceNameAlreadyRegistered

        info.async_clear_cache()
        self._services[info.key] = info
        self.types.setdefault(info.type.lower(), []).append(info.key)
        self.servers.setdefault(info.server_key, []).append(info.key)
        self.has_entries = True

    def _remove(self, infos: list[ServiceInfo]) -> None:
        """Remove a services under the lock."""
        for info in infos:
            old_service_info = self._services.get(info.key)
            if old_service_info is None:
                continue
            assert old_service_info.server_key is not None
            self.types[old_service_info.type.lower()].remove(info.key)
            self.servers[old_service_info.server_key].remove(info.key)
            del self._services[info.key]

        self.has_entries = bool(self._services)
