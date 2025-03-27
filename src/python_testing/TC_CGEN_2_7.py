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


class TC_CGEN_2_7(MatterBaseTest):
    def desc_TC_CGEN_2_7(self) -> str:
        return "[TC-CGEN-2.7] Verification for CommissioningComplete when SetTCAcknowledgements provides invalid terms [DUT as Server]"

    def pics_TC_CGEN_2_7(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CGEN.S", "CGEN.S.F00"]

    def steps_TC_CGEN_2_7(self) -> list[TestStep]:
        return [
            TestStep(1, "TH begins commissioning the DUT and performs the following steps in order:\n* Security setup using PASE\n* Setup fail-safe timer, with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds and the Breadcrumb value as 1\n* Configure information- UTC time, regulatory, etc.", is_commissioning=False),
            TestStep(2, "TH reads from the DUT the attribute TCMinRequiredVersion. Store the value as minVersion."),
            TestStep(3, "TH sends SetTCAcknowledgements to DUT with the following values:\n* TCVersion: minVersion\n* TCUserResponse: 0"),
            TestStep(4, "TH continues commissioning with the DUT and performs the steps from 'Operation CSR exchange' through 'Security setup using CASE'"),
            TestStep(5, "TH sends CommissioningComplete to DUT."),
            TestStep(6, "TH sends SetTCAcknowledgements to DUT with the following values:\n* TCVersion: PIXIT.CGEN.TCRevision\n* TCUserResponse: PIXIT.CGEN.RequiredTCAcknowledgements"),
            TestStep(7, "TH sends CommissioningComplete to DUT."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_7(self):
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

        # Step 2: Read TCMinRequiredVersion
        self.step(2)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion)])
        min_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion]

        # Step 3: Send SetTCAcknowledgements with invalid response
        self.step(3)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=min_version, TCUserResponse=0
            ),
        )

        # Verify error code is RequiredTCNotAccepted
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kRequiredTCNotAccepted,
            "Expected RequiredTCNotAccepted error code",
        )

        # Step 4: Continue with CSR and CASE setup
        self.step(4)
        # Note: CSR and CASE setup is handled by the commissioning process

        # Step 5: Send CommissioningComplete and verify it fails
        self.step(5)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
        )

        # Verify error code is TCAcknowledgementsNotReceived
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCAcknowledgementsNotReceived,
            "Expected TCAcknowledgementsNotReceived error code",
        )

        # Step 6: Send SetTCAcknowledgements with valid values
        self.step(6)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=tc_version_to_simulate, TCUserResponse=tc_user_response_to_simulate
            ),
        )

        # Verify error code is OK
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "Expected OK response for valid TC acknowledgements",
        )

        # Step 7: Send CommissioningComplete and verify success
        self.step(7)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
        )

        # Verify error code is OK
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "Expected OK response for CommissioningComplete",
        )


if __name__ == "__main__":
    default_matter_test_main()
