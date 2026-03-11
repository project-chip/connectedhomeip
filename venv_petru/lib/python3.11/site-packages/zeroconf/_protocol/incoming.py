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

import struct
import sys
from typing import Any

from .._dns import (
    DNSAddress,
    DNSHinfo,
    DNSNsec,
    DNSPointer,
    DNSQuestion,
    DNSRecord,
    DNSService,
    DNSText,
)
from .._exceptions import IncomingDecodeError
from .._logger import log
from .._utils.time import current_time_millis
from ..const import (
    _FLAGS_QR_MASK,
    _FLAGS_QR_QUERY,
    _FLAGS_QR_RESPONSE,
    _FLAGS_TC,
    _TYPE_A,
    _TYPE_AAAA,
    _TYPE_CNAME,
    _TYPE_HINFO,
    _TYPE_NSEC,
    _TYPE_PTR,
    _TYPE_SRV,
    _TYPE_TXT,
    _TYPES,
)

DNS_COMPRESSION_HEADER_LEN = 1
DNS_COMPRESSION_POINTER_LEN = 2
MAX_DNS_LABELS = 128
MAX_NAME_LENGTH = 253

DECODE_EXCEPTIONS = (IndexError, struct.error, IncomingDecodeError)


_seen_logs: dict[str, int | tuple] = {}
_str = str
_int = int


