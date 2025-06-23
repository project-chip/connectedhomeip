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
from ctypes import c_char_p, c_uint32, c_void_p

import chip.native
from chip.webrtc.types import (ErrorCallback_t, IceCallback_t, PeerConnectedCallback_t, PeerDisconnectedCallback_t,
                               SdpAnswerCallback_t, SdpOfferCallback_t, StatsCallback_t)


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
        setter.Set('pychip_webrtc_client_create',
                   ctypes.c_void_p, [None])
        setter.Set('pychip_webrtc_client_destroy',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_create_peer_connection',
                   ctypes.c_void_p, [c_void_p, c_char_p])
        setter.Set('pychip_webrtc_client_create_offer',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_create_answer',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_set_remote_description',
                   ctypes.c_void_p, [c_void_p, c_char_p, c_char_p])
        setter.Set('pychip_webrtc_client_add_ice_candidate',
                   ctypes.c_void_p, [c_void_p, c_char_p, c_char_p])

    return handle
