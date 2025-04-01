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

import chip.webrtc

from chip.webrtc.library_handle import _GetWebRTCLibraryHandle
import chip.webrtc.callbacks

import pdb
import concurrent.futures
import asyncio as aio


def CreateWebrtcClient(client_id):
    handle = _GetWebRTCLibraryHandle()
    return handle.pychip_webrtc_CreateWebrtcClient(client_id)


def InitialiseConnection(Client):
    handle = _GetWebRTCLibraryHandle()
    return handle.pychip_webrtc_InitialiseConnection(Client)


def CloseConnection(Client):
    handle = _GetWebRTCLibraryHandle()
    return handle.pychip_webrtc_CloseConnection(Client)


def DestroyClient(Client):
    handle = _GetWebRTCLibraryHandle()
    return handle.pychip_webrtc_DestroyClient(Client)


def GetStats(Client):
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_GetStats(Client)


def CreateOffer(Client):
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_CreateOffer(Client)
    return chip.webrtc.callbacks.future_offer.result()


def SetAnswer(Client, answer):
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_SetAnswer(Client, answer)


def GetLocalSDP(Client):
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_GetLocalSDP(Client)


def GetCandidates():
    handle = _GetWebRTCLibraryHandle()
    return chip.webrtc.callbacks.future_ice.result()


def SetCandidate(Client, candidate):
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_SetCandidate(Client, candidate)


def SetCallbacks(Client, answer_callback, error_callback, peer_connected_callback, peer_disconnected_callback, stats_callback):
    chip.webrtc.callbacks.future_offer = concurrent.futures.Future()
    chip.webrtc.callbacks.future_ice = concurrent.futures.Future()
    handle = _GetWebRTCLibraryHandle()
    handle.pychip_webrtc_SetCallbacks(Client, chip.webrtc.callbacks.offer_callback, answer_callback,
                                      chip.webrtc.callbacks.ice_callback, error_callback, peer_connected_callback, peer_disconnected_callback, stats_callback)
