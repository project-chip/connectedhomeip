import binascii
import json
import os
import time
from collections import deque
from typing import Any, Deque, Dict, List, Optional

from ..h3.events import Headers
from .packet import (
    PACKET_TYPE_HANDSHAKE,
    PACKET_TYPE_INITIAL,
    PACKET_TYPE_MASK,
    PACKET_TYPE_ONE_RTT,
    PACKET_TYPE_RETRY,
    PACKET_TYPE_ZERO_RTT,
    QuicFrameType,
    QuicStreamFrame,
    QuicTransportParameters,
)
from .rangeset import RangeSet

PACKET_TYPE_NAMES = {
    PACKET_TYPE_INITIAL: "initial",
    PACKET_TYPE_HANDSHAKE: "handshake",
    PACKET_TYPE_ZERO_RTT: "0RTT",
    PACKET_TYPE_ONE_RTT: "1RTT",
    PACKET_TYPE_RETRY: "retry",
}
QLOG_VERSION = "0.3"


def hexdump(data: bytes) -> str:
    return binascii.hexlify(data).decode("ascii")


class QuicLoggerTrace:
    """
    A QUIC event trace.

    Events are logged in the format defined by qlog.

    See:
    - https://datatracker.ietf.org/doc/html/draft-ietf-quic-qlog-main-schema-02
    - https://datatracker.ietf.org/doc/html/draft-marx-quic-qlog-quic-events
    - https://datatracker.ietf.org/doc/html/draft-marx-quic-qlog-h3-events
    """

    def __init__(self, *, is_client: bool, odcid: bytes) -> None:
        self._odcid = odcid
        self._events: Deque[Dict[str, Any]] = deque()
        self._vantage_point = {
            "name": "aioquic",
            "type": "client" if is_client else "server",
        }

    # QUIC

    def encode_ack_frame(self, ranges: RangeSet, delay: float) -> Dict:
        return {
            "ack_delay": self.encode_time(delay),
            "acked_ranges": [[x.start, x.stop - 1] for x in ranges],
            "frame_type": "ack",
        }

    def encode_connection_close_frame(
        self, error_code: int, frame_type: Optional[int], reason_phrase: str
    ) -> Dict:
        attrs = {
            "error_code": error_code,
            "error_space": "application" if frame_type is None else "transport",
            "frame_type": "connection_close",
            "raw_error_code": error_code,
            "reason": reason_phrase,
        }
        if frame_type is not None:
            attrs["trigger_frame_type"] = frame_type

        return attrs

    def encode_connection_limit_frame(self, frame_type: int, maximum: int) -> Dict:
        if frame_type == QuicFrameType.MAX_DATA:
            return {"frame_type": "max_data", "maximum": maximum}
        else:
            return {
                "frame_type": "max_streams",
                "maximum": maximum,
                "stream_type": "unidirectional"
                if frame_type == QuicFrameType.MAX_STREAMS_UNI
                else "bidirectional",
            }

    def encode_crypto_frame(self, frame: QuicStreamFrame) -> Dict:
        return {
            "frame_type": "crypto",
            "length": len(frame.data),
            "offset": frame.offset,
        }

    def encode_data_blocked_frame(self, limit: int) -> Dict:
        return {"frame_type": "data_blocked", "limit": limit}

    def encode_datagram_frame(self, length: int) -> Dict:
        return {"frame_type": "datagram", "length": length}

    def encode_handshake_done_frame(self) -> Dict:
        return {"frame_type": "handshake_done"}

    def encode_max_stream_data_frame(self, maximum: int, stream_id: int) -> Dict:
        return {
            "frame_type": "max_stream_data",
            "maximum": maximum,
            "stream_id": stream_id,
        }

    def encode_new_connection_id_frame(
        self,
        connection_id: bytes,
        retire_prior_to: int,
        sequence_number: int,
        stateless_reset_token: bytes,
    ) -> Dict:
        return {
            "connection_id": hexdump(connection_id),
            "frame_type": "new_connection_id",
            "length": len(connection_id),
            "reset_token": hexdump(stateless_reset_token),
            "retire_prior_to": retire_prior_to,
            "sequence_number": sequence_number,
        }

    def encode_new_token_frame(self, token: bytes) -> Dict:
        return {
            "frame_type": "new_token",
            "length": len(token),
            "token": hexdump(token),
        }

    def encode_padding_frame(self) -> Dict:
        return {"frame_type": "padding"}

    def encode_path_challenge_frame(self, data: bytes) -> Dict:
        return {"data": hexdump(data), "frame_type": "path_challenge"}

    def encode_path_response_frame(self, data: bytes) -> Dict:
        return {"data": hexdump(data), "frame_type": "path_response"}

    def encode_ping_frame(self) -> Dict:
        return {"frame_type": "ping"}

    def encode_reset_stream_frame(
        self, error_code: int, final_size: int, stream_id: int
    ) -> Dict:
        return {
            "error_code": error_code,
            "final_size": final_size,
            "frame_type": "reset_stream",
            "stream_id": stream_id,
        }

    def encode_retire_connection_id_frame(self, sequence_number: int) -> Dict:
        return {
            "frame_type": "retire_connection_id",
            "sequence_number": sequence_number,
        }

    def encode_stream_data_blocked_frame(self, limit: int, stream_id: int) -> Dict:
        return {
            "frame_type": "stream_data_blocked",
            "limit": limit,
            "stream_id": stream_id,
        }

    def encode_stop_sending_frame(self, error_code: int, stream_id: int) -> Dict:
        return {
            "frame_type": "stop_sending",
            "error_code": error_code,
            "stream_id": stream_id,
        }

    def encode_stream_frame(self, frame: QuicStreamFrame, stream_id: int) -> Dict:
        return {
            "fin": frame.fin,
            "frame_type": "stream",
            "length": len(frame.data),
            "offset": frame.offset,
            "stream_id": stream_id,
        }

    def encode_streams_blocked_frame(self, is_unidirectional: bool, limit: int) -> Dict:
        return {
            "frame_type": "streams_blocked",
            "limit": limit,
            "stream_type": "unidirectional" if is_unidirectional else "bidirectional",
        }

    def encode_time(self, seconds: float) -> float:
        """
        Convert a time to milliseconds.
        """
        return seconds * 1000

    def encode_transport_parameters(
        self, owner: str, parameters: QuicTransportParameters
    ) -> Dict[str, Any]:
        data: Dict[str, Any] = {"owner": owner}
        for param_name, param_value in parameters.__dict__.items():
            if isinstance(param_value, bool):
                data[param_name] = param_value
            elif isinstance(param_value, bytes):
                data[param_name] = hexdump(param_value)
            elif isinstance(param_value, int):
                data[param_name] = param_value
        return data

    def packet_type(self, packet_type: int) -> str:
        return PACKET_TYPE_NAMES.get(packet_type & PACKET_TYPE_MASK, "1RTT")

    # HTTP/3

    def encode_http3_data_frame(self, length: int, stream_id: int) -> Dict:
        return {
            "frame": {"frame_type": "data"},
            "length": length,
            "stream_id": stream_id,
        }

    def encode_http3_headers_frame(
        self, length: int, headers: Headers, stream_id: int
    ) -> Dict:
        return {
            "frame": {
                "frame_type": "headers",
                "headers": self._encode_http3_headers(headers),
            },
            "length": length,
            "stream_id": stream_id,
        }

    def encode_http3_push_promise_frame(
        self, length: int, headers: Headers, push_id: int, stream_id: int
    ) -> Dict:
        return {
            "frame": {
                "frame_type": "push_promise",
                "headers": self._encode_http3_headers(headers),
                "push_id": push_id,
            },
            "length": length,
            "stream_id": stream_id,
        }

    def _encode_http3_headers(self, headers: Headers) -> List[Dict]:
        return [
            {"name": h[0].decode("utf8"), "value": h[1].decode("utf8")} for h in headers
        ]

    # CORE

    def log_event(self, *, category: str, event: str, data: Dict) -> None:
        self._events.append(
            {
                "data": data,
                "name": category + ":" + event,
                "time": self.encode_time(time.time()),
            }
        )

    def to_dict(self) -> Dict[str, Any]:
        """
        Return the trace as a dictionary which can be written as JSON.
        """
        return {
            "common_fields": {
                "ODCID": hexdump(self._odcid),
            },
            "events": list(self._events),
            "vantage_point": self._vantage_point,
        }


