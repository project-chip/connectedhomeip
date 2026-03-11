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
import logging
from collections.abc import Sequence
from struct import Struct
from typing import TYPE_CHECKING

from .._dns import DNSPointer, DNSQuestion, DNSRecord
from .._exceptions import NamePartTooLongException
from .._logger import log
from ..const import (
    _CLASS_UNIQUE,
    _DNS_HOST_TTL,
    _DNS_OTHER_TTL,
    _DNS_PACKET_HEADER_LEN,
    _FLAGS_QR_MASK,
    _FLAGS_QR_QUERY,
    _FLAGS_QR_RESPONSE,
    _FLAGS_TC,
    _MAX_MSG_ABSOLUTE,
    _MAX_MSG_TYPICAL,
)
from .incoming import DNSIncoming

str_ = str
float_ = float
int_ = int
bytes_ = bytes
DNSQuestion_ = DNSQuestion
DNSRecord_ = DNSRecord


PACK_BYTE = Struct(">B").pack
PACK_SHORT = Struct(">H").pack
PACK_LONG = Struct(">L").pack

SHORT_CACHE_MAX = 128

BYTE_TABLE = tuple(PACK_BYTE(i) for i in range(256))
SHORT_LOOKUP = tuple(PACK_SHORT(i) for i in range(SHORT_CACHE_MAX))
LONG_LOOKUP = {i: PACK_LONG(i) for i in (_DNS_OTHER_TTL, _DNS_HOST_TTL, 0)}


class State(enum.Enum):
    init = 0
    finished = 1


STATE_INIT = State.init.value
STATE_FINISHED = State.finished.value

LOGGING_IS_ENABLED_FOR = log.isEnabledFor
LOGGING_DEBUG = logging.DEBUG


