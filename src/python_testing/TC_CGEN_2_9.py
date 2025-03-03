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
#           --tc-min-required-version 1
#           --tc-required-acknowledgements 1
#           --custom-flow 2
#           --capabilities 6
#       script-args:
#           --PICS src/app/tests/suites/certification/ci-pics-values
#           --in-test-commissioning-method on-network
#           --int-arg PIXIT.CGEN.FailsafeExpiryLengthSeconds:900
#           --int-arg PIXIT.CGEN.RequiredTCAcknowledgements:1
#           --int-arg PIXIT.CGEN.TCRevision:1
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

    async def remove_commissioner_fabric(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        fabrics: list[Clusters.OperationalCredentials.Structs.FabricDescriptorStruct] = await self.read_single_attribute(
            dev_ctrl=commissioner,
            node_id=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics)

        # Re-order the list of fabrics so that the test harness admin fabric is removed last
        commissioner_fabric = next((fabric for fabric in fabrics if fabric.fabricIndex == commissioner.fabricId), None)
        fabrics.remove(commissioner_fabric)
        fabrics.append(commissioner_fabric)

        for fabric in fabrics:
            response: Clusters.OperationalCredentials.Commands.NOCResponse = await commissioner.SendCommand(
                nodeid=self.dut_node_id,
                endpoint=ROOT_ENDPOINT_ID,
                payload=Clusters.OperationalCredentials.Commands.RemoveFabric(fabric.fabricIndex),
            )
            asserts.assert_equal(response.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk)

    def desc_TC_CGEN_2_9(self) -> str:
        return "[TC-CGEN-2.9] Verification that TCAcknowledgements is reset after all fabrics removed [DUT as Server]"

    def pics_TC_CGEN_2_9(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CGEN.S", "CGEN.S.F00"]

    def steps_TC_CGEN_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "TH begins commissioning the DUT and performs the following steps in order:\n* Security setup using PASE\n* Setup fail-safe timer, with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds and the Breadcrumb value as 1\n* Configure information- UTC time, regulatory, etc.", is_commissioning=False),
            TestStep(2, "TH sends SetTCAcknowledgements to DUT with the following values:\n* TCVersion: PIXIT.CGEN.TCRevision\n* TCUserResponse: PIXIT.CGEN.RequiredTCAcknowledgements"),
            TestStep(3, "TH continues commissioning with the DUT and performs the steps from 'Operation CSR exchange' through 'Security setup using CASE'"),
            TestStep(4, "TH sends CommissioningComplete to DUT."),
            TestStep(5, "TH removes all fabrics from DUT with RemoveFabric."),
            TestStep(6, "Perform the necessary actions to put the DUT into a commissionable state."),
            TestStep(7, "TH begins commissioning the DUT and performs all steps from 'Device discovery and establish commissioning channel' through 'Security setup using CASE', skipping the 'Configure TC acknowledgements' step"),
            TestStep(8, "TH sends CommissioningComplete to DUT."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_9(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        failsafe_expiry_length_seconds = self.matter_test_config.global_test_params['PIXIT.CGEN.FailsafeExpiryLengthSeconds']
        tc_version_to_simulate = self.matter_test_config.global_test_params['PIXIT.CGEN.TCRevision']
        tc_user_response_to_simulate = self.matter_test_config.global_test_params['PIXIT.CGEN.RequiredTCAcknowledgements']

        if not self.check_pics("CGEN.S.F00"):
            asserts.skip('Root endpoint does not support the [commissioning] feature under test')
            return

        # Step 1: Begin commissioning with PASE and failsafe
        self.step(1)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None
        await self.commission_devices()

        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.ArmFailSafe(
                expiryLengthSeconds=failsafe_expiry_length_seconds, breadcrumb=1),
        )
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "ArmFailSafeResponse error code is not OK.",
        )

        # Step 2: Send SetTCAcknowledgements
        self.step(2)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=tc_version_to_simulate, TCUserResponse=tc_user_response_to_simulate
            ),
        )

        # Verify SetTCAcknowledgements response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "SetTCAcknowledgements failed",
        )

        # Step 3: Continue with CSR and CASE setup
        self.step(3)
        # Note: CSR and CASE setup is handled by the commissioning process

        # Step 4: Send CommissioningComplete
        self.step(4)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
        )

        # Verify CommissioningComplete response
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "First CommissioningComplete failed",
        )

        # Step 5: Remove all fabrics
        self.step(5)
        await self.remove_commissioner_fabric()

        # Close the commissioner session with the device to clean up resources
        commissioner.CloseSession(nodeid=self.dut_node_id)

        # Step 6: Put device in commissioning mode (requiring user input, so skip in CI)
        self.step(6)
        if not self.check_pics('PICS_USER_PROMPT'):
            self.skip_all_remaining_steps(7)
            return

        self.wait_for_user_input(prompt_msg="Set the DUT into commissioning mode")

        # Step 7: Commission without TC acknowledgements
        self.step(7)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None
        await self.commission_devices()

        # Step 8: Verify CommissioningComplete fails
        self.step(8)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
        )

        # Verify CommissioningComplete fails with correct error
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCAcknowledgementsNotReceived,
            "Expected TCAcknowledgementsNotReceived error after fabric removal",
        )


if __name__ == "__main__":
    default_matter_test_main()
