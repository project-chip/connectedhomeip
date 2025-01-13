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


class TC_CGEN_2_11(MatterBaseTest):
    def desc_TC_CGEN_2_11(self) -> str:
        return "[TC-CGEN-2.11] Verification that TCAcknowledgements and TCAcceptedVersion can be updated after being commissioned [DUT as Server]"

    def steps_TC_CGEN_2_11(self) -> list[TestStep]:
        return [
            TestStep(1, "TH begins commissioning the DUT with PASE, failsafe setup, and basic configuration"),
            TestStep(2, "TH sends SetTCAcknowledgements with initial TC values"),
            TestStep(3, "TH sends CommissioningComplete to DUT"),
            TestStep(4, "TH sends SetTCAcknowledgements with updated TC version"),
            TestStep(5, "Verify TCAcceptedVersion is updated"),
            TestStep(6, "TH sends SetTCAcknowledgements with maximum acknowledgements"),
            TestStep(7, "Verify TCAcknowledgements is updated")
        ]

    @async_test_body
    async def test_TC_CGEN_2_11(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Step 1: Begin commissioning with PASE and failsafe
        self.step(1)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        # Step 2: Send initial SetTCAcknowledgements
        self.step(2)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=self.pixit['CGEN']['TCRevision'],
                TCUserResponse=self.pixit['CGEN']['RequiredTCAcknowledgements']),
            timedRequestTimeoutMs=1000)

        # Verify initial SetTCAcknowledgements response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'Initial SetTCAcknowledgements failed')

        # Step 3: Send CommissioningComplete
        self.step(3)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)

        # Verify CommissioningComplete response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'CommissioningComplete failed')

        # Step 4: Send SetTCAcknowledgements with updated version
        self.step(4)
        updated_tc_version = self.pixit['CGEN']['TCRevision'] + 1
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=updated_tc_version,
                TCUserResponse=self.pixit['CGEN']['RequiredTCAcknowledgements']),
            timedRequestTimeoutMs=1000)

        # Verify SetTCAcknowledgements response with updated version
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'SetTCAcknowledgements with updated version failed')

        # Step 5: Verify TCAcceptedVersion is updated
        self.step(5)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion)])
        current_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        asserts.assert_equal(
            current_version,
            updated_tc_version,
            'TCAcceptedVersion not updated correctly')

        # Step 6: Send SetTCAcknowledgements with maximum acknowledgements
        self.step(6)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=updated_tc_version,
                TCUserResponse=65535),
            timedRequestTimeoutMs=1000)

        # Verify SetTCAcknowledgements response with maximum acknowledgements
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            'SetTCAcknowledgements with maximum acknowledgements failed')

        # Step 7: Verify TCAcknowledgements is updated
        self.step(7)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements)])
        current_acknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        asserts.assert_equal(
            current_acknowledgements,
            65535,
            'TCAcknowledgements not updated to maximum value')


if __name__ == "__main__":
    default_matter_test_main()
