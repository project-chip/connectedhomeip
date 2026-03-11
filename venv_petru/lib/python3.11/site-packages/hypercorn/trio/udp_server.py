from __future__ import annotations

import trio

from .task_group import TaskGroup
from .worker_context import WorkerContext
from ..config import Config
from ..events import Event, RawData
from ..typing import AppWrapper, ConnectionState, LifespanState
from ..utils import parse_socket_addr

MAX_RECV = 2**16


class UDPServer:
    def __init__(
        self,
        app: AppWrapper,
        config: Config,
        context: WorkerContext,
        state: LifespanState,
        socket: trio.socket.socket,
    ) -> None:
        self.app = app
        self.config = config
        self.context = context
        self.socket = trio.socket.from_stdlib_socket(socket)
        self.state = state

    async def run(
        self, task_status: trio._core._run._TaskStatus = trio.TASK_STATUS_IGNORED
    ) -> None:
        from ..protocol.quic import QuicProtocol  # h3/Quic is an optional part of Hypercorn

        task_status.started()
        server = parse_socket_addr(self.socket.family, self.socket.getsockname())
        async with TaskGroup() as task_group:
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
                data, address = await self.socket.recvfrom(MAX_RECV)
                await self.protocol.handle(RawData(data=data, address=address))

    async def protocol_send(self, event: Event) -> None:
        if isinstance(event, RawData):
            await self.socket.sendto(event.data, event.address)
