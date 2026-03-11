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
import socket
from typing import TYPE_CHECKING, Any, cast

from ._exceptions import AbstractMethodException
from ._utils.net import _is_v6_address
from ._utils.time import current_time_millis
from .const import _CLASS_MASK, _CLASS_UNIQUE, _CLASSES, _TYPE_ANY, _TYPES

_LEN_BYTE = 1
_LEN_SHORT = 2
_LEN_INT = 4

_BASE_MAX_SIZE = _LEN_SHORT + _LEN_SHORT + _LEN_INT + _LEN_SHORT  # type  # class  # ttl  # length
_NAME_COMPRESSION_MIN_SIZE = _LEN_BYTE * 2

_EXPIRE_FULL_TIME_MS = 1000
_EXPIRE_STALE_TIME_MS = 500
_RECENT_TIME_MS = 250

_float = float
_int = int

if TYPE_CHECKING:
    from ._protocol.incoming import DNSIncoming
    from ._protocol.outgoing import DNSOutgoing


@enum.unique
class DNSQuestionType(enum.Enum):
    """An MDNS question type.

    "QU" - questions requesting unicast responses
    "QM" - questions requesting multicast responses
    https://datatracker.ietf.org/doc/html/rfc6762#section-5.4
    """

    QU = 1
    QM = 2


class DNSEntry:  # noqa: PLW1641
    """A DNS entry"""

    __slots__ = ("class_", "key", "name", "type", "unique")

    def __init__(self, name: str, type_: int, class_: int) -> None:
        self._fast_init_entry(name, type_, class_)

    def _fast_init_entry(self, name: str, type_: _int, class_: _int) -> None:
        """Fast init for reuse."""
        self.name = name
        self.key = name.lower()
        self.type = type_
        self.class_ = class_ & _CLASS_MASK
        self.unique = (class_ & _CLASS_UNIQUE) != 0

    def _dns_entry_matches(self, other: DNSEntry) -> bool:
        return self.key == other.key and self.type == other.type and self.class_ == other.class_

    def __eq__(self, other: Any) -> bool:
        """Equality test on key (lowercase name), type, and class"""
        return isinstance(other, DNSEntry) and self._dns_entry_matches(other)

    @staticmethod
    def get_class_(class_: int) -> str:
        """Class accessor"""
        return _CLASSES.get(class_, f"?({class_})")

    @staticmethod
    def get_type(t: int) -> str:
        """Type accessor"""
        return _TYPES.get(t, f"?({t})")

    def entry_to_string(self, hdr: str, other: bytes | str | None) -> str:
        """String representation with additional information"""
        return "{}[{},{}{},{}]{}".format(
            hdr,
            self.get_type(self.type),
            self.get_class_(self.class_),
            "-unique" if self.unique else "",
            self.name,
            f"={cast(Any, other)}" if other is not None else "",
        )


class DNSQuestion(DNSEntry):
    """A DNS question entry"""

    __slots__ = ("_hash",)

    def __init__(self, name: str, type_: int, class_: int) -> None:
        self._fast_init(name, type_, class_)

    def _fast_init(self, name: str, type_: _int, class_: _int) -> None:
        """Fast init for reuse."""
        self._fast_init_entry(name, type_, class_)
        self._hash = hash((self.key, type_, self.class_))

    def answered_by(self, rec: DNSRecord) -> bool:
        """Returns true if the question is answered by the record"""
        return self.class_ == rec.class_ and self.type in (rec.type, _TYPE_ANY) and self.name == rec.name

    def __hash__(self) -> int:
        return self._hash

    def __eq__(self, other: Any) -> bool:
        """Tests equality on dns question."""
        return isinstance(other, DNSQuestion) and self._dns_entry_matches(other)

    @property
    def max_size(self) -> int:
        """Maximum size of the question in the packet."""
        return len(self.name.encode("utf-8")) + _LEN_BYTE + _LEN_SHORT + _LEN_SHORT

    @property
    def unicast(self) -> bool:
        """Returns true if the QU (not QM) is set.

        unique shares the same mask as the one
        used for unicast.
        """
        return self.unique

    @unicast.setter
    def unicast(self, value: bool) -> None:
        """Sets the QU bit (not QM)."""
        self.unique = value

    def __repr__(self) -> str:
        """String representation"""
        return "{}[question,{},{},{}]".format(
            self.get_type(self.type),
            "QU" if self.unicast else "QM",
            self.get_class_(self.class_),
            self.name,
        )


