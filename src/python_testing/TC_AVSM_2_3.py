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


class TC_AVSM_2_3(MatterBaseTest):
    def desc_TC_AVSM_2_3(self) -> str:
        return "[TC-AVSM-2.3] Validate Snapshot Stream Modification functionality with Server as DUT"

    def pics_TC_AVSM_2_3(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_3(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify SNP & (WMARK|OSD) is supported."),
            TestStep(2, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the number of allocated snapshot streams in the list is 1. Store StreamID as aStreamID. If WMARK is supported, store WaterMarkEnabled as aWmark. If OSD is supported, store OSDEnabled as aOSD."),
            TestStep(3, "TH sends the SnapshotStreamModify command with SnapshotStreamID set to aStreamID. If WMARK is supported, set WaterMarkEnabled to !aWmark`and if OSD is supported, set OSDEnabled to `!aOSD in the command.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(4, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the following: If WMARK is supported, verify WaterMarkEnabled == !aWmark. If OSD is supported, verify OSDEnabled == !aOSD."),
        ]

    async def _precondition_one_allocated_snp_stream(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        # First verify that SNP is supported
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = (aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0
        asserts.assert_true(snpSupport, "Snapshot Feature is not supported.")

        # Check if snapshot stream has already been allocated
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        if len(aAllocatedSnapshotStreams) > 0:
            return

        # Allocate one for the test steps based on SnapshotCapabilities
        aSnapshotCapabilities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities)
        logger.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")

        asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    @async_test_body
    async def test_TC_AVSM_2_3(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self._precondition_one_allocated_snp_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = ((aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0)
        wmarkSupport = ((aFeatureMap & cluster.Bitmaps.Feature.kWatermark) > 0)
        osdSupport = ((aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) > 0)
        logger.info(f"Rx'd snpSupport: {snpSupport}, wmarkSupport: {wmarkSupport}, osdSupport: {osdSupport}")
        asserts.assert_true((snpSupport and (wmarkSupport or osdSupport)), cluster.Bitmaps.Feature.kSnapshot, "SNP & (WMARK|OSD) is supported is not supported.")

        self.step(2)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        aStreamID = aAllocatedSnapshotStreams[0].snapshotStreamID
        # TODO: SnapshotStreamStruct does not have WaterMarkEnabled and OSDEnabled fields. Update This.

        self.step(3)
        try:
            # TODO: SnapshotStreamStruct does not have WaterMarkEnabled and OSDEnabled fields. Update This.
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamModify(snapshotStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        # TODO: SnapshotStreamStruct does not have WaterMarkEnabled and OSDEnabled fields. Update This.


if __name__ == "__main__":
    default_matter_test_main()
