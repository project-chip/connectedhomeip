#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${TERMS_AND_CONDITIONS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --KVS kvs1 --tc-min-required-version 1 --tc-required-acknowledgements 1
# test-runner-run/run1/script-args: --in-test-commissioning-method on-network --qr-code MT:-24J0AFN00KA0648G00 --trace-to json:log
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.commissioning import ROOT_ENDPOINT_ID
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CGEN_2_7(MatterBaseTest):
    def desc_TC_CGEN_2_7(self) -> str:
        return "[TC-CGEN-2.7] Verification For CommissioningComplete when SetTCAcknowledgements provides invalid terms [DUT as Server]"

    def steps_TC_CGEN_2_7(self) -> list[TestStep]:
        return [
            TestStep(1,  "TH starts commissioning the DUT. It performs all commissioning steps from ArmFailSafe, except SetTCAcknowledgements and CommissioningComplete.", is_commissioning=False),
            TestStep(2,  "TH sends SetTCAcknowledgements to DUT with the following values:\nTCVersion: Greater than or equal to TCMinRequiredVersion on DUT\nTCUserResponse: 0"),
            TestStep(3,  "TH sends CommissioningComplete to DUT."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_7(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Don't set TCs for the next commissioning and skip CommissioningComplete so we can manually call CommissioningComplete in order to check the response error code
        commissioner.SetTCRequired(False)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method

        self.step(1)
        await self.commission_devices()

        self.step(2)
        response: Clusters.GeneralCommissioning.Commands.SetTCAcknowledgementsResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(TCVersion=2**16 - 1, TCUserResponse=0),
            timedRequestTimeoutMs=1000)

        # Verify that DUT sends SetTCAcknowledgementsResponse Command to TH With ErrorCode as 'RequiredTCNotAccepted'(5).
        asserts.assert_equal(
            response.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kRequiredTCNotAccepted, 'Incorrect error code')

        self.step(3)
        response: Clusters.GeneralCommissioning.Commands.CommissioningCompleteResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)

        # Verify that DUT sends CommissioningCompleteResponse Command to TH With ErrorCode as 'TCAcknowledgementsNotReceived'(6).
        asserts.assert_equal(
            response.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCAcknowledgementsNotReceived, 'Incorrect error code')


if __name__ == "__main__":
    default_matter_test_main()
