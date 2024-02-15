#
#    Copyright (c) 2022 Project CHIP Authors
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

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# Assumes `--enable-key 000102030405060708090a0b0c0d0e0f` on Linux app command line, or a DUT
# that has that Enable Key
kExpectedKey = bytes([b for b in range(16)])

kBadKey = bytes([(b + 1) for b in range(16)])

kAllZerosKey = bytes(b"\x00" * 16)

# Assumes `SampleTestEventTriggerDelegate` as it exists in Linux AppMain.cpp
kValidEventTrigger = 0xFFFF_FFFF_FFF1_0000
kInvalidEventTrigger = 0  # Per TC-DGEN-2.3


class TestEventTrigger(MatterBaseTest):
    @async_test_body
    async def test_all_zeros_key(self):
        dev_ctrl = self.default_controller
        with asserts.assert_raises_regex(InteractionModelError,
                                         "ConstraintError", "All-zero TestEventTrigger key must return ConstraintError"):
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kAllZerosKey,
                                                                              eventTrigger=kValidEventTrigger)
            )

    @async_test_body
    async def test_incorrect_key(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        )
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        with asserts.assert_raises_regex(InteractionModelError,
                                         "ConstraintError", "Bad TestEventTrigger key must return ConstraintError"):
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kBadKey,
                                                                              eventTrigger=kValidEventTrigger)
            )

    @async_test_body
    async def test_correct_key_valid_code(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        )
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        # No response to command --> Success yields "None".
        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kExpectedKey,
                                                                              eventTrigger=kValidEventTrigger)
            )
        )

    @async_test_body
    async def test_correct_key_invalid_code(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        )
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        with asserts.assert_raises_regex(InteractionModelError,
                                         "InvalidCommand",
                                         "Unsupported EventTrigger must return InvalidCommand"):
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                    enableKey=kExpectedKey,
                    eventTrigger=kInvalidEventTrigger
                )
            )

    @async_test_body
    async def test_multiple_command_request_feature_present_if_needed(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        )
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        max_paths_per_invoke = await self.read_single_attribute(self.default_controller, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.BasicInformation.Attributes.MaxPathsPerInvoke)
        feature_map = await self.read_single_attribute(self.default_controller, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.FeatureMap)
        accepted_commands = await self.read_single_attribute(self.default_controller, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.AcceptedCommandList)
        generated_commands = await self.read_single_attribute(self.default_controller, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.GeneratedCommandList)

        has_multiple_paths_per_invoke = max_paths_per_invoke > 1
        has_data_model_test_feature = (feature_map & Clusters.GeneralDiagnostics.Bitmaps.Feature.kDataModelTest) != 0
        has_payload_test_request_command = (Clusters.GeneralDiagnostics.Commands.PayloadTestRequest.command_id in accepted_commands)
        has_payload_test_response_command = (
            Clusters.GeneralDiagnostics.Commands.PayloadTestResponse.command_id in generated_commands)

        logging.info(f"Number of supported invoke paths reported: {max_paths_per_invoke}")
        if has_multiple_paths_per_invoke:
            asserts.assert_true(has_data_model_test_feature, "Must have DMTEST feature when > 1 invoke paths supported")

        if has_data_model_test_feature:
            asserts.assert_true(has_payload_test_request_command, "Must support PayloadTestRequest command when DMTEST supported")
            asserts.assert_true(has_payload_test_response_command,
                                "Must generate PayloadTestResponse command when DMTEST supported")

        if has_data_model_test_feature:
            # Test count too large (above max 2048)
            with asserts.assert_raises_regex(InteractionModelError,
                                             "ConstraintError",
                                             "PayloadTestRequest with request too large (> 2048) must return ConstraintError"):
                await dev_ctrl.SendCommand(
                    self.dut_node_id,
                    endpoint=0,
                    payload=Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
                        enableKey=kExpectedKey,
                        value=0x55,
                        count=2049
                    )
                )

            # Test size > 1 MTU but still legal by constraints, but should fail since that will be too large (unless future TCP transport)
            with asserts.assert_raises_regex(InteractionModelError,
                                             "ResourceExhausted",
                                             "PayloadTestRequest with request too large (> 1 MTU) must return ResourceExhausted"):
                await dev_ctrl.SendCommand(
                    self.dut_node_id,
                    endpoint=0,
                    payload=Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
                        enableKey=kExpectedKey,
                        value=0x55,
                        count=1280 + 1
                    )
                )

            # Test empty count succeeds
            response = await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
                    enableKey=kExpectedKey,
                    value=ord('B'),
                    count=0
                )
            )
            asserts.assert_equal(response.payload, b'', "Expect empty payload for count == 0")

            # Test size that should always fit.
            response = await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
                    enableKey=kExpectedKey,
                    value=ord('A'),
                    count=800
                )
            )
            asserts.assert_equal(response.payload, b'A' * 800, "Expect response to match for count == 800")

        if not has_data_model_test_feature:
            with asserts.assert_raises_regex(InteractionModelError,
                                             "UnsupportedCommand",
                                             "PayloadTestRequest must not be supported if not reported"):
                await dev_ctrl.SendCommand(
                    self.dut_node_id,
                    endpoint=0,
                    payload=Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
                        enableKey=kExpectedKey,
                        value=0x55,
                        count=1
                    )
                )


if __name__ == "__main__":
    default_matter_test_main()
