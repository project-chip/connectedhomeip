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

import asyncio
import json
import logging
import threading
from collections import defaultdict
from ctypes import string_at
from typing import Optional, Union

from .async_websocket_client import AsyncWebSocketClient
from .browser_peer_connection import BrowserPeerConnection
from .libdatachannel_peer_connection import LibdatachannelPeerConnection
from .library_handle import WebRTCRequestorNativeBindings
from .types import IceCandidate, IceCandidateStruct

LOGGER = logging.getLogger(__name__)

# Websocket server URI for WebRTC communication to browser client.
# Always localhost since the server will be running in the backend container in TH.
_ws_uri = "ws://127.0.0.1/api/v1/ws/webrtc/controller"


class WebRTCManager(WebRTCRequestorNativeBindings):
    """Manages WebRTC peer connections and handles various WebRTC-related events and operations.
    Maintains mappings between session IDs <-> node IDs <-> peer connections.

    Args:
        event_loop (asyncio.AbstractEventLoop | None): An event loop to be used for asynchronous operations.
        Defaults to the running loop
    """

    _node_id_map = {}  # dict[Session-id, Node-id]
    _session_id_event_map = defaultdict(threading.Event)
    _peerconnection_map = {}  # dict[Node-id, PeerConnection]
    _lock = threading.RLock()

    def __init__(self, event_loop: Optional[asyncio.AbstractEventLoop] = None):
        super().__init__()
        self.init_webrtc_requestor_server()
        self.set_webrtc_requestor_delegate_callbacks()
        self.event_loop = event_loop or asyncio.get_running_loop()
        self.ws_client = AsyncWebSocketClient(uri=_ws_uri, message_handler=self.handle_ws_message)

    async def ws_connect(self):
        """Connects to the WebSocket server for WebRTC communication.
        This should be called before any other WebRTC operations."""
        await self.ws_client.connect()

    async def ws_disconnect(self):
        """Disconnects from the WebSocket server for WebRTC communication.
        Should be called when no further WebRTC operations are needed."""
        await self.ws_client.disconnect()

    def create_peer(self, node_id: int, fabric_index: int, endpoint: int) -> LibdatachannelPeerConnection:
        """Creates a new PeerConnection instance and associates it with the given node_id.

        Args:
            node_id (int): Peer node id.
            fabric_index (int): The local fabric index.
            endpoint (int): The peer endpoint id.

        Returns:
            PeerConnection: The newly created PeerConnection instance.

        Raises:
            ValueError: If a PeerConnection already exists for the given node_id.

        `session_id_created` has to be called after this method to associate session ID with node ID.
        """
        if node_id in self._peerconnection_map:
            raise ValueError(f"Peer with NodeId:{node_id} already exists")
        peer = LibdatachannelPeerConnection(node_id, fabric_index, endpoint, self.event_loop)
        with self._lock:
            self._peerconnection_map[node_id] = peer
            return self._peerconnection_map[node_id]

    async def create_browser_peer(
        self, media_direction: dict[str, str], node_id: int, fabric_index: int, endpoint: int
    ) -> BrowserPeerConnection:
        """Creates a new PeerConnection instance and associates it with the given node_id.

        Args:
            media_direction (dict[key, value]): Direction for peer transceivers.
                example: {"audio": "sendrecv", "video": "sendrecv"}
                key: audio|video
                value: sendrecv|sendonly|recvonly
            node_id (int): Peer node id.
            fabric_index (int): The local fabric index.
            endpoint (int): The peer endpoint id.

        Returns:
            PeerConnection: The newly created PeerConnection instance.

        Raises:
            ValueError: If a PeerConnection already exists for the given node_id.

        `session_id_created` has to be called after this method to associate session ID with node ID.
        """
        if node_id in self._peerconnection_map:
            raise ValueError(f"Peer with NodeId:{node_id} already exists")
        peer = BrowserPeerConnection(node_id, fabric_index, endpoint, self.ws_client, self.event_loop)
        with self._lock:
            self._peerconnection_map[node_id] = peer
        await peer.create_peer_connection(media_direction)
        return peer

    def session_id_created(self, session_id: int, node_id: int) -> None:
        """Associates a session ID with a node ID.
        Has to be immediately called when the session ID is created for the node.
        """
        with self._lock:
            if session_id in self._node_id_map:
                raise ValueError(f"Peer with SessionId:{session_id} already exists")
            self._node_id_map[session_id] = node_id
        self._session_id_event_map[session_id].set()

    @staticmethod
    def get_peer(
        session_id: Optional[int] = None, node_id: Optional[int] = None
    ) -> Union[LibdatachannelPeerConnection, BrowserPeerConnection, None]:
        """Retrieves the PeerConnection instance associated with the given session ID or node ID."""
        if session_id is None and node_id is None:
            raise ValueError("session_id and node_id both are None")

        # try returning with session id first
        if session_id is not None:
            try:
                event = WebRTCManager._session_id_event_map[session_id]
                # wait for event trigger till 5sec to avoid premature access of session_id.
                if not event.wait(5):
                    return None
                with WebRTCManager._lock:
                    node_id = WebRTCManager._node_id_map[session_id]
                    return WebRTCManager._peerconnection_map[node_id]
            except KeyError:
                return None

        # try directly accessing peer from node_id
        elif node_id is not None:
            try:
                with WebRTCManager._lock:
                    return WebRTCManager._peerconnection_map[node_id]
            except KeyError:
                return None

    @staticmethod
    async def remove_peer(session_id: int) -> None:
        """Removes the PeerConnection instance associated with the given session ID.
        Also removes the mapping between session ID and node ID.
        """
        try:
            with WebRTCManager._lock:
                node_id = WebRTCManager._node_id_map[session_id]
                peer = WebRTCManager._peerconnection_map[node_id]
                if isinstance(peer, BrowserPeerConnection):
                    await peer.close()
                del WebRTCManager._peerconnection_map[node_id]
                del WebRTCManager._node_id_map[session_id]
            WebRTCManager._session_id_event_map[session_id].clear()
        except KeyError:
            pass  # Ignore

    async def close_all(self) -> None:
        """Closes all existing peer connections and clears the internal mappings."""
        self.shutdown_webrtc_requestor_server()
        session_ids = list(WebRTCManager._node_id_map.keys())
        for session_id in session_ids:
            await WebRTCManager.remove_peer(session_id)

    @staticmethod
    def handle_offer(sessionId: int, offerSdp: bytes) -> int:
        """WebRTC Requestor `HandleOffer` delegate callback"""
        LOGGER.debug(f"handle_offer sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or len(offerSdp) == 0:
            return -1
        peer.on_remote_offer(sessionId, string_at(offerSdp).decode("utf-8"))
        return 0

    @staticmethod
    def handle_answer(sessionId: int, answerSdp: bytes) -> int:
        """WebRTC Requestor `HandleAnswer` delegate callback"""
        LOGGER.debug(f"handle_answer for sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or len(answerSdp) == 0:
            LOGGER.error(f"handle answer failed {answerSdp}")
            return -1
        peer.on_remote_answer(sessionId, string_at(answerSdp).decode("utf-8"))
        return 0

    @staticmethod
    def handle_ice_candidates(sessionId: int, iceCandidateList: list[IceCandidateStruct], iceCandidateSize: int) -> int:
        """WebRTC Requestor `HandleIceCandidates` delegate callback"""
        LOGGER.debug(f"handle_ice_candidates sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or iceCandidateSize <= 0:
            return -1

        remote_ice_candidates = []
        for i in range(iceCandidateSize):
            candidate = iceCandidateList[i]
            remote_ice_candidates.append(
                IceCandidate(
                    candidate=string_at(candidate.candidate).decode("utf-8"),
                    sdpMid=string_at(candidate.sdpMid).decode("utf-8") if candidate.sdpMid else None,
                    sdpMLineIndex=candidate.sdpMLineIndex if candidate.sdpMLineIndex != -1 else None,
                )
            )
        peer.on_remote_ice_candidates(sessionId, remote_ice_candidates)
        return 0

    @staticmethod
    def handle_end(sessionId: int, reason: int):
        """WebRTC Requestor `HandleEnd` delegate callback"""
        LOGGER.debug(f"handle_end reason:{reason}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None:
            return -1
        # Deleting PeerConnection will take care of cleanup
        asyncio.run_coroutine_threadsafe(WebRTCManager.remove_peer(sessionId), peer.event_loop).result(timeout=10)
        peer.on_remote_end(sessionId, reason)
        return 0

    def handle_ws_message(self, message):
        """Handles incoming WebSocket messages from the browser client"""
        LOGGER.debug(f"handle_ws_message {message}")
        message = json.loads(message)
        if "sessionId" not in message:
            raise RuntimeError("sessionId missing in message")
        node_id = message["sessionId"]

        peer = WebRTCManager.get_peer(node_id=node_id)
        if peer is None or not isinstance(peer, BrowserPeerConnection):
            raise RuntimeError(f"Browser Peer with node_id:{node_id} not found")

        peer.handle_messsage(message)
