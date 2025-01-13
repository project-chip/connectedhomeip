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


class TC_CGEN_2_9(MatterBaseTest):
    def desc_TC_CGEN_2_9(self) -> str:
        return "[TC-CGEN-2.9] Verification that TCAcknowledgements is reset after all fabrics removed [DUT as Server]"

    def steps_TC_CGEN_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "TH begins commissioning the DUT and performs the following steps in order:\n"
                     "* Security setup using PASE\n"
                     "* Setup fail-safe timer, with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds and the Breadcrumb value as 1\n"
                     "* Configure information- UTC time, regulatory, etc."),
            TestStep(2, "TH sends SetTCAcknowledgements with required values"),
            TestStep(3, "TH continues commissioning with CSR exchange through CASE setup"),
            TestStep(4, "TH sends CommissioningComplete to DUT"),
            TestStep(5, "TH removes all fabrics from DUT with RemoveFabric"),
            TestStep(6, "Put DUT into commissionable state"),
            TestStep(7, "TH begins commissioning without TC acknowledgements"),
            TestStep(8, "Verify CommissioningComplete fails with TCAcknowledgementsNotReceived")
        ]

    @async_test_body
    async def test_TC_CGEN_2_9(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Step 1: Begin commissioning with PASE and failsafe
        self.step(1)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        # Step 2: Send SetTCAcknowledgements
        self.step(2)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=self.pixit['CGEN']['TCRevision'],
                TCUserResponse=self.pixit['CGEN']['RequiredTCAcknowledgements']),
            timedRequestTimeoutMs=1000)

        # Verify SetTCAcknowledgements response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'SetTCAcknowledgements failed')

        # Step 3: Continue with CSR and CASE setup
        self.step(3)
        # Note: CSR and CASE setup is handled by the commissioning process

        # Step 4: Send CommissioningComplete
        self.step(4)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)

        # Verify CommissioningComplete response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'First CommissioningComplete failed')

        # Step 5: Remove all fabrics
        self.step(5)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.OperationalCredentials.Commands.RemoveFabric(),
            timedRequestTimeoutMs=1000)

        # Verify RemoveFabric response
        asserts.assert_equal(
            response.errorCode,
            Clusters.OperationalCredentials.Enums.OperationalCredentialsStatusEnum.kSuccess,
            'RemoveFabric failed')

        # Step 6: Put device in commissioning mode
        self.step(6)
        self.wait_for_user_input(prompt_msg="Set the DUT into commissioning mode")

        # Step 7: Commission without TC acknowledgements
        self.step(7)
        commissioner.SetTCRequired(False)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        # Step 8: Verify CommissioningComplete fails
        self.step(8)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)

        # Verify CommissioningComplete fails with correct error
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCAcknowledgementsNotReceived,
            'Expected TCAcknowledgementsNotReceived error after fabric removal')


if __name__ == "__main__":
    default_matter_test_main()
