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

import ctypes

import chip.native
from ctypes import (CFUNCTYPE, POINTER, _Pointer, c_wchar_p, c_bool, c_char, c_size_t, c_uint8, c_uint32, c_void_p, c_char_p, cast, memmove, py_object,
                    string_at, c_int)
from chip.webrtc.types import (
    SdpOfferCallback_t,
    SdpAnswerCallback_t,
    IceCallback_t,
    ErrorCallback_t,
    PeerConnectedCallback_t,
    PeerDisconnectedCallback_t,
    StatsCallback_t,
)


def _GetWebRTCLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with webrtc method initialization.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
      """

    # Getting a handle without requiring init, as webrtc methods
    # do not require chip stack startup
    handle = chip.native.GetLibraryHandle(chip.native.HandleFlags(0))

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_webrtc_CreateWebrtcClient.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_webrtc_CreateWebrtcClient',
                   ctypes.c_void_p, [c_uint32])
        setter.Set('pychip_webrtc_InitialiseConnection',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_CloseConnection',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_GetStats',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_DestroyClient',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_CreateOffer',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_GetLocalSDP',
                   ctypes.c_char_p, [c_void_p])
        setter.Set('pychip_webrtc_SetAnswer',
                   ctypes.c_void_p, [c_void_p, c_char_p])
        setter.Set('pychip_webrtc_SetCandidate',
                   ctypes.c_void_p, [c_void_p, c_char_p])
        setter.Set('pychip_webrtc_SetCallbacks',
                   ctypes.c_void_p, [c_void_p, SdpOfferCallback_t, SdpAnswerCallback_t, IceCallback_t, ErrorCallback_t, PeerConnectedCallback_t, PeerDisconnectedCallback_t, StatsCallback_t])

    return handle
