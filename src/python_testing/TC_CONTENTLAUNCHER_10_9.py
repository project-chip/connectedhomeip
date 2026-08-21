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

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Candidate MIME types used to build a ContentType the DUT does not advertise. The first
# entry absent from AcceptHeader is used for the negative case in step 7.
_UNSUPPORTED_CONTENT_TYPE_CANDIDATES = (
    "application/vnd.matter.test-unsupported",
    "video/x-matter-test-unsupported",
    "audio/x-matter-test-unsupported",
)


class TC_CONTENTLAUNCHER_10_9(MatterBaseTest):

    def desc_TC_CONTENTLAUNCHER_10_9(self) -> str:
        return "[TC-CONTENTLAUNCHER-10.9] Content Queuing Verification"

    def pics_TC_CONTENTLAUNCHER_10_9(self) -> list[str]:
        return ["CONTENTLAUNCHER.S", "CONTENTLAUNCHER.S.F06", "CONTENTLAUNCHER.S.C01.Rsp"]

    def steps_TC_CONTENTLAUNCHER_10_9(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH sends a LaunchURL command to the DUT with a valid URL and QueueType=Replace (0).",
                     "DUT replies with a success response and immediately interrupts any currently playing content "
                     "to play the new content."),
            TestStep(2, "TH sends a LaunchURL command to the DUT with a valid URL and QueueType=Next (1).",
                     "DUT replies with a success response and queues the new content to play next after the current "
                     "content finishes."),
            TestStep(3, "TH sends a LaunchURL command to the DUT with a valid URL and QueueType=Last (2).",
                     "DUT replies with a success response and queues the new content at the end of the playback "
                     "queue."),
            TestStep(4, "TH sends a LaunchURL command to the DUT with a valid URL and a ContentType field matching "
                     "an entry in the AcceptHeader attribute.",
                     "DUT replies with a success response and begins playing the content using the specified "
                     "content type."),
            TestStep(5, "TH sends a LaunchURL command to the DUT with a valid URL and an OffsetMillisecs value.",
                     "DUT replies with a success response and begins playback starting at the specified time "
                     "offset."),
            TestStep(6, "TH sends a LaunchURL command to the DUT with a valid URL and a NextUrl field set to a "
                     "second valid URL.",
                     "DUT replies with a success response, plays the primary URL content, and then automatically "
                     "begins playing the NextUrl content once the primary content completes."),
            TestStep(7, "TH sends a LaunchURL command to the DUT with a valid URL and a ContentType field that does "
                     "NOT match any entry in the AcceptHeader attribute.",
                     "DUT replies with a URLNotAvailable (1) failure response."),
        ]

    async def _launch_url(self, endpoint, expect_success: bool = True, **kwargs):
        cluster = Clusters.ContentLauncher
        response = await self.send_single_cmd(cmd=cluster.Commands.LaunchURL(**kwargs), endpoint=endpoint)
        if expect_success:
            asserts.assert_equal(response.status, cluster.Enums.StatusEnum.kSuccess,
                                 f"LaunchURL({kwargs}) should have succeeded")
        return response

    def _confirm(self, description: str):
        """Ask the operator to confirm playback behaviour the TH cannot observe over the wire."""
        response = self.wait_for_user_input(
            prompt_msg=f"{description} Enter 'y' if this is what happened, or 'n' if it did not.\n",
            prompt_msg_placeholder="y", default_value="y")
        if response is None:
            log.info("No response to the playback confirmation prompt; continuing")
            return
        asserts.assert_equal(response.lower(), "y", f"Operator reported that the DUT did not: {description}")

    @pixit("content_url", str, "A content URL the DUT can play")
    @pixit("next_content_url", str, "A second content URL, used for the NextUrl field in step 6",
           required=False, default="")
    @pixit("offset_millisecs", int, "Playback offset in milliseconds used in step 5",
           required=False, default=5000)
    @run_if_endpoint_matches(
        has_feature(Clusters.ContentLauncher, Clusters.ContentLauncher.Bitmaps.Feature.kContentQueueing))
    async def test_TC_CONTENTLAUNCHER_10_9(self):
        cluster = Clusters.ContentLauncher
        queue_types = cluster.Enums.QueueTypeEnum
        endpoint = self.get_endpoint()

        content_url = self.pixit("content_url")
        next_content_url = self.pixit("next_content_url") or content_url
        offset_millisecs = self.pixit("offset_millisecs")

        self.step(0)
        accept_header = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AcceptHeader)
        log.info("DUT AcceptHeader: %s", accept_header)

        self.step(1)
        await self._launch_url(endpoint, contentURL=content_url, queueType=queue_types.kReplace)
        self._confirm("The DUT should have interrupted any currently playing content and started the new content.")

        self.step(2)
        await self._launch_url(endpoint, contentURL=content_url, queueType=queue_types.kNext)
        self._confirm("The DUT should have queued the new content to play next, without interrupting playback.")

        self.step(3)
        await self._launch_url(endpoint, contentURL=content_url, queueType=queue_types.kLast)
        self._confirm("The DUT should have queued the new content at the end of the playback queue.")

        self.step(4)
        if accept_header:
            await self._launch_url(endpoint, contentURL=content_url, contentType=accept_header[0],
                                   queueType=queue_types.kReplace)
        else:
            log.info("AcceptHeader is empty; no supported ContentType is advertised")
            self.mark_current_step_skipped()

        self.step(5)
        await self._launch_url(endpoint, contentURL=content_url, offsetMillisecs=offset_millisecs,
                               queueType=queue_types.kReplace)
        self._confirm(f"The DUT should have started playback {offset_millisecs} ms into the content.")

        self.step(6)
        await self._launch_url(endpoint, contentURL=content_url, nextUrl=next_content_url,
                               queueType=queue_types.kReplace)
        if next_content_url == content_url:
            log.info("No distinct next_content_url PIXIT was supplied; NextUrl reuses the primary URL")
        self._confirm("The DUT should play the primary URL and then automatically continue with the NextUrl "
                      "content once the primary content completes.")

        self.step(7)
        unsupported_content_type = next(
            (candidate for candidate in _UNSUPPORTED_CONTENT_TYPE_CANDIDATES if candidate not in accept_header), None)
        if unsupported_content_type is None:
            # Every candidate is advertised as supported, so no negative case can be built.
            log.info("DUT advertises all candidate content types; cannot construct an unsupported ContentType")
            self.mark_current_step_skipped()
            return

        response = await self._launch_url(
            endpoint, expect_success=False, contentURL=content_url, contentType=unsupported_content_type,
            queueType=queue_types.kReplace)
        asserts.assert_equal(
            response.status, cluster.Enums.StatusEnum.kURLNotAvailable,
            f"LaunchURL with the unadvertised ContentType {unsupported_content_type} should return URLNotAvailable")


if __name__ == "__main__":
    default_matter_test_main()
