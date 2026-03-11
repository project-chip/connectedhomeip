from __future__ import annotations

import asyncio
from typing import Optional

from ..config import Config
from ..statsd import StatsdLogger as Base


class _DummyProto(asyncio.DatagramProtocol):
    pass


class StatsdLogger(Base):
    def __init__(self, config: Config) -> None:
        super().__init__(config)
        self.address = config.statsd_host.rsplit(":", 1)
        self.transport: Optional[asyncio.BaseTransport] = None

    async def _socket_send(self, message: bytes) -> None:
        if self.transport is None:
            self.transport, _ = await asyncio.get_event_loop().create_datagram_endpoint(
                _DummyProto, remote_addr=(self.address[0], int(self.address[1]))
            )

        self.transport.sendto(message)  # type: ignore
