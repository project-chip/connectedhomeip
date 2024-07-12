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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time

from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EPM_2_2(MatterBaseTest, EnergyReportingBaseTestHelper):

    def desc_TC_EPM_2_2(self) -> str:
        """Returns a description of this test"""
        return "5.1.3. [TC-EPM-2.2] Primary functionality with DUT as Server"

    def pics_TC_EPM_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["EPM.S"]

    def steps_TC_EPM_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EPM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EPM.TEST_EVENT_TRIGGER for Start Fake Load Test 1kW Event"),
            TestStep("4", "Wait 3 seconds"),
            TestStep("4a", "TH reads from the DUT the ActivePower attribute",
                     "Verify the read is successful and that the value is between 980'000 and 1'020'000 mW. Note the value read."),
            TestStep("4b", "TH reads from the DUT the ActiveCurrent attribute",
                     "Verify the read is successful and that the value is between 3'848 and 4'848 mA. Note the value read."),
            TestStep("4c", "TH reads from the DUT the Voltage attribute",
                     "Verify the read is successful and that the value is between 229'000 and 231'000 mV. Note the value read."),
            TestStep("5", "Wait 3 seconds"),
            TestStep("5a", "TH reads from the DUT the ActivePower attribute",
                     "Verify the read is successful, that the value is between '980'000 and 1'020'000 mW, and the value is different from the value read in step 4a."),
            TestStep("5b", "TH reads from the DUT the ActiveCurrent attribute",
                     "Verify the read is successful, that the value is between 3'848 and 4'848 mA, and the value is different from the value read in step 4b."),
            TestStep("5c", "TH reads from the DUT the Voltage attribute",
                     "Verify the read is successful, that the value is between 229'000 and 231'000 mV, and the value is different from the value read in step 4c."),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EPM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EPM.TEST_EVENT_TRIGGER for Stop Fake Readings Test Event."),
        ]

        return steps

    @async_test_body
    async def test_TC_EPM_2_2(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_start_fake_1kw_load_2s()

        # After 3 seconds...
        self.step("4")
        time.sleep(3)

        self.step("4a")
        # Active power is Mandatory
        # 1kW +/- 20W
        active_power = await self.check_epm_attribute_in_range("ActivePower", 980000, 1020000)

        self.step("4b")
        if self.pics_guard(self.check_pics("EPM.S.A0005")):
            # 4.348 A +/- 500mA
            active_current = await self.check_epm_attribute_in_range("ActiveCurrent", 3848, 4848)

        self.step("4c")
        if self.pics_guard(self.check_pics("EPM.S.A0004")):
            # 230V +/- 1V
            voltage = await self.check_epm_attribute_in_range("Voltage", 229000, 231000)

        self.step("5")
        # After 3 seconds...
        time.sleep(3)

        self.step("5a")
        # Active power is Mandatory
        # 1kW +/- 20W
        active_power2 = await self.check_epm_attribute_in_range("ActivePower", 980000, 1020000)
        asserts.assert_not_equal(active_power, active_power2,
                                 f"Expected ActivePower readings to have changed {active_power}, {active_power2}")

        self.step("5b")
        if self.pics_guard(self.check_pics("EPM.S.A0005")):
            # 4.348 A +/- 500mA
            active_current2 = await self.check_epm_attribute_in_range("ActiveCurrent", 3848, 4848)
            asserts.assert_not_equal(active_current, active_current2,
                                     f"Expected ActiveCurrent readings to have changed {active_current}, {active_current2}")

        self.step("5c")
        if self.pics_guard(self.check_pics("EPM.S.A0004")):
            # 230V +/- 1V
            voltage2 = await self.check_epm_attribute_in_range("Voltage", 229000, 231000)
            asserts.assert_not_equal(
                voltage, voltage2, f"Expected Voltage readings to have changed {voltage}, {voltage2}")

        self.step("6")
        await self.send_test_event_trigger_stop_fake_readings()


if __name__ == "__main__":
    default_matter_test_main()