class DNSRecord(DNSEntry):  # noqa: PLW1641
    """A DNS record - like a DNS entry, but has a TTL"""

    __slots__ = ("created", "ttl")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: _int,
        created: float | None = None,
    ) -> None:
        self._fast_init_record(name, type_, class_, ttl, created or current_time_millis())

    def _fast_init_record(self, name: str, type_: _int, class_: _int, ttl: _int, created: _float) -> None:
        """Fast init for reuse."""
        self._fast_init_entry(name, type_, class_)
        self.ttl = ttl
        self.created = created

    def __eq__(self, other: Any) -> bool:  # pylint: disable=no-self-use
        """Abstract method"""
        raise AbstractMethodException

    def __lt__(self, other: DNSRecord) -> bool:
        return self.ttl < other.ttl

    def suppressed_by(self, msg: DNSIncoming) -> bool:
        """Returns true if any answer in a message can suffice for the
        information held in this record."""
        answers = msg.answers()
        for record in answers:
            if self._suppressed_by_answer(record):
                return True
        return False

    def _suppressed_by_answer(self, other: DNSRecord) -> bool:
        """Returns true if another record has same name, type and class,
        and if its TTL is at least half of this record's."""
        return self == other and other.ttl > (self.ttl / 2)

    def get_expiration_time(self, percent: _int) -> float:
        """Returns the time at which this record will have expired
        by a certain percentage."""
        return self.created + (percent * self.ttl * 10)

    # TODO: Switch to just int here
    def get_remaining_ttl(self, now: _float) -> int | float:
        """Returns the remaining TTL in seconds."""
        remain = (self.created + (_EXPIRE_FULL_TIME_MS * self.ttl) - now) / 1000.0
        return 0 if remain < 0 else remain

    def is_expired(self, now: _float) -> bool:
        """Returns true if this record has expired."""
        return self.created + (_EXPIRE_FULL_TIME_MS * self.ttl) <= now

    def is_stale(self, now: _float) -> bool:
        """Returns true if this record is at least half way expired."""
        return self.created + (_EXPIRE_STALE_TIME_MS * self.ttl) <= now

    def is_recent(self, now: _float) -> bool:
        """Returns true if the record more than one quarter of its TTL remaining."""
        return self.created + (_RECENT_TIME_MS * self.ttl) > now

    def _set_created_ttl(self, created: _float, ttl: _int) -> None:
        """Set the created and ttl of a record."""
        # It would be better if we made a copy instead of mutating the record
        # in place, but records currently don't have a copy method.
        self.created = created
        self.ttl = ttl

    def write(self, out: DNSOutgoing) -> None:  # pylint: disable=no-self-use
        """Abstract method"""
        raise AbstractMethodException

    def to_string(self, other: bytes | str) -> str:
        """String representation with additional information"""
        arg = f"{self.ttl}/{int(self.get_remaining_ttl(current_time_millis()))},{cast(Any, other)}"
        return DNSEntry.entry_to_string(self, "record", arg)


class DNSAddress(DNSRecord):
    """A DNS address record"""

    __slots__ = ("_hash", "address", "scope_id")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: int,
        address: bytes,
        scope_id: int | None = None,
        created: float | None = None,
    ) -> None:
        self._fast_init(name, type_, class_, ttl, address, scope_id, created or current_time_millis())

    def _fast_init(
        self,
        name: str,
        type_: _int,
        class_: _int,
        ttl: _int,
        address: bytes,
        scope_id: _int | None,
        created: _float,
    ) -> None:
        """Fast init for reuse."""
        self._fast_init_record(name, type_, class_, ttl, created)
        self.address = address
        self.scope_id = scope_id
        self._hash = hash((self.key, type_, self.class_, address, scope_id))

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet"""
        out.write_string(self.address)

    def __eq__(self, other: Any) -> bool:
        """Tests equality on address"""
        return isinstance(other, DNSAddress) and self._eq(other)

    def _eq(self, other: DNSAddress) -> bool:
        return (
            self.address == other.address
            and self.scope_id == other.scope_id
            and self._dns_entry_matches(other)
        )

    def __hash__(self) -> int:
        """Hash to compare like DNSAddresses."""
        return self._hash

    def __repr__(self) -> str:
        """String representation"""
        try:
            return self.to_string(
                socket.inet_ntop(
                    socket.AF_INET6 if _is_v6_address(self.address) else socket.AF_INET,
                    self.address,
                )
            )
        except (ValueError, OSError):
            return self.to_string(str(self.address))


class DNSHinfo(DNSRecord):
    """A DNS host information record"""

    __slots__ = ("_hash", "cpu", "os")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: int,
        cpu: str,
        os: str,
        created: float | None = None,
    ) -> None:
        self._fast_init(name, type_, class_, ttl, cpu, os, created or current_time_millis())

    def _fast_init(
        self, name: str, type_: _int, class_: _int, ttl: _int, cpu: str, os: str, created: _float
    ) -> None:
        """Fast init for reuse."""
        self._fast_init_record(name, type_, class_, ttl, created)
        self.cpu = cpu
        self.os = os
        self._hash = hash((self.key, type_, self.class_, cpu, os))

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet"""
        out.write_character_string(self.cpu.encode("utf-8"))
        out.write_character_string(self.os.encode("utf-8"))

    def __eq__(self, other: Any) -> bool:
        """Tests equality on cpu and os."""
        return isinstance(other, DNSHinfo) and self._eq(other)

    def _eq(self, other: DNSHinfo) -> bool:
        """Tests equality on cpu and os."""
        return self.cpu == other.cpu and self.os == other.os and self._dns_entry_matches(other)

    def __hash__(self) -> int:
        """Hash to compare like DNSHinfo."""
        return self._hash

    def __repr__(self) -> str:
        """String representation"""
        return self.to_string(self.cpu + " " + self.os)


