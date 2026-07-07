#
#    Copyright (c) 2025 Project CHIP Authors
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
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, AttributeValue, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.tlv import uint

log = logging.getLogger(__name__)


def current_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False

        return report.value.latch == latch

    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {latch}", matcher=predicate)


def current_position_matcher(position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False

        return report.value.position == position

    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {position}", matcher=predicate)


def main_state_matcher(main_state: Clusters.ClosureControl.Attributes.MainState) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.MainState:
            return False

        return report.value == main_state

    return AttributeMatcher.from_callable(description=f"MainState is {main_state}", matcher=predicate)


class TC_CLCTRL_7_4(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_7_4(self) -> str:
        return "[TC-CLCTRL-7.4] GroupedMoveTo Command CountdownTime with Server as DUT"

    def steps_TC_CLCTRL_7_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read the FeatureMap attribute to determine supported features",
                     "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep("2b", "Read the AttributeList attribute to determine supported attributes",
                     "AttributeList of the ClosureControl cluster is returned by the DUT"),
            TestStep("2c", "If CountdownTime attribute is not supported OR Access feature is supported, skip remaining steps and end test case"),
            TestStep("2d", "Establish a wildcard subscription to all attributes on the ClosureControl cluster",
                     "Subscription successfully established"),
            TestStep("2e", "Check if LT feature is supported", "Skip steps 2f to 2m if LT feature is not supported"),
            TestStep("2f", "Read the LatchControlModes attribute",
                     "LatchControlModes of the ClosureControl cluster is returned by the DUT; Value saved as LatchControlModes"),
            TestStep("2g", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Latching field is saved as CurrentLatch"),
            TestStep("2h", "Preparing Latch-State: If CurrentLatch is False, skip steps 2i to 2m"),
            TestStep("2i", "If LatchControlModes Bit 1 = 0, skip step 2j"),
            TestStep("2j", "Send GroupedMoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("2k", "If LatchControlModes Bit 1 = 1, skip step 2l"),
            TestStep("2l", "Unlatch the device manually"),
            TestStep("2m", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("2n", "If the Groupcast cluster is enabled on EP0, the TH reads the Groupcast membership attribute on the DUT.",
                     "Returns list (possibly empty)"),
            TestStep("2o", "If the Groupcast cluster is enabled on EP0 and membership is not empty, the TH sends the Groupcast LeaveGroup command with GroupID field = 0 to the DUT.",
                     "Receive SUCCESS response from the DUT"),
            TestStep("2p", "If the Groupcast cluster is enabled on EP0, the TH sends Groupcast JoinGroup command with GroupID = 1, Endpoints = endpoint under test, KeySetID = 0x01a1 and Key = a0a1a2a3a4a5a6a7a8a9aaabacadaeaf to the DUT.",
                     "Receive SUCCESS response from the DUT"),
            TestStep(3, "Read the CountdownTime attribute when no operation is in progress", "CountdownTime should be 0 or null"),
            TestStep("4a", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Position field is saved as CurrentPosition"),
            TestStep("4b", "Preparing Position-State: If CurrentPosition is FullyClosed, skip steps 4c and 4d"),
            TestStep("4c", "Send GroupedMoveTo command with Position = MoveToFullyClosed", "Receive SUCCESS response from the DUT"),
            TestStep("4d", "Wait until a subscription report with OverallCurrentState.Position is received",
                     "OverallCurrentState.Position should be FullyClosed"),
            TestStep("4e", "Send GroupedMoveTo command with Position = MoveToFullyOpen", "Receive SUCCESS response from the DUT"),
            TestStep("4f", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("4g", "Read the CountdownTime attribute",
                     "CountdownTime should be between 1 and countdown_time_max, or null; Value saved as CurrentCountdownTime"),
            TestStep("4h", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("4i", "If CurrentCountdownTime is null skip step 4j"),
            TestStep("4j", "Read the CountdownTime attribute", "CountdownTime should be 0"),
            TestStep("5a", "If CurrentCountdownTime is null, skip steps 5b to 5g"),
            TestStep("5b", "Send GroupedMoveTo command with Position = MoveToFullyClosed", "Receive SUCCESS response from the DUT"),
            TestStep("5c", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("5d", "Read the CountdownTime attribute", "CountdownTime should be between 1 and countdown_time_max"),
            TestStep("5e", "Send Stop command", "Receive SUCCESS response from the DUT"),
            TestStep("5f", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("5g", "Read the CountdownTime attribute", "CountdownTime should be 0"),
        ]

    def pics_TC_CLCTRL_7_4(self) -> list[str]:
        return [
            "CLCTRL.S", "CLCTRL.S.A0000", "CLCTRL.S.C03.Rsp"
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def teardown_test(self):
        if getattr(self, 'groupcast_enabled', False):
            membership = await self.read_single_attribute_check_success(
                endpoint=0,
                cluster=Clusters.Groupcast,
                attribute=Clusters.Groupcast.Attributes.Membership,
            )
            if membership:
                try:
                    await self.send_single_cmd(cmd=Clusters.Groupcast.Commands.LeaveGroup(groupID=0), endpoint=0)
                except InteractionModelError as e:
                    if e.status != Status.NotFound:
                        raise
                    log.info("LeaveGroup(groupID=0) returned NotFound during teardown; no groups to clean up")
        super().teardown_test()

    @async_test_body
    async def test_TC_CLCTRL_7_4(self):
        countdown_time_max: uint = 259200

        endpoint: int = self.get_endpoint()
        timeout: uint = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else countdown_time_max
        IANA_ADDR_POLICY = 0
        self.kGroupKeysetId = 0x01a1
        self.kGroupId = 0x0001
        self.kGroupKey = bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")
        self.groupcast_enabled = await is_groupcast_on_root_node(self)
        if self.groupcast_enabled:
            dev_controller = self.default_controller
            dev_controller.SetGroupKeySet(
                keyset_id=self.kGroupKeysetId,
                policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                num_keys=1,
                epoch_key0=self.kGroupKey,
                epoch_start_time0=2220000,
            )
            dev_controller.SetGroupKey(self.kGroupId, self.kGroupKeysetId)
            dev_controller.SetGroupInfo(self.kGroupId, "Closure Control Group", IANA_ADDR_POLICY)
        log.info("Groupcast on root node enabled: %s", self.groupcast_enabled)

        self.step(1)
        attributes: list[uint] = Clusters.ClosureControl.Attributes

        self.step("2a")
        feature_map: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_latching_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_access_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kAccess
        log.info("FeatureMap: %s", feature_map)

        self.step("2b")
        attribute_list: list[uint] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        is_countdown_time_supported: bool = attributes.CountdownTime.attribute_id in attribute_list
        log.info("AttributeList: %s", attribute_list)

        self.step("2c")
        if (not is_countdown_time_supported) or is_access_supported:
            log.info("CountdownTime attribute not supported or Access feature supported, skipping remaining steps")
            self.mark_all_remaining_steps_skipped("2d")
            return

        self.step("2d")
        sub_handler = AttributeSubscriptionHandler(expected_cluster=Clusters.ClosureControl)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        self.step("2e")
        if not is_latching_supported:
            log.info("Latching feature not supported, skipping steps related to latching")
            self.mark_step_range_skipped("2f", "2m")
        else:
            log.info("Latching feature supported, proceeding with latch preparation steps")

            self.step("2f")
            latch_control_modes: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
            log.info("LatchControlModes: %s", latch_control_modes)

            self.step("2g")
            overall_current_state: Nullable | Clusters.ClosureControl.Structs.OverallCurrentStateStruct = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
            current_latch: bool = None

            if overall_current_state is NullValue:
                current_latch: bool = NullValue
            else:
                current_latch: bool = overall_current_state.latch
            log.info("CurrentLatch: %s", current_latch)

            self.step("2h")
            if current_latch is False:
                log.info("CurrentLatch is False, skipping Latch = False preparation steps")
                self.mark_step_range_skipped("2i", "2m")
            else:
                log.info("CurrentLatch is True, proceeding with Latch = False preparation steps")

                self.step("2i")
                # Check if LatchControlModes Bit 1 is 0
                if not latch_control_modes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                    self.skip_step("2j")
                    self.step("2k")
                    log.info("LatchControlModes Bit 1 is 0, unlatch device manually")
                    self.step("2l")
                    self.wait_for_user_input(prompt_msg="Press enter when the device is unlatched")

                else:
                    self.step("2j")
                    log.info("LatchControlModes Bit 1 is 1, sending GroupedMoveTo command with Latch = False")
                    await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.GroupedMoveTo(latch=False))

                    self.step("2k")
                    self.skip_step("2l")

                self.step("2m")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
                log.info("Latch is now False, proceeding with CountdownTime checks")
                sub_handler.reset()

        # STEP 2n: If the Groupcast cluster is enabled on EP0, the TH reads the Groupcast membership attribute on the DUT
        self.step("2n")
        membership = None
        if self.groupcast_enabled:
            log.info("Groupcast cluster is enabled on EP0, reading the Groupcast membership attribute on the DUT")
            membership = await self.read_single_attribute_check_success(
                endpoint=0,
                cluster=Clusters.Groupcast,
                attribute=Clusters.Groupcast.Attributes.Membership
            )
        else:
            log.info("Groupcast cluster is not enabled on EP0, skipping step")
            self.mark_current_step_skipped()

        # STEP 2o: If the Groupcast cluster is enabled on EP0 and membership is not empty, the TH sends the Groupcast LeaveGroup command with GroupID field = 0 to the DUT
        self.step("2o")
        if self.groupcast_enabled:
            log.info("Groupcast cluster is enabled on EP0")
            if membership:
                log.info("Groupcast membership is not empty, sending the Groupcast LeaveGroup command with GroupID field = 0 to the DUT")
                await self.send_single_cmd(cmd=Clusters.Groupcast.Commands.LeaveGroup(groupID=0), endpoint=0)
            else:
                log.info("Groupcast membership is empty, skipping step")
                self.mark_current_step_skipped()
        else:
            log.info("Groupcast cluster is not enabled on EP0, skipping step")
            self.mark_current_step_skipped()

        # STEP 2p: If the Groupcast cluster is enabled on EP0, the TH sends Groupcast JoinGroup command with GroupID = 1, Endpoints = endpoint under test, KeySetID = 0x01a1 and Key = a0a1a2a3a4a5a6a7a8a9aaabacadaeaf to the DUT
        self.step("2p")
        if self.groupcast_enabled:
            log.info("Groupcast cluster is enabled on EP0, sending Groupcast JoinGroup command")
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=self.kGroupId,
                endpoints=[endpoint],
                keySetID=self.kGroupKeysetId,
                key=self.kGroupKey), endpoint=0)

            acl = [
                # ACL entry granting CASE admin access to the controller.
                Clusters.AccessControl.Structs.AccessControlEntryStruct(
                    privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                    authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                    subjects=[self.default_controller.nodeId],
                    targets=NullValue),
                # Grant a additional Group Operate access to the closure cluster.
                Clusters.AccessControl.Structs.AccessControlEntryStruct(
                    privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                    authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                    subjects=[self.kGroupId],
                    targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                        endpoint=endpoint, cluster=Clusters.ClosureControl.id)]),
            ]
            await dev_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        else:
            log.info("Groupcast cluster is not enabled on EP0, skipping step")
            self.mark_current_step_skipped()

        # STEP 3: Verify the CountdownTime when no operation is in progress
        self.step(3)
        countdown_time: NullValue | uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        asserts.assert_true(countdown_time == 0 or countdown_time == NullValue,
                            f"CountdownTime should be 0 or null when no operation is in progress, got: {countdown_time}.")

        # STEP 4: Verify the CountdownTime when an operation is triggered
        self.step("4a")
        overall_current_state: Nullable | Clusters.ClosureControl.Structs.OverallCurrentStateStruct = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
        current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum = None

        if overall_current_state is NullValue:
            current_position = NullValue
        else:
            current_position = overall_current_state.position
        log.info("CurrentPosition: %s", current_position)

        self.step("4b")
        if current_position == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
            log.info("CurrentPosition is FullyClosed, skipping steps 4c and 4d.")
            self.skip_step("4c")
            self.skip_step("4d")
        else:
            log.info("CurrentPosition is not FullyClosed, proceeding with Position = FullyClosed preparation steps")

            self.step("4c")
            if self.groupcast_enabled:
                log.info("Sending GroupedMoveTo command with Position = FullyClosed by groupcast")
                self.default_controller.SendGroupCommand(self.kGroupId, Clusters.ClosureControl.Commands.GroupedMoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed))
            else:
                log.info("Sending GroupedMoveTo command with Position = FullyClosed by unicast")
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.GroupedMoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed))

            self.step("4d")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(
                Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)], timeout_sec=timeout)
            sub_handler.reset()

        self.step("4e")
        if self.groupcast_enabled:
            log.info("Sending GroupedMoveTo command with Position = FullyOpen by groupcast")
            self.default_controller.SendGroupCommand(self.kGroupId, Clusters.ClosureControl.Commands.GroupedMoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen))
        else:
            log.info("Sending GroupedMoveTo command with Position = FullyOpen by unicast")
            await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.GroupedMoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen))

        self.step("4f")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
            Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)

        self.step("4g")
        current_countdown_time: NullValue | uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        asserts.assert_true(current_countdown_time == NullValue or (1 <= current_countdown_time <= countdown_time_max),
                            f"CountdownTime should be between 1 and {countdown_time_max}, or null, got: {current_countdown_time}.")
        log.info("CurrentCountdownTime: %s", current_countdown_time)

        self.step("4h")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
            Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)

        self.step("4i")
        if current_countdown_time is NullValue:
            log.info("CurrentCountdownTime is Null, skipping step 4j.")
            self.skip_step("4j")
        else:
            self.step("4j")
            countdown_time_after_operation: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_equal(countdown_time_after_operation, 0,
                                 f"CountdownTime should be 0 after operation completes, got: {countdown_time_after_operation}.")
            log.info("CountdownTime after operation: %s", countdown_time_after_operation)
        sub_handler.reset()

        # STEP 5: Verify the CountdownTime behavior when an operation is interrupted
        self.step("5a")
        if current_countdown_time is NullValue:
            log.info("CurrentCountdownTime is Null, skipping steps 5b to 5g.")
            self.mark_step_range_skipped("5b", "5f")
        else:
            self.step("5b")
            if self.groupcast_enabled:
                log.info("Sending GroupedMoveTo command with Position = FullyClosed by groupcast")
                self.default_controller.SendGroupCommand(self.kGroupId, Clusters.ClosureControl.Commands.GroupedMoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed))
            else:
                log.info("Sending GroupedMoveTo command with Position = FullyClosed by unicast")
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.GroupedMoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed))

            self.step("5c")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)

            self.step("5d")
            countdown_time_before_interruption: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_true(countdown_time_before_interruption > 0 and countdown_time_before_interruption <=
                                countdown_time_max, f"CountdownTime before interruption: {countdown_time_before_interruption}.")
            log.info("CountdownTime before interruption: %s", countdown_time_before_interruption)

            self.step("5e")
            await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.Stop())
            log.info("Stop command sent, waiting for MainState to become Stopped")

            self.step("5f")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)

            self.step("5g")
            countdown_time_after_interruption: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_true(countdown_time_after_interruption == 0,
                                f"CountdownTime after interruption not 0, but: {countdown_time_after_interruption}.")
            log.info("CountdownTime after interruption not 0, but: %s", countdown_time_after_interruption)
            sub_handler.reset()


if __name__ == "__main__":
    default_matter_test_main()
