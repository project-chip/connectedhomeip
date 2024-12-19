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
# test-runner-run/run1/app-args: --KVS kvs1
# test-runner-run/run1/script-args: --in-test-commissioning-method on-network --qr-code MT:-24J0AFN00KA0648G00 --trace-to json:log
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

        # Don't set TCs for the next commissioning and skip CommissioningComplete so we can manually call CommissioningComplete in order to check the response error code
        self.step(1)
        commissioner.SetTCRequired(False)
        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        self.step(2)
        response: dict[int, Clusters.GeneralCommissioning] = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired),
            ])
        tcAcceptedVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        tcMinRequiredVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion]
        tcAcknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        tcAcknowledgementsRequired = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]

        # Verify that TCAcknowledgementsRequired value is representable in the 'Bool' type
        # Verify that TCAcknowledgementsRequired value is True
        asserts.assert_equal(tcAcknowledgementsRequired, True, 'Incorrect TCAcknowledgementsRequired')

        self.step(3)
        response: Clusters.GeneralCommissioning.Commands.SetTCAcknowledgementsResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(TCVersion=2**16 - 1, TCUserResponse=2**16 - 1),
            timedRequestTimeoutMs=1000)
        # Verify that DUT sends SetTCAcknowledgementsResponse Command to TH With ErrorCode as 'OK'(0).
        asserts.assert_equal(response.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             'Incorrect error code')

        self.step(4)
        response: Clusters.GeneralCommissioning.Commands.CommissioningCompleteResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=1000)
        # Verify that DUT sends CommissioningCompleteResponse Command to TH With ErrorCode as 'OK'(0).
        asserts.assert_equal(response.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             'Incorrect error code')

        # Read attributes of interest
        response: dict[int, Clusters.GeneralCommissioning] = await commissioner.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgements),
                (ROOT_ENDPOINT_ID, Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired),
            ])
        tcAcceptedVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcceptedVersion]
        tcMinRequiredVersion = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCMinRequiredVersion]
        tcAcknowledgements = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgements]
        tcAcknowledgementsRequired = response[ROOT_ENDPOINT_ID][Clusters.GeneralCommissioning][Clusters.GeneralCommissioning.Attributes.TCAcknowledgementsRequired]

        self.step(5)
        # Verify that TCAcceptedVersion value fits in the 'uint16' type
        asserts.assert_less(tcAcceptedVersion, 2**16, 'Incorrect TCAcceptedVersion')
        # Verify that TCAcceptedVersion is the value sent in step 3
        asserts.assert_equal(tcAcceptedVersion, 2**16 - 1, 'Incorrect TCAcceptedVersion')

        self.step(6)
        # Verify that TCAcknowledgements is a value representable in the map16 type
        # Verify that TCAcknowledgements is the value sent in step 3
        asserts.assert_equal(tcAcknowledgements, 2**16 - 1, 'Incorrect TCAcknowledgements')

        self.step(7)
        # Verify that TCMinRequiredVersion value fits in the 'uint16' type
        asserts.assert_less(tcMinRequiredVersion, 2**16, 'Incorrect TCMinRequiredVersion')

        self.step(8)
        # Verify that TCAcknowledgementsRequired value is False
        asserts.assert_equal(tcAcknowledgementsRequired, False, 'Incorrect TCAcknowledgementsRequired')

        self.step(9)
        response: Clusters.GeneralCommissioning.Commands.SetTCAcknowledgementsResponse = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=Clusters.GeneralCommissioning.Commands.SetTCAcknowledgements(TCVersion=0, TCUserResponse=0),
            timedRequestTimeoutMs=1000)
        # Verify that DUT sends SetTCAcknowledgementsResponse Command to TH With ErrorCode as 'TCMinVersionNotMet'(7)
        asserts.assert_equal(response.errorCode,
                             Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kTCMinVersionNotMet, 'Incorrect error code')
        # Verify that TCAcceptedVersion and TCAcknowledgements still contain the values sent in step 3.


if __name__ == "__main__":
    default_matter_test_main()