class DNSPointer(DNSRecord):
    """A DNS pointer record"""

    __slots__ = ("_hash", "alias", "alias_key")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: int,
        alias: str,
        created: float | None = None,
    ) -> None:
        self._fast_init(name, type_, class_, ttl, alias, created or current_time_millis())

    def _fast_init(
        self, name: str, type_: _int, class_: _int, ttl: _int, alias: str, created: _float
    ) -> None:
        self._fast_init_record(name, type_, class_, ttl, created)
        self.alias = alias
        self.alias_key = alias.lower()
        self._hash = hash((self.key, type_, self.class_, self.alias_key))

    @property
    def max_size_compressed(self) -> int:
        """Maximum size of the record in the packet assuming the name has been compressed."""
        return (
            _BASE_MAX_SIZE
            + _NAME_COMPRESSION_MIN_SIZE
            + (len(self.alias) - len(self.name))
            + _NAME_COMPRESSION_MIN_SIZE
        )

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet"""
        out.write_name(self.alias)

    def __eq__(self, other: Any) -> bool:
        """Tests equality on alias."""
        return isinstance(other, DNSPointer) and self._eq(other)

    def _eq(self, other: DNSPointer) -> bool:
        """Tests equality on alias."""
        return self.alias_key == other.alias_key and self._dns_entry_matches(other)

    def __hash__(self) -> int:
        """Hash to compare like DNSPointer."""
        return self._hash

    def __repr__(self) -> str:
        """String representation"""
        return self.to_string(self.alias)


class DNSText(DNSRecord):
    """A DNS text record"""

    __slots__ = ("_hash", "text")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: int,
        text: bytes,
        created: float | None = None,
    ) -> None:
        self._fast_init(name, type_, class_, ttl, text, created or current_time_millis())

    def _fast_init(
        self, name: str, type_: _int, class_: _int, ttl: _int, text: bytes, created: _float
    ) -> None:
        self._fast_init_record(name, type_, class_, ttl, created)
        self.text = text
        self._hash = hash((self.key, type_, self.class_, text))

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet"""
        out.write_string(self.text)

    def __hash__(self) -> int:
        """Hash to compare like DNSText."""
        return self._hash

    def __eq__(self, other: Any) -> bool:
        """Tests equality on text."""
        return isinstance(other, DNSText) and self._eq(other)

    def _eq(self, other: DNSText) -> bool:
        """Tests equality on text."""
        return self.text == other.text and self._dns_entry_matches(other)

    def __repr__(self) -> str:
        """String representation"""
        if len(self.text) > 10:
            return self.to_string(self.text[:7]) + "..."
        return self.to_string(self.text)