class DNSOutgoing:
    """Object representation of an outgoing packet"""

    __slots__ = (
        "additionals",
        "allow_long",
        "answers",
        "authorities",
        "data",
        "finished",
        "flags",
        "id",
        "multicast",
        "names",
        "packets_data",
        "questions",
        "size",
        "state",
    )

    def __init__(self, flags: int, multicast: bool = True, id_: int = 0) -> None:
        self.flags = flags
        self.finished = False
        self.id = id_
        self.multicast = multicast
        self.packets_data: list[bytes] = []

        # these 3 are per-packet -- see also _reset_for_next_packet()
        self.names: dict[str, int] = {}
        self.data: list[bytes] = []
        self.size: int = _DNS_PACKET_HEADER_LEN
        self.allow_long: bool = True

        self.state = STATE_INIT

        self.questions: list[DNSQuestion] = []
        self.answers: list[tuple[DNSRecord, float]] = []
        self.authorities: list[DNSPointer] = []
        self.additionals: list[DNSRecord] = []

    def is_query(self) -> bool:
        """Returns true if this is a query."""
        return (self.flags & _FLAGS_QR_MASK) == _FLAGS_QR_QUERY

    def is_response(self) -> bool:
        """Returns true if this is a response."""
        return (self.flags & _FLAGS_QR_MASK) == _FLAGS_QR_RESPONSE

    def _reset_for_next_packet(self) -> None:
        self.names = {}
        self.data = []
        self.size = _DNS_PACKET_HEADER_LEN
        self.allow_long = True

    def __repr__(self) -> str:
        return "<DNSOutgoing:{}>".format(
            ", ".join(
                [
                    f"multicast={self.multicast}",
                    f"flags={self.flags}",
                    f"questions={self.questions}",
                    f"answers={self.answers}",
                    f"authorities={self.authorities}",
                    f"additionals={self.additionals}",
                ]
            )
        )

    def add_question(self, record: DNSQuestion) -> None:
        """Adds a question"""
        self.questions.append(record)

    def add_answer(self, inp: DNSIncoming, record: DNSRecord) -> None:
        """Adds an answer"""
        if not record.suppressed_by(inp):
            self.add_answer_at_time(record, 0.0)

    def add_answer_at_time(self, record: DNSRecord | None, now: float_) -> None:
        """Adds an answer if it does not expire by a certain time"""
        now_double = now
        if record is not None and (now_double == 0 or not record.is_expired(now_double)):
            self.answers.append((record, now))

    def add_authorative_answer(self, record: DNSPointer) -> None:
        """Adds an authoritative answer"""
        self.authorities.append(record)

    def add_additional_answer(self, record: DNSRecord) -> None:
        """Adds an additional answer

        From: RFC 6763, DNS-Based Service Discovery, February 2013

        12.  DNS Additional Record Generation

           DNS has an efficiency feature whereby a DNS server may place
           additional records in the additional section of the DNS message.
           These additional records are records that the client did not
           explicitly request, but the server has reasonable grounds to expect
           that the client might request them shortly, so including them can
           save the client from having to issue additional queries.

           This section recommends which additional records SHOULD be generated
           to improve network efficiency, for both Unicast and Multicast DNS-SD
           responses.

        12.1.  PTR Records

           When including a DNS-SD Service Instance Enumeration or Selective
           Instance Enumeration (subtype) PTR record in a response packet, the
           server/responder SHOULD include the following additional records:

           o  The SRV record(s) named in the PTR rdata.
           o  The TXT record(s) named in the PTR rdata.
           o  All address records (type "A" and "AAAA") named in the SRV rdata.

        12.2.  SRV Records

           When including an SRV record in a response packet, the
           server/responder SHOULD include the following additional records:

           o  All address records (type "A" and "AAAA") named in the SRV rdata.

        """
        self.additionals.append(record)

    def _write_byte(self, value: int_) -> None:
        """Writes a single byte to the packet"""
        self.data.append(BYTE_TABLE[value])
        self.size += 1

    def _get_short(self, value: int_) -> bytes:
        """Convert an unsigned short to 2 bytes."""
        return SHORT_LOOKUP[value] if value < SHORT_CACHE_MAX else PACK_SHORT(value)

    def _insert_short_at_start(self, value: int_) -> None:
        """Inserts an unsigned short at the start of the packet"""
        self.data.insert(0, self._get_short(value))

    def _replace_short(self, index: int_, value: int_) -> None:
        """Replaces an unsigned short in a certain position in the packet"""
        self.data[index] = self._get_short(value)

    def write_short(self, value: int_) -> None:
        """Writes an unsigned short to the packet"""
        self.data.append(self._get_short(value))
        self.size += 2

    def _write_int(self, value: float | int) -> None:
        """Writes an unsigned integer to the packet"""
        value_as_int = int(value)
        long_bytes = LONG_LOOKUP.get(value_as_int)
        if long_bytes is not None:
            self.data.append(long_bytes)
        else:
            self.data.append(PACK_LONG(value_as_int))
        self.size += 4

    def write_string(self, value: bytes_) -> None:
        """Writes a string to the packet"""
        if TYPE_CHECKING:
            assert isinstance(value, bytes)
        self.data.append(value)
        self.size += len(value)

    def _write_utf(self, s: str_) -> None:
        """Writes a UTF-8 string of a given length to the packet"""
        utfstr = s.encode("utf-8")
        length = len(utfstr)
        if length > 64:
            raise NamePartTooLongException
        self._write_byte(length)
        self.write_string(utfstr)

    def write_character_string(self, value: bytes) -> None:
        if TYPE_CHECKING:
            assert isinstance(value, bytes)
        length = len(value)
        if length > 256:
            raise NamePartTooLongException
        self._write_byte(length)
        self.write_string(value)

    def write_name(self, name: str_) -> None:
        """
        Write names to packet

        18.14. Name Compression

        When generating Multicast DNS messages, implementations SHOULD use
        name compression wherever possible to compress the names of resource
        records, by replacing some or all of the resource record name with a
        compact two-byte reference to an appearance of that data somewhere
        earlier in the message [RFC1035].
        """

        # split name into each label
        if name and name[-1] == ".":
            name = name[:-1]

        index = self.names.get(name, 0)
        if index:
            self._write_link_to_name(index)
            return

        start_size = self.size
        labels = name.split(".")
        # Write each new label or a pointer to the existing one in the packet
        self.names[name] = start_size
        self._write_utf(labels[0])

        name_length = 0
        for count in range(1, len(labels)):
            partial_name = ".".join(labels[count:])
            index = self.names.get(partial_name, 0)
            if index:
                self._write_link_to_name(index)
                return
            if name_length == 0:
                name_length = len(name.encode("utf-8"))
            self.names[partial_name] = start_size + name_length - len(partial_name.encode("utf-8"))
            self._write_utf(labels[count])

        # this is the end of a name
        self._write_byte(0)

    def _write_link_to_name(self, index: int_) -> None:
        # If part of the name already exists in the packet,
        # create a pointer to it
        self._write_byte((index >> 8) | 0xC0)
        self._write_byte(index & 0xFF)

    def _write_question(self, question: DNSQuestion_) -> bool:
        """Writes a question to the packet"""
        start_data_length = len(self.data)
        start_size = self.size
        self.write_name(question.name)
        self.write_short(question.type)
        self._write_record_class(question)
        return self._check_data_limit_or_rollback(start_data_length, start_size)

    def _write_record_class(self, record: DNSQuestion_ | DNSRecord_) -> None:
        """Write out the record class including the unique/unicast (QU) bit."""
        class_ = record.class_
        if record.unique is True and self.multicast:
            self.write_short(class_ | _CLASS_UNIQUE)
        else:
            self.write_short(class_)

    def _write_ttl(self, record: DNSRecord_, now: float_) -> None:
        """Write out the record ttl."""
        self._write_int(record.ttl if now == 0 else record.get_remaining_ttl(now))

    def _write_record(self, record: DNSRecord_, now: float_) -> bool:
        """Writes a record (answer, authoritative answer, additional) to
        the packet.  Returns True on success, or False if we did not
        because the packet because the record does not fit."""
        start_data_length = len(self.data)
        start_size = self.size
        self.write_name(record.name)
        self.write_short(record.type)
        self._write_record_class(record)
        self._write_ttl(record, now)
        index = len(self.data)
        self.write_short(0)  # Will get replaced with the actual size
        record.write(self)
        # Adjust size for the short we will write before this record
        length = 0
        for d in self.data[index + 1 :]:
            length += len(d)
        # Here we replace the 0 length short we wrote
        # before with the actual length
        self._replace_short(index, length)
        return self._check_data_limit_or_rollback(start_data_length, start_size)

    def _check_data_limit_or_rollback(self, start_data_length: int_, start_size: int_) -> bool:
        """Check data limit, if we go over, then rollback and return False."""
        len_limit = _MAX_MSG_ABSOLUTE if self.allow_long else _MAX_MSG_TYPICAL
        self.allow_long = False

        if self.size <= len_limit:
            return True

        if LOGGING_IS_ENABLED_FOR(LOGGING_DEBUG):  # pragma: no branch
            log.debug(
                "Reached data limit (size=%d) > (limit=%d) - rolling back",
                self.size,
                len_limit,
            )
        del self.data[start_data_length:]
        self.size = start_size

        start_size_int = start_size
        rollback_names = [name for name, idx in self.names.items() if idx >= start_size_int]
        for name in rollback_names:
            del self.names[name]
        return False

    def _write_questions_from_offset(self, questions_offset: int_) -> int:
        questions_written = 0
        for question in self.questions[questions_offset:]:
            if not self._write_question(question):
                break
            questions_written += 1
        return questions_written

    def _write_answers_from_offset(self, answer_offset: int_) -> int:
        answers_written = 0
        for answer, time_ in self.answers[answer_offset:]:
            if not self._write_record(answer, time_):
                break
            answers_written += 1
        return answers_written

    def _write_records_from_offset(self, records: Sequence[DNSRecord], offset: int_) -> int:
        records_written = 0
        for record in records[offset:]:
            if not self._write_record(record, 0):
                break
            records_written += 1
        return records_written

    def _has_more_to_add(
        self,
        questions_offset: int_,
        answer_offset: int_,
        authority_offset: int_,
        additional_offset: int_,
    ) -> bool:
        """Check if all questions, answers, authority, and additionals have been written to the packet."""
        return (
            questions_offset < len(self.questions)
            or answer_offset < len(self.answers)
            or authority_offset < len(self.authorities)
            or additional_offset < len(self.additionals)
        )

    def packets(self) -> list[bytes]:
        """Returns a list of bytestrings containing the packets' bytes

        No further parts should be added to the packet once this
        is done.  The packets are each restricted to _MAX_MSG_TYPICAL
        or less in length, except for the case of a single answer which
        will be written out to a single oversized packet no more than
        _MAX_MSG_ABSOLUTE in length (and hence will be subject to IP
        fragmentation potentially)."""
        packets_data = self.packets_data

        if self.state == STATE_FINISHED:
            return packets_data

        questions_offset = 0
        answer_offset = 0
        authority_offset = 0
        additional_offset = 0
        # we have to at least write out the question
        debug_enable = LOGGING_IS_ENABLED_FOR(LOGGING_DEBUG) is True
        has_more_to_add = True

        while has_more_to_add:
            if debug_enable:
                log.debug(
                    "offsets = questions=%d, answers=%d, authorities=%d, additionals=%d",
                    questions_offset,
                    answer_offset,
                    authority_offset,
                    additional_offset,
                )
                log.debug(
                    "lengths = questions=%d, answers=%d, authorities=%d, additionals=%d",
                    len(self.questions),
                    len(self.answers),
                    len(self.authorities),
                    len(self.additionals),
                )

            questions_written = self._write_questions_from_offset(questions_offset)
            answers_written = self._write_answers_from_offset(answer_offset)
            authorities_written = self._write_records_from_offset(self.authorities, authority_offset)
            additionals_written = self._write_records_from_offset(self.additionals, additional_offset)

            made_progress = bool(self.data)

            self._insert_short_at_start(additionals_written)
            self._insert_short_at_start(authorities_written)
            self._insert_short_at_start(answers_written)
            self._insert_short_at_start(questions_written)

            questions_offset += questions_written
            answer_offset += answers_written
            authority_offset += authorities_written
            additional_offset += additionals_written
            if debug_enable:
                log.debug(
                    "now offsets = questions=%d, answers=%d, authorities=%d, additionals=%d",
                    questions_offset,
                    answer_offset,
                    authority_offset,
                    additional_offset,
                )

            has_more_to_add = self._has_more_to_add(
                questions_offset, answer_offset, authority_offset, additional_offset
            )

            if has_more_to_add and self.is_query():
                # https://datatracker.ietf.org/doc/html/rfc6762#section-7.2
                if debug_enable:  # pragma: no branch
                    log.debug("Setting TC flag")
                self._insert_short_at_start(self.flags | _FLAGS_TC)
            else:
                self._insert_short_at_start(self.flags)

            if self.multicast:
                self._insert_short_at_start(0)
            else:
                self._insert_short_at_start(self.id)

            packets_data.append(b"".join(self.data))

            if not made_progress:
                # Generating an empty packet is not a desirable outcome, but currently
                # too many internals rely on this behavior.  So, we'll just return an
                # empty packet and log a warning until this can be refactored at a later
                # date.
                log.warning("packets() made no progress adding records; returning")
                break

            if has_more_to_add:
                self._reset_for_next_packet()

        self.state = STATE_FINISHED
        return packets_data
