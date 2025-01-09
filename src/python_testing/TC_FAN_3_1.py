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
import random

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# import pdb

logger = logging.getLogger(__name__)


class TC_FAN_3_1(MatterBaseTest):
    
    def steps_TC_FAN_3_1(self):
        return [TestStep("1", "Commissioning already done."),
                TestStep("2", "Action",
                         "Verification"),
                TestStep("3", "Action",
                         "Verification"),
                TestStep("4", "Action",
                         "Verification"),
                TestStep("5", "Action",
                         "Verification"),
                TestStep("6", "Action",
                         "Verification"),
                TestStep("7", "Action",
                         "Verification"),
                TestStep("8", "Action",
                         "Verification"),
                TestStep("9", "Action",
                         "Verification"),
                TestStep("10", "Action",
                         "Verification"),
                TestStep("11", "Action",
                         "Verification"),
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
        # pdb.set_trace()

        endpoint = self.get_endpoint(default=1)
        speed_attributes_present = self.check_pics("FAN.S.F00")
        seconds_delay = 1
        
        # FanMode-PercentSetting mapping provided by the Air Purifier app
        # +------------+----------------+----------------------+
        # |            | PercentSetting | PercentSetting after |
        # | FanMode    | range          | setting FanMode      |
        # +------------+----------------+----------------------+
        # | 0 (Off)    | 0              | 0                    |
        # +------------+----------------+----------------------+
        # | 1 (Low)    | 1 - 30         | 10                   |
        # +------------+----------------+----------------------+
        # | 2 (Medium) | 31 - 70        | 40                   |
        # +------------+----------------+----------------------+
        # | 3 (High)   | 71 - 100       | 80                   |
        # +------------+----------------+----------------------+
        percent_setting_range_off = 0
        percent_setting_range_low = random.randint(1, 30)
        percent_setting_range_medium = random.randint(31, 70)
        percent_setting_range_high = random.randint(71, 100)
        
        # *** STEP 1 ***
        # Commissioning already done
        self.step("1")

        # *** STEP 2 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the Low value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are non-zero
        #   Verify that the PercentSetting and PercentCurrent values are greater than the Off FanMode value
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("2")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kLow
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Low")
            percent_setting_low = await self.read_percent_setting(endpoint=endpoint)
            percent_current_low = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_greater(percent_setting_low, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_low, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_low, percent_setting_off, "PercentSetting value in Low FanMode must be greater than in Off FanMode.")
            asserts.assert_greater(percent_current_low, percent_current_off, "PercentCurrent value in Low FanMode must be greater than in Off FanMode.")            
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")

        # *** STEP 3 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the Medium value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are non-zero
        #   Verify that the PercentSetting and PercentCurrent values are greater than the Low FanMode values
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("3")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kMedium
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Medium")
            percent_setting_medium = await self.read_percent_setting(endpoint=endpoint)
            percent_current_medium = await self.read_percent_current(endpoint=endpoint)            
            asserts.assert_greater(percent_setting_medium, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_medium, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_medium, percent_setting_low, "PercentSetting value in Medium FanMode must be greater than in Low FanMode.")
            asserts.assert_greater(percent_current_medium, percent_current_low, "PercentCurrent value in Medium FanMode must be greater than in Low FanMode.")
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")

        # *** STEP 4 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the High value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are non-zero
        #   Verify that the PercentSetting and PercentCurrent values are less than the High FanMode values
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("4")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kHigh
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to High")
            percent_setting_high = await self.read_percent_setting(endpoint=endpoint)
            percent_current_high = await self.read_percent_current(endpoint=endpoint)            
            asserts.assert_greater(percent_setting_high, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_high, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_greater(percent_setting_high, percent_setting_medium, "PercentSetting value in High FanMode must be greater than in Medium FanMode.")
            asserts.assert_greater(percent_current_high, percent_current_medium, "PercentCurrent value in High FanMode must be greater than in Medium FanMode.")
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")

        # *** STEP 5 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the Medium value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are non-zero
        #   Verify that the PercentSetting and PercentCurrent values are less than the High FanMode values
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("5")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kMedium
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Medium")
            percent_setting_medium = await self.read_percent_setting(endpoint=endpoint)
            percent_current_medium = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_greater(percent_setting_medium, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_medium, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_less(percent_setting_medium, percent_setting_high, "PercentSetting value in Medium FanMode must be less than in High FanMode.")
            asserts.assert_less(percent_current_medium, percent_current_high, "PercentCurrent value in Medium FanMode must be less than in High FanMode.")
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")

        # *** STEP 6 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the Low value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are non-zero
        #   Verify that the PercentSetting and PercentCurrent values are less than the Medium FanMode values
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("6")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kLow
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Low")
            percent_setting_low = await self.read_percent_setting(endpoint=endpoint)
            percent_current_low = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_greater(percent_setting_low, 0, "PercentSetting value must be greater than 0.")
            asserts.assert_greater(percent_current_low, 0, "PercentCurrent value must be greater than 0.")
            asserts.assert_less(percent_setting_low, percent_setting_medium, "PercentSetting value in Low FanMode must be less than in Low FanMode.")
            asserts.assert_less(percent_current_low, percent_current_medium, "PercentCurrent value in Low FanMode must be less than in Low FanMode.")
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")

        # *** STEP 7 ***
        # The TH reads the FanMode attribute and stores it before attempting to write to it
        # Verify that the DUT response contains a FanModeEnum value
        # The TH writes the Off value to the FanMode attribute on the DUT and reads it back after a few seconds,
        # and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the FanMode read matches the FanMode written
        #   Verify that the PercentSetting and PercentCurrent values are 0
        # If INVALID_IN_STATE
        #   Verify that the FanMode matches the FanMode before write attempt
        self.step("7")
        fan_mode_before_write = await self.read_fan_mode(endpoint=endpoint)
        asserts.assert_in(fan_mode_before_write, Clusters.FanControl.Enums.FanModeEnum, "The FanMode read response does not contain a FanModeEnum value.")
        fan_mode_write = Clusters.FanControl.Enums.FanModeEnum.kOff
        write_status = await self.write_fan_mode(endpoint=endpoint, fan_mode=fan_mode_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "FanMode write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(fan_mode_read, fan_mode_write, f"FanMode is not set to Off")
            percent_setting_off = await self.read_percent_setting(endpoint=endpoint)
            percent_current_off = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_equal(percent_setting_off, 0, "PercentSetting value must be 0 when a FanMode Off is set.")
            asserts.assert_equal(percent_current_off, 0, "PercentCurrent value must be 0 when a FanMode Off is set.")
        else:
            asserts.assert_equal(fan_mode_read, fan_mode_before_write, "FanMode is not unchanged")



















        # *** STEP 8 ***
        # The TH reads the PercentSetting attribute and stores it before attempting to write to it
        # The TH writes 0 to the PercentSetting attribute on the DUT, reads it back after a few
        # seconds, and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the PercentSetting read matches the PercentSetting written
        #   Verify that the PercentCurrent read matches the PercentSetting written (after a few seconds)
        #   Verify that FanMode = Off
        # If INVALID_IN_STATE
        #   Verify that the PercentSetting read matches the PercentSetting before write attempt
        #   Verify that the PercentCurrent read matches the PercentSetting before write attempt
        self.step("8")
        percent_setting_before_write = await self.read_percent_setting(endpoint=endpoint)
        percent_setting_write = percent_setting_range_off
        write_status = await self.write_percent_setting(endpoint=endpoint, percent_setting=percent_setting_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        percent_setting_read = await self.read_percent_setting(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(percent_setting_read, percent_setting_write, f"PercentSetting not set to {percent_setting_write}")
            fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
            time.sleep(seconds_delay)
            percent_current_read = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_equal(percent_current_read, percent_setting_write, f"PercentCurrent not set to {percent_setting_write}")
            asserts.assert_equal(fan_mode_read, Clusters.FanControl.Enums.FanModeEnum.kOff, "FanMode must be Off.")
        else:
            asserts.assert_equal(percent_setting_read, percent_setting_before_write, "PercentSetting is not unchanged")
            asserts.assert_equal(percent_current_read, percent_setting_before_write, "PercentCurrent is not unchanged")

        # *** STEP 9 ***
        # The TH reads the PercentSetting attribute and stores it before attempting to write to it
        # The TH writes a value corresponding to the Low FanMode range to the PercentSetting attribute
        # on the DUT, reads it back after a few seconds, and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the PercentSetting read matches the PercentSetting written
        #   Verify that the PercentCurrent read matches the PercentSetting written (after a few seconds)
        #   Verify that FanMode = Low
        # If INVALID_IN_STATE
        #   Verify that the PercentSetting read matches the PercentSetting before write attempt
        #   Verify that the PercentCurrent read matches the PercentSetting before write attempt
        self.step("9")
        percent_setting_before_write = await self.read_percent_setting(endpoint=endpoint)
        percent_setting_write = percent_setting_range_low
        write_status = await self.write_percent_setting(endpoint=endpoint, percent_setting=percent_setting_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        percent_setting_read = await self.read_percent_setting(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(percent_setting_read, percent_setting_write, f"PercentSetting not set to {percent_setting_write}")
            fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
            time.sleep(seconds_delay)
            percent_current_read = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_equal(percent_current_read, percent_setting_write, f"PercentCurrent not set to {percent_setting_write}")
            asserts.assert_equal(fan_mode_read, Clusters.FanControl.Enums.FanModeEnum.kLow, "FanMode must be Low.")
        else:
            asserts.assert_equal(percent_setting_read, percent_setting_before_write, "PercentSetting is not unchanged")
            asserts.assert_equal(percent_current_read, percent_setting_before_write, "PercentCurrent is not unchanged")

        # *** STEP 10 ***
        # The TH reads the PercentSetting attribute and stores it before attempting to write to it
        # The TH writes a value corresponding to the Medium FanMode range to the PercentSetting attribute
        # on the DUT, reads it back after a few seconds, and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the PercentSetting read matches the PercentSetting written
        #   Verify that the PercentCurrent read matches the PercentSetting written (after a few seconds)
        #   Verify that FanMode = Medium
        # If INVALID_IN_STATE
        #   Verify that the PercentSetting read matches the PercentSetting before write attempt
        #   Verify that the PercentCurrent read matches the PercentSetting before write attempt
        self.step("10")
        percent_setting_before_write = await self.read_percent_setting(endpoint=endpoint)
        percent_setting_write = percent_setting_range_medium
        write_status = await self.write_percent_setting(endpoint=endpoint, percent_setting=percent_setting_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        percent_setting_read = await self.read_percent_setting(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(percent_setting_read, percent_setting_write, f"PercentSetting not set to {percent_setting_write}")
            fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
            time.sleep(seconds_delay)
            percent_current_read = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_equal(percent_current_read, percent_setting_write, f"PercentCurrent not set to {percent_setting_write}")
            asserts.assert_equal(fan_mode_read, Clusters.FanControl.Enums.FanModeEnum.kMedium, "FanMode must be Medium.")
        else:
            asserts.assert_equal(percent_setting_read, percent_setting_before_write, "PercentSetting is not unchanged")
            asserts.assert_equal(percent_current_read, percent_setting_before_write, "PercentCurrent is not unchanged")

        # *** STEP 11 ***
        # The TH reads the PercentSetting attribute and stores it before attempting to write to it
        # The TH writes a value corresponding to the High FanMode range to the PercentSetting attribute
        # on the DUT, reads it back after a few seconds, and stores it
        # Verify that the DUT returns either a SUCCESS or INVALID_IN_STATE status code
        # If SUCCESS
        #   Verify that the PercentSetting read matches the PercentSetting written
        #   Verify that the PercentCurrent read matches the PercentSetting written (after a few seconds)
        #   Verify that FanMode = High
        # If INVALID_IN_STATE
        #   Verify that the PercentSetting read matches the PercentSetting before write attempt
        #   Verify that the PercentCurrent read matches the PercentSetting before write attempt
        self.step("11")
        percent_setting_before_write = await self.read_percent_setting(endpoint=endpoint)
        percent_setting_write = percent_setting_range_high
        write_status = await self.write_percent_setting(endpoint=endpoint, percent_setting=percent_setting_write)
        write_status_ok = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_ok, "PercentSetting write did not return a value of Success or InvalidInState")
        time.sleep(seconds_delay)
        percent_setting_read = await self.read_percent_setting(endpoint=endpoint)
        if write_status == Status.Success:
            asserts.assert_equal(percent_setting_read, percent_setting_write, f"PercentSetting not set to {percent_setting_write}")
            fan_mode_read = await self.read_fan_mode(endpoint=endpoint)
            time.sleep(seconds_delay)
            percent_current_read = await self.read_percent_current(endpoint=endpoint)
            asserts.assert_equal(percent_current_read, percent_setting_write, f"PercentCurrent not set to {percent_setting_write}")
            asserts.assert_equal(fan_mode_read, Clusters.FanControl.Enums.FanModeEnum.kHigh, "FanMode must be High.")
        else:
            asserts.assert_equal(percent_setting_read, percent_setting_before_write, "PercentSetting is not unchanged")
            asserts.assert_equal(percent_current_read, percent_setting_before_write, "PercentCurrent is not unchanged")




if __name__ == "__main__":
    default_matter_test_main()
