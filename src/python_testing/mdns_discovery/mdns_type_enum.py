from enum import Enum


class MdnsType(Enum):
    COMMISIONER = "_matterd._udp.local."
    COMMISSIONABLE = "_matterc._udp.local."
    OPERATIONAL = "_matter._tcp.local."
    BORDER_ROUTER = "_meshcop._udp.local."
