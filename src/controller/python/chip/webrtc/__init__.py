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

from ctypes import CFUNCTYPE, c_char_p, c_void_p
from .native import PyChipError

import chip.native

lib = chip.native.GetLibraryHandle(chip.native.HandleFlags(0))

WebRTCClientHandle = c_void_p

LocalDescriptionCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)
IceCandidateCallbackType = CFUNCTYPE(None, c_char_p, c_char_p, c_void_p)

lib.webrtc_client_create.restype = WebRTCClientHandle
lib.webrtc_client_destroy.argtypes = [WebRTCClientHandle]

lib.webrtc_client_create_peer_connection.restype = PyChipError
lib.webrtc_client_create_peer_connection.argtypes = [WebRTCClientHandle, c_char_p]
lib.webrtc_client_create_offer.argtypes = [WebRTCClientHandle]
lib.webrtc_client_create_answer.argtypes = [WebRTCClientHandle]
lib.webrtc_client_set_remote_description.argtypes = [WebRTCClientHandle, c_char_p, c_char_p]
lib.webrtc_client_add_ice_candidate.argtypes = [WebRTCClientHandle, c_char_p, c_char_p]

lib.webrtc_client_set_local_description_callback.argtypes = [WebRTCClientHandle, LocalDescriptionCallbackType, c_void_p]
lib.webrtc_client_set_ice_candidate_callback.argtypes = [WebRTCClientHandle, IceCandidateCallbackType, c_void_p]


class WebRTCClient:
    def __init__(self):
        self._handle = lib.webrtc_client_create()
        self._local_desc_cb = None
        self._ice_cand_cb = None

    def __del__(self):
        if self._handle:
            lib.webrtc_client_destroy(self._handle)
            self._handle = None

    def create_peer_connection(self, stun_url):
        return lib.webrtc_client_create_peer_connection(self._handle, stun_url.encode('utf-8'))

    def create_offer(self):
        lib.webrtc_client_create_offer(self._handle)

    def create_answer(self):
        lib.webrtc_client_create_answer(self._handle)

    def set_remote_description(self, sdp, type_):
        lib.webrtc_client_set_remote_description(self._handle, sdp.encode('utf-8'), type_.encode('utf-8'))

    def add_ice_candidate(self, candidate, mid):
        lib.webrtc_client_add_ice_candidate(self._handle, candidate.encode('utf-8'), mid.encode('utf-8'))

    def on_local_description(self, callback):
        def c_callback(sdp, type_, user_data):
            callback(sdp.decode('utf-8'), type_.decode('utf-8'))
        self._local_desc_cb = LocalDescriptionCallbackType(c_callback)
        lib.webrtc_client_set_local_description_callback(self._handle, self._local_desc_cb, None)

    def on_ice_candidate(self, callback):
        def c_callback(candidate, mid, user_data):
            callback(candidate.decode('utf-8'), mid.decode('utf-8'))
        self._ice_cand_cb = IceCandidateCallbackType(c_callback)
        lib.webrtc_client_set_ice_candidate_callback(self._handle, self._ice_cand_cb, None)
