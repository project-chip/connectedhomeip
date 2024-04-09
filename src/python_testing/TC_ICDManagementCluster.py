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

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# Assumes `--enable-key 000102030405060708090a0b0c0d0e0f` on Linux app command line, or a DUT
# that has that Enable Key
kTestEventTriggerKey = bytes([b for b in range(16)])

kAddActiveModeReq = 0x0046000000000001
kRemoveActiveModeReq = 0x0046000000000002


class TestICDManagementCluster(MatterBaseTest):
    @async_test_body
    async def test_active_mode_test_event_trigger(self):
        dev_ctrl = self.default_controller
        is_test_event_triggers_enabled = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        )
        asserts.assert_true(is_test_event_triggers_enabled,
                            "This test expects Test Event Triggers are Enabled")

        # No response to command --> Success yields "None".
        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=kAddActiveModeReq)
            )
        )

        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=0,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=kRemoveActiveModeReq)
            )
        )


if __name__ == "__main__":
    default_matter_test_main()
