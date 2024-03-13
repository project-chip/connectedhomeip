from abc import ABC, abstractmethod


class ProvisionProtocol(ABC):

    def __init__(self, paths, args, conn) -> None:
        pass

    @abstractmethod
    def transmit(self, args, chan):
        pass