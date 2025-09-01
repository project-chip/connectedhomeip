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

from mobly import asserts
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_PAVST_2_4(MatterBaseTest, PAVSTTestBase):
    def desc_TC_PAVST_2_4(self) -> str:
        return "[TC-PAVST-2.4] Attributes with Server as DUT"

    def pics_TC_PAVST_2_4(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_4(self) -> list[TestStep]:
        return [
            TestStep(
                1,
                "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                2,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session.",
                "Verify the number of PushAV Connections in the list is 1. Store the TransportOptions and ConnectionID in the corresponding TransportConfiguration as aTransportOptions and aConnectionID.",
            ),
            TestStep(
                3,
                "TH1 sends the ModifyPushTransport command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                4,
                "TH2 sends the ModifyPushTransport command with ConnectionID = aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                5,
                "TH1 sends the ModifyPushTransport command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                6,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session.",
                "Verify the number of PushAV Connections in the list is 1. Store the TransportOptions and ConnectionID in the corresponding TransportConfiguration as aTransportOptions and aConnectionID.",
            )
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_4(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aTransportOptions = ""
        aConnectionID = ""

        self.step(1)
        # Commission DUT - already done
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
        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aTransportOptions = transportConfigs[0].transportOptions
        aConnectionID = transportConfigs[0].connectionID

        # TH1 sends command
        self.step(3)
        all_connectionID = [tc.connectionID for tc in transportConfigs]
        max_connectionID = max(all_connectionID)
        cmd = pvcluster.Commands.ModifyPushTransport(
            connectionID=max_connectionID + 1,
            transportOptions=aTransportOptions
        )
        status = await self.psvt_modify_push_transport(cmd)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        # TH2 sends command
        self.step(4)
        th2 = await self.psvt_create_test_harness_controller()

        cmd = pvcluster.Commands.ModifyPushTransport(
            connectionID=aConnectionID,
            transportOptions=aTransportOptions
        )
        status = await self.psvt_modify_push_transport(cmd, devCtrl=th2)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        resp = await self.psvt_remove_current_fabric(th2)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk, "Expected removal of TH2's fabric to succeed")

        self.step(5)
        aModifiedTransportOptions = aTransportOptions.expiryTime
        aModifiedTransportOptions = aModifiedTransportOptions + 120
        aTransportOptions.expiryTime = aModifiedTransportOptions
        cmd = pvcluster.Commands.ModifyPushTransport(
            connectionID=aConnectionID,
            transportOptions=aTransportOptions,
        )
        status = await self.psvt_modify_push_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(6)
        transportConfigs = await self.read_pavst_attribute_expect_success(
            endpoint, pvattr.CurrentConnections
        )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        result = (
            transportConfigs[0].transportOptions.expiryTime
            == aModifiedTransportOptions
            and transportConfigs[0].connectionID == aConnectionID
        )
        asserts.assert_true(
            result,
            "ConnectionID or ExpiryTime should match as per the modified transport options"
        )


if __name__ == "__main__":
    default_matter_test_main()
