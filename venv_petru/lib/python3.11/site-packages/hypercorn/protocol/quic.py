from __future__ import annotations

from dataclasses import dataclass
from functools import partial
from typing import Awaitable, Callable, Dict, Optional, Set, Tuple

from aioquic.buffer import Buffer
from aioquic.h3.connection import H3_ALPN
from aioquic.quic.configuration import QuicConfiguration
from aioquic.quic.connection import QuicConnection
from aioquic.quic.events import (
    ConnectionIdIssued,
    ConnectionIdRetired,
    ConnectionTerminated,
    ProtocolNegotiated,
)
from aioquic.quic.packet import (
    encode_quic_version_negotiation,
    PACKET_TYPE_INITIAL,
    pull_quic_header,
)

from .h3 import H3Protocol
from ..config import Config
from ..events import Closed, Event, RawData
from ..typing import AppWrapper, ConnectionState, SingleTask, TaskGroup, WorkerContext


@dataclass
class _Connection:
    cids: Set[bytes]
    quic: QuicConnection
    task: SingleTask
    h3: Optional[H3Protocol] = None


class QuicProtocol:
    def __init__(
        self,
        app: AppWrapper,
        config: Config,
        context: WorkerContext,
        task_group: TaskGroup,
        state: ConnectionState,
        server: Optional[Tuple[str, int]],
        send: Callable[[Event], Awaitable[None]],
    ) -> None:
        self.app = app
        self.config = config
        self.context = context
        self.connections: Dict[bytes, _Connection] = {}
        self.send = send
        self.server = server
        self.task_group = task_group
        self.state = state

        self.quic_config = QuicConfiguration(alpn_protocols=H3_ALPN, is_client=False)
        self.quic_config.load_cert_chain(certfile=config.certfile, keyfile=config.keyfile)

    @property
    def idle(self) -> bool:
        return len(self.connections) == 0

    async def handle(self, event: Event) -> None:
        if isinstance(event, RawData):
            try:
                header = pull_quic_header(Buffer(data=event.data), host_cid_length=8)
            except ValueError:
                return
            if (
                header.version is not None
                and header.version not in self.quic_config.supported_versions
            ):
                data = encode_quic_version_negotiation(
                    source_cid=header.destination_cid,
                    destination_cid=header.source_cid,
                    supported_versions=self.quic_config.supported_versions,
                )
                await self.send(RawData(data=data, address=event.address))
                return

            connection = self.connections.get(header.destination_cid)
            if (
                connection is None
                and len(event.data) >= 1200
                and header.packet_type == PACKET_TYPE_INITIAL
                and not self.context.terminated.is_set()
            ):
                quic_connection = QuicConnection(
                    configuration=self.quic_config,
                    original_destination_connection_id=header.destination_cid,
                )
                connection = _Connection(
                    cids={header.destination_cid, quic_connection.host_cid},
                    quic=quic_connection,
                    task=self.context.single_task_class(),
                )
                self.connections[header.destination_cid] = connection
                self.connections[quic_connection.host_cid] = connection

            if connection is not None:
                connection.quic.receive_datagram(event.data, event.address, now=self.context.time())
                await self._handle_events(connection, event.address)
        elif isinstance(event, Closed):
            pass

    async def send_all(self, connection: _Connection) -> None:
        for data, address in connection.quic.datagrams_to_send(now=self.context.time()):
            await self.send(RawData(data=data, address=address))

        timer = connection.quic.get_timer()
        if timer is not None:
            await connection.task.restart(
                self.task_group, partial(self._handle_timer, timer, connection)
            )

    async def _handle_events(
        self, connection: _Connection, client: Optional[Tuple[str, int]] = None
    ) -> None:
        event = connection.quic.next_event()
        while event is not None:
            if isinstance(event, ConnectionTerminated):
                await connection.task.stop()
                for cid in connection.cids:
                    del self.connections[cid]
                connection.cids = set()
            elif isinstance(event, ProtocolNegotiated):
                connection.h3 = H3Protocol(
                    self.app,
                    self.config,
                    self.context,
                    self.task_group,
                    self.state,
                    client,
                    self.server,
                    connection.quic,
                    partial(self.send_all, connection),
                )
            elif isinstance(event, ConnectionIdIssued):
                connection.cids.add(event.connection_id)
                self.connections[event.connection_id] = connection
            elif isinstance(event, ConnectionIdRetired):
                connection.cids.remove(event.connection_id)
                del self.connections[event.connection_id]

            if connection.h3 is not None:
                await connection.h3.handle(event)

            event = connection.quic.next_event()

        await self.send_all(connection)

    async def _handle_timer(self, timer: float, connection: _Connection) -> None:
        wait = max(0, timer - self.context.time())
        await self.context.sleep(wait)
        connection.quic.handle_timer(now=self.context.time())
        await self._handle_events(connection, None)
