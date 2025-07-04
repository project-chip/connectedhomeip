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

from .library_handle import _GetWebRTCLibraryHandle
from .types import GatheringCompleteCallbackType, IceCandidateCallbackType, LocalDescriptionCallbackType, StateChangeCallback


class WebRTCClient:
    def __init__(self):
        self._lib = _GetWebRTCLibraryHandle()
        self._handle = self._lib.pychip_webrtc_client_create()
        self._local_desc_cb = None
        self._ice_cand_cb = None

    def __del__(self):
        if self._handle:
            self._lib.pychip_webrtc_client_destroy(self._handle)
            self._handle = None

    def create_peer_connection(self, stun_url):
        return self._lib.pychip_webrtc_client_create_peer_connection(self._handle, stun_url.encode("utf-8"))

    def create_offer(self):
        self._lib.pychip_webrtc_client_create_offer(self._handle)

    def create_answer(self):
        self._lib.pychip_webrtc_client_create_answer(self._handle)

    def set_remote_description(self, sdp, type_):
        self._lib.pychip_webrtc_client_set_remote_description(self._handle, sdp.encode("utf-8"), type_.encode("utf-8"))

    def add_ice_candidate(self, candidate, mid):
        self._lib.pychip_webrtc_client_add_ice_candidate(self._handle, candidate.encode("utf-8"), mid.encode("utf-8"))

    def on_local_description(self, callback):
        def c_callback(sdp, type, user_data):
            callback(sdp.decode("utf-8"), type.decode("utf-8"))

        self._local_desc_cb = LocalDescriptionCallbackType(c_callback)
        self._lib.pychip_webrtc_client_set_local_description_callback(self._handle, self._local_desc_cb, None)

    def on_ice_candidate(self, callback):
        def c_callback(candidate, mid, user_data):
            callback(candidate.decode("utf-8"), mid.decode("utf-8"))

        self._ice_cand_cb = IceCandidateCallbackType(c_callback)
        self._lib.pychip_webrtc_client_set_ice_candidate_callback(self._handle, self._ice_cand_cb, None)

    def get_local_description(self):
        return self._lib.pychip_webrtc_get_local_description(self._handle).decode("utf-8")

    def get_peer_connection_state(self):
        return self._lib.pychip_webrtc_get_peer_connection_state(self._handle)

    def on_gathering_complete(self, callback):
        def c_callback():
            callback()

        self._gathering_cb = GatheringCompleteCallbackType(c_callback)
        self._lib.pychip_webrtc_client_set_gathering_complete_callback(self._handle, self._gathering_cb)

    def on_state_change(self, callback):
        def c_callback(state):
            callback(state)

        self._on_state_change_cb = StateChangeCallback(c_callback)
        self._lib.pychip_webrtc_client_set_state_change_callback(self._handle, self._on_state_change_cb)
