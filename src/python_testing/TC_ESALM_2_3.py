#
#    Copyright (c) 2026 Project CHIP Authors
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
# test-runner-runs:
#   run1:
#     app: ${ELECTRICAL_PROTECTION_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 2
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --string-arg PIXIT.ESALM.TEST_EVENT_TRIGGER:0x00a1000000000001
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

cluster = Clusters.ElectricalAlarm
_A = cluster.Bitmaps.AlarmBitmap

# Mask of every defined AlarmBitmap bit, derived from the cluster enum rather than hardcoded.
_ALL_DEFINED_ALARM_BITS = 0
for _alarm_bit in _A:
    _ALL_DEFINED_ALARM_BITS |= _alarm_bit

# Simulate-trigger code per alarm, from the PIXIT Variable Values table of the test plan.
# Step 10a selects the code corresponding to the latched bit under test.
_BASE_TRIGGER = 0x00A1000000000000
_SIMULATE_TRIGGER = {
    _A.kOverVoltage: _BASE_TRIGGER | 0x01,
    _A.kUnderVoltage: _BASE_TRIGGER | 0x03,
    _A.kOverFrequency: _BASE_TRIGGER | 0x05,
    _A.kUnderFrequency: _BASE_TRIGGER | 0x07,
    _A.kOverPower: _BASE_TRIGGER | 0x09,
    _A.kUnderPower: _BASE_TRIGGER | 0x0B,
    _A.kOverCurrent: _BASE_TRIGGER | 0x0D,
    _A.kUnderCurrent: _BASE_TRIGGER | 0x0F,
    _A.kPowerImported: _BASE_TRIGGER | 0x11,
    _A.kPowerExported: _BASE_TRIGGER | 0x13,
}


