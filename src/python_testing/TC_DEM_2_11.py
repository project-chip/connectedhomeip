#
#    Copyright (c) 2027 Project CHIP Authors
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
# pylint: disable=invalid-name

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${EVSE_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --featureSet 0x01
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_11."""

import asyncio
import datetime
import logging
import sys

from mobly import asserts
from TC_DEMTestBase import DEMTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_DEM_2_11(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_11."""

    def desc_TC_DEM_2_11(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.11] Power Range Adjustment feature functionality with DUT as Server"

    def pics_TC_DEM_2_11(self):
        """Return the PICS definitions associated with this test."""
        return [
            "DEM.S.F07",  # Depends on Feature 07 (PowerRangeAdjustment)
        ]

   def steps_TC_DEM_2_11(self) -> list[TestStep]:
        """Execute the test steps."""
        return [
        TestStep("1", "Commission DUT to TH",
                "TH and DUT connection established"),
        TestStep("2", "TH reads FeatureMap attribute",
                "DUT replies with the FeatureMap attribute. Verify PowerRangeAdjustment is supported and that PowerAdjustment is not supported."),
        TestStep("3", "Set up a subscription to all DeviceEnergyManagement cluster events",
                "Subscription to all events is established"),
        TestStep("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                "value is 1 (True)"),
        TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster for Power Range Adjustment Test Event",
                "DUT responds with status SUCCESS"),
        TestStep("5a", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("5b", "TH reads AbsMinPower and AbsMaxPower",
                "DUT replies with with valid values where AbsMinPower < AbsMaxPower. Note AbsMinPower and AbsMaxPower for use in constraints validation."),
        TestStep("5c", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("5d", "TH reads OptOutState",
                "value is 0x00 (NoOptOut)"),
        TestStep("6", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("6a", "TH reads PowerRangeAdjustment",
                "value is MinPower=AbsMinPower, MaxPower=AbsMaxPower, Cause=LocalOptimization, and EndTime in future (approximately 5 seconds from now)"),
        TestStep("6b", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("7", "Wait 5 seconds",
                "Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion"),
        TestStep("7a", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("7b", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("8", "TH sends PowerRangeAdjustRequest with MinPower=AbsMaxPower+1, MaxPower=AbsMaxPower+100, Duration=5, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("9", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower-100, MaxPower=AbsMinPower-1, Duration=5, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("10", "TH sends PowerRangeAdjustRequest with MinPower=AbsMaxPower, MaxPower=AbsMinPower, Duration=5, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("11", "TH sends PowerRangeAdjustRequest with MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization (no MinPower)",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("11a", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("11b", "Wait 5 seconds",
                "Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion"),
        TestStep("11c", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("11d", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("12", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, Duration=5, Cause=LocalOptimization (no MaxPower)",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("12a", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("12b", "Wait 5 seconds",
                "Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion"),
        TestStep("12c", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("12d", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("13", "TH sends PowerRangeAdjustRequest with no MinPower and no MaxPower, Duration=5, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("14", "TH sends PowerRangeAdjustRequest with MaxPower=AbsMaxPower+1, Duration=5, Cause=LocalOptimization (no MinPower)",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("15", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower-1, Duration=5, Cause=LocalOptimization (no MaxPower)",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("16", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Cause=LocalOptimization (no Duration)",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("17", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=86401, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("18", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=0, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("19", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=InvalidValue (out of range)",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("20", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("20a", "TH reads PowerRangeAdjustment",
                "value is MinPower=AbsMinPower, MaxPower=AbsMaxPower, and Cause=GridOptimization"),
        TestStep("20b", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("21", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower+100, MaxPower=AbsMaxPower-100, Duration=7200, Cause=LocalOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("21a", "TH reads PowerRangeAdjustment",
                "value is MinPower=AbsMinPower+100, MaxPower=AbsMaxPower-100, and Cause=LocalOptimization (previous GridOptimization adjustment has been replaced)"),
        TestStep("21b", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("22", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower+200, MaxPower=AbsMaxPower-200, Duration=2000, Cause=GridOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("22a", "TH reads PowerRangeAdjustment",
                "value is MinPower=AbsMinPower+200, MaxPower=AbsMaxPower-200, and Cause=GridOptimization (previous LocalOptimization adjustment has been replaced)"),
        TestStep("22b", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("23", "TH sends CancelPowerRangeAdjustRequest",
                "DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled"),
        TestStep("23a", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("23b", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("24", "TH sends CancelPowerRangeAdjustRequest",
                "DUT responds with status INVALIDINSTATE"),
        TestStep("25", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=2000, Cause=LocalOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("25a", "TH reads ESAState",
                "value is 0x04 (PowerAdjustActive)"),
        TestStep("25b", "Wait 3 seconds",
                "Time elapses (TH waits 3 seconds while LocalOptimization is still active)"),
        TestStep("26", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Local Optimization Test Event",
                "DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=UserOptOut"),
        TestStep("26a", "TH reads ESAState",
                "value is 0x01 (Online)"),
        TestStep("26b", "TH reads PowerRangeAdjustment",
                "value is null"),
        TestStep("26c", "TH reads OptOutState",
                "value is 0x01 (LocalOptOut)"),
        TestStep("27", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Test Event Clear",
                "DUT responds with status SUCCESS"),
        TestStep("27a", "TH reads OptOutState",
                "value is 0x00 (NoOptOut)"),
        TestStep("28", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Local Optimization Test Event",
                "DUT responds with status SUCCESS"),
        TestStep("28a", "TH reads OptOutState",
                "value is 0x01 (LocalOptOut)"),
        TestStep("29", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("30", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("30a", "TH sends CancelPowerRangeAdjustRequest",
                "DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled"),
        TestStep("31", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Grid Optimization Test Event",
                "DUT responds with status SUCCESS"),
        TestStep("31a", "TH reads OptOutState",
                "value is 0x02 (GridOptOut)"),
        TestStep("32", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                "DUT responds with status CONSTRAINT_ERROR"),
        TestStep("33", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                "DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent"),
        TestStep("33a", "TH sends CancelPowerRangeAdjustRequest",
                "DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled"),
        TestStep("34", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Test Event Clear",
                "DUT responds with status SUCCESS"),
        TestStep("34a", "TH reads OptOutState",
                "value is 0x00 (NoOptOut)"),
        TestStep("35", "TH sends TestEventTrigger command to General Diagnostics Cluster for Power Range Adjustment Test Event Clear",
                "DUT responds with status SUCCESS"),
        TestStep("36", "Cancel the subscription to the Device Energy Management cluster",
                "The subscription is cancelled successfully"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_DEM_2_11(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerRangeAdjustment], \
                                        [Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment])

        self.step("3")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.DeviceEnergyManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.get_endpoint())

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        await self.send_test_event_trigger_power_range_adjustment()

        self.step("5a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5b")
        absMinPower = await self.read_dem_attribute_expect_success(attribute="AbsMinPower")
        absMaxPower = await self.read_dem_attribute_expect_success(attribute="AbsMaxPower")

        log.info("absMinPower: %s, absMaxPower: %s", absMinPower, absMaxPower)
        asserts.assert_less_than(absMinPower, absMaxPower)

        self.step("5c")
        await self.check_dem_attribute("PowerRangeAdjustment", None)

        self.step("5d")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("6")
        timeNowEpoch = self.get_current_utc_time_in_seconds()

        await self.send_power_range_adjustment_command(minPower=absMinPower,
                                                 maxPower=absMaxPower,
                                                 duration=5,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("6a")
        powerRangeAdjustment = await self.read_dem_attribute_expect_success(attribute="PowerRangeAdjustment")
        # check MinPower=AbsMinPower, MaxPower=AbsMaxPower, Cause=LocalOptimization and EndTime in future (approximately 5 seconds from now)
        asserts.assert_equal(powerRangeAdjustment.minPower, absMinPower)
        asserts.assert_equal(powerRangeAdjustment.maxPower, absMaxPower)
        asserts.assert_equal(powerRangeAdjustment.cause, Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)
        asserts.assert_greater(powerRangeAdjustment.endTime, timeNowEpoch + 4)  # Allow 1 second margin for test execution time

        self.step("6b")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)


        ########################### GOT HERE
        self.step("7")
        await self.send_cancel_power_adjustment_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("7a")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("7b")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8")
        await self.send_cancel_power_adjustment_command(expected_status=Status.InvalidInState)

        self.step("9")
        await self.send_power_adjustment_command(power=max_power + 1,
                                                 duration=min_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("10")
        await self.send_power_adjustment_command(power=min_power,
                                                 duration=max_duration + 1,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("11")
        await self.send_power_adjustment_command(power=min_power - 1,
                                                 duration=max_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("12")
        await self.send_power_adjustment_command(power=max_power,
                                                 duration=min_duration - 1,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("13")
        start = datetime.datetime.now()
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].minPower,
                                                 duration=min_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustStart)

        self.step("13a")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("14")
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].minDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization)

        # Wait 5 seconds for an event not to be reported
        events_callback.wait_for_event_expect_no_report(5)

        self.step("14a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("14b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kGridOptimizationAdjustment)

        self.step("15")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("15a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("15b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("16")
        await self.send_power_adjustment_command(power=max_power,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("17")
        await self.send_test_event_trigger_user_opt_out_grid()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kUserOptOut)

        # Allow 3s error margin as the CI build system can run out of CPU time
        elapsed = datetime.datetime.now() - start
        asserts.assert_less_equal(abs(elapsed.seconds - event_data.duration), 3)
        asserts.assert_greater_equal(event_data.energyUse, 0)

        self.step("17a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("17b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("17c")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("18")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("18a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("18b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("19")
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].minDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.Success)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustStart)

        self.step("19a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("19b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("20")
        await asyncio.sleep(10)

        # Allow a little tolerance checking the duration returned in the event as CI tests can run "slower"
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_greater_equal(event_data.duration, 10)
        asserts.assert_less_equal(event_data.duration, 12)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)
        asserts.assert_greater(event_data.energyUse, 0)

        self.step("20a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("20b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("21")
        await self.send_test_event_trigger_power_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
