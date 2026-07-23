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
#     app: ${ALL_CLUSTERS_APP}
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
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
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

# AlarmBitmap mask for all defined alarm bits (from electrical-alarm-cluster.xml)
_ALL_DEFINED_ALARM_BITS = 0x7FFF


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

        self.step(2, "TH reads AcceptedCommandList, Supported, and Mask", "SUCCESS for each read.")
        accepted_cmds = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        initial_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)

        CMD_RESET = 0x00
        CMD_MODIFY = 0x01

        has_modify = CMD_MODIFY in accepted_cmds
        has_reset = CMD_RESET in accepted_cmds

        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        has_latch = attrs.Latch.attribute_id in attribute_list

        self.step(3, "TH sends ModifyEnabledAlarms with Mask = Supported (all supported alarms enabled)",
                  "SUCCESS.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=supported), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step(4, "TH reads Mask", "DUT returns value equal to Supported.")
        if has_modify:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, supported, "Mask should equal Supported after ModifyEnabledAlarms(Supported)")
        else:
            self.mark_current_step_skipped()

        self.step(5, "TH sends ModifyEnabledAlarms with Mask = 0 (all alarms disabled)", "SUCCESS.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=0), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH reads Mask", "DUT returns 0.")
        if has_modify:
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, 0, "Mask should be 0 after ModifyEnabledAlarms(0)")
        else:
            self.mark_current_step_skipped()

        self.step(7, "TH sends ModifyEnabledAlarms with Mask containing a bit not set in Supported",
                  "DUT returns INVALID_COMMAND. Mask read-back is unchanged.")
        if has_modify:
            invalid_bit = None
            for bit in range(64):
                candidate = 1 << bit
                if candidate & _ALL_DEFINED_ALARM_BITS and not (supported & candidate):
                    invalid_bit = candidate
                    break
            if invalid_bit is None:
                invalid_bit = 1 << 15  # bit 15 is not defined in AlarmBitmap
            try:
                await self.send_single_cmd(
                    cmd=cmds.ModifyEnabledAlarms(mask=invalid_bit), endpoint=endpoint)
                asserts.fail("Expected INVALID_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand,
                                     "Expected INVALID_COMMAND, got %s" % e.status)
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, 0, "Mask changed after INVALID_COMMAND rejection")
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH sends ModifyEnabledAlarms with Mask = InitialMask (restore)",
                  "SUCCESS. Mask read-back equals InitialMask.")
        if has_modify:
            await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=initial_mask), endpoint=endpoint)
            mask_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
            asserts.assert_equal(mask_val, initial_mask, "Mask should equal InitialMask after restore")
        else:
            self.mark_current_step_skipped()

        self.step(9, "If ModifyEnabledAlarms not in AcceptedCommandList: TH sends ModifyEnabledAlarms (0x01)",
                  "DUT returns UNSUPPORTED_COMMAND.")
        if not has_modify:
            try:
                await self.send_single_cmd(cmd=cmds.ModifyEnabledAlarms(mask=0), endpoint=endpoint)
                asserts.fail("Expected UNSUPPORTED_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedCommand,
                                     "Expected UNSUPPORTED_COMMAND, got %s" % e.status)
        else:
            self.mark_current_step_skipped()

        self.step(10, "If Reset supported and Latch supported: trigger latched alarm via TestEventTrigger; send Reset with that alarm bit",
                  "SUCCESS. State read-back has alarm bit cleared.")
        if has_reset and has_latch:
            await self.check_test_event_triggers_enabled()
            trigger_code = self.user_params.get("PIXIT.ESALM.TEST_EVENT_TRIGGER", None)
            if trigger_code is not None:
                await self.send_test_event_triggers(eventTrigger=int(trigger_code, 0))
                state = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attrs.State)
                latch = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
                triggered_bit = None
                for bit in range(64):
                    candidate = 1 << bit
                    if (state & candidate) and (latch & candidate):
                        triggered_bit = candidate
                        break
                if triggered_bit is not None:
                    await self.send_single_cmd(cmd=cmds.Reset(alarms=triggered_bit), endpoint=endpoint)
                    state_after = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=attrs.State)
                    asserts.assert_equal(state_after & triggered_bit, 0,
                                         "Alarm bit still set in State after Reset")
                else:
                    self.mark_current_step_skipped()
            else:
                self.mark_current_step_skipped()
        else:
            self.mark_current_step_skipped()

        self.step(11, "If Reset supported: send Reset with a bit not active in State",
                  "SUCCESS. State read-back unchanged.")
        if has_reset:
            state = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            inactive_bit = None
            supported_now = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
            for bit in range(64):
                candidate = 1 << bit
                if (supported_now & candidate) and not (state & candidate):
                    inactive_bit = candidate
                    break
            if inactive_bit is not None:
                await self.send_single_cmd(cmd=cmds.Reset(alarms=inactive_bit), endpoint=endpoint)
                state_after = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attrs.State)
                asserts.assert_equal(state_after & inactive_bit, 0,
                                     "Inactive alarm bit unexpectedly set in State after no-op Reset")
            else:
                self.mark_current_step_skipped()
        else:
            self.mark_current_step_skipped()

        self.step(12, "If Reset not in AcceptedCommandList: TH sends Reset (0x00)",
                  "DUT returns UNSUPPORTED_COMMAND.")
        if not has_reset:
            try:
                await self.send_single_cmd(cmd=cmds.Reset(alarms=0), endpoint=endpoint)
                asserts.fail("Expected UNSUPPORTED_COMMAND but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedCommand,
                                     "Expected UNSUPPORTED_COMMAND, got %s" % e.status)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
