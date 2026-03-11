from __future__ import annotations

from enum import auto, Enum
from io import BytesIO, StringIO
from time import time
from typing import Awaitable, Callable, Iterable, List, Optional, Tuple, Union
from urllib.parse import unquote

from wsproto.connection import Connection, ConnectionState, ConnectionType
from wsproto.events import (
    BytesMessage,
    CloseConnection,
    Event as WSProtoEvent,
    Message,
    Ping,
    TextMessage,
)
from wsproto.extensions import Extension, PerMessageDeflate
from wsproto.frame_protocol import CloseReason
from wsproto.handshake import server_extensions_handshake, WEBSOCKET_VERSION
from wsproto.utilities import generate_accept_token, LocalProtocolError, split_comma_header

from .events import Body, Data, EndBody, EndData, Event, Request, Response, StreamClosed
from ..config import Config
from ..typing import (
    AppWrapper,
    ASGISendEvent,
    TaskGroup,
    WebsocketAcceptEvent,
    WebsocketResponseBodyEvent,
    WebsocketResponseStartEvent,
    WebsocketScope,
    WorkerContext,
)
from ..utils import (
    build_and_validate_headers,
    suppress_body,
    UnexpectedMessageError,
    valid_server_name,
)


class ASGIWebsocketState(Enum):
    # Hypercorn supports the ASGI websocket HTTP response extension,
    # which allows HTTP responses rather than acceptance.
    HANDSHAKE = auto()
    CONNECTED = auto()
    RESPONSE = auto()
    CLOSED = auto()
    HTTPCLOSED = auto()


class FrameTooLargeError(Exception):
    pass


class Handshake:
    def __init__(self, headers: List[Tuple[bytes, bytes]], http_version: str) -> None:
        self.accepted = False
        self.http_version = http_version
        self.connection_tokens: Optional[List[str]] = None
        self.extensions: Optional[List[str]] = None
        self.key: Optional[bytes] = None
        self.subprotocols: Optional[List[str]] = None
        self.upgrade: Optional[bytes] = None
        self.version: Optional[bytes] = None
        for name, value in headers:
            name = name.lower()
            if name == b"connection":
                self.connection_tokens = split_comma_header(value)
            elif name == b"sec-websocket-extensions":
                self.extensions = split_comma_header(value)
            elif name == b"sec-websocket-key":
                self.key = value
            elif name == b"sec-websocket-protocol":
                self.subprotocols = split_comma_header(value)
            elif name == b"sec-websocket-version":
                self.version = value
            elif name == b"upgrade":
                self.upgrade = value

    def is_valid(self) -> bool:
        if self.http_version < "1.1":
            return False
        elif self.http_version == "1.1":
            if self.key is None:
                return False
            if self.connection_tokens is None or not any(
                token.lower() == "upgrade" for token in self.connection_tokens
            ):
                return False
            if self.upgrade.lower() != b"websocket":
                return False

        if self.version != WEBSOCKET_VERSION:
            return False
        return True

    def accept(
        self,
        subprotocol: Optional[str],
        additional_headers: Iterable[Tuple[bytes, bytes]],
    ) -> Tuple[int, List[Tuple[bytes, bytes]], Connection]:
        headers = []
        if subprotocol is not None:
            if self.subprotocols is None or subprotocol not in self.subprotocols:
                raise Exception("Invalid Subprotocol")
            else:
                headers.append((b"sec-websocket-protocol", subprotocol.encode()))

        extensions: List[Extension] = [PerMessageDeflate()]
        accepts = None
        if self.extensions is not None:
            accepts = server_extensions_handshake(self.extensions, extensions)

        if accepts:
            headers.append((b"sec-websocket-extensions", accepts))

        if self.key is not None:
            headers.append((b"sec-websocket-accept", generate_accept_token(self.key)))

        status_code = 200
        if self.http_version == "1.1":
            headers.extend([(b"upgrade", b"WebSocket"), (b"connection", b"Upgrade")])
            status_code = 101

        for name, value in additional_headers:
            if b"sec-websocket-protocol" == name or name.startswith(b":"):
                raise Exception(f"Invalid additional header, {name.decode()}")

            headers.append((name, value))

        self.accepted = True
        return status_code, headers, Connection(ConnectionType.SERVER, extensions)


