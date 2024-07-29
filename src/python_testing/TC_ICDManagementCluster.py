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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${LIT_ICD_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import ctypes
from enum import IntEnum

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# Assumes `--enable-key 000102030405060708090a0b0c0d0e0f` on Linux app command line, or a DUT
# that has that Enable Key
kTestEventTriggerKey = bytes([b for b in range(16)])
kRootEndpointId = 0
kMaxUint32Value = 0xFFFFFFFF
kHalfMaxUint32Value = 0xFFFFFFFF >> 1


class ICDTestEventTriggerOperations(IntEnum):
    kAddActiveModeReq = 0x0046000000000001
    kRemoveActiveModeReq = 0x0046000000000002
    kInvalidateHalfCounterValues = 0x0046000000000003
    kInvalidateAllCounterValues = 0x0046000000000004
    kForceMaximumCheckInBackOffState = 0x0046000000000005


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
                endpoint=kRootEndpointId,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=ICDTestEventTriggerOperations.kAddActiveModeReq)
            )
        )

        startingICDCounter = await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=Clusters.Objects.IcdManagement, attribute=Clusters.IcdManagement.Attributes.ICDCounter)

        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=kRootEndpointId,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=ICDTestEventTriggerOperations.kInvalidateHalfCounterValues)
            )
        )
        currentICDCounter = await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=Clusters.Objects.IcdManagement, attribute=Clusters.IcdManagement.Attributes.ICDCounter)

        # Check new current counter value has the expected value
        asserts.assert_equal((ctypes.c_uint32(startingICDCounter + kHalfMaxUint32Value).value),
                             currentICDCounter, "Incorrect ICDCounter value after kInvalidateHalfCounterValues opration")

        # Set starting counter to currentICDCounter for next test
        startingICDCounter = currentICDCounter

        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=kRootEndpointId,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=ICDTestEventTriggerOperations.kInvalidateAllCounterValues)
            )
        )
        currentICDCounter = await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=Clusters.Objects.IcdManagement, attribute=Clusters.IcdManagement.Attributes.ICDCounter)

        # Check new current counter value has the expected value
        asserts.assert_equal((ctypes.c_uint32(startingICDCounter + kMaxUint32Value).value),
                             currentICDCounter, "Incorrect ICDCounter value after kInvalidateHalfCounterValues opration")

        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=kRootEndpointId,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=ICDTestEventTriggerOperations.kRemoveActiveModeReq)
            )
        )

        asserts.assert_is_none(
            await dev_ctrl.SendCommand(
                self.dut_node_id,
                endpoint=kRootEndpointId,
                payload=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=kTestEventTriggerKey,
                                                                              eventTrigger=ICDTestEventTriggerOperations.kForceMaximumCheckInBackOffState)
            )
        )


if __name__ == "__main__":
    default_matter_test_main()
