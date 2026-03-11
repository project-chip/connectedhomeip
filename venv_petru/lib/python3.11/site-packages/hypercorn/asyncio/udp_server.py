from __future__ import annotations

import asyncio
from typing import Optional, Tuple, TYPE_CHECKING

from .task_group import TaskGroup
from .worker_context import WorkerContext
from ..config import Config
from ..events import Event, RawData
from ..typing import AppWrapper, ConnectionState, LifespanState
from ..utils import parse_socket_addr

if TYPE_CHECKING:
    # h3/Quic is an optional part of Hypercorn
    from ..protocol.quic import QuicProtocol  # noqa: F401


class UDPServer(asyncio.DatagramProtocol):
    def __init__(
        self,
        app: AppWrapper,
        loop: asyncio.AbstractEventLoop,
        config: Config,
        context: WorkerContext,
        state: LifespanState,
    ) -> None:
        self.app = app
        self.config = config
        self.context = context
        self.loop = loop
        self.protocol: "QuicProtocol"
        self.protocol_queue: asyncio.Queue = asyncio.Queue(10)
        self.transport: Optional[asyncio.DatagramTransport] = None
        self.state = state

    def connection_made(self, transport: asyncio.DatagramTransport) -> None:  # type: ignore
        self.transport = transport

    def datagram_received(self, data: bytes, address: Tuple[bytes, str]) -> None:  # type: ignore
        try:
            self.protocol_queue.put_nowait(RawData(data=data, address=address))  # type: ignore
        except asyncio.QueueFull:
            pass  # Just throw the data away, is UDP

    async def run(self) -> None:
        # h3/Quic is an optional part of Hypercorn
        from ..protocol.quic import QuicProtocol  # noqa: F811

        socket = self.transport.get_extra_info("socket")
        server = parse_socket_addr(socket.family, socket.getsockname())
        async with TaskGroup(self.loop) as task_group:
            self.protocol = QuicProtocol(
                self.app,
                self.config,
                self.context,
                task_group,
                ConnectionState(self.state.copy()),
                server,
                self.protocol_send,
            )

            while not self.context.terminated.is_set() or not self.protocol.idle:
                event = await self.protocol_queue.get()
                await self.protocol.handle(event)

    async def protocol_send(self, event: Event) -> None:
        if isinstance(event, RawData):
            self.transport.sendto(event.data, event.address)