class WebsocketBuffer:
    def __init__(self, max_length: int) -> None:
        self.value: Optional[Union[BytesIO, StringIO]] = None
        self.length = 0
        self.max_length = max_length

    def extend(self, event: Message) -> None:
        if self.value is None:
            if isinstance(event, TextMessage):
                self.value = StringIO()
            else:
                self.value = BytesIO()
        self.length += self.value.write(event.data)
        if self.length > self.max_length:
            raise FrameTooLargeError()

    def clear(self) -> None:
        self.value = None
        self.length = 0

    def to_message(self) -> dict:
        return {
            "type": "websocket.receive",
            "bytes": self.value.getvalue() if isinstance(self.value, BytesIO) else None,
            "text": self.value.getvalue() if isinstance(self.value, StringIO) else None,
        }


class WSStream:
    def __init__(
        self,
        app: AppWrapper,
        config: Config,
        context: WorkerContext,
        task_group: TaskGroup,
        ssl: bool,
        client: Optional[Tuple[str, int]],
        server: Optional[Tuple[str, int]],
        send: Callable[[Event], Awaitable[None]],
        stream_id: int,
    ) -> None:
        self.app = app
        self.app_put: Optional[Callable] = None
        self.buffer = WebsocketBuffer(config.websocket_max_message_size)
        self.client = client
        self.closed = False
        self.config = config
        self.context = context
        self.task_group = task_group
        self.response: WebsocketResponseStartEvent
        self.scope: WebsocketScope
        self.send = send
        # RFC 8441 for HTTP/2 says use http or https, ASGI says ws or wss
        self.scheme = "wss" if ssl else "ws"
        self.server = server
        self.start_time: float
        self.state = ASGIWebsocketState.HANDSHAKE
        self.stream_id = stream_id

        self.connection: Connection
        self.handshake: Handshake

    @property
    def idle(self) -> bool:
        return self.state in {ASGIWebsocketState.CLOSED, ASGIWebsocketState.HTTPCLOSED}

    async def handle(self, event: Event) -> None:
        if self.closed:
            return
        elif isinstance(event, Request):
            self.start_time = time()
            self.handshake = Handshake(event.headers, event.http_version)
            path, _, query_string = event.raw_path.partition(b"?")
            self.scope = {
                "type": "websocket",
                "asgi": {"spec_version": "2.3", "version": "3.0"},
                "scheme": self.scheme,
                "http_version": event.http_version,
                "path": unquote(path.decode("ascii")),
                "raw_path": path,
                "query_string": query_string,
                "root_path": self.config.root_path,
                "headers": event.headers,
                "client": self.client,
                "server": self.server,
                "state": event.state,
                "subprotocols": self.handshake.subprotocols or [],
                "extensions": {"websocket.http.response": {}},
            }

            if not valid_server_name(self.config, event):
                await self._send_error_response(404)
                self.closed = True
            elif not self.handshake.is_valid():
                await self._send_error_response(400)
                self.closed = True
            else:
                self.app_put = await self.task_group.spawn_app(
                    self.app, self.config, self.scope, self.app_send
                )
                await self.app_put({"type": "websocket.connect"})
        elif isinstance(event, (Body, Data)) and not self.handshake.accepted:
            await self._send_error_response(400)
            self.closed = True
        elif isinstance(event, (Body, Data)):
            self.connection.receive_data(event.data)
            await self._handle_events()
        elif isinstance(event, StreamClosed):
            self.closed = True
            if self.app_put is not None:
                if self.state in {ASGIWebsocketState.HTTPCLOSED, ASGIWebsocketState.CLOSED}:
                    code = CloseReason.NORMAL_CLOSURE.value
                else:
                    code = CloseReason.ABNORMAL_CLOSURE.value
                await self.app_put({"type": "websocket.disconnect", "code": code})

    async def app_send(self, message: Optional[ASGISendEvent]) -> None:
        if self.closed:
            # Allow app to finish after close
            return

        if message is None:  # ASGI App has finished sending messages
            # Cleanup if required
            if self.state == ASGIWebsocketState.HANDSHAKE:
                await self._send_error_response(500)
                await self.config.log.access(
                    self.scope, {"status": 500, "headers": []}, time() - self.start_time
                )
            elif self.state == ASGIWebsocketState.CONNECTED:
                await self._send_wsproto_event(CloseConnection(code=CloseReason.INTERNAL_ERROR))
            await self.send(StreamClosed(stream_id=self.stream_id))
        else:
            if message["type"] == "websocket.accept" and self.state == ASGIWebsocketState.HANDSHAKE:
                await self._accept(message)
            elif (
                message["type"] == "websocket.http.response.start"
                and self.state == ASGIWebsocketState.HANDSHAKE
            ):
                self.response = message
            elif message["type"] == "websocket.http.response.body" and self.state in {
                ASGIWebsocketState.HANDSHAKE,
                ASGIWebsocketState.RESPONSE,
            }:
                await self._send_rejection(message)
            elif message["type"] == "websocket.send" and self.state == ASGIWebsocketState.CONNECTED:
                event: WSProtoEvent
                if message.get("bytes") is not None:
                    event = BytesMessage(data=bytes(message["bytes"]))
                elif not isinstance(message["text"], str):
                    raise TypeError(f"{message['text']} should be a str")
                else:
                    event = TextMessage(data=message["text"])
                await self._send_wsproto_event(event)
            elif (
                message["type"] == "websocket.close" and self.state == ASGIWebsocketState.HANDSHAKE
            ):
                self.state = ASGIWebsocketState.HTTPCLOSED
                await self._send_error_response(403)
            elif message["type"] == "websocket.close":
                self.state = ASGIWebsocketState.CLOSED
                await self._send_wsproto_event(
                    CloseConnection(
                        code=int(message.get("code", CloseReason.NORMAL_CLOSURE)),
                        reason=message.get("reason"),
                    )
                )
                await self.send(EndData(stream_id=self.stream_id))
            else:
                raise UnexpectedMessageError(self.state, message["type"])

    async def _handle_events(self) -> None:
        for event in self.connection.events():
            if isinstance(event, Message):
                try:
                    self.buffer.extend(event)
                except FrameTooLargeError:
                    await self._send_wsproto_event(
                        CloseConnection(code=CloseReason.MESSAGE_TOO_BIG)
                    )
                    break

                if event.message_finished:
                    await self.app_put(self.buffer.to_message())
                    self.buffer.clear()
            elif isinstance(event, Ping):
                await self._send_wsproto_event(event.response())
            elif isinstance(event, CloseConnection):
                if self.connection.state == ConnectionState.REMOTE_CLOSING:
                    await self._send_wsproto_event(event.response())
                await self.send(StreamClosed(stream_id=self.stream_id))

    async def _send_error_response(self, status_code: int) -> None:
        await self.send(
            Response(
                stream_id=self.stream_id,
                status_code=status_code,
                headers=[(b"content-length", b"0"), (b"connection", b"close")],
            )
        )
        await self.send(EndBody(stream_id=self.stream_id))
        await self.config.log.access(
            self.scope, {"status": status_code, "headers": []}, time() - self.start_time
        )

    async def _send_wsproto_event(self, event: WSProtoEvent) -> None:
        try:
            data = self.connection.send(event)
        except LocalProtocolError:
            pass
        else:
            await self.send(Data(stream_id=self.stream_id, data=data))

    async def _accept(self, message: WebsocketAcceptEvent) -> None:
        self.state = ASGIWebsocketState.CONNECTED
        status_code, headers, self.connection = self.handshake.accept(
            message.get("subprotocol"), message.get("headers", [])
        )
        await self.send(
            Response(stream_id=self.stream_id, status_code=status_code, headers=headers)
        )
        await self.config.log.access(
            self.scope, {"status": status_code, "headers": []}, time() - self.start_time
        )
        if self.config.websocket_ping_interval is not None:
            self.task_group.spawn(self._send_pings)

    async def _send_rejection(self, message: WebsocketResponseBodyEvent) -> None:
        body_suppressed = suppress_body("GET", self.response["status"])
        if self.state == ASGIWebsocketState.HANDSHAKE:
            headers = build_and_validate_headers(self.response["headers"])
            await self.send(
                Response(
                    stream_id=self.stream_id,
                    status_code=int(self.response["status"]),
                    headers=headers,
                )
            )
            self.state = ASGIWebsocketState.RESPONSE
        if not body_suppressed:
            await self.send(Body(stream_id=self.stream_id, data=bytes(message.get("body", b""))))
        if not message.get("more_body", False):
            self.state = ASGIWebsocketState.HTTPCLOSED
            await self.send(EndBody(stream_id=self.stream_id))
            await self.config.log.access(self.scope, self.response, time() - self.start_time)

    async def _send_pings(self) -> None:
        while not self.closed:
            await self._send_wsproto_event(Ping())
            await self.context.sleep(self.config.websocket_ping_interval)
