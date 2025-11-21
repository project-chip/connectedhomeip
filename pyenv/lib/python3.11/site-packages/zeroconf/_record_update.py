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

from ._dns import DNSRecord

_DNSRecord = DNSRecord


class RecordUpdate:
    __slots__ = ("new", "old")

    def __init__(self, new: DNSRecord, old: DNSRecord | None = None) -> None:
        """RecordUpdate represents a change in a DNS record."""
        self._fast_init(new, old)

    def _fast_init(self, new: _DNSRecord, old: _DNSRecord | None) -> None:
        """Fast init for RecordUpdate."""
        self.new = new
        self.old = old

    def __getitem__(self, index: int) -> DNSRecord | None:
        """Get the new or old record."""
        if index == 0:
            return self.new
        if index == 1:
            return self.old
        raise IndexError(index)
