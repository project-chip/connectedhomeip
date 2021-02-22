from enum import IntEnum
import ctypes

class IPAddressType(IntEnum):
    Unknown = 0
    IPv4 = 1
    IPv6 = 2
    Any = 3

class MdnsServiceProtocol(IntEnum):
    Udp = 0
    Tcp = 1
    Unknown = 255

NodeId = ctypes.c_uint64


__all__ = [
    "IPAddressType"
    "MdnsServiceProtocol"
    "NodeId"
]
