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

from collections.abc import Iterable
from heapq import heapify, heappop, heappush
from typing import Union, cast

from ._dns import (
    DNSAddress,
    DNSEntry,
    DNSHinfo,
    DNSNsec,
    DNSPointer,
    DNSRecord,
    DNSService,
    DNSText,
)
from ._utils.time import current_time_millis
from .const import _ONE_SECOND, _TYPE_PTR

_UNIQUE_RECORD_TYPES = (DNSAddress, DNSHinfo, DNSPointer, DNSText, DNSService)
_UniqueRecordsType = Union[DNSAddress, DNSHinfo, DNSPointer, DNSText, DNSService]
_DNSRecordCacheType = dict[str, dict[DNSRecord, DNSRecord]]
_DNSRecord = DNSRecord
_str = str
_float = float
_int = int

# The minimum number of scheduled record expirations before we start cleaning up
# the expiration heap. This is a performance optimization to avoid cleaning up the
# heap too often when there are only a few scheduled expirations.
_MIN_SCHEDULED_RECORD_EXPIRATION = 100


def _remove_key(cache: _DNSRecordCacheType, key: _str, record: _DNSRecord) -> None:
    """Remove a key from a DNSRecord cache

    This function must be run in from event loop.
    """
    record_cache = cache[key]
    del record_cache[record]
    if not record_cache:
        del cache[key]


