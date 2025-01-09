#
#    Copyright (c) 2024 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

import pdb

logger = logging.getLogger(__name__)


class TC_FAN_3_1(MatterBaseTest):
    
    def steps_TC_FAN_3_1(self):
        return [TestStep("1", "Commissioning, already done."),
                TestStep("2", "TH reads from the DUT the FanMode attribute and stores it",
                         "Verify that the DUT response contains a FanModeEnum"),
                TestStep("3",
                         "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold.", ""),
                TestStep("4", "If supports_icd is true, TH reads FeatureMap from the ICD Management cluster on EP0. If the LITS feature is set, set supports_lit to true. Otherwise set supports_lit to false.", ""),
                TestStep("5", "TH constructs the instance name for the DUT as the 64-bit compressed Fabric identifier, and the assigned 64-bit Node identifier, each expressed as a fixed-length sixteen-character hexadecimal string, encoded as ASCII (UTF-8) text using capital letters, separated by a hyphen.", ""),
                TestStep("6", "TH performs a query for the SRV record against the qname instance_qname.",
                         "Verify SRV record is returned"),
                # TestStep(7, "TH performs a query for the TXT record against the qname instance_qname.",
                #          "Verify TXT record is returned"),
                # TestStep(8, "TH performs a query for the AAAA record against the target listed in the SRV record",
                #          "Verify AAAA record is returned"),
                # TestStep(9, "TH verifies the following from the returned records:",
                #          "TH verifies the following from the returned records: The hostname must be a fixed-length twelve-character (or sixteen-character) hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.. ICD TXT key: • If supports_lit is false, verify that the ICD key is NOT present in the TXT record • If supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII) SII TXT key: • If supports_icd is true and supports_lit is false, set sit_mode to true • If supports_icd is true and supports_lit is true, set sit_mode to true if ICD=0 otherwise set sit_mode to false • If supports_icd is false, set sit_mode to false • If sit_mode is true, verify that the SII key IS present in the TXT record • if the SII key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms) SAI TXT key: • if supports_icd is true, verify that the SAI key is present in the TXT record • If the SAI key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms)"),
                # TestStep(10, "TH performs a DNS-SD browse for _I<hhhh>._sub._matter._tcp.local, where <hhhh> is the 64-bit compressed Fabric identifier, expressed as a fixed-length, sixteencharacter hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.",
                #          "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                # TestStep(11, "TH performs a DNS-SD browse for _matter._tcp.local",
                #          "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                ]

    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_fan_mode(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.FanMode)

    async def read_percent_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentSetting)

    async def read_percent_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentCurrent)

    async def read_speed_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedSetting)

    async def read_speed_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedCurrent)

    async def read_fan_mode_sequence(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.FanModeSequence)

    async def write_fan_mode(self, endpoint, fan_mode) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.FanMode(fan_mode))])
        return result[0].Status

    async def write_percent_setting(self, endpoint, percent_setting) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.PercentSetting(percent_setting))])
        return result[0].Status

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        endpoint = self.get_endpoint(default=1)
        speed_attributes_present = self.check_pics("FAN.S.F00")
        print(f"\n\n\n\t\t\t[FANS] speed_attributes_present: {speed_attributes_present}\n\n\n")

        existing_speed_setting = await self.read_speed_setting(endpoint=endpoint)
        print(f"\n\n\n\t\t\t[FANS] existing_speed_setting: {existing_speed_setting}\n\n\n")

        existing_speed_current = await self.read_speed_current(endpoint=endpoint)
        print(f"\n\n\n\t\t\t[FANS] existing_speed_current: {existing_speed_current}\n\n\n")


        pdb.set_trace()


        # *** STEP 1 ***
        # Commissioning, already done.
        self.step("1")

        # *** STEP 2 ***
        # The TH reads the initial FanMode attribute value from the DUT and stores it
        # Verify that the DUT response contains a FanModeEnum value
        self.step("2")
        initial_fan_mode = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(initial_fan_mode, Clusters.FanControl.Enums.FanModeEnum, "Response doesn't contain a FanModeEnum value")

        initial_percent_setting = await self.read_percent_setting(endpoint=endpoint)
        initial_percent_current = await self.read_percent_current(endpoint=endpoint)
        
        print(f"\n\n\n\t\t\t[FANS] initial_fan_mode: {initial_fan_mode}\n\n\n")
        print(f"\n\n\n\t\t\t[FANS] initial_percent_setting: {initial_percent_setting}\n\n\n")
        print(f"\n\n\n\t\t\t[FANS] initial_percent_current: {initial_percent_current}\n\n\n")

        # *** STEP 3 ***
        # The TH writes the Off value to the FanMode attribute on the DUT and reads it back after a few seconds
        # The DUT shall return either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS, the fan mode read should match the fan mode written, verify that the PercentSetting and
        # PercentCurrent values are zero
        # If INVALID_IN_STATE, the fan mode read should match the initial fan mode
        self.step("3")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kOff
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(3)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Off")
            percent_setting_off = await self.read_percent_setting(endpoint=endpoint)
            percent_current_off = await self.read_percent_current(endpoint=endpoint)
            print(f"\n\n\n\t\t\t [FANS] OFF percent_setting: {percent_setting_off}")
            print(f"\n\n\n\t\t\t [FANS] OFF percent_setting: {percent_current_off}")
            
            asserts.assert_equal(percent_setting_off, 0, "PercentSetting value must be 0 when a fan mode Off is set.")
            asserts.assert_equal(percent_current_off, 0, "PercentCurrent value must be 0 when a fan mode Off is set.")
        else:
            asserts.assert_equal(fan_mode_read, initial_fan_mode, "FanMode is not unchanged")

        # *** STEP 4 ***
        # The TH writes the Low value to the FanMode attribute on the DUT and reads it back after a few seconds
        # The DUT shall return either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS, the fan mode read should match the fan mode written, verify that the PercentSetting and
        # PercentCurrent values are non-zero and greater than the ones from the Off fan mode from the previous step
        # If INVALID_IN_STATE, the fan mode read should match the initial fan mode
        self.step("4")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kLow
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(3)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Low")
            percent_setting_low = await self.read_percent_setting(endpoint=endpoint)
            percent_current_low = await self.read_percent_current(endpoint=endpoint)
            print(f"\n\n\n\t\t\t [FANS] LOW percent_setting: {percent_setting_low}")
            print(f"\n\n\n\t\t\t [FANS] LOW percent_setting: {percent_current_low}")
            
            asserts.assert_greater(percent_setting_low, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_low, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_low, percent_setting_off, "PercentSetting value in Low fan mode must be greater than in Off fan mode.")
            asserts.assert_greater(percent_current_low, percent_current_off, "PercentCurrent value in Low fan mode must be greater than in Off fan mode.")            
        else:
            asserts.assert_equal(fan_mode_read, initial_fan_mode, "FanMode is not unchanged")

        # *** STEP 5 ***
        # The TH writes the Medium value to the FanMode attribute on the DUT and reads it back after a few seconds
        # The DUT shall return either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS, the fan mode read should match the fan mode written, verify that the PercentSetting and
        # PercentCurrent values are non-zero and greater than the ones from the Low fan mode from the previous step
        # If INVALID_IN_STATE, the fan mode read should match the initial fan mode
        self.step("5")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kMedium
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(3)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Medium")
            percent_setting_medium = await self.read_percent_setting(endpoint=endpoint)
            percent_current_medium = await self.read_percent_current(endpoint=endpoint)
            print(f"\n\n\n\t\t\t [FANS] MEDIUM percent_setting: {percent_setting_medium}")
            print(f"\n\n\n\t\t\t [FANS] MEDIUM percent_setting: {percent_current_medium}")
            
            asserts.assert_greater(percent_setting_medium, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_medium, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_medium, percent_setting_low, "PercentSetting value in Medium fan mode must be greater than in Low fan mode.")
            asserts.assert_greater(percent_current_medium, percent_current_low, "PercentCurrent value in Medium fan mode must be greater than in Low fan mode.")
        else:
            asserts.assert_equal(fan_mode_read, initial_fan_mode, "FanMode is not unchanged")

        # # *** STEP 6 ***
        # The TH writes the High value to the FanMode attribute on the DUT and reads it back after a few seconds
        # The DUT shall return either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS, the fan mode read should match the fan mode written, verify that the PercentSetting and
        # PercentCurrent values are non-zero and greater than the ones from the Medium fan mode from the previous step
        # If INVALID_IN_STATE, the fan mode read should match the initial fan mode
        self.step("6")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kHigh
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(3)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to High")
            percent_setting_high = await self.read_percent_setting(endpoint=endpoint)
            percent_current_high = await self.read_percent_current(endpoint=endpoint)
            print(f"\n\n\n\t\t\t [FANS] HIGH percent_setting: {percent_setting_high}")
            print(f"\n\n\n\t\t\t [FANS] HIGH percent_setting: {percent_current_high}")
            
            asserts.assert_greater(percent_setting_high, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_high, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_high, percent_setting_medium, "PercentSetting value in High fan mode must be greater than in Medium fan mode.")
            asserts.assert_greater(percent_current_high, percent_current_medium, "PercentCurrent value in High fan mode must be greater than in Medium fan mode.")
        else:
            asserts.assert_equal(fan_mode_read, initial_fan_mode, "FanMode is not unchanged")


        















        # fan_mode_sequence = await self.read_fan_mode_sequence(endpoint=endpoint)
        # print(f"\n\n\n\n\t\t\t [FANS] fan_mode_sequence: {fan_mode_sequence}\n\n\n\n\n")

















        # self.print_step("3b", "Write Off to Fan Mode")
        # status = await self.write_fan_mode(endpoint=endpoint, fan_mode=Clusters.FanControl.Enums.FanModeEnum.kOff)
        # status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        # asserts.assert_true(status_ok, "FanMode write did not return a value of Success or InvalidInState")

        # self.print_step("3c", "After a few seconds, read from the DUT the PercentSetting attribute")
        # time.sleep(3)

        # new_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        # if status == Status.Success:
        #     asserts.assert_equal(new_percent_setting, Clusters.FanControl.Enums.FanModeEnum.kOff, "PercentSetting is not Off")
        # else:
        #     asserts.assert_equal(new_percent_setting, existing_percent_setting, "PercentSetting is not unchanged")

        # self.print_step("3d", "Read from the DUT the PercentCurrent attribute")
        # percent_current = await self.read_percent_current(endpoint=endpoint)

        # if status == Status.Success:
        #     asserts.assert_equal(percent_current, 0, "PercentCurrent is not 0")
        # else:
        #     asserts.assert_equal(percent_current, existing_percent_setting, "PercentCurrent is not unchanged")

        # self.print_step("4a", "Read from the DUT the PercentSetting attribute and store")
        # existing_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        # self.print_step("4b", "Write PercentSetting to 30")
        # status = await self.write_percent_setting(endpoint=endpoint, percent_setting=30)
        # status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        # asserts.assert_true(status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        # self.print_step("4c", "After a few seconds, read from the DUT the PercentSetting attribute")
        # time.sleep(3)

        # new_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        # if status == Status.Success:
        #     asserts.assert_equal(new_percent_setting, 30, "PercentSetting is not 30")
        # else:
        #     asserts.assert_equal(new_percent_setting, existing_percent_setting, "PercentSetting is not unchanged")

        # self.print_step("4d", "Read from the DUT the PercentCurrent attribute")
        # percent_current = await self.read_percent_current(endpoint=endpoint)

        # if status == Status.Success:
        #     asserts.assert_equal(percent_current, 30, "PercentCurrent is not 30")
        # else:
        #     asserts.assert_equal(percent_current, existing_percent_setting, "PercentCurrent is not unchanged")

        # self.print_step("5a", "Read from the DUT the FanMode attribute and store")
        # current_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        # self.print_step("5b", "Write PercentSetting to 0")
        # status = await self.write_percent_setting(endpoint=endpoint, percent_setting=0)
        # status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        # asserts.assert_true(status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        # self.print_step("5c", "After a few seconds, read from the DUT the FanMode attribute")
        # time.sleep(3)

        # new_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        # if status == Status.Success:
        #     asserts.assert_equal(new_fan_mode, Clusters.FanControl.Enums.FanModeEnum.kOff, "FanMode is not Off")
        # else:
        #     asserts.assert_equal(new_fan_mode, current_fan_mode, "FanMode is not unchanged")


if __name__ == "__main__":
    default_matter_test_main()
