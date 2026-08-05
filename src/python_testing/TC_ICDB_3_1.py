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
Verify that a LIT ICD with the Dynamic SIT/LIT feature can transition between LIT and SIT
operating modes using a vendor-specific mechanism when client registration is present.
Specifically: the DUT starts in SIT, transitions to LIT when a client registers, can be forced
back to SIT via vendor mechanism while registration persists, and returns to SIT after the
client unregisters.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-3-1

Notes/Considerations
In CI:
    The vendor-specific SIT/LIT transition is simulated via test event triggers
    kDSLSForceSitMode and kDSLSWithdrawSitMode.
    --icdActiveModeDurationMs is set to 10000ms to extend the Active Mode window.
On a real DUT:
    The vendor-specific mechanism is performed physically per device documentation.
    Use --timeout <seconds> in the script-args if test duration is large.
'''

import logging
import os

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, ICDTestEventTriggerOperations

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
modes = cluster.Enums.OperatingModeEnum
clientTypeEnum = cluster.Enums.ClientTypeEnum


class TC_ICDB_3_1(ICDBaseTest):

    def desc_TC_ICDB_3_1(self) -> str:
        return "[TC-ICDB-3.1] ICD Dynamic SIT/LIT - Verify OperatingMode transition between LIT and SIT when there is client registration [DUT as Server]"

    def steps_TC_ICDB_3_1(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH reads from the DUT the RegisteredClients attribute.",
                     "RegisteredClients is empty. If not empty, TH sends command UnregisterClient to clear all clients in RegisteredClients."),
            TestStep("1b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)."),
            TestStep("1c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)."),
            TestStep(2, "TH sends RegisterClient command.",
                     "Verify that the DUT response contains ICDCounter."),
            TestStep("3a", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=1 (LIT Operating Mode)."),
            TestStep("3b", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=1 (LIT mode)."),
            TestStep("4a", "Apply vendor specific mechanism to transition DUT to from LIT operating mode to SIT operating mode."),
            TestStep("4b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)."),
            TestStep("4c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)."),
            TestStep("5a", "Apply vendor specific mechanism to transition DUT to from SIT operating mode to LIT operating mode."),
            TestStep("5b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=1 (LIT Operating Mode)."),
            TestStep("5c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=1."),
            TestStep("6a", "TH sends UnregisterClient command with the CheckInNodeID.",
                     "UnregisterClient command completes successfully."),
            TestStep("6b", "TH reads from the DUT the OperatingMode attribute.",
                     "Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)."),
            TestStep("6c", "TH reads the ICD DNS-SD TXT key",
                     "Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)."),
        ]

    def pics_TC_ICDB_3_1(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F02",
            "ICDM.S.F03",
        ]

    @async_test_body
    async def test_TC_ICDB_3_1(self):

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
        log.info("OperatingMode: %s", operating_mode)

        # Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kSit,
                             f"OperatingMode must be SIT (0) before any client is registered. Got: {operating_mode}")

        # *** STEP 1c ***
        # TH reads the ICD DNS-SD TXT key
        self.step("1c")
        icd_value = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=0 (SIT mode)
        asserts.assert_equal(icd_value, modes.kSit,
                             f"ICD DNS-SD TXT key must show ICD=0 (SIT) before client registration. Got: ICD={icd_value}")

        # *** STEP 2 ***
        # TH sends RegisterClient command
        self.step(2)
        check_in_node_id = self.default_controller.nodeId
        monitored_subject = self.default_controller.nodeId
        key = os.urandom(16)
        try:
            cmd = commands.RegisterClient(
                checkInNodeID=check_in_node_id,
                monitoredSubject=monitored_subject,
                key=key,
                clientType=clientTypeEnum.kPermanent)
            response = await self.send_single_icdm_command(cmd)
        except InteractionModelError as e:
            asserts.assert_fail(f"Unexpected error returned when registering client: {e}")

        # Verify that the DUT response contains ICDCounter
        log.info("RegisterClient response ICDCounter: %s", response.ICDCounter)
        asserts.assert_is_not_none(response.ICDCounter, "RegisterClient response must contain ICDCounter.")

        # *** STEP 3a ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("3a")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)
        log.info("OperatingMode after RegisterClient: %s", operating_mode)

        # Verify that the DUT response contains value of OperatingMode=1 (LIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kLit,
                             f"OperatingMode must be LIT (1) after client registration. Got: {operating_mode}")

        # *** STEP 3b ***
        # Verify that mDNS is advertising ICD key
        self.step("3b")
        icd_txt = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=1
        asserts.assert_equal(icd_txt, modes.kLit,
                             f"ICD DNS-SD TXT key must show ICD=1 (LIT) after client registration. Got: ICD={icd_txt}")

        # *** STEP 4a ***
        # Apply vendor specific mechanism to transition DUT to from LIT operating mode to SIT operating mode
        self.step("4a")
        if self.is_pics_sdk_ci_only:
            await self.send_test_event_triggers(eventTrigger=ICDTestEventTriggerOperations.kDSLSForceSitMode)
        else:
            self.wait_for_user_input(
                prompt_msg="Apply the vendor-specific mechanism on the DUT to transition it from LIT to SIT operating mode, then press Enter.")

        # *** STEP 4b ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("4b")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)

        # Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kSit,
                             f"OperatingMode must be SIT (0) after vendor SIT transition. Got: {operating_mode}")

        # *** STEP 4c ***
        # Verify that the ICD DNS-SD TXT key is present
        self.step("4c")
        icd_txt = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=0
        asserts.assert_equal(icd_txt, modes.kSit,
                             f"ICD DNS-SD TXT key must show ICD=0 (SIT) after vendor SIT transition. Got: ICD={icd_txt}")

        # *** STEP 5a ***
        # Apply vendor specific mechanism to transition DUT to from SIT operating mode to LIT operating mode
        self.step("5a")
        if self.is_pics_sdk_ci_only:
            await self.send_test_event_triggers(eventTrigger=ICDTestEventTriggerOperations.kDSLSWithdrawSitMode)
        else:
            self.wait_for_user_input(
                prompt_msg="Apply the vendor-specific mechanism on the DUT to transition it from SIT to LIT operating mode, then press Enter.")

        # *** STEP 5b ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("5b")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)

        # Verify that the DUT response contains value of OperatingMode=1 (LIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kLit,
                             f"OperatingMode must be LIT (1) after vendor LIT transition. Got: {operating_mode}")

        # *** STEP 5c ***
        # Verify that the ICD DNS-SD TXT key is present
        self.step("5c")
        icd_txt = await self.get_icd_txt_key()
        log.info("ICD DNS-SD TXT key after DSLS withdraw SIT: ICD=%s", icd_txt)

        # Verify that ICD DNS-SD TXT key shows ICD=1
        asserts.assert_equal(icd_txt, modes.kLit,
                             f"ICD DNS-SD TXT key must show ICD=1 (LIT) after vendor LIT transition. Got: ICD={icd_txt}")

        # *** STEP 6a ***
        # TH sends UnregisterClient command with the CheckInNodeID
        self.step("6a")
        try:
            await self.send_single_icdm_command(commands.UnregisterClient(checkInNodeID=check_in_node_id))
        except InteractionModelError as e:
            asserts.assert_fail(f"Unexpected error returned when unregistering client: {e}")
        log.info("UnregisterClient SUCCESS for checkInNodeID=%s", check_in_node_id)

        # *** STEP 6b ***
        # TH reads from the DUT the OperatingMode attribute
        self.step("6b")
        operating_mode = await self.read_icdm_attribute_expect_success(attributes.OperatingMode)

        # Verify that the DUT response contains value of OperatingMode=0 (SIT Operating Mode)
        asserts.assert_equal(operating_mode, modes.kSit,
                             f"OperatingMode must be SIT (0) after client unregistration. Got: {operating_mode}")

        # *** STEP 6c ***
        # Verify that the ICD DNS-SD TXT key is present
        self.step("6c")
        icd_txt = await self.get_icd_txt_key()

        # Verify that ICD DNS-SD TXT key shows ICD=0
        asserts.assert_equal(icd_txt, modes.kSit,
                             f"ICD DNS-SD TXT key must show ICD=0 (SIT) after client unregistration. Got: ICD={icd_txt}")


if __name__ == "__main__":
    default_matter_test_main()
