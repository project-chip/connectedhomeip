# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tests for the NAN simulator inside the wpa_supplicant mock."""

import asyncio
import sys
import unittest

# The mock is Linux-only: wifi.py imports sdbus, which is only installed on Linux.
try:
    from matter.testing.linux.wifi import NANSimulator
except ImportError:
    NANSimulator = None


class RecordingSignal:
    """Stands in for a sdbus signal, recording what would have been emitted."""

    def __init__(self) -> None:
        self.emissions: list[dict] = []

    def emit(self, args: dict) -> None:
        self.emissions.append(args)


class RecordingInterface:
    """The parts of WpaSupplicantMock.WpaInterface the simulator touches.

    Using a stand-in keeps these tests off D-Bus entirely: the simulator only
    needs an index, a MAC and the three NAN signals.
    """

    def __init__(self, index: int) -> None:
        self.index = index
        self.mock_mac = f"00:11:22:33:44:{index:02x}"
        self.interface_name_in_sim = ""
        self.NANDiscoveryResult = RecordingSignal()
        self.NANReplied = RecordingSignal()
        self.NANReceive = RecordingSignal()


@unittest.skipIf(NANSimulator is None, "wpa_supplicant mock is only available on Linux")
class TestNANSimulator(unittest.TestCase):

    SERVICE = "_matterc._udp"

    def setUp(self) -> None:
        self.simulator = NANSimulator()
        self.publisher = RecordingInterface(0)
        self.subscriber = RecordingInterface(1)
        self.simulator.register_interface("wlx-app", self.publisher)
        self.simulator.register_interface("wlx-cp", self.subscriber)

    def discover(self, **subscribe_args) -> None:
        """Publish on one interface, then subscribe on the other."""
        self.simulator.on_publish_started("wlx-app", 1, {"srv_name": self.SERVICE, "srv_proto_type": 3})
        asyncio.run(self.simulator.on_subscribe_started(
            "wlx-cp", 1, {"srv_name": self.SERVICE, "srv_proto_type": 3, **subscribe_args}))

    def test_subscriber_is_told_about_the_publisher(self):
        self.discover()
        self.assertEqual(len(self.subscriber.NANDiscoveryResult.emissions), 1)
        self.assertEqual(self.subscriber.NANDiscoveryResult.emissions[0]["peer_addr"],
                         ("s", self.publisher.mock_mac))

    def test_publisher_is_told_the_subscriber_replied(self):
        self.discover()
        self.assertEqual(len(self.publisher.NANReplied.emissions), 1)

    def test_discovery_only_subscriber_does_not_reply(self):
        """A discovery-only subscriber is scanning, so it must not reply.

        wpa_supplicant suppresses the automatic passive-subscriber Follow-up for
        these, because a reply would start session setup with a publisher the
        scan only meant to observe. Discovery itself must still be reported, or
        the scan returns nothing.
        """
        self.discover(discovery_only=True)
        self.assertEqual(len(self.subscriber.NANDiscoveryResult.emissions), 1)
        self.assertEqual(len(self.publisher.NANReplied.emissions), 0)

    def test_discovery_only_false_still_replies(self):
        self.discover(discovery_only=False)
        self.assertEqual(len(self.publisher.NANReplied.emissions), 1)

    def test_data_is_routed_to_the_peer_holding_the_address(self):
        asyncio.run(self.simulator.on_transmit(
            sender_iface=self.subscriber, handle=1, req_instance_id=2,
            peer_addr=self.publisher.mock_mac, ssi=b"payload"))
        self.assertEqual(len(self.publisher.NANReceive.emissions), 1)
        self.assertEqual(self.publisher.NANReceive.emissions[0]["ssi"], ("ay", b"payload"))
        self.assertEqual(len(self.subscriber.NANReceive.emissions), 0)


if __name__ == "__main__":
    sys.exit(unittest.main())
