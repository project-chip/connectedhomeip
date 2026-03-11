import binascii
import ipaddress
import os
from dataclasses import dataclass
from enum import IntEnum
from typing import List, Optional, Tuple

from cryptography.hazmat.primitives.ciphers.aead import AESGCM

from ..buffer import Buffer
from .rangeset import RangeSet

PACKET_LONG_HEADER = 0x80
PACKET_FIXED_BIT = 0x40
PACKET_SPIN_BIT = 0x20

PACKET_TYPE_INITIAL = PACKET_LONG_HEADER | PACKET_FIXED_BIT | 0x00
PACKET_TYPE_ZERO_RTT = PACKET_LONG_HEADER | PACKET_FIXED_BIT | 0x10
PACKET_TYPE_HANDSHAKE = PACKET_LONG_HEADER | PACKET_FIXED_BIT | 0x20
PACKET_TYPE_RETRY = PACKET_LONG_HEADER | PACKET_FIXED_BIT | 0x30
PACKET_TYPE_ONE_RTT = PACKET_FIXED_BIT
PACKET_TYPE_MASK = 0xF0

CONNECTION_ID_MAX_SIZE = 20
PACKET_NUMBER_MAX_SIZE = 4
RETRY_AEAD_KEY_DRAFT_29 = binascii.unhexlify("ccce187ed09a09d05728155a6cb96be1")
RETRY_AEAD_KEY_VERSION_1 = binascii.unhexlify("be0c690b9f66575a1d766b54e368c84e")
RETRY_AEAD_NONCE_DRAFT_29 = binascii.unhexlify("e54930f97f2136f0530a8c1c")
RETRY_AEAD_NONCE_VERSION_1 = binascii.unhexlify("461599d35d632bf2239825bb")
RETRY_INTEGRITY_TAG_SIZE = 16
STATELESS_RESET_TOKEN_SIZE = 16


class QuicErrorCode(IntEnum):
    NO_ERROR = 0x0
    INTERNAL_ERROR = 0x1
    CONNECTION_REFUSED = 0x2
    FLOW_CONTROL_ERROR = 0x3
    STREAM_LIMIT_ERROR = 0x4
    STREAM_STATE_ERROR = 0x5
    FINAL_SIZE_ERROR = 0x6
    FRAME_ENCODING_ERROR = 0x7
    TRANSPORT_PARAMETER_ERROR = 0x8
    CONNECTION_ID_LIMIT_ERROR = 0x9
    PROTOCOL_VIOLATION = 0xA
    INVALID_TOKEN = 0xB
    APPLICATION_ERROR = 0xC
    CRYPTO_BUFFER_EXCEEDED = 0xD
    KEY_UPDATE_ERROR = 0xE
    AEAD_LIMIT_REACHED = 0xF
    CRYPTO_ERROR = 0x100


class QuicProtocolVersion(IntEnum):
    NEGOTIATION = 0
    VERSION_1 = 0x00000001
    DRAFT_29 = 0xFF00001D
    DRAFT_30 = 0xFF00001E
    DRAFT_31 = 0xFF00001F
    DRAFT_32 = 0xFF000020


@dataclass
class QuicHeader:
    is_long_header: bool
    version: Optional[int]
    packet_type: int
    destination_cid: bytes
    source_cid: bytes
    token: bytes = b""
    integrity_tag: bytes = b""
    rest_length: int = 0


def decode_packet_number(truncated: int, num_bits: int, expected: int) -> int:
    """
    Recover a packet number from a truncated packet number.

    See: Appendix A - Sample Packet Number Decoding Algorithm
    """
    window = 1 << num_bits
    half_window = window // 2
    candidate = (expected & ~(window - 1)) | truncated
    if candidate <= expected - half_window and candidate < (1 << 62) - window:
        return candidate + window
    elif candidate > expected + half_window and candidate >= window:
        return candidate - window
    else:
        return candidate


def get_retry_integrity_tag(
    packet_without_tag: bytes, original_destination_cid: bytes, version: int
) -> bytes:
    """
    Calculate the integrity tag for a RETRY packet.
    """
    # build Retry pseudo packet
    buf = Buffer(capacity=1 + len(original_destination_cid) + len(packet_without_tag))
    buf.push_uint8(len(original_destination_cid))
    buf.push_bytes(original_destination_cid)
    buf.push_bytes(packet_without_tag)
    assert buf.eof()

    if is_draft_version(version):
        aead_key = RETRY_AEAD_KEY_DRAFT_29
        aead_nonce = RETRY_AEAD_NONCE_DRAFT_29
    else:
        aead_key = RETRY_AEAD_KEY_VERSION_1
        aead_nonce = RETRY_AEAD_NONCE_VERSION_1

    # run AES-128-GCM
    aead = AESGCM(aead_key)
    integrity_tag = aead.encrypt(aead_nonce, b"", buf.data)
    assert len(integrity_tag) == RETRY_INTEGRITY_TAG_SIZE
    return integrity_tag


