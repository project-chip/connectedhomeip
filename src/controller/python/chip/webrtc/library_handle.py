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

from ctypes import CDLL, c_char_p, c_int, c_void_p

from ..native import GetLibraryHandle, HandleFlags, PyChipError
from .types import (
    GatheringCompleteCallbackType,
    IceCandidateCallbackType,
    LocalDescriptionCallbackType,
    StateChangeCallback,
    WebRTCClientHandle,
)


def _GetWebRTCLibraryHandle() -> CDLL:
    """ Get the native library handle with webrtc method initialization.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
    """

    # Getting a handle without requiring init, as webrtc methods
    # do not require chip stack startup
    lib = GetLibraryHandle(HandleFlags(0))

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not lib.pychip_webrtc_client_create.argtypes:
        lib.pychip_webrtc_client_create.restype = WebRTCClientHandle
        lib.pychip_webrtc_client_destroy.argtypes = [WebRTCClientHandle]

        lib.pychip_webrtc_client_create_peer_connection.restype = PyChipError
        lib.pychip_webrtc_client_create_peer_connection.argtypes = [WebRTCClientHandle, c_char_p]
        lib.pychip_webrtc_client_create_offer.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_client_create_answer.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_client_set_remote_description.argtypes = [WebRTCClientHandle, c_char_p, c_char_p]
        lib.pychip_webrtc_client_add_ice_candidate.argtypes = [WebRTCClientHandle, c_char_p, c_char_p]

        lib.pychip_webrtc_client_set_local_description_callback.argtypes = [
            WebRTCClientHandle, LocalDescriptionCallbackType, c_void_p]
        lib.pychip_webrtc_client_set_ice_candidate_callback.argtypes = [WebRTCClientHandle, IceCandidateCallbackType, c_void_p]

        lib.pychip_webrtc_get_local_description.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_get_local_description.restype = c_char_p

        lib.pychip_webrtc_get_peer_connection_state.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_get_peer_connection_state.restype = c_int

        lib.pychip_webrtc_client_set_gathering_complete_callback.argtypes = [WebRTCClientHandle, GatheringCompleteCallbackType]
        lib.pychip_webrtc_client_set_state_change_callback.argtypes = [WebRTCClientHandle, StateChangeCallback]

    return lib
