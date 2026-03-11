from __future__ import annotations

from dataclasses import dataclass
from typing import List, Tuple

from hypercorn.typing import ConnectionState


@dataclass(frozen=True)
class Event:
    stream_id: int


@dataclass(frozen=True)
class Request(Event):
    headers: List[Tuple[bytes, bytes]]
    http_version: str
    method: str
    raw_path: bytes
    state: ConnectionState


@dataclass(frozen=True)
class Body(Event):
    data: bytes


@dataclass(frozen=True)
class EndBody(Event):
    pass


@dataclass(frozen=True)
class Trailers(Event):
    headers: List[Tuple[bytes, bytes]]


@dataclass(frozen=True)
class Data(Event):
    data: bytes


@dataclass(frozen=True)
class EndData(Event):
    pass


@dataclass(frozen=True)
class Response(Event):
    headers: List[Tuple[bytes, bytes]]
    status_code: int


@dataclass(frozen=True)
class InformationalResponse(Event):
    headers: List[Tuple[bytes, bytes]]
    status_code: int

    def __post_init__(self) -> None:
        if self.status_code >= 200 or self.status_code < 100:
            raise ValueError(f"Status code must be 1XX not {self.status_code}")


@dataclass(frozen=True)
class StreamClosed(Event):
    pass
