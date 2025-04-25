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

from ctypes import CFUNCTYPE, POINTER, c_char_p, c_int, c_uint32



class ErrorType(c_int):
    InvalidLocalOffer = 0
    InvalidRemoteOffer = 1


# Define the callback types using ctypes
SdpOfferCallback_t = CFUNCTYPE(
    None,  # void return
    c_char_p, c_uint32  # sdp offer and peer id
)

SdpAnswerCallback_t = CFUNCTYPE(
    None,  # void return
    c_char_p, c_uint32  # sdp answer and peer id
)

StatsCallback_t = CFUNCTYPE(
    None,  # void return
    c_char_p, c_uint32  # stats and peer id
)

IceCallback_t = CFUNCTYPE(
    None,  # void return
    POINTER(c_char_p), c_uint32  # ice candidate and peer id
)

ErrorCallback_t = CFUNCTYPE(
    None,  # void return
    ErrorType, c_uint32  # error message and peer id
)

PeerConnectedCallback_t = CFUNCTYPE(
    None,  # void return
    c_uint32  # peer id
)

PeerDisconnectedCallback_t = CFUNCTYPE(
    None,  # void return
    c_uint32  # peer id
)
