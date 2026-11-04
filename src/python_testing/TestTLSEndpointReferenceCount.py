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

"""Checks that push transports are counted in the ReferenceCount of their TLS endpoint.

A TLS endpoint used by a push transport must report ReferenceCount 1, must refuse RemoveEndpoint with INVALID_IN_STATE,
and must keep its ReferenceCount when it is re-provisioned with the same EndpointID.
"""

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main


class TestTLSEndpointReferenceCount(MatterTestCommissionedDevice, PAVSTTestBase, PAVSTIUtils):
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
        await self.postcondition_remove_tls_endpoint(self.tlsEndpointId)
        super().teardown_test()

    async def find_tls_endpoint(self, tls_utils: TLSUtils):
        return (await tls_utils.send_find_tls_endpoint_command(endpoint_id=self.tlsEndpointId)).endpoint

    @async_test_body
    async def test_push_transport_holds_tls_endpoint_reference(self):
        endpoint = self.get_endpoint()
        pvcluster = Clusters.PushAvStreamTransport
        tls_utils = TLSUtils(self, endpoint=0)

        self.tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            server=self.server, host_ip=self.user_params.get("host_ip", None))
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf)
        asserts.assert_equal((await self.find_tls_endpoint(tls_utils)).referenceCount, 0,
                             "A newly provisioned TLS endpoint must have ReferenceCount 0")

        await self.check_and_delete_all_push_av_transports(endpoint, pvcluster.Attributes)
        status = await self.allocate_one_pushav_transport(
            endpoint,
            trigger_Options={"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kCommand, "maxPreRollLen": 4000},
            tlsEndPoint=self.tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success, "AllocatePushTransport failed")
        asserts.assert_equal((await self.find_tls_endpoint(tls_utils)).referenceCount, 1,
                             "AllocatePushTransport must increment the TLS endpoint ReferenceCount")

        # The endpoint is in use by the transport, so it cannot be removed.
        await tls_utils.send_remove_tls_endpoint_command(endpoint_id=self.tlsEndpointId,
                                                         expected_status=Status.InvalidInState)

        # Re-provisioning updates the endpoint's fields but not its ReferenceCount.
        tls_endpoint = await self.find_tls_endpoint(tls_utils)
        await tls_utils.send_provision_tls_endpoint_command(
            hostname=tls_endpoint.hostname,
            port=tls_endpoint.port,
            caid=tls_endpoint.caid,
            ccdid=tls_endpoint.ccdid,
            endpoint_id=self.tlsEndpointId,
        )
        asserts.assert_equal((await self.find_tls_endpoint(tls_utils)).referenceCount, 1,
                             "Re-provisioning the TLS endpoint must keep its ReferenceCount")

        await self.check_and_delete_all_push_av_transports(endpoint, pvcluster.Attributes)
        asserts.assert_equal((await self.find_tls_endpoint(tls_utils)).referenceCount, 0,
                             "DeallocatePushTransport must decrement the TLS endpoint ReferenceCount")


if __name__ == "__main__":
    default_matter_test_main()
