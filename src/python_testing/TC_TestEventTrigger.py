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

from imaplib import Commands
from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
from chip.interaction_model import Status, InteractionModelError
import chip.clusters as Clusters
import logging
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
        with asserts.assert_raises_regex(InteractionModelError, "ConstraintError", "All-zero TestEventTrigger key must return ConstraintError"):
            await dev_ctrl.SendCommand(self.dut_node_id, endpoint=0, payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kAllZerosKey, eventTrigger=kValidEventTrigger))

    @async_test_body
    async def test_incorrect_key(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(dev_ctrl, self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled)
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        with asserts.assert_raises_regex(InteractionModelError, "ConstraintError", "Bad TestEventTrigger key must return ConstraintError"):
            await dev_ctrl.SendCommand(self.dut_node_id, endpoint=0, payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kBadKey, eventTrigger=kValidEventTrigger))

    @async_test_body
    async def test_correct_key_valid_code(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(dev_ctrl, self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled)
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        # No response to command --> Success yields "None".
        asserts.assert_is_none(await dev_ctrl.SendCommand(self.dut_node_id, endpoint=0, payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kExpectedKey, eventTrigger=kValidEventTrigger)))

    @async_test_body
    async def test_correct_key_invalid_code(self):
        dev_ctrl = self.default_controller
        test_event_triggers_enabled = await self.read_single_attribute(dev_ctrl, self.dut_node_id, endpoint=0, attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled)
        asserts.assert_true(test_event_triggers_enabled, "This test expects Test Event Triggers are Enabled")

        with asserts.assert_raises_regex(InteractionModelError, "InvalidCommand", "Unsupported EventTrigger must return InvalidCommand"):
            await dev_ctrl.SendCommand(self.dut_node_id, endpoint=0, payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kExpectedKey, eventTrigger=kInvalidEventTrigger))


if __name__ == "__main__":
    default_matter_test_main()
