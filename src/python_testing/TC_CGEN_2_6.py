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
#       app: ${TERMS_AND_CONDITIONS_APP}
#       app-args: >
#           --KVS kvs1
#           --tc-min-required-version 1
#           --tc-required-acknowledgements 1
#           --custom-flow 2
#           --capabilities 6
#       script-args:
#           --in-test-commissioning-method on-network
#           --tc-version-to-simulate 1
#           --tc-user-response-to-simulate 1
#           --qr-code MT:-24J0AFN00KA0648G00
#           --trace-to json:log
#       factoryreset: True
#       quiet: True
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.commissioning import ROOT_ENDPOINT_ID
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CGEN_2_6(MatterBaseTest):
    def desc_TC_CGEN_2_6(self) -> str:
        return "[TC-CGEN-2.6] Verification for CommissioningComplete no terms accepted when required [DUT as Server]"

    def steps_TC_CGEN_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "TH starts commissioning the DUT. It performs all commissioning steps from 'Device discovery and establish commissioning channel' to 'Security setup using CASE', except for TC configuration with SetTCAcknowledgements."),
            TestStep(2, "TH sends CommissioningComplete to DUT and verifies error response."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_6(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Step 1: Commission device without setting TC acknowledgements
        self.step(1)
        # Don't set TCs for the next commissioning and skip CommissioningComplete
        # so we can manually call CommissioningComplete to check the response error code
        commissioner.SetTCRequired(False)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        # Step 2: Send CommissioningComplete and verify error response
        self.step(2)
        response: Clusters.GeneralCommissioning.Commands.CommissioningCompleteResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)

        # Verify that DUT sends CommissioningCompleteResponse Command to TH
        # With ErrorCode as 'TCAcknowledgementsNotReceived'(6)
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCAcknowledgementsNotReceived,
            'Expected TCAcknowledgementsNotReceived error code')


if __name__ == "__main__":
    default_matter_test_main()
