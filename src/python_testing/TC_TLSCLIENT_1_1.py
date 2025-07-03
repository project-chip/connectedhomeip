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
#

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --app-pipe_prefix /tmp/chip_all_clusters_fifo_
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
from typing import Union

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import Nullable, NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches,
                                         type_matches)
from chip.tlv import uint
from mobly import asserts


class TC_TLSCLIENT_1_1(MatterBaseTest):
    def desc_TC_TLSCLIENT_1_1(self) -> str:
        return "[TC-TLSCLIENT-1.1] Primary functionality with DUT as Server"

    def steps_TC_TLSCLIENT_1_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Sends the ProvisionEndpoint command to the TlsClientNamanement cluster",
                     "Verify that the DUT sends ProvisionEndpointResponse."),
        ]
        return steps

    async def send_provision_command(self, endpoint: int, hostname: bytes, port: uint, caid: uint,
                                     ccdid: Union[Nullable, uint] = NullValue, expected_status: Status = Status.Success) -> Union[Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse, InteractionModelError]:
        try:
            result = await self.send_single_cmd(cmd=Clusters.TlsClientManagement.Commands.ProvisionEndpoint(hostname=hostname, port=port, caid=caid, ccdid=ccdid),
                                                endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

            asserts.assert_true(type_matches(result, Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse),
                                "Unexpected return type for ProvisionEndpoint")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_1_1(self):

        endpoint = self.get_endpoint(default=1)

        self.step(1)

        self.step(2)
        e = await self.send_provision_command(endpoint=endpoint, expected_status=Status.Failure, hostname=b"my_hostname", port=1000, caid=10)
        asserts.assert_equal(
            e.clusterStatus, Clusters.TlsClientManagement.Enums.StatusCodeEnum.kRootCertificateNotFound, "Unexpected error returned")


if __name__ == "__main__":
    default_matter_test_main()