def get_spin_bit(first_byte: int) -> bool:
    return bool(first_byte & PACKET_SPIN_BIT)


def is_draft_version(version: int) -> bool:
    return version in (
        QuicProtocolVersion.DRAFT_29,
        QuicProtocolVersion.DRAFT_30,
        QuicProtocolVersion.DRAFT_31,
        QuicProtocolVersion.DRAFT_32,
    )


def is_long_header(first_byte: int) -> bool:
    return bool(first_byte & PACKET_LONG_HEADER)


def pull_quic_header(buf: Buffer, host_cid_length: Optional[int] = None) -> QuicHeader:
    first_byte = buf.pull_uint8()

    integrity_tag = b""
    token = b""
    if is_long_header(first_byte):
        # long header packet
        version = buf.pull_uint32()

        destination_cid_length = buf.pull_uint8()
        if destination_cid_length > CONNECTION_ID_MAX_SIZE:
            raise ValueError(
                "Destination CID is too long (%d bytes)" % destination_cid_length
            )
        destination_cid = buf.pull_bytes(destination_cid_length)

        source_cid_length = buf.pull_uint8()
        if source_cid_length > CONNECTION_ID_MAX_SIZE:
            raise ValueError("Source CID is too long (%d bytes)" % source_cid_length)
        source_cid = buf.pull_bytes(source_cid_length)

        if version == QuicProtocolVersion.NEGOTIATION:
            # version negotiation
            packet_type = None
            rest_length = buf.capacity - buf.tell()
        else:
            if not (first_byte & PACKET_FIXED_BIT):
                raise ValueError("Packet fixed bit is zero")

            packet_type = first_byte & PACKET_TYPE_MASK
            if packet_type == PACKET_TYPE_INITIAL:
                token_length = buf.pull_uint_var()
                token = buf.pull_bytes(token_length)
                rest_length = buf.pull_uint_var()
            elif packet_type == PACKET_TYPE_RETRY:
                token_length = buf.capacity - buf.tell() - RETRY_INTEGRITY_TAG_SIZE
                token = buf.pull_bytes(token_length)
                integrity_tag = buf.pull_bytes(RETRY_INTEGRITY_TAG_SIZE)
                rest_length = 0
            else:
                rest_length = buf.pull_uint_var()

            # check remainder length
            if rest_length > buf.capacity - buf.tell():
                raise ValueError("Packet payload is truncated")

        return QuicHeader(
            is_long_header=True,
            version=version,
            packet_type=packet_type,
            destination_cid=destination_cid,
            source_cid=source_cid,
            token=token,
            integrity_tag=integrity_tag,
            rest_length=rest_length,
        )
    else:
        # short header packet
        if not (first_byte & PACKET_FIXED_BIT):
            raise ValueError("Packet fixed bit is zero")

        packet_type = first_byte & PACKET_TYPE_MASK
        destination_cid = buf.pull_bytes(host_cid_length)
        return QuicHeader(
            is_long_header=False,
            version=None,
            packet_type=packet_type,
            destination_cid=destination_cid,
            source_cid=b"",
            token=b"",
            rest_length=buf.capacity - buf.tell(),
        )


def encode_quic_retry(
    version: int,
    source_cid: bytes,
    destination_cid: bytes,
    original_destination_cid: bytes,
    retry_token: bytes,
) -> bytes:
    buf = Buffer(
        capacity=7
        + len(destination_cid)
        + len(source_cid)
        + len(retry_token)
        + RETRY_INTEGRITY_TAG_SIZE
    )
    buf.push_uint8(PACKET_TYPE_RETRY)
    buf.push_uint32(version)
    buf.push_uint8(len(destination_cid))
    buf.push_bytes(destination_cid)
    buf.push_uint8(len(source_cid))
    buf.push_bytes(source_cid)
    buf.push_bytes(retry_token)
    buf.push_bytes(
        get_retry_integrity_tag(buf.data, original_destination_cid, version=version)
    )
    assert buf.eof()
    return buf.data


def encode_quic_version_negotiation(
    source_cid: bytes, destination_cid: bytes, supported_versions: List[int]
) -> bytes:
    buf = Buffer(
        capacity=7
        + len(destination_cid)
        + len(source_cid)
        + 4 * len(supported_versions)
    )
    buf.push_uint8(os.urandom(1)[0] | PACKET_LONG_HEADER)
    buf.push_uint32(QuicProtocolVersion.NEGOTIATION)
    buf.push_uint8(len(destination_cid))
    buf.push_bytes(destination_cid)
    buf.push_uint8(len(source_cid))
    buf.push_bytes(source_cid)
    for version in supported_versions:
        buf.push_uint32(version)
    return buf.data


