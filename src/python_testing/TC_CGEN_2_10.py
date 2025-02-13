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


class TC_CGEN_2_10(MatterBaseTest):
    def desc_TC_CGEN_2_10(self) -> str:
        return "[TC-CGEN-2.10] Verification that required terms can't be unset from TCAcknowledgements with SetTCAcknowledgements [DUT as Server]"

    def pics_TC_CGEN_2_10(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CGEN.S", "CGEN.S.F00"]

    def steps_TC_CGEN_2_10(self) -> list[TestStep]:
        return [
            TestStep(1, "TH reads from the DUT the attribute TCAcceptedVersion. Store the value as acceptedVersion.", is_commissioning=False),
            TestStep(2, "TH reads from the DUT the attribute TCAcknowledgements. Store the value as userAcknowledgements."),
            TestStep(3, "TH Sends the SetTCAcknowledgements command to the DUT with the fields set as follows:\n* TCVersion: 0\n* TCUserResponse: 65535"),
            TestStep(4, "TH reads from the DUT the attribute TCAcceptedVersion."),
            TestStep(5, "TH reads from the DUT the attribute TCAcknowledgements."),
            TestStep(6, "TH Sends the SetTCAcknowledgements command to the DUT with the fields set as follows:\n* TCVersion: acceptedVersion + 1\n* TCUserResponse: 0"),
            TestStep(7, "TH reads from the DUT the attribute TCAcceptedVersion."),
            TestStep(8, "TH reads from the DUT the attribute TCAcknowledgements."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_10(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        if not self.check_pics("CGEN.S.F00"):
            asserts.skip('Root endpoint does not support the [commissioning] feature under test')
            return

        # Step 1: Begin commissioning with PASE and failsafe
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None
        await self.commission_devices()

        # Step 1: Read TCAcceptedVersion
        self.step(1)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion)])
        accepted_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]

        # Step 2: Read TCAcknowledgements
        self.step(2)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements)])
        user_acknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]

        # Step 3: Send SetTCAcknowledgements with invalid version
        self.step(3)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(TCVersion=0, TCUserResponse=65535),
        )

        # Verify TCMinVersionNotMet error
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCMinVersionNotMet,
            "Expected TCMinVersionNotMet error",
        )

        # Step 4: Verify TCAcceptedVersion unchanged
        self.step(4)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion)])
        current_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        asserts.assert_equal(current_version, accepted_version, "TCAcceptedVersion changed unexpectedly")

        # Step 5: Verify TCAcknowledgements unchanged
        self.step(5)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements)])
        current_acknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        asserts.assert_equal(current_acknowledgements, user_acknowledgements, "TCAcknowledgements changed unexpectedly")

        # Step 6: Send SetTCAcknowledgements with invalid response
        self.step(6)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=accepted_version + 1, TCUserResponse=0
            ),
        )

        # Verify RequiredTCNotAccepted error
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kRequiredTCNotAccepted,
            "Expected RequiredTCNotAccepted error",
        )

        # Step 7: Verify TCAcceptedVersion still unchanged
        self.step(7)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion)])
        current_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        asserts.assert_equal(current_version, accepted_version, "TCAcceptedVersion changed unexpectedly after second attempt")

        # Step 8: Verify TCAcknowledgements still unchanged
        self.step(8)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements)])
        current_acknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        asserts.assert_equal(
            current_acknowledgements,
            user_acknowledgements,
            "TCAcknowledgements changed unexpectedly after second attempt",
        )


if __name__ == "__main__":
    default_matter_test_main()
