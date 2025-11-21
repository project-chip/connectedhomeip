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

from typing import TYPE_CHECKING

from ._dns import DNSRecord
from ._record_update import RecordUpdate

if TYPE_CHECKING:
    from ._core import Zeroconf


float_ = float


class RecordUpdateListener:
    """Base call for all record listeners.

    All listeners passed to async_add_listener should use RecordUpdateListener
    as a base class. In the future it will be required.
    """

    def update_record(  # pylint: disable=no-self-use
        self, zc: Zeroconf, now: float, record: DNSRecord
    ) -> None:
        """Update a single record.

        This method is deprecated and will be removed in a future version.
        update_records should be implemented instead.
        """
        raise RuntimeError("update_record is deprecated and will be removed in a future version.")

    def async_update_records(self, zc: Zeroconf, now: float_, records: list[RecordUpdate]) -> None:
        """Update multiple records in one shot.

        All records that are received in a single packet are passed
        to update_records.

        This implementation is a compatibility shim to ensure older code
        that uses RecordUpdateListener as a base class will continue to
        get calls to update_record. This method will raise
        NotImplementedError in a future version.

        At this point the cache will not have the new records

        Records are passed as a list of RecordUpdate.  This
        allows consumers of async_update_records to avoid cache lookups.

        This method will be run in the event loop.
        """
        for record in records:
            self.update_record(zc, now, record.new)

    def async_update_records_complete(self) -> None:
        """Called when a record update has completed for all handlers.

        At this point the cache will have the new records.

        This method will be run in the event loop.
        """
