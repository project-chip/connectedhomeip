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

import logging

from mobly import asserts
from TC_MESSTestBase import MESSAGE_ID_1, MESSAGE_ID_2, MESSTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_MESS_3_3(MatterBaseTest, MESSTestBase):

    def desc_TC_MESS_3_3(self) -> str:
        return "[TC-MESS-3.3] MessageNotPresented Event Verification (DUT as Server)"

    def pics_TC_MESS_3_3(self) -> list[str]:
        return ["MESS.S", "MESS.S.F05", "MESS.S.E03"]

    def steps_TC_MESS_3_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH subscribes to events on the DUT.",
                     is_commissioning=True),
            TestStep(1, "Place the DUT in a muted or do-not-disturb state as per manufacturer documentation.",
                     "DUT is in a muted or do-not-disturb state."),
            TestStep(2, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "AABBCCDDEEFF00112233445566778899, Priority=Low (0), MessageControl with the AudioMessage bit "
                     "(bit 6) set and MessageURI pointing to a valid audio resource.",
                     "Verify that a successful (status 0) response is received."),
            TestStep(3, "TH waits for the DUT to drop the Low priority audio message.",
                     "Verify that a MessageNotPresented event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899 and RemovedFromQueue=true."),
            TestStep(4, "TH reads the Messages attribute.",
                     "Verify that the message with MessageID=AABBCCDDEEFF00112233445566778899 is no longer present "
                     "in the messages list."),
            TestStep(5, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "BBCCDDEE0011223344556677889900AA, Priority=High (2), MessageControl with the AudioMessage bit "
                     "(bit 6) set and MessageURI pointing to a valid audio resource.",
                     "Verify that a successful (status 0) response is received."),
            TestStep(6, "TH waits for the DUT to defer the High priority audio message.",
                     "Verify that a MessageNotPresented event is generated with MessageID="
                     "BBCCDDEE0011223344556677889900AA and RemovedFromQueue=false."),
            TestStep(7, "TH reads the Messages attribute.",
                     "Verify that the message with MessageID=BBCCDDEE0011223344556677889900AA remains in the "
                     "messages list."),
            TestStep(8, "TH sends a CancelMessagesRequest command to the DUT with MessageIDs="
                     "[BBCCDDEE0011223344556677889900AA].",
                     "Verify that a successful (status 0) response is received."),
        ]

    @pixit("audio_uri", str,
           "URI (bdx: or https:) of a valid audio resource in a format the DUT supports")
    @pixit("event_timeout_sec", int, "Seconds to wait for each MessageNotPresented event",
           required=False, default=60)
    @run_if_endpoint_matches(
        has_feature(Clusters.Messages, Clusters.Messages.Bitmaps.Feature.kAudioMessages))
    async def test_TC_MESS_3_3(self):
        cluster = Clusters.Messages
        events = cluster.Events
        endpoint = self.get_endpoint()

        audio_uri = self.pixit("audio_uri")
        timeout_sec = self.pixit("event_timeout_sec")
        audio_control = cluster.Bitmaps.MessageControlBitmap.kAudioMessage

        self.step(0)
        event_handler = await self.start_message_event_subscription(endpoint)

        self.step(1)
        self.wait_for_user_input(
            prompt_msg="Place the DUT in a muted or do-not-disturb state, per the manufacturer's documentation, "
                       "then press Enter.\n")

        # A Low priority audio message cannot be presented while muted, so the DUT drops it.
        self.step(2)
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_1, message_control=audio_control,
            priority=cluster.Enums.MessagePriorityEnum.kLow, message_uri=audio_uri)

        self.step(3)
        event_data = self.wait_for_message_event(
            event_handler, events.MessageNotPresented, MESSAGE_ID_1, timeout_sec=timeout_sec)
        asserts.assert_true(
            event_data.removedFromQueue,
            "RemovedFromQueue should be true for a dropped Low priority message")

        self.step(4)
        await self.assert_message_in_list(endpoint, MESSAGE_ID_1, expected_present=False)

        # A High priority audio message is deferred rather than dropped, so it stays queued.
        self.step(5)
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_2, message_control=audio_control,
            priority=cluster.Enums.MessagePriorityEnum.kHigh, message_uri=audio_uri)

        self.step(6)
        event_data = self.wait_for_message_event(
            event_handler, events.MessageNotPresented, MESSAGE_ID_2, timeout_sec=timeout_sec)
        asserts.assert_false(
            event_data.removedFromQueue,
            "RemovedFromQueue should be false for a deferred High priority message")

        self.step(7)
        await self.assert_message_in_list(endpoint, MESSAGE_ID_2, expected_present=True)

        self.step(8)
        await self.send_cancel_message(endpoint, [MESSAGE_ID_2])

        event_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
