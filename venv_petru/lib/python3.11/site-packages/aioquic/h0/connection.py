from typing import Dict, List

from aioquic.h3.events import DataReceived, H3Event, Headers, HeadersReceived
from aioquic.quic.connection import QuicConnection
from aioquic.quic.events import QuicEvent, StreamDataReceived

H0_ALPN = ["hq-interop", "hq-32", "hq-31", "hq-30", "hq-29"]


class H0Connection:
    """
    An HTTP/0.9 connection object.
    """

    def __init__(self, quic: QuicConnection):
        self._buffer: Dict[int, bytes] = {}
        self._headers_received: Dict[int, bool] = {}
        self._is_client = quic.configuration.is_client
        self._quic = quic

    def handle_event(self, event: QuicEvent) -> List[H3Event]:
        http_events: List[H3Event] = []

        if isinstance(event, StreamDataReceived) and (event.stream_id % 4) == 0:
            data = self._buffer.pop(event.stream_id, b"") + event.data
            if not self._headers_received.get(event.stream_id, False):
                if self._is_client:
                    http_events.append(
                        HeadersReceived(
                            headers=[], stream_ended=False, stream_id=event.stream_id
                        )
                    )
                elif data.endswith(b"\r\n") or event.end_stream:
                    method, path = data.rstrip().split(b" ", 1)
                    http_events.append(
                        HeadersReceived(
                            headers=[(b":method", method), (b":path", path)],
                            stream_ended=False,
                            stream_id=event.stream_id,
                        )
                    )
                    data = b""
                else:
                    # incomplete request, stash the data
                    self._buffer[event.stream_id] = data
                    return http_events
                self._headers_received[event.stream_id] = True

            http_events.append(
                DataReceived(
                    data=data, stream_ended=event.end_stream, stream_id=event.stream_id
                )
            )

        return http_events

    def send_data(self, stream_id: int, data: bytes, end_stream: bool) -> None:
        self._quic.send_stream_data(stream_id, data, end_stream)

    def send_headers(
        self, stream_id: int, headers: Headers, end_stream: bool = False
    ) -> None:
        if self._is_client:
            headers_dict = dict(headers)
            data = headers_dict[b":method"] + b" " + headers_dict[b":path"] + b"\r\n"
        else:
            data = b""
        self._quic.send_stream_data(stream_id, data, end_stream)
