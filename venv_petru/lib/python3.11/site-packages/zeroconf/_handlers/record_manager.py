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

from typing import TYPE_CHECKING, cast

from .._cache import _UniqueRecordsType
from .._dns import DNSQuestion, DNSRecord
from .._logger import log
from .._protocol.incoming import DNSIncoming
from .._record_update import RecordUpdate
from .._updates import RecordUpdateListener
from .._utils.time import current_time_millis
from ..const import _ADDRESS_RECORD_TYPES, _DNS_PTR_MIN_TTL, _TYPE_PTR

if TYPE_CHECKING:
    from .._core import Zeroconf

_float = float


class RecordManager:
    """Process records into the cache and notify listeners."""

    __slots__ = ("cache", "listeners", "zc")

    def __init__(self, zeroconf: Zeroconf) -> None:
        """Init the record manager."""
        self.zc = zeroconf
        self.cache = zeroconf.cache
        self.listeners: set[RecordUpdateListener] = set()

    def async_updates(self, now: _float, records: list[RecordUpdate]) -> None:
        """Used to notify listeners of new information that has updated
        a record.

        This method must be called before the cache is updated.

        This method will be run in the event loop.
        """
        for listener in self.listeners.copy():
            listener.async_update_records(self.zc, now, records)

    def async_updates_complete(self, notify: bool) -> None:
        """Used to notify listeners of new information that has updated
        a record.

        This method must be called after the cache is updated.

        This method will be run in the event loop.
        """
        for listener in self.listeners.copy():
            listener.async_update_records_complete()
        if notify:
            self.zc.async_notify_all()

    def async_updates_from_response(self, msg: DNSIncoming) -> None:
        """Deal with incoming response packets.  All answers
        are held in the cache, and listeners are notified.

        This function must be run in the event loop as it is not
        threadsafe.
        """
        updates: list[RecordUpdate] = []
        address_adds: list[DNSRecord] = []
        other_adds: list[DNSRecord] = []
        removes: set[DNSRecord] = set()
        now = msg.now
        unique_types: set[tuple[str, int, int]] = set()
        cache = self.cache
        answers = msg.answers()

        for record in answers:
            # Protect zeroconf from records that can cause denial of service.
            #
            # We enforce a minimum TTL for PTR records to avoid
            # ServiceBrowsers generating excessive queries refresh queries.
            # Apple uses a 15s minimum TTL, however we do not have the same
            # level of rate limit and safe guards so we use 1/4 of the recommended value.
            record_type = record.type
            record_ttl = record.ttl
            if record_ttl and record_type == _TYPE_PTR and record_ttl < _DNS_PTR_MIN_TTL:
                log.debug(
                    "Increasing effective ttl of %s to minimum of %s to protect against excessive refreshes.",
                    record,
                    _DNS_PTR_MIN_TTL,
                )
                # Safe because the record is never in the cache yet
                record._set_created_ttl(record.created, _DNS_PTR_MIN_TTL)

            if record.unique:  # https://tools.ietf.org/html/rfc6762#section-10.2
                unique_types.add((record.name, record_type, record.class_))

            if TYPE_CHECKING:
                record = cast(_UniqueRecordsType, record)

            maybe_entry = cache.async_get_unique(record)
            if not record.is_expired(now):
                if record_type in _ADDRESS_RECORD_TYPES:
                    address_adds.append(record)
                else:
                    other_adds.append(record)
                rec_update = RecordUpdate.__new__(RecordUpdate)
                rec_update._fast_init(record, maybe_entry)
                updates.append(rec_update)
            # This is likely a goodbye since the record is
            # expired and exists in the cache
            elif maybe_entry is not None:
                rec_update = RecordUpdate.__new__(RecordUpdate)
                rec_update._fast_init(record, maybe_entry)
                updates.append(rec_update)
                removes.add(record)

        if unique_types:
            cache.async_mark_unique_records_older_than_1s_to_expire(unique_types, answers, now)

        if updates:
            self.async_updates(now, updates)
        # The cache adds must be processed AFTER we trigger
        # the updates since we compare existing data
        # with the new data and updating the cache
        # ahead of update_record will cause listeners
        # to miss changes
        #
        # We must process address adds before non-addresses
        # otherwise a fetch of ServiceInfo may miss an address
        # because it thinks the cache is complete
        #
        # The cache is processed under the context manager to ensure
        # that any ServiceBrowser that is going to call
        # zc.get_service_info will see the cached value
        # but ONLY after all the record updates have been
        # processed.
        new = False
        if other_adds or address_adds:
            new = cache.async_add_records(address_adds)
            if cache.async_add_records(other_adds):
                new = True
        # Removes are processed last since
        # ServiceInfo could generate an un-needed query
        # because the data was not yet populated.
        if removes:
            cache.async_remove_records(removes)
        if updates:
            self.async_updates_complete(new)

    def async_add_listener(
        self,
        listener: RecordUpdateListener,
        question: DNSQuestion | list[DNSQuestion] | None,
    ) -> None:
        """Adds a listener for a given question.  The listener will have
        its update_record method called when information is available to
        answer the question(s).

        This function is not thread-safe and must be called in the eventloop.
        """
        if not isinstance(listener, RecordUpdateListener):
            log.error(  # type: ignore[unreachable]
                "listeners passed to async_add_listener must inherit from RecordUpdateListener;"
                " In the future this will fail"
            )

        self.listeners.add(listener)

        if question is None:
            return

        questions = [question] if isinstance(question, DNSQuestion) else question
        self._async_update_matching_records(listener, questions)

    def _async_update_matching_records(
        self, listener: RecordUpdateListener, questions: list[DNSQuestion]
    ) -> None:
        """Calls back any existing entries in the cache that answer the question.

        This function must be run from the event loop.
        """
        now = current_time_millis()
        records: list[RecordUpdate] = [
            RecordUpdate(record, None)
            for question in questions
            for record in self.cache.async_entries_with_name(question.name)
            if not record.is_expired(now) and question.answered_by(record)
        ]
        if not records:
            return
        listener.async_update_records(self.zc, now, records)
        listener.async_update_records_complete()
        self.zc.async_notify_all()

    def async_remove_listener(self, listener: RecordUpdateListener) -> None:
        """Removes a listener.

        This function is not threadsafe and must be called in the eventloop.
        """
        try:
            self.listeners.remove(listener)
            self.zc.async_notify_all()
        except ValueError as e:
            log.exception("Failed to remove listener: %r", e)
