#
#    Copyright (c) 2024-2025 Project CHIP Authors
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


class TC_CGEN_2_5(MatterBaseTest):
    def desc_TC_CGEN_2_5(self) -> str:
        return "[TC-CGEN-2.5] Verification For SetTCAcknowledgements [DUT as Server]"

    def steps_TC_CGEN_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "TH starts commissioning the DUT. It performs all commissioning steps from ArmFailSafe, except SetTCAcknowledgements and CommissioningComplete.", is_commissioning=False),
            TestStep(2, "TH reads TCAcknowledgementsRequired attribute from the DUT."),
            TestStep(3, "TH sends SetTCAcknowledgements to DUT with the following values:\nTCVersion: Greater than or equal to TCMinRequiredVersion on DUT\nTCUserResponse: All terms required by DUT accepted"),
            TestStep(4, "TH sends CommissioningComplete to DUT."),
            TestStep(5, "TH reads TCAcceptedVersion attribute from the DUT."),
            TestStep(6, "TH reads TCAcknowledgements attribute from the DUT."),
            TestStep(7, "TH reads TCMinRequiredVersion attribute from the DUT."),
            TestStep(8, "TH reads TCAcknowledgementsRequired attribute from the DUT."),
            TestStep(9, "TH sends the SetTCAcknowledgements command to the DUT with the fields set as follows:\nTCVersion: 0\nTCUserResponse: 0"),
        ]

    @async_test_body
    async def test_TC_CGEN_2_5(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Step 1: Begin commissioning with PASE and failsafe
        self.step(1)
        commissioner.SetTCRequired(False)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        # Step 2: Read TCAcknowledgementsRequired
        self.step(2)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired)])
        tcAcknowledgementsRequired = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tcAcknowledgementsRequired, True, 'Incorrect TCAcknowledgementsRequired')

        # Step 3: Read TCUpdateDeadline
        self.step(3)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCUpdateDeadline)])
        tcUpdateDeadline = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCUpdateDeadline]
        asserts.assert_less(tcUpdateDeadline, 2**32, 'TCUpdateDeadline exceeds uint32 range')

        # Step 4: Read FeatureMap
        self.step(4)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.FeatureMap)])
        featureMap = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.FeatureMap]
        asserts.assert_equal(featureMap & 0x1, 0x1, 'TC feature flag not set')

        # Step 5: Send SetTCAcknowledgements
        self.step(5)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(
                TCVersion=self.pixit['CGEN']['TCRevision'],
                TCUserResponse=self.pixit['CGEN']['RequiredTCAcknowledgements']),
            timedRequestTimeoutMs=1000)
        asserts.assert_equal(response.errorCode,
                             Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             'Incorrect error code')

        # Step 6: Verify TCAcknowledgementsRequired is False
        self.step(6)
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired)])
        tcAcknowledgementsRequired = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tcAcknowledgementsRequired, False, 'TCAcknowledgementsRequired should be False')

        # Step 7: Complete CSR exchange and CASE security setup
        self.step(7)
        # Note: Implementation needed for CSR exchange and CASE setup

        # Step 8: Send CommissioningComplete
        self.step(8)
        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)
        asserts.assert_equal(response.errorCode,
                             Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             'Incorrect error code')

        # Steps 9-12: Read and verify final attribute values
        response = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired),
            ])

        # Step 9: Verify TCAcceptedVersion
        self.step(9)
        tcAcceptedVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        asserts.assert_less(tcAcceptedVersion, 2**16, 'TCAcceptedVersion exceeds uint16 range')
        asserts.assert_equal(tcAcceptedVersion, self.pixit['CGEN']['TCRevision'], 'Incorrect TCAcceptedVersion')

        # Step 10: Verify TCAcknowledgements
        self.step(10)
        tcAcknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        asserts.assert_less(tcAcknowledgements, 2**16, 'TCAcknowledgements exceeds map16 range')
        asserts.assert_equal(tcAcknowledgements, self.pixit['CGEN']['RequiredTCAcknowledgements'], 'Incorrect TCAcknowledgements')

        # Step 11: Verify TCMinRequiredVersion
        self.step(11)
        tcMinRequiredVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion]
        asserts.assert_less(tcMinRequiredVersion, 2**16, 'TCMinRequiredVersion exceeds uint16 range')

        # Step 12: Verify TCAcknowledgementsRequired
        self.step(12)
        tcAcknowledgementsRequired = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]
        asserts.assert_equal(tcAcknowledgementsRequired, False, 'TCAcknowledgementsRequired should be False')


if __name__ == "__main__":
    default_matter_test_main()
