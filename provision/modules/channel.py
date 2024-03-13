from enum import Enum
from abc import ABC, abstractmethod
import modules.util as _util


class Channel(ABC):
    NONE = 0x00
    RTT = 0x01
    BLE = 0x02

    def __init__(self, type) -> None:
        self.type = type

    @abstractmethod
    def open(self):
        pass

    @abstractmethod
    def close(self):
        pass

    @abstractmethod
    def write(self, data):
        pass

    @abstractmethod
    def read(self):
        pass

    @staticmethod
    def create(paths, args, conn):
        return JLinkChannel(paths, args, conn)
