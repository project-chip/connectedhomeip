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

"""Shared helpers for the spoken and audio Messages cluster test cases.

TC-MESS-3.1, 3.2 and 3.3 all present a message and then follow its lifecycle through
the MessageQueued / MessagePresented / MessageComplete / MessageNotPresented events.
"""

import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import EndpointCheckFunction, has_feature
from matter.testing.event_attribute_reporting import EventSubscriptionHandler

log = logging.getLogger(__name__)

_MESSAGES = Clusters.Messages

# MessageIDs used by the test plan. Both are 16-byte octet strings.
MESSAGE_ID_1 = bytes.fromhex("AABBCCDDEEFF00112233445566778899")
MESSAGE_ID_2 = bytes.fromhex("BBCCDDEE0011223344556677889900AA")


def has_spoken_or_audio_messages() -> EndpointCheckFunction:
    """Accept endpoints implementing either the SpokenMessages or AudioMessages feature."""
    spoken = has_feature(_MESSAGES, _MESSAGES.Bitmaps.Feature.kSpokenMessages)
    audio = has_feature(_MESSAGES, _MESSAGES.Bitmaps.Feature.kAudioMessages)

    def accept(wildcard, endpoint) -> bool:
        return spoken(wildcard, endpoint) or audio(wildcard, endpoint)

    return accept


class MESSTestBase:

    async def start_message_event_subscription(self, endpoint) -> EventSubscriptionHandler:
        """Subscribe to all Messages cluster events on the given endpoint."""
        handler = EventSubscriptionHandler(expected_cluster=_MESSAGES)
        await handler.start(self.default_controller, self.dut_node_id, endpoint)
        return handler

    async def send_present_message(self, endpoint, message_id: bytes, message_control: int,
                                   priority=_MESSAGES.Enums.MessagePriorityEnum.kLow,
                                   message_text: str = "", language_code: str | None = None,
                                   message_uri: str | None = None):
        """Send PresentMessagesRequest and assert it is accepted."""
        return await self.send_single_cmd(
            cmd=_MESSAGES.Commands.PresentMessagesRequest(
                messageID=message_id,
                priority=priority,
                messageControl=message_control,
                startTime=NullValue,
                duration=NullValue,
                messageText=message_text,
                languageCode=language_code,
                messageURI=message_uri),
            endpoint=endpoint)

    async def send_cancel_message(self, endpoint, message_ids: list[bytes]):
        return await self.send_single_cmd(
            cmd=_MESSAGES.Commands.CancelMessagesRequest(messageIDs=message_ids), endpoint=endpoint)

    def wait_for_message_event(self, handler: EventSubscriptionHandler, event, message_id: bytes,
                               timeout_sec: float = 30.0):
        """Wait for ``event`` carrying ``message_id``, ignoring events for other messages.

        Messages left over from an earlier step can still be in flight, so events for other
        MessageIDs are discarded rather than failing the wait.
        """
        deadline = time.monotonic() + timeout_sec
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                asserts.fail(f"Timed out waiting for {event.__name__} with MessageID {message_id.hex()}")
            data = handler.wait_for_event_type_report(event, timeout_sec=remaining)
            if data.messageID == message_id:
                return data
            log.info("Ignoring %s for unrelated MessageID %s", event.__name__, data.messageID.hex())

    async def read_messages_attribute(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_MESSAGES, attribute=_MESSAGES.Attributes.Messages)

    async def assert_message_in_list(self, endpoint, message_id: bytes, expected_present: bool):
        messages = await self.read_messages_attribute(endpoint)
        present = any(message.messageID == message_id for message in messages)
        if expected_present:
            asserts.assert_true(present, f"MessageID {message_id.hex()} should still be in the Messages attribute")
        else:
            asserts.assert_false(present, f"MessageID {message_id.hex()} should no longer be in the Messages attribute")
        return messages
