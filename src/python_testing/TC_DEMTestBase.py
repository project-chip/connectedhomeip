#
#    Copyright (c) 2023 Project CHIP Authors
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
from chip.interaction_model import InteractionModelError, Status
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts

logger = logging.getLogger(__name__)


class DEMTestBase:

    async def read_dem_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        cluster = Clusters.Objects.DeviceEnergyManagement
        full_attr = getattr(cluster.Attributes, attribute)
        logging.info(f"endpoint {endpoint} full_attr {full_attr}")
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_dem_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_dem_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def validate_feature_map(self, must_have_features, must_not_have_features):
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")
        for must_have_feature in must_have_features:
            asserts.assert_true(feature_map & must_have_feature,
                                f"{must_have_feature.name} must be set but is not. feature_map 0x{feature_map:x}")

        for must_not_have_feature in must_not_have_features:
            asserts.assert_false(feature_map & must_not_have_feature,
                                 f"{must_not_have_feature.name} is not allowed to be set. feature_map 0x{feature_map:x}")

    async def validate_pfr_or_sfr_in_feature_map(self):
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")

        illegal_combination = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting | Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting
        asserts.assert_not_equal(feature_map & illegal_combination, illegal_combination,
                                 f"Cannot have kPowerForecastReporting and kStateForecastReporting both set. feature_map 0x{feature_map:x}")

        asserts.assert_not_equal(feature_map & illegal_combination, 0,
                                 f"Must have one of kPowerForecastReporting and kStateForecastReporting set. feature_map 0x{feature_map:x}")

    async def send_power_adjustment_command(self, power: int, duration: int,
                                            cause: Clusters.Objects.DeviceEnergyManagement.Enums.CauseEnum,
                                            endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                            expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.PowerAdjustRequest(
                power=power,
                duration=duration,
                cause=cause),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_cancel_power_adjustment_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                                   expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.CancelPowerAdjustRequest(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_start_time_adjust_request_command(self, requestedStartTime: int,
                                                     cause: Clusters.Objects.DeviceEnergyManagement.Enums.CauseEnum,
                                                     endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                                     expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.StartTimeAdjustRequest(
                requestedStartTime=requestedStartTime,
                cause=cause),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_start_time_adjust_clear_command(self,
                                                   endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                                   expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.StartTimeAdjustClear(),  # StartTimeAdjustmentClear(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_cancel_request_command(self,
                                          endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                          expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.CancelRequest(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_pause_request_command(self, duration: int, cause:
                                         Clusters.Objects.DeviceEnergyManagement.Enums.AdjustmentCauseEnum,
                                         endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                         expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.PauseRequest(
                duration=duration,
                cause=cause),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_resume_request_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                          expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.ResumeRequest(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_modify_forecast_request_command(self, forecastID: int,
                                                   slotAdjustments: list[Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct],
                                                   cause: Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum,
                                                   endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                                   expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.ModifyForecastRequest(forecastID=forecastID,
                                                                                                          slotAdjustments=slotAdjustments,
                                                                                                          cause=cause),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_request_constraint_based_forecast(self, constraintList: list[Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct],
                                                     cause: Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum,
                                                     endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                                     expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyManagement.Commands.RequestConstraintBasedForecast(constraints=constraintList,
                                                                                                                   cause=cause),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    def print_forecast(self, forecast):
        for index, slot in enumerate(forecast.slots):
            logging.info(
                f"   [{index}] MinDuration: {slot.minDuration} MaxDuration: {slot.maxDuration} DefaultDuration: {slot.defaultDuration}")
            logging.info(f"       ElapseSlotTime: {slot.elapsedSlotTime} RemainingSlotTime: {slot.remainingSlotTime}")
            logging.info(
                f"       SlotIsPausable: {slot.slotIsPausable} MinPauseDuration: {slot.minPauseDuration} MaxPauseDuration: {slot.maxPauseDuration}")
            logging.info(f"       ManufacturerESAState: {slot.manufacturerESAState}")
            logging.info(f"       NominalPower: {slot.nominalPower} MinPower: {slot.minPower} MaxPower: {slot.maxPower}")
            logging.info(f"       MinPowerAdjustment: {slot.minPowerAdjustment} MaxPowerAdjustment: {slot.maxPowerAdjustment}")
            logging.info(
                f"       MinDurationAdjustment: {slot.minDurationAdjustment} MaxDurationAdjustment: {slot.maxDurationAdjustment}")
            if slot.costs is not None:
                for cost_index, cost in enumerate(slot):
                    logging.info(
                        f"   Cost: [{cost_index}]  CostType:{cost.costType} Value: {cost.value} DecimalPoints: {cost.decimalPoints} Currency: {cost.currency}")

    def get_current_utc_time_in_seconds(self):
        microseconds_in_second = 1000000
        return int(utc_time_in_matter_epoch()/microseconds_in_second)

    async def send_test_event_trigger_power_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000000)

    async def send_test_event_trigger_power_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000001)

    async def send_test_event_trigger_user_opt_out_local(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000002)

    async def send_test_event_trigger_user_opt_out_grid(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000003)

    async def send_test_event_trigger_user_opt_out_clear_all(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000004)

    async def send_test_event_trigger_start_time_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000005)

    async def send_test_event_trigger_start_time_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000006)

    async def send_test_event_trigger_pausable(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000007)

    async def send_test_event_trigger_pausable_next_slot(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000008)

    async def send_test_event_trigger_pausable_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000009)

    async def send_test_event_trigger_forecast_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000A)

    async def send_test_event_trigger_forecast_adjustment_next_slot(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000B)

    async def send_test_event_trigger_forecast_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000C)

    async def send_test_event_trigger_constraint_based_adjustment(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000D)

    async def send_test_event_trigger_constraint_based_adjustment_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000E)

    async def send_test_event_trigger_forecast(self):
        await self.send_test_event_triggers(eventTrigger=0x009800000000000F)

    async def send_test_event_trigger_forecast_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0098000000000010)
