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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SMOKECOALARM(MatterBaseTest):
    """Tests for chef smokecoalarm device."""

    ENDPOINT = 1

    def desc_TC_SMOKECOALARM(self) -> str:
        return "[TC_SMOKECOALARM] chef smokecoalarm functionality test."

    def steps_TC_SMOKECOALARM(self):
        return [TestStep(1, "[TC_SMOKECOALARM] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT] Test Carbon Monoxide Concentration Measurement."),
                TestStep(3, "[TC_GROUPS] Test Groups."),
                TestStep(4, "[TC_IDENTIFY] Test Identify."),
                TestStep(5, "[TC_RELATIVE_HUMIDITY_MEASUREMENT] Test Relative Humidity Measurement."),
                TestStep(6, "[TC_SMOKE_CO_ALARM] Test Smoke CO Alarm."),
                TestStep(7, "[TC_TEMPERATURE_MEASUREMENT] Test Temperature Measurement.")]

    # CarbonMonoxideConcentrationMeasurement Cluster Helper Methods
    async def _read_co_measured_value(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.CarbonMonoxideConcentrationMeasurement, attribute=Clusters.Objects.CarbonMonoxideConcentrationMeasurement.Attributes.MeasuredValue)

    # Groups Cluster Helper Methods
    async def _read_groups_name_support(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.Groups, attribute=Clusters.Objects.Groups.Attributes.NameSupport)

    # Identify Cluster Helper Methods
    async def _read_identify_identify_time(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.Identify, attribute=Clusters.Objects.Identify.Attributes.IdentifyTime)

    async def _read_identify_identify_type(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.Identify, attribute=Clusters.Objects.Identify.Attributes.IdentifyType)

    # RelativeHumidityMeasurement Cluster Helper Methods
    async def _read_humidity_measured_value(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.RelativeHumidityMeasurement, attribute=Clusters.Objects.RelativeHumidityMeasurement.Attributes.MeasuredValue)

    # SmokeCoAlarm Cluster Helper Methods
    async def _read_smoke_co_expressed_state(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.ExpressedState)

    async def _read_smoke_co_smoke_state(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.SmokeState)

    async def _read_smoke_co_co_state(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.COState)

    async def _read_smoke_co_battery_alert(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.BatteryAlert)

    async def _read_smoke_co_device_muted(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.DeviceMuted)

    async def _read_smoke_co_test_in_progress(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.TestInProgress)

    async def _read_smoke_co_hardware_fault_alert(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.HardwareFaultAlert)

    async def _read_smoke_co_end_of_service_alert(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.SmokeCoAlarm, attribute=Clusters.Objects.SmokeCoAlarm.Attributes.EndOfServiceAlert)

    # TemperatureMeasurement Cluster Helper Methods
    async def _read_temperature_measured_value(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Objects.TemperatureMeasurement, attribute=Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue)

    # Cluster Tests
    async def carbon_monoxide_concentration_measurement_test(self, endpoint):
        measured_value = await self._read_co_measured_value(endpoint)
        asserts.assert_true(measured_value is not None, "CO MeasuredValue should not be None.")

    async def groups_test(self, endpoint):
        name_support = await self._read_groups_name_support(endpoint)
        asserts.assert_true(name_support is not None, "Groups NameSupport should not be None.")

    async def identify_test(self, endpoint):
        identify_time = await self._read_identify_identify_time(endpoint)
        asserts.assert_equal(identify_time, 0, "IdentifyTime should be 0 initially.")

        identify_type = await self._read_identify_identify_type(endpoint)
        asserts.assert_true(identify_type is not None, "IdentifyType should not be None.")

        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(identifyTime=10),
            endpoint=endpoint,
        )
        identify_time = await self._read_identify_identify_time(endpoint)
        asserts.assert_equal(identify_time, 10, "IdentifyTime should be 10 after Identify command.")

    async def relative_humidity_measurement_test(self, endpoint):
        measured_value = await self._read_humidity_measured_value(endpoint)
        asserts.assert_true(measured_value is not None, "Humidity MeasuredValue should not be None.")

    async def smoke_co_alarm_test(self, endpoint):
        expressed_state = await self._read_smoke_co_expressed_state(endpoint)
        asserts.assert_true(expressed_state is not None, "ExpressedState should not be None.")

        smoke_state = await self._read_smoke_co_smoke_state(endpoint)
        asserts.assert_true(smoke_state is not None, "SmokeState should not be None.")

        co_state = await self._read_smoke_co_co_state(endpoint)
        asserts.assert_true(co_state is not None, "COState should not be None.")

        battery_alert = await self._read_smoke_co_battery_alert(endpoint)
        asserts.assert_true(battery_alert is not None, "BatteryAlert should not be None.")

        device_muted = await self._read_smoke_co_device_muted(endpoint)
        asserts.assert_true(device_muted is not None, "DeviceMuted should not be None.")

        test_in_progress = await self._read_smoke_co_test_in_progress(endpoint)
        asserts.assert_false(test_in_progress, "TestInProgress should be False initially.")

        hardware_fault_alert = await self._read_smoke_co_hardware_fault_alert(endpoint)
        asserts.assert_false(hardware_fault_alert, "HardwareFaultAlert should be False initially.")

        end_of_service_alert = await self._read_smoke_co_end_of_service_alert(endpoint)
        asserts.assert_true(end_of_service_alert is not None, "EndOfServiceAlert should not be None.")

        await self.send_single_cmd(
            cmd=Clusters.Objects.SmokeCoAlarm.Commands.SelfTestRequest(),
            endpoint=endpoint,
        )

    async def temperature_measurement_test(self, endpoint):
        measured_value = await self._read_temperature_measured_value(endpoint)
        asserts.assert_true(measured_value is not None, "Temperature MeasuredValue should not be None.")

    @async_test_body
    async def test_TC_SMOKECOALARM(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        # [TC_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT] Test Carbon Monoxide Concentration Measurement.
        self.step(2)
        await self.carbon_monoxide_concentration_measurement_test(self.ENDPOINT)

        # [TC_GROUPS] Test Groups.
        self.step(3)
        await self.groups_test(self.ENDPOINT)

        # [TC_IDENTIFY] Test Identify.
        self.step(4)
        await self.identify_test(self.ENDPOINT)

        # [TC_RELATIVE_HUMIDITY_MEASUREMENT] Test Relative Humidity Measurement.
        self.step(5)
        await self.relative_humidity_measurement_test(self.ENDPOINT)

        # [TC_SMOKE_CO_ALARM] Test Smoke CO Alarm.
        self.step(6)
        await self.smoke_co_alarm_test(self.ENDPOINT)

        # [TC_TEMPERATURE_MEASUREMENT] Test Temperature Measurement.
        self.step(7)
        await self.temperature_measurement_test(self.ENDPOINT)


if __name__ == "__main__":
    default_matter_test_main()
