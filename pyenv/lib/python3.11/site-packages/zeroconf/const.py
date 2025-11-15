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

import re
import socket

# Some timing constants

_UNREGISTER_TIME = 125  # ms
_CHECK_TIME = 500  # ms
_REGISTER_TIME = 225  # ms
_LISTENER_TIME = 200  # ms
_BROWSER_TIME = 10000  # ms
_DUPLICATE_PACKET_SUPPRESSION_INTERVAL = 1000  # ms
_DUPLICATE_QUESTION_INTERVAL = 999  # ms # Must be 1ms less than _DUPLICATE_PACKET_SUPPRESSION_INTERVAL
_CACHE_CLEANUP_INTERVAL = 10  # s
_LOADED_SYSTEM_TIMEOUT = 10  # s
_STARTUP_TIMEOUT = 9  # s must be lower than _LOADED_SYSTEM_TIMEOUT
_ONE_SECOND = 1000  # ms

# If the system is loaded or the event
# loop was blocked by another task that was doing I/O in the loop
# (shouldn't happen but it does in practice) we need to give
# a buffer timeout to ensure a coroutine can finish before
# the future times out

# Some DNS constants

_MDNS_ADDR = "224.0.0.251"
_MDNS_ADDR6 = "ff02::fb"
_MDNS_PORT = 5353
_DNS_PORT = 53
_DNS_HOST_TTL = 120  # two minute for host records (A, SRV etc) as-per RFC6762
_DNS_OTHER_TTL = 4500  # 75 minutes for non-host records (PTR, TXT etc) as-per RFC6762
# Currently we enforce a minimum TTL for PTR records to avoid
# ServiceBrowsers generating excessive queries refresh queries.
# Apple uses a 15s minimum TTL, however we do not have the same
# level of rate limit and safe guards so we use 1/4 of the recommended value
_DNS_PTR_MIN_TTL = 1125

_DNS_PACKET_HEADER_LEN = 12

_MAX_MSG_TYPICAL = 1460  # unused
_MAX_MSG_ABSOLUTE = 8966

_FLAGS_QR_MASK = 0x8000  # query response mask
_FLAGS_QR_QUERY = 0x0000  # query
_FLAGS_QR_RESPONSE = 0x8000  # response

_FLAGS_AA = 0x0400  # Authoritative answer
_FLAGS_TC = 0x0200  # Truncated
_FLAGS_RD = 0x0100  # Recursion desired
_FLAGS_RA = 0x8000  # Recursion available

_FLAGS_Z = 0x0040  # Zero
_FLAGS_AD = 0x0020  # Authentic data
_FLAGS_CD = 0x0010  # Checking disabled

_CLASS_IN = 1
_CLASS_CS = 2
_CLASS_CH = 3
_CLASS_HS = 4
_CLASS_NONE = 254
_CLASS_ANY = 255
_CLASS_MASK = 0x7FFF
_CLASS_UNIQUE = 0x8000
_CLASS_IN_UNIQUE = _CLASS_IN | _CLASS_UNIQUE

_TYPE_A = 1
_TYPE_NS = 2
_TYPE_MD = 3
_TYPE_MF = 4
_TYPE_CNAME = 5
_TYPE_SOA = 6
_TYPE_MB = 7
_TYPE_MG = 8
_TYPE_MR = 9
_TYPE_NULL = 10
_TYPE_WKS = 11
_TYPE_PTR = 12
_TYPE_HINFO = 13
_TYPE_MINFO = 14
_TYPE_MX = 15
_TYPE_TXT = 16
_TYPE_AAAA = 28
_TYPE_SRV = 33
_TYPE_NSEC = 47
_TYPE_ANY = 255

# Mapping constants to names

_CLASSES = {
    _CLASS_IN: "in",
    _CLASS_CS: "cs",
    _CLASS_CH: "ch",
    _CLASS_HS: "hs",
    _CLASS_NONE: "none",
    _CLASS_ANY: "any",
}

_TYPES = {
    _TYPE_A: "a",
    _TYPE_NS: "ns",
    _TYPE_MD: "md",
    _TYPE_MF: "mf",
    _TYPE_CNAME: "cname",
    _TYPE_SOA: "soa",
    _TYPE_MB: "mb",
    _TYPE_MG: "mg",
    _TYPE_MR: "mr",
    _TYPE_NULL: "null",
    _TYPE_WKS: "wks",
    _TYPE_PTR: "ptr",
    _TYPE_HINFO: "hinfo",
    _TYPE_MINFO: "minfo",
    _TYPE_MX: "mx",
    _TYPE_TXT: "txt",
    _TYPE_AAAA: "quada",
    _TYPE_SRV: "srv",
    _TYPE_ANY: "any",
    _TYPE_NSEC: "nsec",
}

_ADDRESS_RECORD_TYPES = {_TYPE_A, _TYPE_AAAA}

_HAS_A_TO_Z = re.compile(r"[A-Za-z]")
_HAS_ONLY_A_TO_Z_NUM_HYPHEN = re.compile(r"^[A-Za-z0-9\-]+$")
_HAS_ONLY_A_TO_Z_NUM_HYPHEN_UNDERSCORE = re.compile(r"^[A-Za-z0-9\-\_]+$")
_HAS_ASCII_CONTROL_CHARS = re.compile(r"[\x00-\x1f\x7f]")

_EXPIRE_REFRESH_TIME_PERCENT = 75

_LOCAL_TRAILER = ".local."
_TCP_PROTOCOL_LOCAL_TRAILER = "._tcp.local."
_NONTCP_PROTOCOL_LOCAL_TRAILER = "._udp.local."

# https://datatracker.ietf.org/doc/html/rfc6763#section-9
_SERVICE_TYPE_ENUMERATION_NAME = "_services._dns-sd._udp.local."

_IPPROTO_IPV6 = socket.IPPROTO_IPV6
