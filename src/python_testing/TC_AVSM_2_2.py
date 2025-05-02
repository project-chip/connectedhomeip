#
#    Copyright (c) 2025 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_2(MatterBaseTest):
    def desc_TC_AVSM_2_2(self) -> str:
        return "[TC-AVSM-2.2] Validate Snapshot Stream Allocation functionality with Server as DUT"

    def pics_TC_AVSM_2_2(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify SNP is supported"
            ),
            TestStep(
                2,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads SnapshotCapabilities attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aSnapshotCapabilities.",
            ),
            TestStep(
                4,
                "TH sends the SnapshotStreamAllocate command with valid values of ImageCodec, MaxFrameRate, MinResolution=MaxResolution=Resolution from aSnapshotCapabilities and Quality set to 90.",
                "DUT responds with SnapshotStreamAllocateResponse command with a valid SnapshotStreamID.",
            ),
            TestStep(
                5,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 1.",
            ),
            TestStep(
                6,
                "TH sends the SnapshotStreamAllocate command with values from step 3 except with MaxFrameRate set to 0(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                7,
                "TH sends the SnapshotStreamAllocate command with values from step 3 except with Quality set to 101(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            # NOTE: Test Plan has a step 8, which is a duplicate of step 7, so it has not been added here.
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot)
    )
    async def test_TC_AVSM_2_2(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = (aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0
        asserts.assert_true(snpSupport, "Snapshot Feature is not supported.")

        self.step(2)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 0, "The number of allocated snapshot streams in the list is not 0.")

        self.step(3)
        aSnapshotCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        logger.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")

        self.step(4)
        asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            logger.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")

        self.step(6)
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=0,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            asserts.assert_true(
                False, "Unexpected success when expecting CONSTRAINT_ERROR due to MaxFrameRate set to 0(outside of valid range)"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to MaxFrameRate set to 0(outside of valid range)",
            )
            pass

        self.step(7)
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=101,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            asserts.assert_true(
                False, "Unexpected success when expecting CONSTRAINT_ERROR due to Quality set to 101(outside of valid range)"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to Quality set to 101(outside of valid range)",
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