# TLS EXTENSION


@dataclass
class QuicPreferredAddress:
    ipv4_address: Optional[Tuple[str, int]]
    ipv6_address: Optional[Tuple[str, int]]
    connection_id: bytes
    stateless_reset_token: bytes


@dataclass
class QuicTransportParameters:
    original_destination_connection_id: Optional[bytes] = None
    max_idle_timeout: Optional[int] = None
    stateless_reset_token: Optional[bytes] = None
    max_udp_payload_size: Optional[int] = None
    initial_max_data: Optional[int] = None
    initial_max_stream_data_bidi_local: Optional[int] = None
    initial_max_stream_data_bidi_remote: Optional[int] = None
    initial_max_stream_data_uni: Optional[int] = None
    initial_max_streams_bidi: Optional[int] = None
    initial_max_streams_uni: Optional[int] = None
    ack_delay_exponent: Optional[int] = None
    max_ack_delay: Optional[int] = None
    disable_active_migration: Optional[bool] = False
    preferred_address: Optional[QuicPreferredAddress] = None
    active_connection_id_limit: Optional[int] = None
    initial_source_connection_id: Optional[bytes] = None
    retry_source_connection_id: Optional[bytes] = None
    max_datagram_frame_size: Optional[int] = None
    quantum_readiness: Optional[bytes] = None


PARAMS = {
    0x00: ("original_destination_connection_id", bytes),
    0x01: ("max_idle_timeout", int),
    0x02: ("stateless_reset_token", bytes),
    0x03: ("max_udp_payload_size", int),
    0x04: ("initial_max_data", int),
    0x05: ("initial_max_stream_data_bidi_local", int),
    0x06: ("initial_max_stream_data_bidi_remote", int),
    0x07: ("initial_max_stream_data_uni", int),
    0x08: ("initial_max_streams_bidi", int),
    0x09: ("initial_max_streams_uni", int),
    0x0A: ("ack_delay_exponent", int),
    0x0B: ("max_ack_delay", int),
    0x0C: ("disable_active_migration", bool),
    0x0D: ("preferred_address", QuicPreferredAddress),
    0x0E: ("active_connection_id_limit", int),
    0x0F: ("initial_source_connection_id", bytes),
    0x10: ("retry_source_connection_id", bytes),
    # extensions
    0x0020: ("max_datagram_frame_size", int),
    0x0C37: ("quantum_readiness", bytes),
}


def pull_quic_preferred_address(buf: Buffer) -> QuicPreferredAddress:
    ipv4_address = None
    ipv4_host = buf.pull_bytes(4)
    ipv4_port = buf.pull_uint16()
    if ipv4_host != bytes(4):
        ipv4_address = (str(ipaddress.IPv4Address(ipv4_host)), ipv4_port)

    ipv6_address = None
    ipv6_host = buf.pull_bytes(16)
    ipv6_port = buf.pull_uint16()
    if ipv6_host != bytes(16):
        ipv6_address = (str(ipaddress.IPv6Address(ipv6_host)), ipv6_port)

    connection_id_length = buf.pull_uint8()
    connection_id = buf.pull_bytes(connection_id_length)
    stateless_reset_token = buf.pull_bytes(16)

    return QuicPreferredAddress(
        ipv4_address=ipv4_address,
        ipv6_address=ipv6_address,
        connection_id=connection_id,
        stateless_reset_token=stateless_reset_token,
    )


def push_quic_preferred_address(
    buf: Buffer, preferred_address: QuicPreferredAddress
) -> None:
    if preferred_address.ipv4_address is not None:
        buf.push_bytes(ipaddress.IPv4Address(preferred_address.ipv4_address[0]).packed)
        buf.push_uint16(preferred_address.ipv4_address[1])
    else:
        buf.push_bytes(bytes(6))

    if preferred_address.ipv6_address is not None:
        buf.push_bytes(ipaddress.IPv6Address(preferred_address.ipv6_address[0]).packed)
        buf.push_uint16(preferred_address.ipv6_address[1])
    else:
        buf.push_bytes(bytes(18))

    buf.push_uint8(len(preferred_address.connection_id))
    buf.push_bytes(preferred_address.connection_id)
    buf.push_bytes(preferred_address.stateless_reset_token)


