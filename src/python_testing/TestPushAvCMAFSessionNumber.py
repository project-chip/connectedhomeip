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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc --camera-test-audiosrc
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
#       --timeout 300
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Checks the CMAFSessionNumber that a camera reports in PushTransportBegin and PushTransportEnd.

Each trigger activation starts a new CMAF session, so over consecutive manual triggers on one transport:
  * PushTransportEnd reports the same session number as the PushTransportBegin before it.
  * Each PushTransportBegin reports exactly 1 more than the previous one.
"""

import asyncio
import logging

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

kTriggerCount = 3
kInitialDurationS = 3


class TestPushAvCMAFSessionNumber(MatterTestCommissionedDevice, PAVSTTestBase, PAVSTIUtils):
    @async_test_body
    async def setup_class(self):
        self.server = PushAvServerProcess(server_path=self.user_params.get("th_server_app_path", None))
        self.server.start(expected_output="Running on https://0.0.0.0:1234", timeout=30)
        self.tlsEndpointId = None
        super().setup_class()

    def teardown_class(self):
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    @async_test_body
    async def teardown_test(self):
        await self.check_and_delete_all_push_av_transports(self.get_endpoint(), Clusters.PushAvStreamTransport.Attributes)
        await self.postcondition_remove_tls_endpoint(self.tlsEndpointId)
        super().teardown_test()

    @async_test_body
    async def test_session_number_per_manual_trigger(self):
        endpoint = self.get_endpoint()
        pvcluster = Clusters.PushAvStreamTransport

        self.tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            server=self.server, host_ip=self.user_params.get("host_ip", None))
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf)

        await self.check_and_delete_all_push_av_transports(endpoint, pvcluster.Attributes)
        # allocate_one_pushav_transport() expects the streams to be allocated already.
        await self.allocate_one_video_stream()
        await self.allocate_one_audio_stream()
        status = await self.allocate_one_pushav_transport(
            endpoint,
            trigger_Options={"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kCommand, "maxPreRollLen": 4000},
            tlsEndPoint=self.tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success, "AllocatePushTransport failed")

        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint, pvcluster.Attributes.CurrentConnections)
        asserts.assert_equal(len(transportConfigs), 1, "Expected exactly one allocated transport")
        connectionID = transportConfigs[0].connectionID

        status = await self.psvt_set_transport_status(pvcluster.Commands.SetTransportStatus(
            connectionID=connectionID, transportStatus=pvcluster.Enums.TransportStatusEnum.kActive))
        asserts.assert_equal(status, Status.Success, "SetTransportStatus(Active) failed")

        event_callback = EventSubscriptionHandler(expected_cluster=pvcluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        timeControl = {"initialDuration": kInitialDurationS, "augmentationDuration": 1,
                       "maxDuration": kInitialDurationS, "blindDuration": 1}
        previous_session_number = None
        for iteration in range(kTriggerCount):
            status = await self.psvt_manually_trigger_transport(pvcluster.Commands.ManuallyTriggerTransport(
                connectionID=connectionID,
                activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                timeControl=timeControl))
            asserts.assert_equal(status, Status.Success, f"ManuallyTriggerTransport {iteration} failed")

            begin = event_callback.wait_for_event_report(pvcluster.Events.PushTransportBegin, timeout_sec=10)
            asserts.assert_equal(begin.connectionID, connectionID, "PushTransportBegin for the wrong transport")
            asserts.assert_is_not_none(begin.CMAFSessionNumber, "PushTransportBegin has no CMAFSessionNumber")
            if previous_session_number is not None:
                asserts.assert_equal(begin.CMAFSessionNumber, previous_session_number + 1,
                                     f"Trigger {iteration} did not start the next CMAF session")

            end = event_callback.wait_for_event_report(pvcluster.Events.PushTransportEnd, timeout_sec=kInitialDurationS + 15)
            asserts.assert_equal(end.connectionID, connectionID, "PushTransportEnd for the wrong transport")
            asserts.assert_equal(end.CMAFSessionNumber, begin.CMAFSessionNumber,
                                 "PushTransportEnd reports a different session than its PushTransportBegin")

            previous_session_number = begin.CMAFSessionNumber
            # Give the camera time to release the finished recorder before the next trigger.
            await asyncio.sleep(1)

        event_callback.cancel()


if __name__ == "__main__":
    default_matter_test_main()