class QuicLogger:
    """
    A QUIC event logger which stores traces in memory.
    """

    def __init__(self) -> None:
        self._traces: List[QuicLoggerTrace] = []

    def start_trace(self, is_client: bool, odcid: bytes) -> QuicLoggerTrace:
        trace = QuicLoggerTrace(is_client=is_client, odcid=odcid)
        self._traces.append(trace)
        return trace

    def end_trace(self, trace: QuicLoggerTrace) -> None:
        assert trace in self._traces, "QuicLoggerTrace does not belong to QuicLogger"

    def to_dict(self) -> Dict[str, Any]:
        """
        Return the traces as a dictionary which can be written as JSON.
        """
        return {
            "qlog_format": "JSON",
            "qlog_version": QLOG_VERSION,
            "traces": [trace.to_dict() for trace in self._traces],
        }


class QuicFileLogger(QuicLogger):
    """
    A QUIC event logger which writes one trace per file.
    """

    def __init__(self, path: str) -> None:
        if not os.path.isdir(path):
            raise ValueError("QUIC log output directory '%s' does not exist" % path)
        self.path = path
        super().__init__()

    def end_trace(self, trace: QuicLoggerTrace) -> None:
        trace_dict = trace.to_dict()
        trace_path = os.path.join(
            self.path, trace_dict["common_fields"]["ODCID"] + ".qlog"
        )
        with open(trace_path, "w") as logger_fp:
            json.dump(
                {
                    "qlog_format": "JSON",
                    "qlog_version": QLOG_VERSION,
                    "traces": [trace_dict],
                },
                logger_fp,
            )
        self._traces.remove(trace)
