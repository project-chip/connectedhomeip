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
from matter.testing import matter_asserts
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

        self.step(2, "TH reads from the DUT the AcceptedCommandList.",
                  expectation="Verify that the DUT response contains a list of accepted command IDs. Store the "
                              "value as AcceptedCmds.")
        accepted_cmds = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
        has_modify = cmds.ModifyEnabledAlarms.command_id in accepted_cmds
        has_reset = cmds.Reset.command_id in accepted_cmds

        self.step("2a", "TH reads from the DUT the Supported.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value. Store the "
                              "value as Supported.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        matter_asserts.assert_valid_uint32(supported, "Supported attribute (map32 AlarmBitmap)")

        self.step("2b", "TH reads from the DUT the Mask.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value. Store the "
                              "value as InitialMask.")
        initial_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        matter_asserts.assert_valid_uint32(initial_mask, "Mask attribute (map32 AlarmBitmap)")

        self.step("2c", "TH reads from the DUT the State.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value. Store the "
                              "value as InitialState.")
        initial_state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        matter_asserts.assert_valid_uint32(initial_state, "State attribute (map32 AlarmBitmap)")

        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        has_latch = attrs.Latch.attribute_id in attribute_list

        # Steps 3-8a exercise ModifyEnabledAlarms; if the command is not supported the whole
        # block is skipped in one range and step 9 checks the UNSUPPORTED_COMMAND response.
        if has_modify:
            self.step(3, "TH sends command ModifyEnabledAlarms with Mask set to Supported (enable all "
                      "supported alarms).", expectation="Verify DUT responds w/ status SUCCESS(0x00).")
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=supported), endpoint=endpoint)

            self.step(4, "TH reads from the DUT the Mask.",
                      expectation="Verify that the DUT response contains a value equal to Supported.")
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, supported, "Mask should equal Supported after ModifyEnabledAlarms(Supported)")

            # Alarm Base: "A server that is unable to enable a currently suppressed alarm, or is unable
            # to suppress a currently enabled alarm SHALL respond with a status code of FAILURE". A
            # server with a non-suppressible alarm is therefore conformant when it refuses this, and
            # leaves Mask alone.
            self.step(5, "TH sends command ModifyEnabledAlarms with Mask set to 0 (disable all alarms).",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00), or FAILURE if the server cannot "
                                  "suppress a currently enabled alarm.")
            suppressed = False
            try:
                await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=0), endpoint=endpoint)
                suppressed = True
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Failure,
                                     f"Expected SUCCESS or FAILURE for suppress-all, got {e.status}")

            self.step(6, "TH reads from the DUT the Mask.",
                      expectation="Verify that the DUT response contains a value equal to 0 if step 5 succeeded, "
                                  "otherwise the value is unchanged. Store the value as MaskAfterDisable.")
            mask_after_disable = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            if suppressed:
                asserts.assert_equal(mask_after_disable, 0, "Mask should be 0 after ModifyEnabledAlarms(0)")
            else:
                asserts.assert_equal(mask_after_disable, supported,
                                     "A refused ModifyEnabledAlarms must not change Mask")

            # Choose a mask bit not set in Supported to exercise the invalid-mask rejection. Prefer a
            # defined AlarmBitmap bit (0-14) that the DUT does not support; if the DUT supports every
            # defined bit, fall back to the lowest reserved bit so the negative path still runs.
            unsupported_bit = None
            for _bit in range(32):
                _candidate = 1 << _bit
                if (_candidate & _ALL_DEFINED_ALARM_BITS) and not (int(supported) & _candidate):
                    unsupported_bit = _candidate
                    break
            if unsupported_bit is None:
                for _bit in range(32):
                    _candidate = 1 << _bit
                    if not (int(supported) & _candidate):
                        unsupported_bit = _candidate
                        break

            self.step(7, "TH sends command ModifyEnabledAlarms with Mask containing a bit not set in Supported.",
                      expectation="Verify that the DUT response contains INVALID_COMMAND.")
            try:
                await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=unsupported_bit), endpoint=endpoint)
                asserts.fail("Expected INVALID_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand, f"Expected INVALID_COMMAND, got {e.status}")

            self.step("7a", "TH reads from the DUT the Mask.",
                      expectation="Verify that the DUT response contains a value equal to MaskAfterDisable.")
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, mask_after_disable, "Mask changed after a rejected ModifyEnabledAlarms")

            self.step(8, "TH sends command ModifyEnabledAlarms with Mask set to InitialMask (restore).",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00).")
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=initial_mask), endpoint=endpoint)

            self.step("8a", "TH reads from the DUT the Mask.",
                      expectation="Verify that the DUT response contains a value equal to InitialMask.")
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, initial_mask, "Mask should equal InitialMask after restore")
        else:
            self.mark_step_range_skipped(3, "8a")

        self.step(9, "IF ModifyEnabledAlarms is not present in AcceptedCmds: TH sends command "
                  "ModifyEnabledAlarms (0x01).",
                  expectation="Verify that the DUT response contains UNSUPPORTED_COMMAND.")
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
        self.step(10, "TH reads from the DUT the Latch. Store the value as LatchBits. Identify the "
                  "lowest-numbered bit that is set in LatchBits. Store the value as TargetBit.",
                  expectation="Verify that the DUT response contains a map32 AlarmBitmap. Store the value as "
                              "LatchBits. If LatchBits equals 0, steps 10a–10d are not applicable; skip to "
                              "step 11.")
        if has_latch:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
            matter_asserts.assert_valid_uint32(latch, "Latch attribute (map32 AlarmBitmap)")
            for _bit in range(32):
                _candidate = 1 << _bit
                if int(latch) & _candidate & int(supported):
                    target_bit = _candidate
                    break
        else:
            self.mark_current_step_skipped()

        can_latch_test = target_bit is not None and target_bit in _SIMULATE_TRIGGER and has_reset

        # Steps 10a-10d are the latched-alarm lifecycle; if there is no latched target to test
        # (LatchBits empty, no trigger, or Reset unsupported) they skip together as a range.
        if can_latch_test:
            self.step("10a", "Select the TestEventTrigger code from the PIXIT Variable Values table "
                      "corresponding to TargetBit. TH sends TestEventTrigger command to General Diagnostics "
                      "Cluster on Endpoint 0 with EnableKey field set to PIXIT.ESALM.TEST_EVENT_TRIGGER_KEY and "
                      "EventTrigger field set to PIXIT.ESALM.TEST_EVENT_TRIGGER with that code.",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00).")
            await self.send_test_event_triggers(eventTrigger=_SIMULATE_TRIGGER[target_bit])

            self.step("10b", "TH reads from the DUT the State.",
                      expectation="Verify that the DUT response contains an AlarmBitmap with TargetBit set.")
            state = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_true(int(state) & target_bit, "TargetBit should be set in State after the trigger")

            self.step("10c", "TH sends command Reset with TargetBit set in the Alarms field.",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00).")
            await self.send_single_cmd(cmd=cmds.Reset(alarms=target_bit), endpoint=endpoint)

            self.step("10d", "TH reads from the DUT the State.",
                      expectation="TargetBit is cleared in State.")
            state_after = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_equal(int(state_after) & target_bit, 0,
                                 "TargetBit should be cleared in State after Reset")
        else:
            self.mark_step_range_skipped("10a", "10d")

        inactive_bit = None
        self.step(11, "TH sends command Reset with a bit that is not active in State.",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00).")
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

        self.step("11a", "TH reads from the DUT the State.",
                  expectation="Verify that the DUT response contains an AlarmBitmap (map32) value equal to "
                              "InitialState (step 2c).")
        if has_reset and inactive_bit is not None:
            state_after = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_equal(state_after, initial_state,
                                 "State must equal InitialState (step 2c) after Reset with an inactive alarm bit")
        else:
            self.mark_current_step_skipped()

        self.step(12, "IF Reset is not present in AcceptedCmds: TH sends command Reset (0x00).",
                  expectation="Verify that the DUT response contains UNSUPPORTED_COMMAND.")
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
