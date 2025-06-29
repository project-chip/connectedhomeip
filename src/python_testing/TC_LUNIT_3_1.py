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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_attribute, has_cluster,
                                         run_if_endpoint_matches)
from mobly import asserts


class TC_LUNIT_3_1(MatterBaseTest):
    async def read_lunit_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.UnitLocalization
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_lunit_temp_unit(self,
                                    endpoint,
                                    temp_unit,
                                    dev_ctrl: ChipDeviceCtrl = None,
                                    expected_status: list[Status] = [Status.Success]) -> Status:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        cluster = Clusters.Objects.UnitLocalization
        result = await dev_ctrl.WriteAttribute(self.dut_node_id, [(endpoint, cluster.Attributes.TemperatureUnit(temp_unit))])
        status = result[0].Status
        asserts.assert_in(status, expected_status,
                          f"TemperatureUnit write returned {status.name}; expected {[e.name for e in expected_status]}")
        return status

    def desc_TC_LUNIT_3_1(self) -> str:
        return "[TC-LUNIT-2.1] Read and Write Unit Localization Cluster Attributes with DUT as Server"

    def steps_TC_LUNIT_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commission DUT if required", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the TemperatureUnit attribute"),
            TestStep(2, "If supported, TH reads from the DUT the SupportedTemperatureUnits attribute",
                     "Verify the list length is between 2 and 3 inclusive.\nVerify that each entry in the list is a valid\nTempUnitEnum value and is unique."),
            TestStep(3, "If the SupportedTemperatureUnits is supported, TH creates a `supported` list With each entry in SupportedUnitsList. If SupportedTemperatureUnits is not supported, TH creates a `supported` list with Celsius and Fahrenheit. TH writes each entry in the `supported` list to the DUT the TemperatureUnit attribute", "Write is successful"),
            TestStep(4, "If the SupportedTemperatureUnits is supported, TH construct a list with valid TempUnitEnum values that are not contained in SupportedUnitsList"),
            TestStep(5, "if the SupportedTemperatureUnits is supported, With each entry in UnsupportedUnitsList, TH writes to the DUT the TemperatureUnit attribute",
                     "Write returns CONSTRAINT_ERROR"),
            TestStep(6, "If SupportedTemperatureUnits is not supported, TH writes Kelvin",
                     "Write either returns SUCCESS or CONSTRAINT_ERROR")
        ]
        return steps

    def pics_TC_LUNIT_3_1(self) -> list[str]:
        pics = [
            "LUNIT.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.UnitLocalization) and has_attribute(Clusters.UnitLocalization.Attributes.TemperatureUnit))
    async def test_TC_LUNIT_3_1(self):

        endpoint = self.get_endpoint(default=0)
        attributes = Clusters.UnitLocalization.Attributes
        features = await self.read_lunit_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        self.supports_TEMP = bool(features & Clusters.UnitLocalization.Bitmaps.Feature.kTemperatureUnit)
        attribute_list = await self.read_lunit_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        has_supported_temperature_units = attributes.SupportedTemperatureUnits.attribute_id in attribute_list

        # Step 0 - Commissioning, already done", is_commissioning=True)

        self.step(0)  # commissioning - already done

        if self.supports_TEMP:
            # Step 1 - TH reads from the DUT the TemperatureUnit attribute")
            self.step(1)
            temperature_unit = await self.read_lunit_attribute_expect_success(endpoint=endpoint, attribute=attributes.TemperatureUnit)

            asserts.assert_greater_equal(temperature_unit, Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kFahrenheit,
                                         "TemperatureUnit has to be Fahrenheit, Celsius or Kelvin")
            asserts.assert_less_equal(temperature_unit, Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kKelvin,
                                      "TemperatureUnit has to be Fahrenheit, Celsius or Kelvin")

            # Step 2 - TH reads from the DUT the SupportedTemperatureUnits attribute")
            self.step(2)
            if has_supported_temperature_units:
                supported_temperature_units = await self.read_lunit_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedTemperatureUnits)
                asserts.assert_greater_equal(len(supported_temperature_units), 2,
                                             "SupportedTemperatureUnits must have at least two entry in the list")
                asserts.assert_less_equal(len(supported_temperature_units), 3,
                                          "SupportedTemperatureUnits may have a maximum of three entries in the list")
            else:
                # Per CCB 4201, support Kelvin is not enforced for devices that predate SupportedTemperatureUnits
                supported_temperature_units = [Clusters.UnitLocalization.Enums.TempUnitEnum.kCelsius,
                                               Clusters.UnitLocalization.Enums.TempUnitEnum.kFahrenheit]
            for unit in supported_temperature_units:
                asserts.assert_greater_equal(unit, Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kFahrenheit,
                                             "Each entry in SupportedTemperatureUnits has to be Fahrenheit, Celsius or Kelvin")
                asserts.assert_less_equal(unit, Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kKelvin,
                                          "Each entry in SupportedTemperatureUnits has to be Fahrenheit, Celsius or Kelvin")

            # Step 3 - With each entry in SupportedUnitsList, TH writes to the DUT the TemperatureUnit attribute")
            self.step(3)
            for unit in supported_temperature_units:
                await self.write_lunit_temp_unit(endpoint=endpoint, temp_unit=unit, expected_status=[Status.Success])

            # Step 4 - Construct a list with valid TempUnitEnum values that are not contained in

            self.step(4)
            if has_supported_temperature_units:
                unsupported_temperature_units = []
                for unit in [Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kFahrenheit,
                             Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kCelsius,
                             Clusters.Objects.UnitLocalization.Enums.TempUnitEnum.kKelvin]:
                    if unit not in supported_temperature_units:
                        unsupported_temperature_units.append(unit)
            else:
                self.mark_current_step_skipped()

            # Step 5 - With each entry in UnsupportedUnitsList, TH writes to the DUT the TemperatureUnit attribute")
            self.step(5)
            if has_supported_temperature_units:
                for unit in unsupported_temperature_units:
                    await self.write_lunit_temp_unit(endpoint=endpoint, temp_unit=unit, expected_status=[Status.ConstraintError])
            else:
                self.mark_current_step_skipped()

            self.step(6)
            if not has_supported_temperature_units:
                await self.write_lunit_temp_unit(endpoint=endpoint, temp_unit=unit, expected_status=[Status.Success, Status.ConstraintError])
            else:
                self.mark_current_step_skipped()

        else:
            self.skip_all_remaining_steps(1)
            return


if __name__ == "__main__":
    default_matter_test_main()
