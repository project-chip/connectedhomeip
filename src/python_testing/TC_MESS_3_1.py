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

from TC_MESSTestBase import MESSAGE_ID_1, MESSAGE_ID_2, MESSTestBase

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_MESSAGE_TEXT = "Hello from Matter"

# A well-formed RFC 5646 tag from the private-use range, so a conformant DUT will not
# have it in SupportedLanguageCodes.
_UNSUPPORTED_LANGUAGE_CODE = "xx-XX"


class TC_MESS_3_1(MatterBaseTest, MESSTestBase):

    def desc_TC_MESS_3_1(self) -> str:
        return "[TC-MESS-3.1] Spoken Message Delivery Verification (DUT as Server)"

    def pics_TC_MESS_3_1(self) -> list[str]:
        return ["MESS.S", "MESS.S.F04"]

    def steps_TC_MESS_3_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH subscribes to events on the DUT.",
                     is_commissioning=True),
            TestStep(1, "TH reads the SupportedLanguageCodes attribute.",
                     "Verify that at least one language tag is returned. Record the first supported language tag as "
                     "supportedLangCode."),
            TestStep(2, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "AABBCCDDEEFF00112233445566778899, Priority=Low (0), MessageControl with the SpokenMessage bit "
                     "(bit 5) set, MessageText='Hello from Matter' and LanguageCode=supportedLangCode.",
                     "Verify that a successful (status 0) response is received."),
            TestStep(3, "TH waits for the DUT to queue the message.",
                     "Verify that a MessageQueued event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(4, "TH waits for the DUT to begin TTS playback.",
                     "Verify that a MessagePresented event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(5, "TH waits for the DUT to finish TTS playback.",
                     "Verify that a MessageComplete event is generated with MessageID="
                     "AABBCCDDEEFF00112233445566778899."),
            TestStep(6, "TH sends a PresentMessagesRequest command to the DUT with MessageID="
                     "BBCCDDEE0011223344556677889900AA, Priority=Low (0), MessageControl with the SpokenMessage bit "
                     "(bit 5) set, MessageText='Hello from Matter' and LanguageCode set to an unsupported language "
                     "tag (xx-XX).",
                     "Verify that a successful (status 0) response is received and the DUT presents the message "
                     "using its default locale or a best-match language. Verify that MessagePresented and "
                     "MessageComplete events are generated."),
        ]

    @pixit("tts_timeout_sec", int, "Seconds to wait for each text-to-speech lifecycle event",
           required=False, default=60)
    @run_if_endpoint_matches(
        has_feature(Clusters.Messages, Clusters.Messages.Bitmaps.Feature.kSpokenMessages))
    async def test_TC_MESS_3_1(self):
        cluster = Clusters.Messages
        events = cluster.Events
        endpoint = self.get_endpoint()
        timeout_sec = self.pixit("tts_timeout_sec")

        # Only the SpokenMessage bit is set, so the DUT must render the message via TTS.
        spoken_control = cluster.Bitmaps.MessageControlBitmap.kSpokenMessage

        self.step(0)
        event_handler = await self.start_message_event_subscription(endpoint)

        self.step(1)
        language_codes = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedLanguageCodes)
        matter_asserts.assert_list(language_codes, "SupportedLanguageCodes", min_length=1)
        supported_lang_code = language_codes[0]
        log.info("Using supported language code %s", supported_lang_code)

        self.step(2)
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_1, message_control=spoken_control,
            priority=cluster.Enums.MessagePriorityEnum.kLow,
            message_text=_MESSAGE_TEXT, language_code=supported_lang_code)

        self.step(3)
        self.wait_for_message_event(event_handler, events.MessageQueued, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(4)
        self.wait_for_message_event(event_handler, events.MessagePresented, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(5)
        self.wait_for_message_event(event_handler, events.MessageComplete, MESSAGE_ID_1, timeout_sec=timeout_sec)

        self.step(6)
        # An unsupported LanguageCode must not prevent presentation: the DUT falls back to its
        # default locale or a best match, so the lifecycle events still arrive.
        await self.send_present_message(
            endpoint, message_id=MESSAGE_ID_2, message_control=spoken_control,
            priority=cluster.Enums.MessagePriorityEnum.kLow,
            message_text=_MESSAGE_TEXT, language_code=_UNSUPPORTED_LANGUAGE_CODE)
        self.wait_for_message_event(event_handler, events.MessagePresented, MESSAGE_ID_2, timeout_sec=timeout_sec)
        self.wait_for_message_event(event_handler, events.MessageComplete, MESSAGE_ID_2, timeout_sec=timeout_sec)

        event_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
