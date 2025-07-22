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

from ctypes import CDLL, c_char_p, c_int, c_size_t, c_uint8, c_uint16, c_uint32, c_void_p, py_object

from ..clusters.Command import (_OnCommandSenderDoneCallbackFunct, _OnCommandSenderErrorCallbackFunct,
                                _OnCommandSenderResponseCallbackFunct)
from ..native import GetLibraryHandle, HandleFlags, PyChipError
from .types import (GatheringCompleteCallbackType, IceCandidateCallbackType, LocalDescriptionCallbackType, OnAnswerCallbackFunct,
                    OnEndCallbackFunct, OnICECandidatesCallbackFunct, OnOfferCallbackFunct, StateChangeCallback, WebRTCClientHandle)


def _GetWebRTCLibraryHandle() -> CDLL:
    """Get the native library handle with webrtc method initialization.

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
            WebRTCClientHandle,
            LocalDescriptionCallbackType,
            c_void_p,
        ]
        lib.pychip_webrtc_client_set_ice_candidate_callback.argtypes = [WebRTCClientHandle, IceCandidateCallbackType, c_void_p]

        lib.pychip_webrtc_get_local_description.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_get_local_description.restype = c_char_p

        lib.pychip_webrtc_get_peer_connection_state.argtypes = [WebRTCClientHandle]
        lib.pychip_webrtc_get_peer_connection_state.restype = c_int

        lib.pychip_webrtc_client_set_gathering_complete_callback.argtypes = [WebRTCClientHandle, GatheringCompleteCallbackType]
        lib.pychip_webrtc_client_set_state_change_callback.argtypes = [WebRTCClientHandle, StateChangeCallback]

        lib.pychip_webrtc_provider_client_init.argtypes = [WebRTCClientHandle, c_uint32, c_uint8, c_uint16]
        lib.pychip_webrtc_provider_client_init_commandsender_callbacks.argtypes = [
            WebRTCClientHandle,
            _OnCommandSenderResponseCallbackFunct,
            _OnCommandSenderErrorCallbackFunct,
            _OnCommandSenderDoneCallbackFunct,
        ]
        lib.pychip_webrtc_provider_client_send_command.restype = PyChipError
        lib.pychip_webrtc_provider_client_send_command.argtypes = [
            WebRTCClientHandle,
            py_object,
            c_uint16,
            c_uint32,
            c_uint32,
            c_char_p,
            c_size_t,
        ]

    return lib


def get_webrtc_requestor_handle() -> CDLL:
    lib = GetLibraryHandle()

    if not lib.pychip_WebRTCTransportRequestor_InitCallbacks.argtypes:
        lib.pychip_WebRTCTransportRequestor_Init.argtypes = []
        lib.pychip_WebRTCTransportRequestor_InitCallbacks.argtypes = [
            OnOfferCallbackFunct,
            OnAnswerCallbackFunct,
            OnICECandidatesCallbackFunct,
            OnEndCallbackFunct,
        ]
    return lib


class WebRTCRequestorNativeBindings:
    """This class is intended to be used only by WebRTCManager.

    Exposes python methods for accessing native APIs. Also holds
    reference to the callback objects for WebRTC Requestor delegates.
    """

    def __init__(self):
        self.handle_offer_cb = OnOfferCallbackFunct(self.handle_offer)
        self.handle_answer_cb = OnAnswerCallbackFunct(self.handle_answer)
        self.handle_ice_candidates_cb = OnICECandidatesCallbackFunct(self.handle_ice_candidates)
        self.handle_end_cb = OnEndCallbackFunct(self.handle_end)

    def init_webrtc_requestor_server(self):
        handle = get_webrtc_requestor_handle()
        handle.pychip_WebRTCTransportRequestor_Init()

    def set_webrtc_requestor_delegate_callbacks(self):
        handle = get_webrtc_requestor_handle()
        handle.pychip_WebRTCTransportRequestor_InitCallbacks(
            self.handle_offer_cb, self.handle_answer_cb, self.handle_ice_candidates_cb, self.handle_end_cb
        )
