#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

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


if __name__ == "__main__":
    default_matter_test_main()