class TC_ESALM_2_3(MatterBaseTest):

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_ESALM_2_3(self):
        """[TC-ESALM-2.3] ModifyEnabledAlarms and Reset Commands with Server as DUT

        Verify that the ModifyEnabledAlarms command correctly updates the Mask attribute,
        enforces constraint violations for unsupported alarm bits, and that the Reset
        command clears latched alarms from the State attribute.
        """
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes
        cmds = cluster.Commands

        self.step(1, "Commission DUT to TH", is_commissioning=True)

        self.step(2, "TH reads AcceptedCommandList.", expectation="SUCCESS.")
        accepted_cmds = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
        has_modify = cmds.ModifyEnabledAlarms.command_id in accepted_cmds
        has_reset = cmds.Reset.command_id in accepted_cmds

        self.step("2a", "TH reads Supported.", expectation="DUT returns a map32 AlarmBitmap.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)

        self.step("2b", "TH reads Mask. Store value as InitialMask.",
                  expectation="DUT returns a map32 AlarmBitmap.")
        initial_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)

        self.step("2c", "TH reads State.", expectation="DUT returns a map32 AlarmBitmap.")
        initial_state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)

        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        has_latch = attrs.Latch.attribute_id in attribute_list

        self.step(3, "TH sends ModifyEnabledAlarms with Mask set to Supported.", expectation="SUCCESS.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=supported), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step(4, "TH reads Mask.", expectation="DUT returns a value equal to Supported.")
        if has_modify:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, supported, "Mask should equal Supported after ModifyEnabledAlarms(Supported)")
        else:
            self.mark_current_step_skipped()

        self.step(5, "TH sends ModifyEnabledAlarms with Mask set to 0.", expectation="SUCCESS.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=0), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH reads Mask.", expectation="DUT returns 0.")
        if has_modify:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, 0, "Mask should be 0 after ModifyEnabledAlarms(0)")
        else:
            self.mark_current_step_skipped()

        unsupported_bit = None
        for _bit in range(32):
            _candidate = 1 << _bit
            if (_candidate & _ALL_DEFINED_ALARM_BITS) and not (int(supported) & _candidate):
                unsupported_bit = _candidate
                break

        self.step(7, "TH sends ModifyEnabledAlarms with Mask containing a bit not set in Supported.",
                  expectation="DUT returns CONSTRAINT_ERROR or INVALID_COMMAND.")
        if has_modify and unsupported_bit is not None:
            try:
                await self.send_single_cmd(
                    cmd=cmds.ModifyEnabledAlarms(mask=unsupported_bit), endpoint=endpoint)
                asserts.fail("Expected INVALID_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand,
                                     f"Expected INVALID_COMMAND, got {e.status}")
        else:
            self.mark_current_step_skipped()

        self.step("7a", "TH reads Mask.", expectation="Mask is unchanged by the rejected command.")
        if has_modify and unsupported_bit is not None:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, 0, "Mask changed after a rejected ModifyEnabledAlarms")
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH sends ModifyEnabledAlarms with Mask set to InitialMask.", expectation="SUCCESS.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=initial_mask), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step("8a", "TH reads Mask.", expectation="DUT returns a value equal to InitialMask.")
        if has_modify:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, initial_mask, "Mask should equal InitialMask after restore")
        else:
            self.mark_current_step_skipped()

        self.step(9, "IF ModifyEnabledAlarms is not in AcceptedCommandList: TH sends ModifyEnabledAlarms.",
                  expectation="DUT returns UNSUPPORTED_COMMAND.")
        if not has_modify:
            try:
                await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=0), endpoint=endpoint)
                asserts.fail("Expected UNSUPPORTED_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedCommand,
                                     f"Expected UNSUPPORTED_COMMAND, got {e.status}")
        else:
            self.mark_current_step_skipped()

        target_bit = None
        self.step(10, "TH reads Latch. Store value as LatchBits and identify the lowest-numbered bit set.",
                  expectation="DUT returns a map32 AlarmBitmap.")
        if has_latch:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
            for _bit in range(32):
                _candidate = 1 << _bit
                if int(latch) & _candidate & int(supported):
                    target_bit = _candidate
                    break
        else:
            self.mark_current_step_skipped()

        can_latch_test = target_bit is not None and target_bit in _SIMULATE_TRIGGER and has_reset

        self.step("10a", "TH sends the TestEventTrigger corresponding to TargetBit.", expectation="SUCCESS.")
        if can_latch_test:
            await self.send_test_event_triggers(eventTrigger=_SIMULATE_TRIGGER[target_bit])
        else:
            self.mark_current_step_skipped()

        self.step("10b", "TH reads State.", expectation="TargetBit is set in State.")
        if can_latch_test:
            state = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_true(int(state) & target_bit, "TargetBit should be set in State after the trigger")
        else:
            self.mark_current_step_skipped()

        self.step("10c", "TH sends Reset with TargetBit set in the Alarms field.", expectation="SUCCESS.")
        if can_latch_test:
            await self.send_single_cmd(cmd=cmds.Reset(alarms=target_bit), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step("10d", "TH reads State.", expectation="TargetBit is cleared in State.")
        if can_latch_test:
            state_after = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_equal(int(state_after) & target_bit, 0,
                                 "TargetBit should be cleared in State after Reset")
        else:
            self.mark_current_step_skipped()

        inactive_bit = None
        self.step(11, "TH sends Reset with a bit that is not active in State.", expectation="SUCCESS.")
        state_before_11 = initial_state
        if has_reset:
            state_before_11 = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            for _bit in range(32):
                _candidate = 1 << _bit
                if (int(supported) & _candidate) and not (int(state_before_11) & _candidate):
                    inactive_bit = _candidate
                    break
            if inactive_bit is not None:
                await self.send_single_cmd(cmd=cmds.Reset(alarms=inactive_bit), endpoint=endpoint)
            else:
                self.mark_current_step_skipped()
        else:
            self.mark_current_step_skipped()

        self.step("11a", "TH reads State.", expectation="State is unchanged.")
        if has_reset and inactive_bit is not None:
            state_after = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_equal(state_after, state_before_11,
                                 "State must be unchanged after Reset with an inactive alarm bit")
        else:
            self.mark_current_step_skipped()

        self.step(12, "IF Reset is not in AcceptedCommandList: TH sends Reset (0x00).",
                  expectation="DUT returns UNSUPPORTED_COMMAND.")
        if not has_reset:
            try:
                await self.send_single_cmd(cmd=cmds.Reset(alarms=0), endpoint=endpoint)
                asserts.fail("Expected UNSUPPORTED_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedCommand,
                                     f"Expected UNSUPPORTED_COMMAND, got {e.status}")
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
