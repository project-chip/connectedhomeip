import asyncio
import logging
import threading
from collections import defaultdict

from .library_handle import WebRTCRequestorNativeBindings
from .peer_connection import PeerConnection


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

    def __init__(self, event_loop: asyncio.AbstractEventLoop | None = None):
        super().__init__()
        self.init_webrtc_requestor_server()
        self.set_webrtc_requestor_delegate_callbacks()
        self.event_loop = event_loop or asyncio.get_running_loop()

    def create_peer(self, node_id: int, fabric_index: int, endpoint: int) -> PeerConnection:
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
        peer = PeerConnection(node_id, fabric_index, endpoint, self.event_loop)
        with self._lock:
            self._peerconnection_map[node_id] = peer
            return self._peerconnection_map[node_id]

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
    def get_peer(session_id: int | None = None, node_id: int | None = None) -> PeerConnection | None:
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
    def remove_peer(session_id: int) -> None:
        """Removes the PeerConnection instance associated with the given session ID.
        Also removes the mapping between session ID and node ID.
        """
        try:
            with WebRTCManager._lock:
                node_id = WebRTCManager._node_id_map[session_id]
                del WebRTCManager._node_id_map[session_id]
                del WebRTCManager._peerconnection_map[node_id]
            WebRTCManager._session_id_event_map[session_id].clear()
        except KeyError:
            pass  # Ignore

    def close_all(self) -> None:
        """Closes all existing peer connections and clears the internal mappings."""
        with self._lock:
            self._node_id_map.clear()
            self._peerconnection_map.clear()

    @staticmethod
    def handle_offer(sessionId: int, offerSdp: str) -> int:
        """WebRTC Requestor `HandleOffer` delegate callback"""
        logging.debug(f"handle_offer sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or len(offerSdp) == 0:
            return -1
        peer.on_remote_offer(sessionId, offerSdp.decode("utf-8"))
        return 0

    @staticmethod
    def handle_answer(sessionId: int, answerSdp: str) -> int:
        """WebRTC Requestor `HandleAnswer` delegate callback"""
        logging.debug(f"handle_answer for sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or len(answerSdp) == 0:
            logging.error(f"handle answer failed {answerSdp}")
            return -1
        peer.on_remote_answer(sessionId, answerSdp.decode("utf-8"))
        return 0

    @staticmethod
    def handle_ice_candidates(sessionId: int, iceCandidateList: list[str], iceCandidateSize: int) -> int:
        """WebRTC Requestor `HandleIceCandidates` delegate callback"""
        logging.debug(f"handle_ice_candidates sessionId:{sessionId}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None or iceCandidateSize <= 0:
            return -1
        remote_ice_candidates = [iceCandidateList[i].decode("utf-8") for i in range(iceCandidateSize)]
        peer.on_remote_ice_candidates(sessionId, remote_ice_candidates)
        return 0

    @staticmethod
    def handle_end(sessionId: int, reason: int):
        """WebRTC Requestor `HandleEnd` delegate callback"""
        logging.debug(f"handle_end reason:{reason}")
        peer = WebRTCManager.get_peer(session_id=sessionId)
        if peer is None:
            return -1
        # Deleting PeerConnection will take care of cleanup
        WebRTCManager.remove_peer(sessionId)
        return 0
