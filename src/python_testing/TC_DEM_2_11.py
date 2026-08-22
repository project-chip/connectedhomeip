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
#       --featureSet 0x80
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
import logging

from mobly import asserts
from TC_DEMTestBase import DEMTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DEM_2_11(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_11."""

    def pics_TC_DEM_2_11(self):
        """Return the PICS definitions associated with this test."""
        return [
            "DEM.S.F07",  # Depends on Feature 07 (PowerRangeAdjustment)
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_DEM_2_11(self):
        # pylint: disable=too-many-locals, too-many-statements
        """4.1.3. [TC-DEM-2.11] Power Range Adjustment feature functionality with DUT as Server"""

        self.step("1", "Commission DUT to TH", is_commissioning=True,
                  expectation="TH and DUT connection established")
        # Commission DUT - already done

        self.step("2", "TH reads FeatureMap attribute",
                  expectation="DUT replies with the FeatureMap attribute. Verify PowerRangeAdjustment is supported and that PowerAdjustment is not supported.")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerRangeAdjustment],
                                        [Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment])

        self.step("3", "Set up a subscription to all DeviceEnergyManagement cluster events",
                  expectation="Subscription to all events is established")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.DeviceEnergyManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.get_endpoint())

        self.step("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                  expectation="value is 1 (True)")
        await self.check_test_event_triggers_enabled()

        self.step("5", "TH sends TestEventTrigger command to General Diagnostics Cluster for Power Range Adjustment Test Event",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_power_range_adjustment()

        self.step("5a", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5b", "TH reads AbsMinPower and AbsMaxPower",
                  expectation="DUT replies with valid values where AbsMinPower < AbsMaxPower. Note AbsMinPower and AbsMaxPower for use in constraints validation.")
        absMinPower = await self.read_dem_attribute_expect_success(attribute="AbsMinPower")
        absMaxPower = await self.read_dem_attribute_expect_success(attribute="AbsMaxPower")

        log.info("absMinPower: %s, absMaxPower: %s", absMinPower, absMaxPower)
        asserts.assert_less(absMinPower, absMaxPower)

        self.step("5c", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("5d", "TH reads OptOutState", expectation="value is 0x00 (NoOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("6", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        timeNowEpoch = self.get_current_utc_time_in_seconds()

        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("6a", "TH reads PowerRangeAdjustment",
                  expectation="value is MinPower=AbsMinPower, MaxPower=AbsMaxPower, Cause=LocalOptimization, and EndTime in future (approximately 5 seconds from now)")
        powerRangeAdjustment = await self.read_dem_attribute_expect_success(attribute="PowerRangeAdjustment")
        # check MinPower=AbsMinPower, MaxPower=AbsMaxPower, Cause=LocalOptimization and EndTime in future (approximately 5 seconds from now)
        asserts.assert_equal(powerRangeAdjustment.minPower, absMinPower)
        asserts.assert_equal(powerRangeAdjustment.maxPower, absMaxPower)
        asserts.assert_equal(powerRangeAdjustment.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)
        asserts.assert_greater(powerRangeAdjustment.endTime, timeNowEpoch + 4)  # Allow 1 second margin for test execution time

        self.step("6b", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("7", "Wait up to 10 seconds", expectation="Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion")
        event_data = events_callback.wait_for_event_report(
            Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd, timeout_sec=10)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)

        self.step("7a", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("7b", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8", "TH sends PowerRangeAdjustRequest with MinPower=AbsMaxPower+1, MaxPower=AbsMaxPower+100, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMaxPower + 1,
                                                       maxPower=absMaxPower + 100,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("9", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower-100, MaxPower=AbsMinPower-1, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower - 100,
                                                       maxPower=absMinPower - 1,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("10", "TH sends PowerRangeAdjustRequest with MinPower=AbsMaxPower, MaxPower=AbsMinPower, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMaxPower,
                                                       maxPower=absMinPower,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("11", "TH sends PowerRangeAdjustRequest with MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization (no MinPower)",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       maxPower=absMaxPower,
                                                       duration=5)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("11a", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("11b", "Wait up to 10 seconds", expectation="Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion")
        event_data = events_callback.wait_for_event_report(
            Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd, timeout_sec=10)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)

        self.step("11c", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("11d", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("12", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, Duration=5, Cause=LocalOptimization (no MaxPower)",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       duration=5)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("12a", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("12b", "Wait up to 10 seconds", expectation="Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=NormalCompletion")
        event_data = events_callback.wait_for_event_report(
            Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd, timeout_sec=10)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)

        self.step("12c", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("12d", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("13", "TH sends PowerRangeAdjustRequest with no MinPower and no MaxPower, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("14", "TH sends PowerRangeAdjustRequest with MaxPower=AbsMaxPower+1, Duration=5, Cause=LocalOptimization (no MinPower)",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       maxPower=absMaxPower + 1,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("15", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower-1, Duration=5, Cause=LocalOptimization (no MaxPower)",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower - 1,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("16", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=86401, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=86401,
                                                       expected_status=Status.ConstraintError)

        self.step("17", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=0, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=0,
                                                       expected_status=Status.ConstraintError)

        self.step("18", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=InvalidValue (out of range)",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        # Send command with invalid cause enum value
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.PowerRangeAdjustRequest(
                minPower=absMinPower,
                maxPower=absMaxPower,
                duration=5,
                cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kUnknownEnumValue),  # Invalid enum value
                endpoint=self.get_endpoint(),
                timedRequestTimeoutMs=3000)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned")

        self.step("19", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("19a", "TH reads PowerRangeAdjustment",
                  expectation="value is MinPower=AbsMinPower, MaxPower=AbsMaxPower, and Cause=GridOptimization")
        powerRangeAdjustment = await self.read_dem_attribute_expect_success(attribute="PowerRangeAdjustment")
        asserts.assert_equal(powerRangeAdjustment.minPower, absMinPower)
        asserts.assert_equal(powerRangeAdjustment.maxPower, absMaxPower)
        asserts.assert_equal(powerRangeAdjustment.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kGridOptimizationAdjustment)

        self.step("19b", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("20", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower+100, MaxPower=AbsMaxPower-100, Duration=7200, Cause=LocalOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower + 100,
                                                       maxPower=absMaxPower - 100,
                                                       duration=7200)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("20a", "TH reads PowerRangeAdjustment",
                  expectation="value is MinPower=AbsMinPower+100, MaxPower=AbsMaxPower-100, and Cause=LocalOptimization (previous GridOptimization adjustment has been replaced)")
        powerRangeAdjustment = await self.read_dem_attribute_expect_success(attribute="PowerRangeAdjustment")
        asserts.assert_equal(powerRangeAdjustment.minPower, absMinPower + 100)
        asserts.assert_equal(powerRangeAdjustment.maxPower, absMaxPower - 100)
        asserts.assert_equal(powerRangeAdjustment.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("20b", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("21", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower+200, MaxPower=AbsMaxPower-200, Duration=2000, Cause=GridOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                                       minPower=absMinPower + 200,
                                                       maxPower=absMaxPower - 200,
                                                       duration=2000)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("21a", "TH reads PowerRangeAdjustment",
                  expectation="value is MinPower=AbsMinPower+200, MaxPower=AbsMaxPower-200, and Cause=GridOptimization (previous LocalOptimization adjustment has been replaced)")
        powerRangeAdjustment = await self.read_dem_attribute_expect_success(attribute="PowerRangeAdjustment")
        asserts.assert_equal(powerRangeAdjustment.minPower, absMinPower + 200)
        asserts.assert_equal(powerRangeAdjustment.maxPower, absMaxPower - 200)
        asserts.assert_equal(powerRangeAdjustment.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kGridOptimizationAdjustment)

        self.step("21b", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("22", "TH sends CancelPowerRangeAdjustRequest",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled")
        await self.send_cancel_power_range_adjustment_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("22a", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("22b", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("23", "TH sends CancelPowerRangeAdjustRequest", expectation="DUT responds with status INVALIDINSTATE")
        await self.send_cancel_power_range_adjustment_command(expected_status=Status.InvalidInState)

        self.step("24", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=2000, Cause=LocalOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=2000)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("24a", "TH reads ESAState", expectation="value is 0x03 (PowerAdjustActive)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("24b", "Wait 2 seconds",
                  expectation="Time elapses (TH waits 2 seconds while LocalOptimization is still active)")
        await asyncio.sleep(2)

        self.step("25", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Local Optimization Test Event",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=UserOptOut")
        await self.send_test_event_trigger_user_opt_out_local()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kUserOptOut)

        self.step("25a", "TH reads ESAState", expectation="value is 0x01 (Online)")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("25b", "TH reads PowerRangeAdjustment", expectation="value is null")
        await self.check_dem_attribute("PowerRangeAdjustment", NullValue)

        self.step("25c", "TH reads OptOutState", expectation="value is 0x01 (LocalOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("26", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Test Event Clear",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("26a", "TH reads OptOutState", expectation="value is 0x00 (NoOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("27", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Local Optimization Test Event",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("27a", "TH reads OptOutState", expectation="value is 0x01 (LocalOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("28", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("29", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("29a", "TH sends CancelPowerRangeAdjustRequest",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled")
        await self.send_cancel_power_range_adjustment_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("29b", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Test Event Clear",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("29c", "TH reads OptOutState", expectation="value is 0x00 (NoOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("30", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Grid Optimization Test Event",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("30a", "TH reads OptOutState", expectation="value is 0x02 (GridOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kGridOptOut)

        self.step("31", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=GridOptimization",
                  expectation="DUT responds with status CONSTRAINT_ERROR")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5,
                                                       expected_status=Status.ConstraintError)

        self.step("32", "TH sends PowerRangeAdjustRequest with MinPower=AbsMinPower, MaxPower=AbsMaxPower, Duration=5, Cause=LocalOptimization",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E04(PowerRangeAdjustStart) sent")
        await self.send_power_range_adjustment_command(cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                       minPower=absMinPower,
                                                       maxPower=absMaxPower,
                                                       duration=5)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustStart)

        self.step("32a", "TH sends CancelPowerRangeAdjustRequest",
                  expectation="DUT responds with status SUCCESS and Event DEM.S.E05(PowerRangeAdjustEnd) sent with Cause=Cancelled")
        await self.send_cancel_power_range_adjustment_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerRangeAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("33", "TH sends TestEventTrigger command to General Diagnostics Cluster for User Opt-out Test Event Clear",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("33a", "TH reads OptOutState", expectation="value is 0x00 (NoOptOut)")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("34", "TH sends TestEventTrigger command to General Diagnostics Cluster for Power Range Adjustment Test Event Clear",
                  expectation="DUT responds with status SUCCESS")
        await self.send_test_event_trigger_power_range_adjustment_clear()

        self.step("35", "Cancel the subscription to the Device Energy Management cluster",
                  expectation="The subscription is cancelled successfully")
        events_callback.cancel()


if __name__ == "__main__":
    default_matter_test_main()