class DNSService(DNSRecord):
    """A DNS service record"""

    __slots__ = ("_hash", "port", "priority", "server", "server_key", "weight")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: int,
        priority: int,
        weight: int,
        port: int,
        server: str,
        created: float | None = None,
    ) -> None:
        self._fast_init(
            name, type_, class_, ttl, priority, weight, port, server, created or current_time_millis()
        )

    def _fast_init(
        self,
        name: str,
        type_: _int,
        class_: _int,
        ttl: _int,
        priority: _int,
        weight: _int,
        port: _int,
        server: str,
        created: _float,
    ) -> None:
        self._fast_init_record(name, type_, class_, ttl, created)
        self.priority = priority
        self.weight = weight
        self.port = port
        self.server = server
        self.server_key = server.lower()
        self._hash = hash((self.key, type_, self.class_, priority, weight, port, self.server_key))

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet"""
        out.write_short(self.priority)
        out.write_short(self.weight)
        out.write_short(self.port)
        out.write_name(self.server)

    def __eq__(self, other: Any) -> bool:
        """Tests equality on priority, weight, port and server"""
        return isinstance(other, DNSService) and self._eq(other)

    def _eq(self, other: DNSService) -> bool:
        """Tests equality on priority, weight, port and server."""
        return (
            self.priority == other.priority
            and self.weight == other.weight
            and self.port == other.port
            and self.server_key == other.server_key
            and self._dns_entry_matches(other)
        )

    def __hash__(self) -> int:
        """Hash to compare like DNSService."""
        return self._hash

    def __repr__(self) -> str:
        """String representation"""
        return self.to_string(f"{self.server}:{self.port}")


class DNSNsec(DNSRecord):
    """A DNS NSEC record"""

    __slots__ = ("_hash", "next_name", "rdtypes")

    def __init__(
        self,
        name: str,
        type_: int,
        class_: int,
        ttl: _int,
        next_name: str,
        rdtypes: list[int],
        created: float | None = None,
    ) -> None:
        self._fast_init(name, type_, class_, ttl, next_name, rdtypes, created or current_time_millis())

    def _fast_init(
        self,
        name: str,
        type_: _int,
        class_: _int,
        ttl: _int,
        next_name: str,
        rdtypes: list[_int],
        created: _float,
    ) -> None:
        self._fast_init_record(name, type_, class_, ttl, created)
        self.next_name = next_name
        self.rdtypes = sorted(rdtypes)
        self._hash = hash((self.key, type_, self.class_, next_name, *self.rdtypes))

    def write(self, out: DNSOutgoing) -> None:
        """Used in constructing an outgoing packet."""
        bitmap = bytearray(b"\0" * 32)
        total_octets = 0
        for rdtype in self.rdtypes:
            if rdtype > 255:  # mDNS only supports window 0
                raise ValueError(f"rdtype {rdtype} is too large for NSEC")
            byte = rdtype // 8
            total_octets = byte + 1
            bitmap[byte] |= 0x80 >> (rdtype % 8)
        if total_octets == 0:
            # NSEC must have at least one rdtype
            # Writing an empty bitmap is not allowed
            raise ValueError("NSEC must have at least one rdtype")
        out_bytes = bytes(bitmap[0:total_octets])
        out.write_name(self.next_name)
        out._write_byte(0)  # Always window 0
        out._write_byte(len(out_bytes))
        out.write_string(out_bytes)

    def __eq__(self, other: Any) -> bool:
        """Tests equality on next_name and rdtypes."""
        return isinstance(other, DNSNsec) and self._eq(other)

    def _eq(self, other: DNSNsec) -> bool:
        """Tests equality on next_name and rdtypes."""
        return (
            self.next_name == other.next_name
            and self.rdtypes == other.rdtypes
            and self._dns_entry_matches(other)
        )

    def __hash__(self) -> int:
        """Hash to compare like DNSNSec."""
        return self._hash

    def __repr__(self) -> str:
        """String representation"""
        return self.to_string(
            self.next_name + "," + "|".join([self.get_type(type_) for type_ in self.rdtypes])
        )


_DNSRecord = DNSRecord


class DNSRRSet:
    """A set of dns records with a lookup to get the ttl."""

    __slots__ = ("_lookup", "_records")

    def __init__(self, records: list[DNSRecord]) -> None:
        """Create an RRset from records sets."""
        self._records = records
        self._lookup: dict[DNSRecord, DNSRecord] | None = None

    @property
    def lookup(self) -> dict[DNSRecord, DNSRecord]:
        """Return the lookup table."""
        return self._get_lookup()

    def lookup_set(self) -> set[DNSRecord]:
        """Return the lookup table as aset."""
        return set(self._get_lookup())

    def _get_lookup(self) -> dict[DNSRecord, DNSRecord]:
        """Return the lookup table, building it if needed."""
        if self._lookup is None:
            # Build the hash table so we can lookup the record ttl
            self._lookup = {record: record for record in self._records}
        return self._lookup

    def suppresses(self, record: _DNSRecord) -> bool:
        """Returns true if any answer in the rrset can suffice for the
        information held in this record."""
        lookup = self._get_lookup()
        other = lookup.get(record)
        if other is None:
            return False
        return other.ttl > (record.ttl / 2)
