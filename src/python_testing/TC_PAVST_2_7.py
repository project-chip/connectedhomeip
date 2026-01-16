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
#       --string-arg th_server_app_path:${PUSH_AV_SERVER}
#       --string-arg host_ip:localhost
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

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main


class TC_PAVST_2_7(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def TC_PAVST_2_7(self) -> str:
        return "[TC-PAVST-2.7] Manually Trigger PushAV Transport Flow with Server as DUT"

    def pics_TC_PAVST_2_7(self):
        return ["PAVST.S"]

    @async_test_body
    async def setup_class(self):
        th_server_app = self.user_params.get("th_server_app_path", None)
        self.server = PushAvServerProcess(server_path=th_server_app)
        self.server.start(
            expected_output="Running on https://0.0.0.0:1234",
            timeout=30,
        )
        super().setup_class()

    def teardown_class(self):
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    def steps_TC_PAVST_2_7(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1,
                "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport with TriggerType = Continuous.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                2,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                "Verify the number of PushAV Connections in the list is 1. Store the TransportStatus and ConnectionID in the corresponding TransportConfiguration as aTransportStatus and aConnectionID.",
            ),
            TestStep(
                3,
                "TH1 sends the ManuallyTriggerTransport command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                4,
                "TH2 sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                5,
                "TH1 sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Inactive.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                6,
                "TH1 sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with InvalidTransportStatus.",
            ),
            TestStep(
                7,
                "TH1 sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                8,
                "TH1 sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with InvalidTriggerType.",
            ),
            TestStep(
                9,
                "TH1 sends the DeallocatePushTransport command with ConnectionID = aConnectionID.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                10,
                "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport with TriggerType = Command.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                11,
                "TH1 sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                12,
                "TH1 sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID and TimeControl field is omitted.",
                "DUT responds with DYNAMIC_CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                13,
                "If privacy is supported, TH1 sets SoftPrivacy to True then sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                14,
                "If privacy is supported and was set, it is reverted to False. Then TH1 sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with SUCCESS status code.",
            ),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_7(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aConnectionID = ""

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream()

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

        status = await self.allocate_one_pushav_transport(endpoint, triggerType=pvcluster.Enums.TransportTriggerTypeEnum.kContinuous,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
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
        aConnectionID = transportConfigs[0].connectionID

        # TH1 sends command
        self.step(3)
        all_connectionID = [tc.connectionID for tc in transportConfigs]
        max_connectionID = max(all_connectionID)
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=max_connectionID + 1,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kEmergency
        )
        status = await self.psvt_manually_trigger_transport(cmd)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        # TH2 sends command
        self.step(4)
        # Establishing TH2 controller
        th2 = await self.psvt_create_test_harness_controller()
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kEmergency
        )
        status = await self.psvt_manually_trigger_transport(cmd, devCtrl=th2)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        resp = await self.psvt_remove_current_fabric(th2)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk, "Expected removal of TH2's fabric to succeed")

        self.step(5)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kInactive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(6)
        timeControl = {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kEmergency,
            timeControl=timeControl
        )
        status = await self.psvt_manually_trigger_transport(cmd, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidTransportStatus)
        asserts.assert_true(
            status == pvcluster.Enums.StatusCodeEnum.kInvalidTransportStatus,
            "DUT must respond with TransportStatus Inactive.",
        )

        self.step(7)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(8)
        timeControl = {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kEmergency,
            timeControl=timeControl
        )
        status = await self.psvt_manually_trigger_transport(cmd, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidTriggerType)
        asserts.assert_true(
            status == pvcluster.Enums.StatusCodeEnum.kInvalidTriggerType,
            "DUT must respond with InvalidTriggerType status code.",
        )

        self.step(9)
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(10)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        aAllocatedVideoStreams = await self.allocate_one_video_stream()

        aAllocatedAudioStreams = await self.allocate_one_audio_stream()

        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kCommand,
                          "maxPreRollLen": 4000}

        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId,
                                                          url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )

        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aConnectionID = transportConfigs[0].connectionID

        self.step(11)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(12)
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
        )
        status = await self.psvt_manually_trigger_transport(cmd, expected_status=Status.DynamicConstraintError)
        asserts.assert_true(
            status == Status.DynamicConstraintError,
            "DUT responds with DynamicConstraintError status code.",
        )

        self.step(13)
        timeControl = {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
            timeControl=timeControl
        )

        # Check if privacy feature is supported before testing privacy mode
        aFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.FeatureMap
        )
        privacySupported = aFeatureMap & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kPrivacy
        if privacySupported:
            # The stream usage will be the first item in supported stream usages. Set the privacy appropriate to the usage
            aStreamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.StreamUsagePriorities
            )

            streamUsage = aStreamUsagePriorities[0]

            if (streamUsage == Globals.Enums.StreamUsageEnum.kRecording) or (streamUsage == Globals.Enums.StreamUsageEnum.kAnalysis):
                # Write SoftRecordingPrivacyModeEnabled=true and test INVALID_IN_STATE
                await self.write_single_attribute(
                    attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled(True),
                    endpoint_id=endpoint,
                )
            else:
                # Livestream
                # Write SoftLivestreamPrivacyModeEnabled=true and test INVALID_IN_STATE
                await self.write_single_attribute(
                    attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(True),
                    endpoint_id=endpoint,
                )

            status = await self.psvt_manually_trigger_transport(cmd, expected_status=Status.InvalidInState)
            asserts.assert_true(status == Status.InvalidInState,
                                (f"Unexpected response {status} received on Manually Triggered push "
                                 "with privacy mode enabled")
                                )

            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled(False),
                endpoint_id=endpoint,
            )

            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(False),
                endpoint_id=endpoint,
            )

        self.step(14)
        status = await self.psvt_manually_trigger_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with Success status code.",
        )


if __name__ == "__main__":
    default_matter_test_main()
