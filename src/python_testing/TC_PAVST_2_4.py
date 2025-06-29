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

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_PAVST_2_4(MatterBaseTest):
    def desc_TC_PAVST_2_4(self) -> str:
        return "[TC-PAVST-2.9] Validate Transport allocation with an ExpiryTime with Server as DUT"

    def pics_TC_PAVST_2_4(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport.",
                     "Verify successful completion of all steps."),
            TestStep(2, "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                     "Verify the number of PushAV Connections in the list is 1. Store the TransportOptions and ConnectionID in the corresponding TransportConfiguration as aTransportOptions and aConnectionID."),
            TestStep(3, "TH1 sends the ModifyPushTransport command with ConnectionID != aConnectionID.",
                     "DUT responds with NOT_FOUND status code."),
            TestStep(4, "TH2 sends the ModifyPushTransport command with ConnectionID = aConnectionID.",
                     "DUT responds with NOT_FOUND status code."),
            TestStep(5, "TH1 sends the ModifyPushTransport command with ConnectionID = aConnectionID and aTransportOptions, with ExpiryTime incremented by 120 in aTransportOptions.",
                     "DUT responds with SUCCESS status code."),
            TestStep(6, "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                     "Verify that ConnectionID == aConnectionID and ExpiryTime in TransportConfiguration.TransportOptions is incremented by 120."),
        ]

    @async_test_body
    async def test_TC_PAVST_2_4(self):
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
                    await self.send_single_cmd(cmd=cluster.Commands.DeallocatePushTransport(ConnectionID=config.ConnectionID),
                                               endpoint=endpoint)
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        aSupportedFormats = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedContainerFormats
        )

        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedVideoStreams
        )

        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedAudioStreams
        )

        response = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
            # TransportOptions=
            {"streamUsage": 0,
             "videoStreamID": 1,
             "audioStreamID": 1,
             "endpointID": 1,
             "url": "https://localhost:1234/streams/1",
             "triggerOptions": {"triggerType": 2},
             "ingestMethod": 0,
             "containerFormat": 0,
             "containerOptions": {"containerType": 0},
             "expiryTime": 50,
             }), endpoint=endpoint)

        self.step(2)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
        )
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1")

        aConnectionID = transport_configs.ConnectionID
        aTransportOptions = transport_configs.TransportOptions

        self.step(3)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.FindTransport(
                ConnectionID=aConnectionID+1),
                endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Notfound, "Unexpected error returned")

        self.step(4)
        # Need to check how to handle when 2THs are onboarded camera

        self.step(5)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.FindTransport(
                ConnectionID=aConnectionID),
                endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Expected success")

        self.step(6)
        response = self.send_single_cmd(cmd=cluster.Commands.FindTransport(
            ConnectionID=Null),
            endpoint=endpoint)
        # response.TransportConfiguration
        # Add code specific to step 6


if __name__ == "__main__":
    default_matter_test_main()
