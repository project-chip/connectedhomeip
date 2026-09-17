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
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_NON_SUCCESS_REPLICATION_STATUSES = (
    Clusters.ContentLauncher.Enums.StatusEnum.kReplicationNotAllowed,
    Clusters.ContentLauncher.Enums.StatusEnum.kReplicationNotSupported,
)


class TC_CONTENTLAUNCHER_10_8(MatterBaseTest):

    def desc_TC_CONTENTLAUNCHER_10_8(self) -> str:
        return "[TC-CONTENTLAUNCHER-10.8] Content Replication Verification"

    def pics_TC_CONTENTLAUNCHER_10_8(self) -> list[str]:
        return ["CONTENTLAUNCHER.S", "CONTENTLAUNCHER.S.F05"]

    def steps_TC_CONTENTLAUNCHER_10_8(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the Movable attribute from the DUT while content is playing.",
                     "DUT replies with a bool value indicating whether the current content can be replicated."),
            TestStep(2, "TH sends a ContentReplicationRequest command to the DUT while the Movable attribute is TRUE.",
                     "DUT replies with a ContentReplicationResponse with Status=Success and a populated "
                     "ReplicationInfo field containing at least one of LaunchUrlInfo or ContentAppInfo (non-null)."),
            TestStep(3, "TH sends a ContentReplicationRequest command to the DUT while the Movable attribute is "
                     "FALSE.",
                     "DUT replies with a ContentReplicationResponse with a non-Success Status (ReplicationNotAllowed "
                     "or ReplicationNotSupported) and a null ReplicationInfo field."),
            TestStep(4, "TH stops any playing content, then sends a ContentReplicationRequest command to the DUT "
                     "while no content is playing.",
                     "DUT replies with a ContentReplicationResponse with a non-Success Status and a null "
                     "ReplicationInfo field."),
        ]

    async def _read_movable(self, endpoint) -> bool:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.ContentLauncher,
            attribute=Clusters.ContentLauncher.Attributes.Movable)

    async def _request_replication(self, endpoint):
        return await self.send_single_cmd(
            cmd=Clusters.ContentLauncher.Commands.ContentReplicationRequest(), endpoint=endpoint)

    def _verify_replication_refused(self, response, context: str):
        """Assert a non-Success response carrying no replication information."""
        asserts.assert_in(
            response.status, _NON_SUCCESS_REPLICATION_STATUSES,
            f"ContentReplicationRequest {context} should be refused with ReplicationNotAllowed or "
            f"ReplicationNotSupported, got {response.status}")
        asserts.assert_true(response.replicationInfo in (None, NullValue),
                            f"ReplicationInfo must be null when replication is refused ({context})")

    @run_if_endpoint_matches(
        has_feature(Clusters.ContentLauncher, Clusters.ContentLauncher.Bitmaps.Feature.kContentReplication))
    async def test_TC_CONTENTLAUNCHER_10_8(self):
        cluster = Clusters.ContentLauncher
        endpoint = self.get_endpoint()

        self.step(0)
        self.wait_for_user_input(
            prompt_msg="Ensure the DUT is powered on and playing content that CAN be replicated (Movable is TRUE), "
                       "then press Enter.\n")

        self.step(1)
        movable = await self._read_movable(endpoint)
        asserts.assert_is_instance(movable, bool, "Movable must be a bool value")
        log.info("Movable reported as %s", movable)

        self.step(2)
        if not movable:
            # The DUT is not in the state this step exercises, and Movable is read-only, so the
            # operator is the only way to reach it.
            self.wait_for_user_input(
                prompt_msg="Movable is FALSE. Start content that CAN be replicated on the DUT, then press Enter.\n")
            movable = await self._read_movable(endpoint)

        if movable:
            response = await self._request_replication(endpoint)
            asserts.assert_equal(response.status, cluster.Enums.StatusEnum.kSuccess,
                                 "ContentReplicationRequest should succeed while Movable is TRUE")
            asserts.assert_true(response.replicationInfo not in (None, NullValue),
                                "ReplicationInfo must be populated when replication succeeds")
            has_launch_url = response.replicationInfo.launchUrlInfo not in (None, NullValue)
            has_content_app = response.replicationInfo.contentAppInfo not in (None, NullValue)
            asserts.assert_true(
                has_launch_url or has_content_app,
                "ReplicationInfo must contain at least one of LaunchUrlInfo or ContentAppInfo")
            if has_launch_url:
                asserts.assert_true(bool(response.replicationInfo.launchUrlInfo.url),
                                    "LaunchUrlInfo.URL must not be empty")
        else:
            log.info("DUT could not be placed in a movable-content state; skipping the success case")
            self.mark_current_step_skipped()

        self.step(3)
        self.wait_for_user_input(
            prompt_msg="Start content on the DUT that CANNOT be replicated (Movable becomes FALSE), per the "
                       "manufacturer's documentation, then press Enter.\n")
        movable = await self._read_movable(endpoint)
        if not movable:
            response = await self._request_replication(endpoint)
            self._verify_replication_refused(response, "while Movable is FALSE")
        else:
            log.info("DUT still reports Movable=TRUE; no non-movable content is available to test")
            self.mark_current_step_skipped()

        self.step(4)
        await self._stop_playback(endpoint)
        response = await self._request_replication(endpoint)
        self._verify_replication_refused(response, "while no content is playing")

    async def _stop_playback(self, endpoint) -> None:
        """Stop playback via Media Playback when available, otherwise ask the operator."""
        server_list = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        if Clusters.MediaPlayback.id in server_list:
            accepted_commands = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.MediaPlayback,
                attribute=Clusters.MediaPlayback.Attributes.AcceptedCommandList)
            if Clusters.MediaPlayback.Commands.Stop.command_id in accepted_commands:
                await self.send_single_cmd(cmd=Clusters.MediaPlayback.Commands.Stop(), endpoint=endpoint)
                return

        self.wait_for_user_input(
            prompt_msg="Stop all content playback on the DUT, then press Enter.\n")


if __name__ == "__main__":
    default_matter_test_main()
