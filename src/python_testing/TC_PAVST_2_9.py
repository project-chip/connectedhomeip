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
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_PAVST_2_9(MatterBaseTest):
    def desc_TC_PAVST_2_9(self) -> str:
        return "[TC-PAVST-2.9] Validate Transport allocation with an ExpiryTime with Server as DUT"

    def pics_TC_PAVST_2_9(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with `ConnectionID to remove any connections."),
            TestStep(2, "TH Reads SupportedIngestMethods attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedIngestMethods."),
            TestStep(3, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedFormats."),
            TestStep(4, "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Store value as aAllocatedVideoStreams."),
            TestStep(5, "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Store value as aAllocatedAudioStreams."),
            TestStep(6, "TH sends the AllocatePushTransport command with valid parameters and ExpiryTime set to 5 seconds.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct."),
            TestStep(7, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportStatus field is Inactive."),
            TestStep(8, "After > 5 seconds, TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 0."),
        ]

    @async_test_body
    async def test_TC_PAVST_2_9(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.Objects.CameraAvStreamManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.Objects.CameraAvStreamManagement.Attributes

        # Commission DUT - already done

        self.step(1)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
        )
        for config in transport_configs:
            if config.ConnectionID != 0:
                try:
                    await self.send_single_cmd(cmd=pvcluster.Commands.DeallocatePushTransport(ConnectionID=config.ConnectionID),
                                               endpoint=endpoint)
                except InteractionModelError as e:
                    asserts.assert_true(e.status == Status.Success, "Unexpected error returned")

        self.step(2)
        aSupportedIngestMethods = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedIngestMethods
        )
        logger.info(f"SupportedIngestMethods: {aSupportedIngestMethods}")

        self.step(3)
        aSupportedFormats = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedContainerFormats
        )
        logger.info(f"SupportedContainerFormats: {aSupportedFormats}")

        self.step(4)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedVideoStreams
        )
        logger.info(f"AllocatedVideoStreams: {aAllocatedVideoStreams}")

        self.step(5)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedAudioStreams
        )
        logger.info(f"AllocatedAudioStreams: {aAllocatedAudioStreams}")

        self.step(6)
        await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
            {"streamUsage": 0,
             "videoStreamID": 1,
             "audioStreamID": 1,
             "endpointID": 1,
             "url": "https://localhost:1234/streams/1",
             "triggerOptions": {"triggerType": 2},
             "ingestMethod": 0,
             "containerFormat": 0,
             "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
             "expiryTime": 5
             }), endpoint=endpoint)

        self.step(7)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
        )
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1")
        asserts.assert_true(transport_configs[0].transportStatus ==
                            pvcluster.Enums.TransportStatusEnum.kInactive, "Transport status should be Inactive")

        logger.info("Wait for 6 secs to PushAVTransport expiry")
        time.sleep(6)

        self.step(8)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
        )
        asserts.assert_equal(len(transport_configs), 0, "TransportConfigurations must be empty")


if __name__ == "__main__":
    default_matter_test_main()
