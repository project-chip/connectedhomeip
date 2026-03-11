from __future__ import annotations

import trio

from ..config import Config
from ..statsd import StatsdLogger as Base


class StatsdLogger(Base):
    def __init__(self, config: Config) -> None:
        super().__init__(config)
        self.address = tuple(config.statsd_host.rsplit(":", 1))
        self.socket = trio.socket.socket(trio.socket.AF_INET, trio.socket.SOCK_DGRAM)

    async def _socket_send(self, message: bytes) -> None:
        await self.socket.sendto(message, self.address)
