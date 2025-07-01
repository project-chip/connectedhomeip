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

from ctypes import CFUNCTYPE, c_char_p, c_void_p

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments, PyChipError

LocalDescriptionCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
IceCandidateCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)

def _GetWebRTCLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with webrtc method initialization.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
      """

    # Getting a handle without requiring init, as webrtc methods
    # do not require chip stack startup
    handle = GetLibraryHandle(HandleFlags(0))

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_webrtc_CreateWebrtcClient.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_webrtc_client_create',
                   ctypes.c_void_p, [None])
        setter.Set('pychip_webrtc_client_destroy',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_create_peer_connection',
                   PyChipError, [c_void_p, c_char_p])
        setter.Set('pychip_webrtc_client_create_offer',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_create_answer',
                   ctypes.c_void_p, [c_void_p])
        setter.Set('pychip_webrtc_client_set_remote_description',
                   ctypes.c_void_p, [c_void_p, c_char_p, c_char_p])
        setter.Set('pychip_webrtc_client_add_ice_candidate',
                   ctypes.c_void_p, [c_void_p, c_char_p, c_char_p])
        setter.Set('pychip_webrtc_client_set_local_description_callback',
                   ctypes.c_void_p, [c_void_p, LocalDescriptionCallbackType, c_void_p])
        setter.Set('pychip_webrtc_client_set_ice_candidate_callback',
                   ctypes.c_void_p, [c_void_p, IceCandidateCallbackType, c_void_p])

    return handle
