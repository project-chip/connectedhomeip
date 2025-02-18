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
from chip.clusters.Types import Nullable
from chip.commissioning import ROOT_ENDPOINT_ID
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CGEN_2_5(MatterBaseTest):
    def desc_TC_CGEN_2_5(self) -> str:
        return "[TC-CGEN-2.5] Verification for SetTCAcknowledgements [DUT as Server]"

    def pics_TC_CGEN_2_5(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CGEN.S", "CGEN.S.F00"]

    def steps_TC_CGEN_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "TH begins commissioning the DUT and performs the following steps in order:\n* Security setup using PASE\n* Setup fail-safe timer, with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds and the Breadcrumb value as 1\n* Configure information- UTC time, regulatory, etc.", is_commissioning=False),
            TestStep(2, "TH reads TCAcknowledgementsRequired attribute from the DUT"),
            TestStep(3, "TH reads TCUpdateDeadline attribute from the DUT"),
            TestStep(4, "TH reads the FeatureMap from the General Commissioning Cluster."),
            TestStep(5, "TH sends SetTCAcknowledgements to DUT with the following values:\n* TCVersion: PIXIT.CGEN.TCRevision\n* TCUserResponse: PIXIT.CGEN.RequiredTCAcknowledgements"),
            TestStep(6, "TH reads TCAcknowledgementsRequired attribute from the DUT"),
            TestStep(7, "TH continues commissioning with the DUT and performs the steps from 'Operation CSR exchange' through 'Security setup using CASE'"),
            TestStep(8, "TH sends CommissioningComplete to DUT."),
            TestStep(9, "TH reads from the DUT the attribute TCAcceptedVersion."),
            TestStep(10, "TH reads from the DUT the attribute TCAcknowledgements."),
            TestStep(11, "TH reads from the DUT the attribute TCMinRequiredVersion."),
            TestStep(12, "TH reads from the DUT the attribute TCAcknowledgementsRequired."),
        ]

    @async_test_body
    async def test_TC_CGEN_2_5(self):
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

        # Step 2: Read TCAcknowledgementsRequired
        self.step(2)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired)])
        tc_acknowledgements_required = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tc_acknowledgements_required, True, "TCAcknowledgementsRequired should be True.")

        # Step 3: Read TCUpdateDeadline
        self.step(3)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCUpdateDeadline)],
        )
        tc_update_deadline = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCUpdateDeadline]

        # Validate the value is of type Optional[uint32], e.g. either Nullable or within the 32-bit range.
        if not isinstance(tc_update_deadline, Nullable):
            matter_asserts.assert_valid_uint32(tc_update_deadline, "TCUpdateDeadline exceeds uint32 range")

        # Step 4: Verify TC feature flag in FeatureMap
        self.step(4)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.FeatureMap)],
        )
        feature_map = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.FeatureMap]
        asserts.assert_equal(feature_map & Clusters.GeneralCommissioning.Bitmaps.Feature.kTermsAndConditions,
                             Clusters.GeneralCommissioning.Bitmaps.Feature.kTermsAndConditions, "TC feature flag is not set.")

        # Step 5: Send SetTCAcknowledgements
        self.step(5)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=tc_version_to_simulate, TCUserResponse=tc_user_response_to_simulate
            ),
        )
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "SetTCAcknowledgementsResponse error code is not OK.",
        )

        # Step 6: Verify TCAcknowledgementsRequired is False
        self.step(6)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired)])
        tc_acknowledgements_required = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tc_acknowledgements_required, False, "TCAcknowledgementsRequired should be False.")

        # Step 7: Continue with CSR and CASE setup
        self.step(7)

        # Step 8: Send CommissioningComplete
        self.step(8)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
        )
        asserts.assert_equal(
            response.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "CommissioningCompleteResponse error code is not OK.",
        )

        # Step 9: Verify TCAcceptedVersion
        self.step(9)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion)])
        accepted_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        asserts.assert_equal(accepted_version, tc_version_to_simulate, "TCAcceptedVersion does not match expected value.")
        matter_asserts.assert_valid_uint16(accepted_version, "TCAcceptedVersion is not a uint16 type.")

        # Step 10: Verify TCAcknowledgements
        self.step(10)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements)])
        acknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        asserts.assert_equal(acknowledgements, tc_user_response_to_simulate, "TCAcknowledgements does not match expected value.")
        matter_asserts.assert_valid_uint16(accepted_version, "TCAcknowledgements is not a map16 type.")

        # Step 11: Verify TCMinRequiredVersion
        self.step(11)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion)])
        min_required_version = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion]
        matter_asserts.assert_valid_uint16(min_required_version, "TCMinRequiredVersion is not a uint16 type.")

        # Step 12: Verify TCAcknowledgementsRequired is False again
        self.step(12)
        response = await commissioner.ReadAttribute(nodeid=self.dut_node_id, attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired)])
        tc_acknowledgements_required = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tc_acknowledgements_required, False, "TCAcknowledgementsRequired should be False.")


if __name__ == "__main__":
    default_matter_test_main()
