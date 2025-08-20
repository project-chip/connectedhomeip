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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_PAVSTTestBase import PAVSTTestBase

logger = logging.getLogger(__name__)


class TC_PAVST_2_8(MatterBaseTest, PAVSTTestBase):
    def desc_TC_PAVST_2_8(self) -> str:
        return "[TC-PAVST-2.8] Attributes with Server as DUT"

    def pics_TC_PAVST_2_8(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_8(self) -> list[TestStep]:
        return [
            TestStep(
                1,
                "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                2,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session.",
                "Verify the number of PushAV Connections in the list is 1. Store the TransportStatus and ConnectionID in the corresponding TransportConfiguration as aTransportStatus and aConnectionID. Store TriggerType as aTriggerType.",
            ),
            TestStep(
                3,
                "TH1 sends the FindTransport command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                4,
                "TH2 sends the FindTransport command with ConnectionID = aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                5,
                "TH1 sends the FindTransport command with ConnectionID = aConnectionID.",
                "DUT responds with FindTransportResponse with the TransportConfiguration corresponding to aConnectionID.",
            ),
            TestStep(
                6,
                "TH1 sends the FindTransport command with ConnectionID = Null.",
                "DUT responds with FindTransportResponse with the TransportConfiguration corresponding to aConnectionID.",
            ),
        ]

    @async_test_body
    async def test_TC_PAVST_2_8(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aTransportOptions = ""
        aConnectionID = ""
        aTransportStatus = ""

        self.step(1)
        # Commission DUT - already done
        # @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
                len(aAllocatedVideoStreams),
                1,
                "AllocatedVideoStreams must not be empty",
            )

        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        status = await self.allocate_one_pushav_transport(endpoint)
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )
    
        self.step(2)
        # @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport)):
        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
            pvattr.CurrentConnections,
        )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aTransportOptions = transportConfigs[0].transportOptions
        aConnectionID = transportConfigs[0].connectionID
        aTransportStatus = transportConfigs[0].transportStatus

        # TH1 sends command
        self.step(3)
        # @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport)):
        cmd = pvcluster.Commands.FindTransport(
                connectionID = 10
        )
        status = await  self.psvt_find_transport(cmd)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        # TH2 sends command
        self.step(4)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
        # Establishing TH2 controller
            th2_certificate_authority = (
                self.certificate_authority_manager.NewCertificateAuthority()
            )
            th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
                vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1
            )
            self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
            #@run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport)):
            cmd = pvcluster.Commands.FindTransport(
                    connectionID = aConnectionID,
            )
            status = await self.th2.psvt_find_transport(cmd)
            asserts.assert_true(
                status == Status.NOT_FOUND,
                "DUT responds with NOT_FOUND status code.",
            )

        self.step(5)
        #@run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport)):
        cmd = pvcluster.Commands.FindTransport(
                connectionID = aConnectionID,
        )
        status = await  self.psvt_find_transport(cmd, expected_connectionID = aConnectionID)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(6)
        #@run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport)):
        cmd = pvcluster.Commands.FindTransport(
                connectionID = None,
        )
        status = await  self.psvt_find_transport(cmd, expected_connectionID = aConnectionID)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")


if __name__ == "__main__":
    default_matter_test_main()
