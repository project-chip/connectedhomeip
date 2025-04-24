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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_4(MatterBaseTest):
    def desc_TC_AVSM_2_4(self) -> str:
        return "[TC-AVSM-2.4] Validate Snapshot Stream Deallocation functionality with Server as DUT"

    def pics_TC_AVSM_2_4(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify SNP is supported."),
            TestStep(2, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the number of allocated snapshot streams in the list is 1. Store StreamID as aStreamIDToDelete."),
            TestStep(3, "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamIDToDelete + 1.",
                     "DUT responds with a NOT_FOUND status code."),
            TestStep(4, "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamIDToDelete.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(5, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the number of allocated snapshot streams in the list is 0."),
        ]

    @async_test_body
    async def test_TC_AVSM_2_4(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = aFeatureMap & cluster.Bitmaps.Feature.kSnapshot
        asserts.assert_equal(snpSupport, cluster.Bitmaps.Feature.kSnapshot, "Snapshot Feature is not supported.")

        self.step(2)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        aStreamIDToDelete = aAllocatedSnapshotStreams[0].snapshotStreamID

        self.step(3)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=(aStreamIDToDelete+1)))
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.step(4)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=(aStreamIDToDelete)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 0, "The number of allocated snapshot streams in the list is not 0.")


if __name__ == "__main__":
    default_matter_test_main()