class DNSIncoming:
    """Object representation of an incoming DNS packet"""

    __slots__ = (
        "_answers",
        "_data_len",
        "_did_read_others",
        "_has_qu_question",
        "_name_cache",
        "_num_additionals",
        "_num_answers",
        "_num_authorities",
        "_num_questions",
        "_questions",
        "data",
        "flags",
        "id",
        "now",
        "offset",
        "scope_id",
        "source",
        "valid",
        "view",
    )

    def __init__(
        self,
        data: bytes,
        source: tuple[str, int] | None = None,
        scope_id: int | None = None,
        now: float | None = None,
    ) -> None:
        """Constructor from string holding bytes of packet"""
        self.flags = 0
        self.offset = 0
        self.data = data
        self.view = data
        self._data_len = len(data)
        self._name_cache: dict[int, list[str]] = {}
        self._questions: list[DNSQuestion] = []
        self._answers: list[DNSRecord] = []
        self.id = 0
        self._num_questions = 0
        self._num_answers = 0
        self._num_authorities = 0
        self._num_additionals = 0
        self.valid = False
        self._did_read_others = False
        self.now = now or current_time_millis()
        self.source = source
        self.scope_id = scope_id
        self._has_qu_question = False
        try:
            self._initial_parse()
        except DECODE_EXCEPTIONS:
            self._log_exception_debug(
                "Received invalid packet from %s at offset %d while unpacking %r",
                self.source,
                self.offset,
                self.data,
            )

    def is_query(self) -> bool:
        """Returns true if this is a query."""
        return (self.flags & _FLAGS_QR_MASK) == _FLAGS_QR_QUERY

    def is_response(self) -> bool:
        """Returns true if this is a response."""
        return (self.flags & _FLAGS_QR_MASK) == _FLAGS_QR_RESPONSE

    def has_qu_question(self) -> bool:
        """Returns true if any question is a QU question."""
        return self._has_qu_question

    @property
    def truncated(self) -> bool:
        """Returns true if this is a truncated."""
        return (self.flags & _FLAGS_TC) == _FLAGS_TC

    @property
    def questions(self) -> list[DNSQuestion]:
        """Questions in the packet."""
        return self._questions

    @property
    def num_questions(self) -> int:
        """Number of questions in the packet."""
        return self._num_questions

    @property
    def num_answers(self) -> int:
        """Number of answers in the packet."""
        return self._num_answers

    @property
    def num_authorities(self) -> int:
        """Number of authorities in the packet."""
        return self._num_authorities

    @property
    def num_additionals(self) -> int:
        """Number of additionals in the packet."""
        return self._num_additionals

    def _initial_parse(self) -> None:
        """Parse the data needed to initialize the packet object."""
        self._read_header()
        self._read_questions()
        if not self._num_questions:
            self._read_others()
        self.valid = True

    @classmethod
    def _log_exception_debug(cls, *logger_data: Any) -> None:
        log_exc_info = False
        exc_info = sys.exc_info()
        exc_str = str(exc_info[1])
        if exc_str not in _seen_logs:
            # log the trace only on the first time
            _seen_logs[exc_str] = exc_info
            log_exc_info = True
        log.debug(*(logger_data or ["Exception occurred"]), exc_info=log_exc_info)

    def answers(self) -> list[DNSRecord]:
        """Answers in the packet."""
        if not self._did_read_others:
            try:
                self._read_others()
            except DECODE_EXCEPTIONS:
                self._log_exception_debug(
                    "Received invalid packet from %s at offset %d while unpacking %r",
                    self.source,
                    self.offset,
                    self.data,
                )
        return self._answers

    def is_probe(self) -> bool:
        """Returns true if this is a probe."""
        return self._num_authorities > 0

    def __repr__(self) -> str:
        return "<DNSIncoming:{}>".format(
            ", ".join(
                [
                    f"id={self.id}",
                    f"flags={self.flags}",
                    f"truncated={self.truncated}",
                    f"n_q={self._num_questions}",
                    f"n_ans={self._num_answers}",
                    f"n_auth={self._num_authorities}",
                    f"n_add={self._num_additionals}",
                    f"questions={self._questions}",
                    f"answers={self.answers()}",
                ]
            )
        )

    def _read_header(self) -> None:
        """Reads header portion of packet"""
        view = self.view
        offset = self.offset
        self.offset += 12
        # The header has 6 unsigned shorts in network order
        self.id = view[offset] << 8 | view[offset + 1]
        self.flags = view[offset + 2] << 8 | view[offset + 3]
        self._num_questions = view[offset + 4] << 8 | view[offset + 5]
        self._num_answers = view[offset + 6] << 8 | view[offset + 7]
        self._num_authorities = view[offset + 8] << 8 | view[offset + 9]
        self._num_additionals = view[offset + 10] << 8 | view[offset + 11]

    def _read_questions(self) -> None:
        """Reads questions section of packet"""
        view = self.view
        questions = self._questions
        for _ in range(self._num_questions):
            name = self._read_name()
            offset = self.offset
            self.offset += 4
            # The question has 2 unsigned shorts in network order
            type_ = view[offset] << 8 | view[offset + 1]
            class_ = view[offset + 2] << 8 | view[offset + 3]
            question = DNSQuestion.__new__(DNSQuestion)
            question._fast_init(name, type_, class_)
            if question.unique:  # QU questions use the same bit as unique
                self._has_qu_question = True
            questions.append(question)

    def _read_character_string(self) -> str:
        """Reads a character string from the packet"""
        length = self.view[self.offset]
        self.offset += 1
        info = self.data[self.offset : self.offset + length].decode("utf-8", "replace")
        self.offset += length
        return info

    def _read_string(self, length: _int) -> bytes:
        """Reads a string of a given length from the packet"""
        info = self.data[self.offset : self.offset + length]
        self.offset += length
        return info

    def _read_others(self) -> None:
        """Reads the answers, authorities and additionals section of the
        packet"""
        self._did_read_others = True
        view = self.view
        n = self._num_answers + self._num_authorities + self._num_additionals
        for _ in range(n):
            domain = self._read_name()
            offset = self.offset
            self.offset += 10
            # type_, class_ and length are unsigned shorts in network order
            # ttl is an unsigned long in network order https://www.rfc-editor.org/errata/eid2130
            type_ = view[offset] << 8 | view[offset + 1]
            class_ = view[offset + 2] << 8 | view[offset + 3]
            ttl = view[offset + 4] << 24 | view[offset + 5] << 16 | view[offset + 6] << 8 | view[offset + 7]
            length = view[offset + 8] << 8 | view[offset + 9]
            end = self.offset + length
            rec = None
            try:
                rec = self._read_record(domain, type_, class_, ttl, length)
            except DECODE_EXCEPTIONS:
                # Skip records that fail to decode if we know the length
                # If the packet is really corrupt read_name and the unpack
                # above would fail and hit the exception catch in read_others
                self.offset = end
                log.debug(
                    "Unable to parse; skipping record for %s with type %s at offset %d while unpacking %r",
                    domain,
                    _TYPES.get(type_, type_),
                    self.offset,
                    self.data,
                    exc_info=True,
                )
            if rec is not None:
                self._answers.append(rec)

    def _read_record(
        self, domain: _str, type_: _int, class_: _int, ttl: _int, length: _int
    ) -> DNSRecord | None:
        """Read known records types and skip unknown ones."""
        if type_ == _TYPE_A:
            address_rec = DNSAddress.__new__(DNSAddress)
            address_rec._fast_init(domain, type_, class_, ttl, self._read_string(4), None, self.now)
            return address_rec
        if type_ in (_TYPE_CNAME, _TYPE_PTR):
            pointer_rec = DNSPointer.__new__(DNSPointer)
            pointer_rec._fast_init(domain, type_, class_, ttl, self._read_name(), self.now)
            return pointer_rec
        if type_ == _TYPE_TXT:
            text_rec = DNSText.__new__(DNSText)
            text_rec._fast_init(domain, type_, class_, ttl, self._read_string(length), self.now)
            return text_rec
        if type_ == _TYPE_SRV:
            view = self.view
            offset = self.offset
            self.offset += 6
            # The SRV record has 3 unsigned shorts in network order
            priority = view[offset] << 8 | view[offset + 1]
            weight = view[offset + 2] << 8 | view[offset + 3]
            port = view[offset + 4] << 8 | view[offset + 5]
            srv_rec = DNSService.__new__(DNSService)
            srv_rec._fast_init(
                domain,
                type_,
                class_,
                ttl,
                priority,
                weight,
                port,
                self._read_name(),
                self.now,
            )
            return srv_rec
        if type_ == _TYPE_HINFO:
            hinfo_rec = DNSHinfo.__new__(DNSHinfo)
            hinfo_rec._fast_init(
                domain,
                type_,
                class_,
                ttl,
                self._read_character_string(),
                self._read_character_string(),
                self.now,
            )
            return hinfo_rec
        if type_ == _TYPE_AAAA:
            address_rec = DNSAddress.__new__(DNSAddress)
            address_rec._fast_init(
                domain,
                type_,
                class_,
                ttl,
                self._read_string(16),
                self.scope_id,
                self.now,
            )
            return address_rec
        if type_ == _TYPE_NSEC:
            name_start = self.offset
            nsec_rec = DNSNsec.__new__(DNSNsec)
            nsec_rec._fast_init(
                domain,
                type_,
                class_,
                ttl,
                self._read_name(),
                self._read_bitmap(name_start + length),
                self.now,
            )
            return nsec_rec
        # Try to ignore types we don't know about
        # Skip the payload for the resource record so the next
        # records can be parsed correctly
        self.offset += length
        return None

    def _read_bitmap(self, end: _int) -> list[int]:
        """Reads an NSEC bitmap from the packet."""
        rdtypes = []
        view = self.view
        while self.offset < end:
            offset = self.offset
            offset_plus_one = offset + 1
            offset_plus_two = offset + 2
            window = view[offset]
            bitmap_length = view[offset_plus_one]
            bitmap_end = offset_plus_two + bitmap_length
            for i, byte in enumerate(self.data[offset_plus_two:bitmap_end]):
                for bit in range(8):
                    if byte & (0x80 >> bit):
                        rdtypes.append(bit + window * 256 + i * 8)
            self.offset += 2 + bitmap_length
        return rdtypes

    def _read_name(self) -> str:
        """Reads a domain name from the packet."""
        labels: list[str] = []
        seen_pointers: set[int] = set()
        original_offset = self.offset
        self.offset = self._decode_labels_at_offset(original_offset, labels, seen_pointers)
        self._name_cache[original_offset] = labels
        name = ".".join(labels) + "."
        if len(name) > MAX_NAME_LENGTH:
            raise IncomingDecodeError(
                f"DNS name {name} exceeds maximum length of {MAX_NAME_LENGTH} from {self.source}"
            )
        return name

    def _decode_labels_at_offset(self, off: _int, labels: list[str], seen_pointers: set[int]) -> int:
        # This is a tight loop that is called frequently, small optimizations can make a difference.
        view = self.view
        while off < self._data_len:
            length = view[off]
            if length == 0:
                return off + DNS_COMPRESSION_HEADER_LEN

            if length < 0x40:
                label_idx = off + DNS_COMPRESSION_HEADER_LEN
                labels.append(self.data[label_idx : label_idx + length].decode("utf-8", "replace"))
                off += DNS_COMPRESSION_HEADER_LEN + length
                continue

            if length < 0xC0:
                raise IncomingDecodeError(
                    f"DNS compression type {length} is unknown at {off} from {self.source}"
                )

            # We have a DNS compression pointer
            link_data = view[off + 1]
            link = (length & 0x3F) * 256 + link_data
            link_py_int = link
            if link > self._data_len:
                raise IncomingDecodeError(
                    f"DNS compression pointer at {off} points to {link} beyond packet from {self.source}"
                )
            if link == off:
                raise IncomingDecodeError(
                    f"DNS compression pointer at {off} points to itself from {self.source}"
                )
            if link_py_int in seen_pointers:
                raise IncomingDecodeError(
                    f"DNS compression pointer at {off} was seen again from {self.source}"
                )
            linked_labels = self._name_cache.get(link_py_int)
            if not linked_labels:
                linked_labels = []
                seen_pointers.add(link_py_int)
                self._decode_labels_at_offset(link, linked_labels, seen_pointers)
                self._name_cache[link_py_int] = linked_labels
            labels.extend(linked_labels)
            if len(labels) > MAX_DNS_LABELS:
                raise IncomingDecodeError(
                    f"Maximum dns labels reached while processing pointer at {off} from {self.source}"
                )
            return off + DNS_COMPRESSION_POINTER_LEN

        raise IncomingDecodeError(f"Corrupt packet received while decoding name from {self.source}")
