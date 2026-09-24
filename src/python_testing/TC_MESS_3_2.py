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
import time

from mobly import asserts
from TC_MESSTestBase import MESSAGE_ID_1, MESSAGE_ID_2, MESSTestBase

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Devices may truncate audio retrieved from a MessageURI after 20 seconds.
_TRUNCATION_LIMIT_SEC = 20

# The elapsed time measured here is wall time between two subscription reports, not the
# DUT's own playback clock, so it includes fetching the resource, starting playback, and
# delivering both event reports. The allowance covers that overhead while still failing a
# DUT that does not truncate at all, since the audio under test runs well beyond 25s.
_TRUNCATION_TOLERANCE_SEC = 5


class TC_MESS_3_2(MatterBaseTest, MESSTestBase):

    def desc_TC_MESS_3_2(self) -> str:
        return "[TC-MESS-3.2] Audio Message Delivery Verification (DUT as Server)"

    def pics_TC_MESS_3_2(self) -> list[str]:
        return ["MESS.S", "MESS.S.F05"]

    def steps_TC_MESS_3_2(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH subscribes to events on the DUT.",
                     is_commissioning=True),
            TestStep(1, "TH reads the SupportedMimeTypes attribute.",
                     "Verify that at least one MIME type is returned. Record the supported MIME types as "
                     "supportedMimeTypes."),
            TestStep(2, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "AABBCCDDEEFF00112233445566778899, Priority=Low (0), MessageControl with the AudioMessage bit "
                     "(bit 6) set, MessageText='' and MessageURI pointing to a valid audio resource in a format "
                     "from supportedMimeTypes with a duration under 20 seconds.",
                     "Verify that a successful (status 0) response is received."),
            TestStep(3, "TH waits for the DUT to queue the message.",
                     "Verify that a MessageQueued event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(4, "TH waits for the DUT to begin audio playback.",
                     "Verify that a MessagePresented event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(5, "TH waits for the DUT to finish audio playback.",
                     "Verify that a MessageComplete event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(6, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "BBCCDDEE0011223344556677889900AA, Priority=Low (0), MessageControl with the AudioMessage bit "
                     "(bit 6) set, MessageText='' and MessageURI pointing to an audio resource longer than 20 "
                     "seconds.",
                     "Verify that a successful (status 0) response is received."),
            TestStep(7, "TH waits for the DUT to truncate audio playback.",
                     "Verify that a MessageComplete event is generated with MessageID="
                     "BBCCDDEE0011223344556677889900AA within approximately 20 seconds of the corresponding "
                     "MessagePresented event."),
        ]

    @pixit("short_audio_uri", str,
           "URI (bdx: or https:) of an audio resource shorter than 20 seconds, in a format the DUT supports")
    @pixit("long_audio_uri", str,
           "URI (bdx: or https:) of an audio resource comfortably longer than 20 seconds (30s or more is "
           "recommended so truncation is unambiguous), in a format the DUT supports")
    @pixit("audio_timeout_sec", int, "Seconds to wait for each audio lifecycle event",
           required=False, default=60)
    @run_if_endpoint_matches(
        has_feature(Clusters.Messages, Clusters.Messages.Bitmaps.Feature.kAudioMessages))
    async def test_TC_MESS_3_2(self):
        cluster = Clusters.Messages
        events = cluster.Events
        endpoint = self.get_endpoint()

        short_audio_uri = self.pixit("short_audio_uri")
        long_audio_uri = self.pixit("long_audio_uri")
        timeout_sec = self.pixit("audio_timeout_sec")

        # Only the AudioMessage bit is set, so the DUT must play the resource at MessageURI.
        audio_control = cluster.Bitmaps.MessageControlBitmap.kAudioMessage

        self.step(0)
        event_handler = await self.start_message_event_subscription(endpoint)

        self.step(1)
        mime_types = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedMimeTypes)
        matter_asserts.assert_list(mime_types, "SupportedMimeTypes", min_length=1)
        log.info("DUT supports MIME types: %s. Ensure the supplied audio URIs use one of them.", mime_types)

        self.step(2)
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_1, message_control=audio_control,
            priority=cluster.Enums.MessagePriorityEnum.kLow,
            message_text="", message_uri=short_audio_uri)

        self.step(3)
        self.wait_for_message_event(event_handler, events.MessageQueued, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(4)
        self.wait_for_message_event(event_handler, events.MessagePresented, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(5)
        self.wait_for_message_event(event_handler, events.MessageComplete, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(6)
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_2, message_control=audio_control,
            priority=cluster.Enums.MessagePriorityEnum.kLow,
            message_text="", message_uri=long_audio_uri)

        self.step(7)
        # The events carry no timestamps that can be compared directly, so the elapsed wall
        # time between receiving MessagePresented and MessageComplete stands in for the
        # presentation length.
        self.wait_for_message_event(event_handler, events.MessagePresented, MESSAGE_ID_2, timeout_sec=timeout_sec)
        presented_at = time.monotonic()
        self.wait_for_message_event(event_handler, events.MessageComplete, MESSAGE_ID_2, timeout_sec=timeout_sec)
        elapsed = time.monotonic() - presented_at
        log.info("Long audio message completed %.1fs after it was presented", elapsed)
        asserts.assert_less_equal(
            elapsed, _TRUNCATION_LIMIT_SEC + _TRUNCATION_TOLERANCE_SEC,
            f"Audio longer than {_TRUNCATION_LIMIT_SEC}s should have been truncated, but playback ran for "
            f"{elapsed:.1f}s")

        event_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
