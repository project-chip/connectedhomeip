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

import chip.clusters as Clusters
from chip.testing.event_attribute_reporting import AttributeChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_PUMP(MatterBaseTest):
    """Tests for chef pump device."""

    # Ignore report if it is < 1s since the last report.
    _SUBSCRIPTION_MIN_INTERVAL_SEC = 0

    # Set this to a large value so any liveliness updates aren't received during test.
    _SUBSCRIPTION_MAX_INTERVAL_SEC = 3600

    _PUMP_ENDPOINT = 1

    _MIN_LEVEL = 1

    def desc_TC_PUMP(self) -> str:
        return "[TC_PUMP] Mandatory functionality with chef pump device as server"

    def steps_TC_PUMP(self):
        return [TestStep(1, "[PUMP] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[PUMP] Assert initial attribute values are expected."),
                TestStep(3, "[PUMP] Subscribe to all required attributes."),
                TestStep(4, "[PUMP] Turn on pump."),
                TestStep(5, "[PUMP] Increase level."),
                TestStep(6, "[PUMP] Turn off pump.")]

    async def _read_on_off(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.OnOff, attribute=Clusters.Objects.OnOff.Attributes.OnOff)

    async def _read_current_level(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.LevelControl, attribute=Clusters.Objects.LevelControl.Attributes.CurrentLevel)

    async def _read_temperature(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.TemperatureMeasurement, attribute=Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue,
        )

    async def _read_pressure(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.PressureMeasurement, attribute=Clusters.Objects.PressureMeasurement.Attributes.MeasuredValue,
        )

    async def _read_flow(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.FlowMeasurement, attribute=Clusters.Objects.FlowMeasurement.Attributes.MeasuredValue,
        )

    async def _read_pump_capacity(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.PumpConfigurationAndControl, attribute=Clusters.Objects.PumpConfigurationAndControl.Attributes.Capacity,
        )

    async def _read_pump_status(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._PUMP_ENDPOINT, cluster=Clusters.Objects.PumpConfigurationAndControl, attribute=Clusters.Objects.PumpConfigurationAndControl.Attributes.PumpStatus,
        )

    async def _subscribe_attribute(self, attribute):
        sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(self._PUMP_ENDPOINT, attribute)],
            reportInterval=(self._SUBSCRIPTION_MIN_INTERVAL_SEC, self._SUBSCRIPTION_MAX_INTERVAL_SEC),
            keepSubscriptions=True,
        )
        attr_cb = AttributeChangeCallback(attribute)
        sub.SetAttributeUpdateCallback(attr_cb)
        return sub, attr_cb

    @async_test_body
    async def test_TC_PUMP(self):
        # *** STEP 1 ***
        # Commissioning already done.
        self.step(1)

        # ** STEP 2 ***
        # Assert initial attribute values are expected.
        self.step(2)
        asserts.assert_equal(await self._read_on_off(), False)
        asserts.assert_equal(await self._read_current_level(), self._MIN_LEVEL)
        temp = await self._read_temperature()
        asserts.assert_equal(temp, 0)
        pressure = await self._read_pressure()
        asserts.assert_equal(pressure, 0)
        flow = await self._read_flow()
        asserts.assert_equal(flow, 0)
        pump_capacity = await self._read_pump_capacity()
        asserts.assert_equal(pump_capacity, 0)
        asserts.assert_equal(await self._read_pump_status(), 0)

        # ** STEP 3 ***
        self.step(3)
        # Subscribe to all required attributes.
        on_off_sub, on_off_cb = await self._subscribe_attribute(Clusters.Objects.OnOff.Attributes.OnOff)
        current_level_sub, current_level_cb = await self._subscribe_attribute(Clusters.Objects.LevelControl.Attributes.CurrentLevel)
        temp_sub, temp_cb = await self._subscribe_attribute(Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue)
        pressure_sub, pressure_cb = await self._subscribe_attribute(Clusters.Objects.PressureMeasurement.Attributes.MeasuredValue)
        flow_sub, flow_cb = await self._subscribe_attribute(Clusters.Objects.FlowMeasurement.Attributes.MeasuredValue)
        capacity_sub, capacity_cb = await self._subscribe_attribute(Clusters.Objects.PumpConfigurationAndControl.Attributes.Capacity)
        status_sub, status_cb = await self._subscribe_attribute(Clusters.Objects.PumpConfigurationAndControl.Attributes.PumpStatus)

        # ** STEP 4 **
        # Turn on pump.
        self.step(4)
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.On(),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._PUMP_ENDPOINT,
        )
        asserts.assert_equal(on_off_cb.wait_for_report(), True)
        asserts.assert_equal(await self._read_current_level(), self._MIN_LEVEL)
        temp_ = temp_cb.wait_for_report()
        asserts.assert_true(temp_ > temp, "Temperature raise not reported.")
        temp = temp_
        pressure_ = pressure_cb.wait_for_report()
        asserts.assert_true(pressure_ > pressure, "Pressure raise not reported.")
        pressure = pressure_
        flow_ = flow_cb.wait_for_report()
        asserts.assert_true(flow_ > flow, "Flow raise not reported.")
        flow = flow_
        asserts.assert_equal(capacity_cb.wait_for_report(), 0)  # Capacity is 0 at min level.
        asserts.assert_equal(status_cb.wait_for_report(),
                             Clusters.Objects.PumpConfigurationAndControl.Bitmaps.PumpStatusBitmap.kRunning)

        # ** STEP 5 **
        # Increase level.
        self.step(5)
        levels = [50, 100, 150]
        for level in levels:
            # Move to level
            await self.send_single_cmd(
                cmd=Clusters.Objects.LevelControl.Commands.MoveToLevel(level=level),
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self._PUMP_ENDPOINT,
            )
            asserts.assert_equal(current_level_cb.wait_for_report(), level)
            temp_ = temp_cb.wait_for_report()
            asserts.assert_true(temp_ > temp, "Temperature raise not reported.")
            temp = temp_
            pressure_ = pressure_cb.wait_for_report()
            asserts.assert_true(pressure_ > pressure, "Pressure raise not reported.")
            pressure = pressure_
            flow_ = flow_cb.wait_for_report()
            asserts.assert_true(flow_ > flow, "Flow raise not reported.")
            flow = flow_
            pump_capacity_ = capacity_cb.wait_for_report()
            asserts.assert_true(pump_capacity_ > pump_capacity, "Pump capacity raise not reported.")
            pump_capacity = pump_capacity_
            # On/Off and status unchanged
            asserts.assert_equal(await self._read_on_off(), True)
            asserts.assert_equal(await self._read_pump_status(), Clusters.Objects.PumpConfigurationAndControl.Bitmaps.PumpStatusBitmap.kRunning)

        # ** STEP 6 **
        # Turn Off pump.
        self.step(6)
        current_level = await self._read_current_level()
        await self.send_single_cmd(
            cmd=Clusters.Objects.OnOff.Commands.Off(),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._PUMP_ENDPOINT,
        )
        asserts.assert_equal(on_off_cb.wait_for_report(), False)
        asserts.assert_equal(await self._read_current_level(), current_level)
        asserts.assert_equal(temp_cb.wait_for_report(), 0)
        asserts.assert_equal(pressure_cb.wait_for_report(), 0)
        asserts.assert_equal(flow_cb.wait_for_report(), 0)
        asserts.assert_equal(capacity_cb.wait_for_report(), 0)
        asserts.assert_equal(status_cb.wait_for_report(), 0)


if __name__ == "__main__":
    default_matter_test_main()
