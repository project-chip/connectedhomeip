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

# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto

import time

from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper


class TC_EEM_2_4(MatterBaseTest, EnergyReportingBaseTestHelper):

    def desc_TC_EEM_2_4(self) -> str:
        """Returns a description of this test"""
        return "5.1.5. [TC-EEM-2.4] Optional periodic imported energy attributes with DUT as Server"

    def pics_TC_EEM_2_4(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEM.S", "EEM.S.F03", "EEM.S.F00"]

    def steps_TC_EEM_2_4(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEM.TEST_EVENT_TRIGGER for Start Fake Load Test 1kW Event"),
            TestStep("4", "Wait 3 seconds"),
            TestStep("4a", "TH reads from the DUT the PeriodicEnergyImported attribute. Verify the read is successful and note the value read."),
            TestStep("5", "Wait 3 seconds"),
            TestStep("5a", "TH reads from the DUT the PeriodicEnergyImported attribute. Verify the read is successful and that the value read has to be different from value measure in step 4a."),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEM.TEST_EVENT_TRIGGER for Stop Fake Readings Test Event."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEM_2_4(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_start_fake_1kw_load_2s()

        self.step("4")
        time.sleep(3)

        self.step("4a")
        periodic_energy_imported = await self.read_eem_attribute_expect_success("PeriodicEnergyImported")

        self.step("5")
        time.sleep(3)

        self.step("5a")
        periodic_energy_imported_2 = await self.read_eem_attribute_expect_success("PeriodicEnergyImported")
        asserts.assert_not_equal(periodic_energy_imported_2.energy, periodic_energy_imported.energy,
                                 f"Expected different periodic energy readings {periodic_energy_imported_2.energy} to be != {periodic_energy_imported.energy}")

        self.step("6")
        await self.send_test_event_trigger_stop_fake_readings()


if __name__ == "__main__":
    default_matter_test_main()
