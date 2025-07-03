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
from ctypes import CFUNCTYPE, POINTER, c_char_p, c_int, c_uint8, c_uint64, c_void_p
from dataclasses import dataclass, field
from enum import Enum, auto

WebRTCClientHandle = c_void_p
LocalDescriptionCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
IceCandidateCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
GatheringCompleteCallbackType = CFUNCTYPE(None)
StateChangeCallback = CFUNCTYPE(None, c_int)

# Callback types for WebRTCRequestor server
OnOfferCallbackFunct = CFUNCTYPE(c_int, c_uint64, c_char_p)
OnAnswerCallbackFunct = CFUNCTYPE(c_int, c_uint64, c_char_p)
OnICECandidatesCallbackFunct = CFUNCTYPE(c_int, c_uint64, POINTER(c_char_p), c_int)
OnEndCallbackFunct = CFUNCTYPE(c_int, c_uint64, c_uint8)


class PeerConnectionState(Enum):
    NEW = 0
    CONNECTING = 1
    CONNECTED = 2
    DISCONNECTED = 3
    FAILED = 4
    CLOSED = 5
    INVALID = 6

    @classmethod
    def _missing_(cls, value):
        return cls.INVALID


class Events(Enum):
    OFFER = auto()
    ANSWER = auto()
    ICE_CANDIDATE = auto()
    PEER_CONNECTION_STATE = auto()


@dataclass
class IceCandiate:
    candidate: str
    mid: str


@dataclass
class IceCandidateList:
    candidates: list[IceCandiate] = field(default_factory=list)
    gathering_complete: bool = False
