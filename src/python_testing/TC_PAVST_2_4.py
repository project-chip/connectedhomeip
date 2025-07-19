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

logger = logging.getLogger(__name__)


class TC_PAVST_2_4(MatterBaseTest):
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
            ),
            TestStep(
                7,
                "TH1 sends the ModifyPushTransport command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
        ]

    @async_test_body
    async def test_TC_PAVST_2_4(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.CameraAvStreamManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.CameraAvStreamManagement.Attributes
        aSupportedFormats = []
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aTransportOptions = ""
        aConnectionID = ""

        self.step(1)
        # Commission DUT - already done
        if self.pics_guard(self.check_pics("PAVST.S")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=pvcluster,
                attribute=pvattr.CurrentConnections,
            )
            for config in transport_configs:
                if config.ConnectionID != 0:
                    try:
                        await self.send_single_cmd(
                            cmd=pvcluster.Commands.DeallocatePushTransport(
                                ConnectionID=config.ConnectionID
                            ),
                            endpoint=endpoint,
                        )
                    except InteractionModelError as e:
                        asserts.assert_true(
                            e.status == Status.Success, "Unexpected error returned"
                        )

            aSupportedFormats = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats
            )
            asserts.assert_greater_equal(
                len(aSupportedFormats), 1, "SupportedFormats must not be empty!"
            )

            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamAllocate(
                    streamUsage=0,
                    videoCodec=0,
                    minFrameRate=30,
                    maxFrameRate=120,
                    minResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                        width=400, height=300
                    ),
                    maxResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                        width=1920, height=1080
                    ),
                    minBitRate=20000,
                    maxBitRate=150000,
                    minKeyFrameInterval=4000,
                    maxKeyFrameInterval=4000,
                    watermarkEnabled=1,
                    OSDEnabled=1,
                ),
                endpoint=endpoint,
            )

            aAllocatedVideoStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=avcluster,
                attribute=avattr.AllocatedVideoStreams,
            )
            asserts.assert_greater_equal(
                len(aAllocatedVideoStreams),
                1,
                "AllocatedVideoStreams must not be empty",
            )

            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamAllocate(
                    streamUsage=0,
                    audioCodec=0,
                    channelCount=2,
                    sampleRate=48000,
                    bitRate=96000,
                    bitDepth=16,
                ),
                endpoint=endpoint,
            )

            aAllocatedAudioStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=avcluster,
                attribute=avattr.AllocatedAudioStreams,
            )
            asserts.assert_greater_equal(
                len(aAllocatedAudioStreams),
                1,
                "AllocatedAudioStreams must not be empty",
            )

            await self.send_single_cmd(
                cmd=pvcluster.Commands.AllocatePushTransport(
                    {
                        "streamUsage": 0,
                        "videoStreamID": 1,
                        "audioStreamID": 1,
                        "endpointID": 1,
                        "url": "https://localhost:1234/streams/1",
                        "triggerOptions": {"triggerType": 2},
                        "ingestMethod": 0,
                        "containerFormat": 0,
                        "containerOptions": {
                            "containerType": 0,
                            "CMAFContainerOptions": {"chunkDuration": 4},
                        },
                        "expiryTime": 5,
                    }
                ),
                endpoint=endpoint,
            )

        self.step(2)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=pvcluster,
                attribute=pvattr.CurrentConnections,
            )
            asserts.assert_greater_equal(
                len(transport_configs), 1, "TransportConfigurations must not be empty!"
            )
            aTransportOptions = transport_configs[0].TransportOptions
            aConnectionID = transport_configs[0].ConnectionID

        # TH1 sends command
        self.step(3)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
            status = await self.send_single_cmd(
                cmd=pvcluster.Commands.ModifyPushTransport(
                    {
                        "connectionID": 10,
                    }
                ),
                endpoint=endpoint,
            )
            asserts.assert_true(
                status == pvattr.Status.NOT_FOUND,
                "DUT responds with NOT_FOUND status code.",
            )

        # TH2 sends command
        self.step(4)
        # Establishing TH2 controller
        th2_certificate_authority = (
            self.certificate_authority_manager.NewCertificateAuthority()
        )
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=self.fabricId + 1
        )
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        if th2.pics_guard(th2.check_pics("PAVST.S.A0001")):
            status = await th2.send_single_cmd(
                cmd=pvcluster.Commands.ModifyPushTransport(
                    {
                        "connectionID": aConnectionID,
                    }
                ),
                endpoint=endpoint,
            )
            asserts.assert_true(
                status == pvattr.Status.NOT_FOUND,
                "DUT responds with NOT_FOUND status code.",
            )

        self.step(5)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
            aModifiedTransportOptions = aTransportOptions.ExpiryTime
            aModifiedTransportOptions = aModifiedTransportOptions + 120
            status = await self.send_single_cmd(
                cmd=pvcluster.Commands.ModifyPushTransport(
                    {
                        "connectionID": aConnectionID,
                        "transportOptions": aModifiedTransportOptions,
                    }
                ),
                endpoint=endpoint,
            )
            asserts.assert_true(
                status == pvattr.Status.Success,
                "DUT responds with SUCCESS status code.",
            )

        self.step(6)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=pvcluster,
                attribute=pvattr.CurrentConnections,
            )
            asserts.assert_greater_equal(
                len(transport_configs), 1, "TransportConfigurations must not be empty!"
            )
            result = (
                transport_configs[0].TransportOptions.ExpiryTime
                == aModifiedTransportOptions
                and transport_configs[0].ConnectionID == aConnectionID
            )
            asserts.assert_true(
                result,
                "ConnectionID or ExpiryTime should match as per the modified transport options",
            )


if __name__ == "__main__":
    default_matter_test_main()
