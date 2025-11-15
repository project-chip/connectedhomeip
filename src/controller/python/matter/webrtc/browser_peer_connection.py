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
import logging
from typing import Optional

from .async_websocket_client import AsyncWebSocketClient
from .browser_webrtc_client import BrowserWebRTCClient
from .command import WebRTCProviderCommand
from .types import Events, IceCandidate, IceCandidateList, PeerConnectionState
from .utils import AsyncEventQueue

LOGGER = logging.getLogger(__name__)


class BrowserPeerConnection(BrowserWebRTCClient):
    """Manages a browser WebRTC peer connection communicating via websocket, handling events
    such as offers, answers, ICE candidates, and state changes including events through matter
    commands. Provides asynchronous methods to interact with the connection, including session
    setup, description and ice candidate setting and handle incoming commands from matter.

    Args:
        node_id (int): Peer node id.
        fabric_index (int): The local fabric index.
        endpoint (int): The peer endpoint id.
        event_loop (Optional[asyncio.AbstractEventLoop]): Optional event loop to use for async operations.
            Defaults to the running loop.

    Returns:
        BrowserPeerConnection
    """

    def __init__(
        self,
        node_id: int,
        fabric_index: int,
        endpoint: int,
        ws_client: AsyncWebSocketClient,
        event_loop: Optional[asyncio.AbstractEventLoop] = None,
    ):
        super().__init__(ws_client=ws_client, id=node_id)
        self.event_loop = event_loop or asyncio.get_running_loop()

        self._ice_candidates = IceCandidateList()
        # Sentinel to indicate end of ice candidates collection.
        self._ice_sentinel = "___ICE_DONE___"

        self._peer_state = PeerConnectionState.NEW

        # remote events to capture remote events from matter commands
        self._remote_events = {
            Events.OFFER: AsyncEventQueue(loop=event_loop),
            Events.ANSWER: AsyncEventQueue(loop=event_loop),
            Events.ICE_CANDIDATE: AsyncEventQueue(loop=event_loop),
        }

        # local events to capture local events from webrtc client library
        self._local_events = {
            Events.OFFER: AsyncEventQueue(loop=event_loop),
            Events.ANSWER: AsyncEventQueue(loop=event_loop),
            Events.ICE_CANDIDATE: AsyncEventQueue(loop=event_loop),
            Events.PEER_CONNECTION_STATE: AsyncEventQueue(loop=event_loop),
        }

        # Initialise WebRTCProviderClient
        self._command_sender = WebRTCProviderCommand()
        self._command_sender.init(node_id, fabric_index, endpoint)

        # Set native callbacks
        self.on_ice_candidate(self.on_ice_candidate_cb)
        self.on_local_description(self.on_local_description_cb)
        self.on_gathering_complete(self.on_gathering_complete_cb)
        self.on_state_change(self.on_state_change_cb)

    async def get_local_ice_candidates(
        self, timeout_s: Optional[int] = None, wait_for_gathering_complete: bool = False
    ) -> list[IceCandidate]:
        """Retrieves the local ICE candidates for the WebRTC peer connection.
        Ensure that the WebRTC peer connection is properly initialized before calling this function.

        Args:
            timeout_s (Optional[int]): The maximum time in seconds to wait for a candidate.
            If None, the function will wait indefinitely.
            wait_for_gathering_complete (bool): When True, Waits for gathering complete to return all
            ice candidates. When False, returns the first 3 candidates whenever available. Default is False.

        Returns:
            list: A list of local ICE candidates.

        Raises:
            asyncio.TimeoutError: If no candidate is received within the specified timeout period.
        """
        if self._ice_candidates.gathering_complete:
            return self._ice_candidates.candidates

        while wait_for_gathering_complete or len(self._ice_candidates.candidates) < 3:
            candidate = await self._local_events[Events.ICE_CANDIDATE].get(timeout_s)
            if candidate == self._ice_sentinel:
                self._ice_candidates.gathering_complete = True
                break
            self._ice_candidates.candidates.append(candidate)
        return self._ice_candidates.candidates

    async def set_remote_ice_candidates(self, remote_candidates: list[IceCandidate]) -> None:
        """Sets the remote ICE candidates for the WebRTC peer connection.

        Args:
            remote_candidates (list[IceCandidate]): A list of remote ICE candidates to be set.
        """
        # for candidate in remote_candidates:
        await self.set_remote_icecandidates(remote_candidates)

    async def get_local_answer(self, timeout_sec: Optional[int] = None) -> str:
        """Fetches the local SDP answer for the WebRTC peer connection.

        Args:
            timeout_sec (Optional[int]): The maximum time in seconds to wait for a local answer sdp.

        Returns:
            str: The local SDP answer.

        Raises:
            asyncio.TimeoutError: If timeout period is elapsed waiting.
        """
        return await self._local_events[Events.ANSWER].get(timeout_sec)

    async def get_local_description_with_ice_candidates(self) -> str:
        """Retrieves the local SDP including ICE candidates.
        Waits for ICE Gathering Complete first. Then fetches the local description and returns it.

        Returns:
            str: The local SDP description including ICE candidates.

        Raises:
            RuntimeError: If no local sdp is available. This indicates invalid state.
        """
        await self.get_local_ice_candidates(wait_for_gathering_complete=True)
        sdp = await self.get_local_description()
        if len(sdp) == 0:
            raise RuntimeError("Invalid State: No local sdp available")
        return sdp

    async def set_remote_answer(self, answer_sdp: str) -> None:
        """Sets the remote SDP answer for the WebRTC peer connection.

        Args:
            answer_sdp (str): The remote SDP answer to be set.
        """
        await self.set_remote_description(answer_sdp, "answer")

    async def get_local_offer(self, timeout_sec: Optional[int] = None) -> str:
        """Fetches the local SDP offer for the WebRTC peer connection.

        Args:
            timeout_sec (Optional[int]): The maximum time in seconds to wait for a local offer sdp.

        Returns:
            str: The local SDP offer.

        Raises:
            asyncio.TimeoutError: If timeout period is elapsed waiting.
        """

        return await self._local_events[Events.OFFER].get(timeout_sec)

    async def set_remote_offer(self, offer_sdp: str) -> None:
        """Sets the remote SDP offer for the WebRTC peer connection.

        Args:
            offer_sdp (str): The remote SDP offer to be set.
        """
        await self.set_remote_description(offer_sdp, "offer")

    async def get_remote_offer(self, timeout_s: Optional[int] = None) -> tuple[int, str]:
        """Waits for a remote SDP offer to be received through a matter command.

        Args:
            timeout_s (Optional[int]): The maximum time in seconds to wait for a remote offer.
            If None, the function will wait indefinitely.

        Returns:
            tuple[int, str]: A tuple containing the session ID and the remote SDP offer.

        Raises:
            asyncio.TimeoutError: If no remote offer is received within the specified timeout period.
        """
        LOGGER.debug("Waiting for remote offer")
        return await self._remote_events[Events.OFFER].get(timeout_s)

    async def get_remote_answer(self, timeout_s: Optional[int] = None) -> tuple[int, str]:
        """Waits for a remote SDP answer to be received through a matter command.

        Args:
            timeout_s (Optional[int]): The maximum time in seconds to wait for a remote offer.
            If None, the function will wait indefinitely.

        Returns:
            tuple[int, str]: A tuple containing the session ID and the remote SDP offer.

        Raises:
            asyncio.TimeoutError: If no remote offer is received within the specified timeout period.
        """
        LOGGER.debug("Waiting for remote answer")
        return await self._remote_events[Events.ANSWER].get(timeout_s)

    async def get_remote_ice_candidates(self, timeout_s: Optional[int] = None) -> tuple[int, list[IceCandidate]]:
        """Waits for a list of remote ICE Candidates to be received through a matter command.

        Args:
            timeout_s (Optional[int]): The maximum time in seconds to wait for a list of remote candidates.
            If None, the function will wait indefinitely.

        Returns:
            tuple[int, list[str]]: A tuple containing the session ID and list of ice candidate strings.

        Raises:
            asyncio.TimeoutError: If no remote offer is received within the specified timeout period.
        """
        LOGGER.debug("waiting for remote iceCandidates")
        return await self._remote_events[Events.ICE_CANDIDATE].get(timeout_s)

    async def check_for_session_establishment(self) -> bool:
        """Monitors the peer connection state and determines if a session has been successfully established.

        This function checks the state of the peer connection by waiting and processing pending events from
        the event queue.

        Returns:
            bool: `True` if the connection state reaches `CONNECTED`, `False` otherwise.
        """
        # Sync all pending events first
        event_queue = self._local_events[Events.PEER_CONNECTION_STATE]
        while not event_queue.empty():
            self._peer_state = event_queue.get_nowait()
        while True:
            if self._peer_state in [
                PeerConnectionState.CLOSED,
                PeerConnectionState.DISCONNECTED,
                PeerConnectionState.FAILED,
                PeerConnectionState.INVALID,
            ]:
                return False
            elif self._peer_state == PeerConnectionState.CONNECTED:
                return True
            self._peer_state = await event_queue.get(timeout=30)

    async def close(self):
        """Closes the peer connection"""
        await super().close_peer_connection()

    async def is_session_connected(self) -> bool:
        """Check if the peer connection session is currently connected.

        This function clears any old events related to the peer connection state,
        retrieves the latest state of the peer connection state to determine if
        the session is active.

        Returns:
            bool: True if the peer connection is in the `CONNECTED` state, False otherwise.
        """

        # clear old events
        self._local_events[Events.PEER_CONNECTION_STATE].clear()
        # get latest state
        self._peer_state = PeerConnectionState(await self.get_peer_connection_state())
        return self._peer_state == PeerConnectionState.CONNECTED

    async def send_command(self, *args, **kwargs):
        """Sends a command using WebRTC Provdier Client. **This should be used to send
        ProvideOffer and SolicitOffer Commands only as these commands require custom
        response handling.** Rest of the remaining commands can be sent via the default command sender.

        Parameters:
            endpoint (int): The endpoint ID of the peer device for the command.
            cmd (ClusterObjects.ClusterCommand): Payload of the command to be sent.

        Returns:
            Response Type if applicable.
        """
        return await self._command_sender.send_webrtc_provider_command(*args, **kwargs)

    # Callbacks from WebRTC Client Library
    def on_ice_candidate_cb(self, candidate: str, mid: str, index: int) -> None:
        """Callback function called when a local ICE candidate is received."""
        if not len(candidate):
            return
        self._local_events[Events.ICE_CANDIDATE].put(IceCandidate(candidate=candidate, sdpMid=mid, sdpMLineIndex=index))

    def on_local_description_cb(self, sdp: str, type: str) -> None:
        """Callback function called when a local SDP description is received."""
        event = Events.OFFER if type.lower() == "offer" else Events.ANSWER
        self._local_events[event].put(sdp)

    def on_gathering_complete_cb(self) -> None:
        """Callback function called when the ICE candidate gathering process is complete."""
        self._local_events[Events.ICE_CANDIDATE].put(self._ice_sentinel)

    def on_state_change_cb(self, state: str) -> None:
        """Callback function called when the state of the WebRTC peer connection changes."""
        self._local_events[Events.PEER_CONNECTION_STATE].put(PeerConnectionState(state))

    # Callbacks from WebRTC Requestor Delegates
    def on_remote_offer(self, sessionId: int, offer_sdp: str) -> None:
        """Callback function called when a remote SDP offer is received through a matter command."""
        self._remote_events[Events.OFFER].put((sessionId, offer_sdp))

    def on_remote_answer(self, sessionId: int, answer_sdp: str) -> None:
        """Callback function called when a remote SDP answer is received through a matter command."""
        self._remote_events[Events.ANSWER].put((sessionId, answer_sdp))

    def on_remote_ice_candidates(self, sessionId: int, candidates: list[IceCandidate]) -> None:
        """Callback function called when a remote ICE candidates are received through a matter command.

        Implements trickle ICE by immediately applying received candidates to the peer connection.
        Also stores them in the event queue for tests that may need to wait for and verify them.
        """
        # Immediately apply candidates for trickle ICE support
        LOGGER.debug(f"Applying {len(candidates)} candidates for trickle ICE support: {candidates}")
        self.set_remote_ice_candidates(candidates)

        # Also put in event queue for any waiting consumers
        self._remote_events[Events.ICE_CANDIDATE].put((sessionId, candidates))
