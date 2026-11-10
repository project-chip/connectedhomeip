#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import asyncio
import subprocess
import sys
import threading
import unittest
from unittest import mock

from matter.webrtc.libdatachannel_peer_connection import LibdatachannelPeerConnection
from matter.webrtc.libdatachannel_webrtc_client import LibdatachannelWebRTCClient
from matter.webrtc.types import IceCandidate

_FINGERPRINT = ":".join(["AB"] * 32)
_MEDIA_ATTRIBUTES = ["a=sendonly", "a=rtcp-mux", "a=ice-ufrag:ufrag", "a=ice-pwd:icepasswordicepassword01",
                     f"a=fingerprint:sha-256 {_FINGERPRINT}", "a=setup:actpass"]
OFFER_SDP = "\r\n".join([
    "v=0", "o=- 1 1 IN IP4 127.0.0.1", "s=-", "t=0 0", "a=group:BUNDLE video audio",
    "m=video 9 UDP/TLS/RTP/SAVPF 96", "c=IN IP4 0.0.0.0", "a=mid:video", "a=rtpmap:96 H264/90000", *_MEDIA_ATTRIBUTES,
    "m=audio 9 UDP/TLS/RTP/SAVPF 111", "c=IN IP4 0.0.0.0", "a=mid:audio", "a=rtpmap:111 opus/48000/2", *_MEDIA_ATTRIBUTES,
    "",
])
ANSWER_SDP = OFFER_SDP.replace("a=setup:actpass", "a=setup:active")
CANDIDATE_1 = IceCandidate(candidate="candidate:1 1 UDP 2122317823 192.168.1.10 50000 typ host", sdpMid="video")
CANDIDATE_2 = IceCandidate(candidate="candidate:2 1 UDP 2122317823 192.168.1.10 50002 typ host", sdpMid="audio")
CANDIDATE_3 = IceCandidate(candidate="candidate:3 1 UDP 2122317823 192.168.1.10 50004 typ host", sdpMid=None)

# Runs in a child process: libdatachannel aborts the process on a remote candidate without a remote description.
_REAL_NATIVE_SCRIPT = """
import asyncio
import sys
from unittest import mock

from matter.webrtc.libdatachannel_peer_connection import LibdatachannelPeerConnection
from matter.webrtc.types import IceCandidate

offer_sdp, candidate = sys.argv[1], sys.argv[2]
with mock.patch("matter.webrtc.libdatachannel_peer_connection.WebRTCProviderCommand"):
    peer = LibdatachannelPeerConnection(node_id=1, fabric_index=1, endpoint=1, event_loop=asyncio.new_event_loop())
peer.on_remote_ice_candidates(1, [IceCandidate(candidate=candidate, sdpMid="video")])
peer.set_remote_offer(offer_sdp)
peer.on_remote_ice_candidates(1, [IceCandidate(candidate=candidate, sdpMid="audio")])
peer.close()
"""


def native_add(candidate: IceCandidate):
    return mock.call.add_ice_candidate(candidate.candidate, candidate.sdpMid or "video")


class TestRemoteIceCandidates(unittest.TestCase):
    """Remote ICE candidates reach the native client only after a remote description is set."""

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.addCleanup(self.loop.close)
        # WebRTCProviderCommand needs a commissioned DUT; these tests send no commands.
        with mock.patch("matter.webrtc.libdatachannel_peer_connection.WebRTCProviderCommand"):
            self.peer = LibdatachannelPeerConnection(node_id=1, fabric_index=1, endpoint=1, event_loop=self.loop)
        self.addCleanup(self.peer.close)

        # Records the native calls, in order, without reaching libdatachannel.
        self.native = mock.Mock()
        for name in ("set_remote_description", "add_ice_candidate"):
            patcher = mock.patch.object(LibdatachannelWebRTCClient, name, getattr(self.native, name))
            patcher.start()
            self.addCleanup(patcher.stop)

    def deliver_from_another_thread(self, candidates: list[IceCandidate]):
        thread = threading.Thread(target=self.peer.on_remote_ice_candidates, args=(1, candidates))
        thread.start()
        thread.join(timeout=5)
        self.assertFalse(thread.is_alive(), "on_remote_ice_candidates blocked")

    def test_candidates_before_remote_description_are_held(self):
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.native.add_ice_candidate.assert_not_called()

    def test_held_candidates_are_still_queued_for_the_test(self):
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        received = self.loop.run_until_complete(self.peer.get_remote_ice_candidates(timeout_s=5))
        self.assertEqual(received, (1, [CANDIDATE_1]))

    def test_remote_offer_applies_held_candidates_in_order(self):
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_2, CANDIDATE_3])
        self.peer.set_remote_offer(OFFER_SDP)
        self.assertEqual(self.native.mock_calls, [
            mock.call.set_remote_description(OFFER_SDP, "offer"),
            native_add(CANDIDATE_1), native_add(CANDIDATE_2), native_add(CANDIDATE_3),
        ])

    def test_remote_answer_applies_held_candidates(self):
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.set_remote_answer(ANSWER_SDP)
        self.assertEqual(self.native.mock_calls, [
            mock.call.set_remote_description(ANSWER_SDP, "answer"), native_add(CANDIDATE_1),
        ])

    def test_candidates_after_remote_description_are_applied_at_once(self):
        self.peer.set_remote_offer(OFFER_SDP)
        self.native.reset_mock()
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.set_remote_ice_candidates([CANDIDATE_2])
        self.assertEqual(self.native.mock_calls, [native_add(CANDIDATE_1), native_add(CANDIDATE_2)])

    def test_held_candidates_are_applied_once(self):
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.set_remote_offer(OFFER_SDP)
        self.peer.set_remote_offer(OFFER_SDP)
        self.assertEqual(self.native.add_ice_candidate.mock_calls, [mock.call(CANDIDATE_1.candidate, "video")])

    def test_candidate_received_while_remote_description_is_set_is_applied_after_it(self):
        self.native.set_remote_description.side_effect = lambda *_: self.deliver_from_another_thread([CANDIDATE_2])
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.set_remote_offer(OFFER_SDP)
        self.assertEqual(self.native.mock_calls, [
            mock.call.set_remote_description(OFFER_SDP, "offer"), native_add(CANDIDATE_1), native_add(CANDIDATE_2),
        ])

    def test_candidate_received_while_held_candidates_are_applied_is_applied(self):
        def deliver_once(*_):
            if self.native.add_ice_candidate.call_count == 1:
                self.deliver_from_another_thread([CANDIDATE_2])

        self.native.add_ice_candidate.side_effect = deliver_once
        self.peer.on_remote_ice_candidates(1, [CANDIDATE_1])
        self.peer.set_remote_offer(OFFER_SDP)
        self.assertEqual(self.native.mock_calls, [
            mock.call.set_remote_description(OFFER_SDP, "offer"), native_add(CANDIDATE_1), native_add(CANDIDATE_2),
        ])


class TestRemoteIceCandidatesWithLibdatachannel(unittest.TestCase):
    def test_candidates_before_remote_offer_do_not_abort(self):
        result = subprocess.run([sys.executable, "-c", _REAL_NATIVE_SCRIPT, OFFER_SDP, CANDIDATE_1.candidate],
                                capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0, f"child process failed:\n{result.stderr[-2000:]}")


if __name__ == "__main__":
    unittest.main()
