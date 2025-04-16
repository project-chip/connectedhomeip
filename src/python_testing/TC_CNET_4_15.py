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
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from matter_testing_infrastructure.chip.testing.matter_testing import (
    MatterBaseTest,
    TestStep,
    async_test_body,
    default_matter_test_main,
)


class TC_CNET_4_15(MatterBaseTest):
    def steps_TC_CNET_4_15(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900'),
                TestStep(2, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, which does not match the provisioned network, and Breadcrumb field set to 1'),
                TestStep(3, 'TH sends ConnectNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, which does not match the provisioned network, and Breadcrumb field set to 1')
                ]

    def def_TC_CNET_4_15(self):
        return '[TC-CNET-4.15] [Wi-Fi] NetworkIDNotFound returned in LastNetworkingStatus field validation [DUT-Server]'

    def pics_TC_CNET_4_15(self):
        return ['CNET.S.F00(WI)']

    @async_test_body
    async def test_TC_CNET_4_15(self):
        cnet = Clusters.NetworkCommissioning

        # Commissioning is already done
        self.step("precondition")
        await self.commission_devices()

        self.step(1)
        # TH sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900
        send_arm = await self.send_arm_failsafe_command(900)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        await self.expect_command(Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse, send_arm)

        self.step(2)
        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # which does not match the provisioned network, and Breadcrumb field set to 1
        network_id = self.matter_test_config.global_test_params['PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID']
        send_remove = await self.send_remove_network_command(network_id, 1)

        # Verify that DUT sends NetworkConfigResponse command to the TH1 with NetworkingStatus field set as NetworkIDNotFound which is '3'
        await self.expect_command(cnet.Commands.NetworkConfigResponse, send_remove)

        self.step(3)
        # TH sends ConnectNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # which does not match the provisioned network, and Breadcrumb field set to 1
        send_connect = await self.send_connect_network_command(network_id, 1)

        # Verify that DUT sends ConnectNetworkResponse command to the TH with NetworkingStatus field set as NetworkIDNotFound which is '3'
        await self.expect_command(Clusters.GeneralCommissioning.Commands.ConnectNetworkResponse, send_connect)


if __name__ == "__main__":
    default_matter_test_main()