class DNSCache:
    """A cache of DNS entries."""

    def __init__(self) -> None:
        self.cache: _DNSRecordCacheType = {}
        self._expire_heap: list[tuple[float, DNSRecord]] = []
        self._expirations: dict[DNSRecord, float] = {}
        self.service_cache: _DNSRecordCacheType = {}

    # Functions prefixed with async_ are NOT threadsafe and must
    # be run in the event loop.

    def _async_add(self, record: _DNSRecord) -> bool:
        """Adds an entry.

        Returns true if the entry was not already in the cache.

        This function must be run in from event loop.
        """
        # Previously storage of records was implemented as a list
        # instead a dict. Since DNSRecords are now hashable, the implementation
        # uses a dict to ensure that adding a new record to the cache
        # replaces any existing records that are __eq__ to each other which
        # removes the risk that accessing the cache from the wrong
        # direction would return the old incorrect entry.
        if (store := self.cache.get(record.key)) is None:
            store = self.cache[record.key] = {}
        new = record not in store and not isinstance(record, DNSNsec)
        store[record] = record
        when = record.created + (record.ttl * 1000)
        if self._expirations.get(record) != when:
            # Avoid adding duplicates to the heap
            heappush(self._expire_heap, (when, record))
            self._expirations[record] = when

        if isinstance(record, DNSService):
            service_record = record
            if (service_store := self.service_cache.get(service_record.server_key)) is None:
                service_store = self.service_cache[service_record.server_key] = {}
            service_store[service_record] = service_record
        return new

    def async_add_records(self, entries: Iterable[DNSRecord]) -> bool:
        """Add multiple records.

        Returns true if any of the records were not in the cache.

        This function must be run in from event loop.
        """
        new = False
        for entry in entries:
            if self._async_add(entry):
                new = True
        return new

    def _async_remove(self, record: _DNSRecord) -> None:
        """Removes an entry.

        This function must be run in from event loop.
        """
        if isinstance(record, DNSService):
            service_record = record
            _remove_key(self.service_cache, service_record.server_key, service_record)
        _remove_key(self.cache, record.key, record)
        self._expirations.pop(record, None)

    def async_remove_records(self, entries: Iterable[DNSRecord]) -> None:
        """Remove multiple records.

        This function must be run in from event loop.
        """
        for entry in entries:
            self._async_remove(entry)

    def async_expire(self, now: _float) -> list[DNSRecord]:
        """Purge expired entries from the cache.

        This function must be run in from event loop.

        :param now: The current time in milliseconds.
        """
        if not (expire_heap_len := len(self._expire_heap)):
            return []

        expired: list[DNSRecord] = []
        # Find any expired records and add them to the to-delete list
        while self._expire_heap:
            when_record = self._expire_heap[0]
            when = when_record[0]
            if when > now:
                break
            heappop(self._expire_heap)
            # Check if the record hasn't been re-added to the heap
            # with a different expiration time as it will be removed
            # later when it reaches the top of the heap and its
            # expiration time is met.
            record = when_record[1]
            if self._expirations.get(record) == when:
                expired.append(record)

        # If the expiration heap grows larger than the number expirations
        # times two, we clean it up to avoid keeping expired entries in
        # the heap and consuming memory. We guard this with a minimum
        # threshold to avoid cleaning up the heap too often when there are
        # only a few scheduled expirations.
        if (
            expire_heap_len > _MIN_SCHEDULED_RECORD_EXPIRATION
            and expire_heap_len > len(self._expirations) * 2
        ):
            # Remove any expired entries from the expiration heap
            # that do not match the expiration time in the expirations
            # as it means the record has been re-added to the heap
            # with a different expiration time.
            self._expire_heap = [
                entry for entry in self._expire_heap if self._expirations.get(entry[1]) == entry[0]
            ]
            heapify(self._expire_heap)

        self.async_remove_records(expired)
        return expired

    def async_get_unique(self, entry: _UniqueRecordsType) -> DNSRecord | None:
        """Gets a unique entry by key.  Will return None if there is no
        matching entry.

        This function is not threadsafe and must be called from
        the event loop.
        """
        store = self.cache.get(entry.key)
        if store is None:
            return None
        return store.get(entry)

    def async_all_by_details(self, name: _str, type_: _int, class_: _int) -> list[DNSRecord]:
        """Gets all matching entries by details.

        This function is not thread-safe and must be called from
        the event loop.
        """
        key = name.lower()
        records = self.cache.get(key)
        matches: list[DNSRecord] = []
        if records is None:
            return matches
        for record in records.values():
            if type_ == record.type and class_ == record.class_:
                matches.append(record)
        return matches

    def async_entries_with_name(self, name: str) -> list[DNSRecord]:
        """Returns a dict of entries whose key matches the name.

        This function is not threadsafe and must be called from
        the event loop.
        """
        return self.entries_with_name(name)

    def async_entries_with_server(self, name: str) -> list[DNSRecord]:
        """Returns a dict of entries whose key matches the server.

        This function is not threadsafe and must be called from
        the event loop.
        """
        return self.entries_with_server(name)

    # The below functions are threadsafe and do not need to be run in the
    # event loop, however they all make copies so they significantly
    # inefficient.

    def get(self, entry: DNSEntry) -> DNSRecord | None:
        """Gets an entry by key.  Will return None if there is no
        matching entry."""
        if isinstance(entry, _UNIQUE_RECORD_TYPES):
            return self.cache.get(entry.key, {}).get(entry)
        for cached_entry in reversed(list(self.cache.get(entry.key, {}).values())):
            if entry.__eq__(cached_entry):
                return cached_entry
        return None

    def get_by_details(self, name: str, type_: _int, class_: _int) -> DNSRecord | None:
        """Gets the first matching entry by details. Returns None if no entries match.

        Calling this function is not recommended as it will only
        return one record even if there are multiple entries.

        For example if there are multiple A or AAAA addresses this
        function will return the last one that was added to the cache
        which may not be the one you expect.

        Use get_all_by_details instead.
        """
        key = name.lower()
        records = self.cache.get(key)
        if records is None:
            return None
        for cached_entry in reversed(list(records.values())):
            if type_ == cached_entry.type and class_ == cached_entry.class_:
                return cached_entry
        return None

    def get_all_by_details(self, name: str, type_: _int, class_: _int) -> list[DNSRecord]:
        """Gets all matching entries by details."""
        key = name.lower()
        records = self.cache.get(key)
        if records is None:
            return []
        return [entry for entry in list(records.values()) if type_ == entry.type and class_ == entry.class_]

    def entries_with_server(self, server: str) -> list[DNSRecord]:
        """Returns a list of entries whose server matches the name."""
        if entries := self.service_cache.get(server.lower()):
            return list(entries.values())
        return []

    def entries_with_name(self, name: str) -> list[DNSRecord]:
        """Returns a list of entries whose key matches the name."""
        if entries := self.cache.get(name.lower()):
            return list(entries.values())
        return []

    def current_entry_with_name_and_alias(self, name: str, alias: str) -> DNSRecord | None:
        now = current_time_millis()
        for record in reversed(self.entries_with_name(name)):
            if (
                record.type == _TYPE_PTR
                and not record.is_expired(now)
                and cast(DNSPointer, record).alias == alias
            ):
                return record
        return None

    def names(self) -> list[str]:
        """Return a copy of the list of current cache names."""
        return list(self.cache)

    def async_mark_unique_records_older_than_1s_to_expire(
        self,
        unique_types: set[tuple[_str, _int, _int]],
        answers: Iterable[DNSRecord],
        now: _float,
    ) -> None:
        # rfc6762#section-10.2 para 2
        # Since unique is set, all old records with that name, rrtype,
        # and rrclass that were received more than one second ago are declared
        # invalid, and marked to expire from the cache in one second.
        answers_rrset = set(answers)
        for name, type_, class_ in unique_types:
            for record in self.async_all_by_details(name, type_, class_):
                created_double = record.created
                if (now - created_double > _ONE_SECOND) and record not in answers_rrset:
                    # Expire in 1s
                    self._async_set_created_ttl(record, now, 1)

    def _async_set_created_ttl(self, record: DNSRecord, now: _float, ttl: _int) -> None:
        """Set the created time and ttl of a record."""
        # It would be better if we made a copy instead of mutating the record
        # in place, but records currently don't have a copy method.
        record._set_created_ttl(now, ttl)
        self._async_add(record)
