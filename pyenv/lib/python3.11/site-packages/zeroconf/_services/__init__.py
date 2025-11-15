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

import enum
from typing import TYPE_CHECKING, Any, Callable

if TYPE_CHECKING:
    from .._core import Zeroconf


@enum.unique
class ServiceStateChange(enum.Enum):
    Added = 1
    Removed = 2
    Updated = 3


class ServiceListener:
    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        raise NotImplementedError

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        raise NotImplementedError

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        raise NotImplementedError


class Signal:
    __slots__ = ("_handlers",)

    def __init__(self) -> None:
        self._handlers: list[Callable[..., None]] = []

    def fire(self, **kwargs: Any) -> None:
        for h in self._handlers[:]:
            h(**kwargs)

    @property
    def registration_interface(self) -> SignalRegistrationInterface:
        return SignalRegistrationInterface(self._handlers)


class SignalRegistrationInterface:
    __slots__ = ("_handlers",)

    def __init__(self, handlers: list[Callable[..., None]]) -> None:
        self._handlers = handlers

    def register_handler(self, handler: Callable[..., None]) -> SignalRegistrationInterface:
        self._handlers.append(handler)
        return self

    def unregister_handler(self, handler: Callable[..., None]) -> SignalRegistrationInterface:
        self._handlers.remove(handler)
        return self
