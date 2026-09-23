#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app: ${LIT_ICD_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --KVS kvs1
#       --trace-to json:${TRACE_TEST_JSON}-app.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --icdActiveModeDurationMs 10000
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

'''
Purpose
Verify that a LIT ICD with the Dynamic SIT/LIT feature remains in SIT operating mode when
no client registration is present, even when a vendor-specific mechanism attempts to trigger
a transition to LIT operating mode.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-3-2

Notes/Considerations
In CI:
    The vendor-specific LIT transition attempt is simulated via the test event trigger
    kDSLSWithdrawSitMode. With no clients registered, the DUT must remain in SIT.
    --icdActiveModeDurationMs is set to 10000ms to extend the Active Mode window.
On a real DUT:
    The vendor-specific mechanism is performed physically per device documentation.
    Use --timeout <seconds> in the script-args if test duration is large.
'''

import logging

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, ICDTestEventTriggerOperations

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
modes = cluster.Enums.OperatingModeEnum


class TC_ICDB_3_2(ICDBaseTest):

    def desc_TC_ICDB_3_2(self) -> str:
        return "[TC-ICDB-3.2] ICD Dynamic SIT/LIT - Verify OperatingMode does not transition between LIT and SIT when there is no client registration [DUT as Server]"

    def steps_TC_ICDB_3_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH reads from the DUT the RegisteredClients attribute.",
                     "RegisteredClients is empty. If not empty, TH sends command UnregisterClient to clear all clients in RegisteredClients."),
            TestStep("1b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)."),
            TestStep("1c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)."),
            TestStep("2a", "Apply vendor specific mechanism to transition DUT from SIT operating mode to LIT operating mode."),
            TestStep("2b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)."),
            TestStep("2c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)."),
        ]

    def pics_TC_ICDB_3_2(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F02",
            "ICDM.S.F03",
        ]

    @async_test_body
    async def test_TC_ICDB_3_2(self):

        # *** PRECONDITION ***
        # Commissioning, already done
        self.step("precondition")

        # *** STEP 1a ***
        # TH reads from the DUT the RegisteredClients attribute
        # If clients are registered, unregister them
        self.step("1a")
        await self.unregister_all_clients()

        # *** STEP 1b ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("1b")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)

        # Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kSit,
                             f"OperatingMode must be SIT (0) with no clients registered. Got: {operating_mode}")

        # *** STEP 1c ***
        # TH reads the ICD DNS-SD TXT key
        self.step("1c")
        icd_value = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)
        asserts.assert_equal(icd_value, modes.kSit,
                             f"ICD DNS-SD TXT key must show ICD=0 (SIT) with no clients registered. Got: ICD={icd_value}")

        # *** STEP 2a ***
        # Apply vendor specific mechanism to transition DUT from SIT to LIT operating mode
        self.step("2a")
        if self.is_pics_sdk_ci_only:
            await self.send_test_event_triggers(eventTrigger=ICDTestEventTriggerOperations.kDSLSWithdrawSitMode)
        else:
            self.wait_for_user_input(
                prompt_msg="Apply the vendor-specific mechanism on the DUT to attempt a transition from SIT to LIT operating mode, then press Enter.")

        # *** STEP 2b ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("2b")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)

        # Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kSit,
                             f"OperatingMode must remain SIT (0) when no clients are registered. Got: {operating_mode}")

        # *** STEP 2c ***
        # TH reads the ICD DNS-SD TXT key
        self.step("2c")
        icd_txt = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=0
        asserts.assert_equal(icd_txt, modes.kSit,
                             f"ICD DNS-SD TXT key must remain ICD=0 (SIT) when no clients are registered. Got: ICD={icd_txt}")


if __name__ == "__main__":
    default_matter_test_main()
