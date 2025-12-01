#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
from ctypes import CFUNCTYPE, POINTER, Structure, c_char_p, c_int, c_uint8, c_uint16, c_void_p
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Any, Optional


class PeerConnectionState(str, Enum):
    NEW = "new"
    CONNECTING = "connecting"
    CONNECTED = "connected"
    DISCONNECTED = "disconnected"
    FAILED = "failed"
    CLOSED = "closed"
    INVALID = "invalid"

    @classmethod
    def _missing_(cls, value):
        return cls.INVALID


class Events(Enum):
    OFFER = auto()
    ANSWER = auto()
    ICE_CANDIDATE = auto()
    PEER_CONNECTION_STATE = auto()
    END = auto()


class IceCandidateStruct(Structure):
    _fields_ = [("candidate", c_char_p), ("sdpMid", c_char_p), ("sdpMLineIndex", c_int)]


@dataclass
class IceCandidate:
    candidate: str
    sdpMid: Optional[str] = None
    sdpMLineIndex: Optional[int] = None


@dataclass
class IceCandidateList:
    candidates: list[IceCandidate] = field(default_factory=list)
    gathering_complete: bool = False


@dataclass
class WebSocketMessage:
    type: str
    sessionId: int
    data: Any = None
    error: Optional[str] = None


WebRTCClientHandle = c_void_p
LocalDescriptionCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
IceCandidateCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
GatheringCompleteCallbackType = CFUNCTYPE(None)
StateChangeCallback = CFUNCTYPE(None, c_char_p)

# Callback types for WebRTCRequestor server
OnOfferCallbackFunct = CFUNCTYPE(c_int, c_uint16, c_char_p)
OnAnswerCallbackFunct = CFUNCTYPE(c_int, c_uint16, c_char_p)
OnICECandidatesCallbackFunct = CFUNCTYPE(c_int, c_uint16, POINTER(IceCandidateStruct), c_int)
OnEndCallbackFunct = CFUNCTYPE(c_int, c_uint16, c_uint8)