def pull_quic_transport_parameters(buf: Buffer) -> QuicTransportParameters:
    params = QuicTransportParameters()
    while not buf.eof():
        param_id = buf.pull_uint_var()
        param_len = buf.pull_uint_var()
        param_start = buf.tell()
        if param_id in PARAMS:
            # parse known parameter
            param_name, param_type = PARAMS[param_id]
            if param_type == int:
                setattr(params, param_name, buf.pull_uint_var())
            elif param_type == bytes:
                setattr(params, param_name, buf.pull_bytes(param_len))
            elif param_type == QuicPreferredAddress:
                setattr(params, param_name, pull_quic_preferred_address(buf))
            else:
                setattr(params, param_name, True)
        else:
            # skip unknown parameter
            buf.pull_bytes(param_len)
        assert buf.tell() == param_start + param_len

    return params


def push_quic_transport_parameters(
    buf: Buffer, params: QuicTransportParameters
) -> None:
    for param_id, (param_name, param_type) in PARAMS.items():
        param_value = getattr(params, param_name)
        if param_value is not None and param_value is not False:
            param_buf = Buffer(capacity=65536)
            if param_type == int:
                param_buf.push_uint_var(param_value)
            elif param_type == bytes:
                param_buf.push_bytes(param_value)
            elif param_type == QuicPreferredAddress:
                push_quic_preferred_address(param_buf, param_value)
            buf.push_uint_var(param_id)
            buf.push_uint_var(param_buf.tell())
            buf.push_bytes(param_buf.data)


# FRAMES


class QuicFrameType(IntEnum):
    PADDING = 0x00
    PING = 0x01
    ACK = 0x02
    ACK_ECN = 0x03
    RESET_STREAM = 0x04
    STOP_SENDING = 0x05
    CRYPTO = 0x06
    NEW_TOKEN = 0x07
    STREAM_BASE = 0x08
    MAX_DATA = 0x10
    MAX_STREAM_DATA = 0x11
    MAX_STREAMS_BIDI = 0x12
    MAX_STREAMS_UNI = 0x13
    DATA_BLOCKED = 0x14
    STREAM_DATA_BLOCKED = 0x15
    STREAMS_BLOCKED_BIDI = 0x16
    STREAMS_BLOCKED_UNI = 0x17
    NEW_CONNECTION_ID = 0x18
    RETIRE_CONNECTION_ID = 0x19
    PATH_CHALLENGE = 0x1A
    PATH_RESPONSE = 0x1B
    TRANSPORT_CLOSE = 0x1C
    APPLICATION_CLOSE = 0x1D
    HANDSHAKE_DONE = 0x1E
    DATAGRAM = 0x30
    DATAGRAM_WITH_LENGTH = 0x31


NON_ACK_ELICITING_FRAME_TYPES = frozenset(
    [
        QuicFrameType.ACK,
        QuicFrameType.ACK_ECN,
        QuicFrameType.PADDING,
        QuicFrameType.TRANSPORT_CLOSE,
        QuicFrameType.APPLICATION_CLOSE,
    ]
)
NON_IN_FLIGHT_FRAME_TYPES = frozenset(
    [
        QuicFrameType.ACK,
        QuicFrameType.ACK_ECN,
        QuicFrameType.TRANSPORT_CLOSE,
        QuicFrameType.APPLICATION_CLOSE,
    ]
)

PROBING_FRAME_TYPES = frozenset(
    [
        QuicFrameType.PATH_CHALLENGE,
        QuicFrameType.PATH_RESPONSE,
        QuicFrameType.PADDING,
        QuicFrameType.NEW_CONNECTION_ID,
    ]
)


@dataclass
class QuicResetStreamFrame:
    error_code: int
    final_size: int
    stream_id: int


@dataclass
class QuicStopSendingFrame:
    error_code: int
    stream_id: int


@dataclass
class QuicStreamFrame:
    data: bytes = b""
    fin: bool = False
    offset: int = 0


def pull_ack_frame(buf: Buffer) -> Tuple[RangeSet, int]:
    rangeset = RangeSet()
    end = buf.pull_uint_var()  # largest acknowledged
    delay = buf.pull_uint_var()
    ack_range_count = buf.pull_uint_var()
    ack_count = buf.pull_uint_var()  # first ack range
    rangeset.add(end - ack_count, end + 1)
    end -= ack_count
    for _ in range(ack_range_count):
        end -= buf.pull_uint_var() + 2
        ack_count = buf.pull_uint_var()
        rangeset.add(end - ack_count, end + 1)
        end -= ack_count
    return rangeset, delay


def push_ack_frame(buf: Buffer, rangeset: RangeSet, delay: int) -> int:
    ranges = len(rangeset)
    index = ranges - 1
    r = rangeset[index]
    buf.push_uint_var(r.stop - 1)
    buf.push_uint_var(delay)
    buf.push_uint_var(index)
    buf.push_uint_var(r.stop - 1 - r.start)
    start = r.start
    while index > 0:
        index -= 1
        r = rangeset[index]
        buf.push_uint_var(start - r.stop - 1)
        buf.push_uint_var(r.stop - r.start - 1)
        start = r.start
    return ranges
