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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.clusters.Types import NullValue
from TC_HSTAT_Test_Base import HSTATBase

log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-HSTAT-2.5] Mist functionality with DUT as Server


class TC_HSTAT_2_5(MatterBaseTest, HSTATBase):

    def pics_TC_HSTAT_2_5(self) -> list[str]:
        return [
            "HSTAT.S",
            "HSTAT.S.F00" # Supports the Humidifier feature
        ]

    def desc_TC_HSTAT_2_5(self) -> str:
        return "[TC-HSTAT-2.5] Mist functionality with DUT as Server"

    def steps_TC_HSTAT_2_5(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Humidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with only the MistWarm bit of the MistType field set",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains a value with the MistWarm bit set. Store the value as MistSetting"),
            TestStep(6, "TH writes to the DUT the MistType attribute with only the MistCold bit set",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains a value with the MistCold bit set."),
            TestStep(8, "Individually subscribe to the MistType attribute",
                     "This will receive updates when these attributes change value."),
            TestStep(9, "TH writes to the DUT the MistType attribute with only the MistWarm bit of the MistType field set",
                     "Verify that an attribute report was received for MistType and that the value received has only the MistWarm bit set."),
            TestStep(10, "TH sends command SetSettings with the MistType field set to MistWarm",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(11, "TH sends command SetSettings with the MistType field set to MistCold",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(12, "TH sends command Off to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(13, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains the NULL value."),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_5(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        await self.setup()

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_on_cmd_expect_success()

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Humidifier
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(mode=self.modeHumidifier)

        if self.warmFeatureSupported:
            self.step(4)
            # TH sends command SetSettings with only the MistWarm bit of the MistType field set
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(mistType=self.mistBitmap.kMistWarm)

            self.step(5)
            # TH reads from the DUT the MistType attribute.
            # Verify that the DUT response contains a value with the MistWarm bit set. Store the value as MistSetting
            dut_MistType = await self.read_attribute_expect_success(attribute=self.attributes.MistType)
            asserts.assert_equal(dut_MistType, self.mistBitmap.kMistWarm, "MistType is not MistWarm as expected")
        else:
            self.mark_step_range_skipped(4, 5)

        if self.coldFeatureSupported:
            self.step(6)
            # TH writes to the DUT the MistType attribute with only the MistCold bit set
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(mistType=self.mistBitmap.kMistCold)

            self.step(7)
            # TH reads from the DUT the MistType attribute.
            # Verify that the DUT response contains a value with the MistCold bit set.
            dut_MistType = await self.read_attribute_expect_success(attribute=self.attributes.MistType)
            asserts.assert_equal(dut_MistType, self.mistBitmap.kMistCold, "MistType is not MistCold as expected")
        else:
            self.mark_step_range_skipped(6, 7)

        if self.warmFeatureSupported and self.coldFeatureSupported:
            self.step(8)
            # Individually subscribe to the MistType attribute
            # This will receive updates when these attributes change value.
            mistTypeSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.MistType)
            await mistTypeSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)

            self.step(9)
            # TH writes to the DUT the MistType attribute with only the MistWarm bit of the MistType field set
            # Verify that an attribute report was received for MistType and that the value received has only the MistWarm bit set.
            await self.write_attribute_expect_success(attribute=self.attributes.MistType(self.mistBitmap.kMistWarm))
            mistTypeReportValue = mistTypeSubscription.wait_for_attribute_report().value
            asserts.assert_equal(mistTypeReportValue, self.mistBitmap.kMistWarm, "MistType report is not MistWarm as expected")
        else:
            self.mark_step_range_skipped(8, 9)

        if not self.warmFeatureSupported:
            self.step(10)
            # TH sends command SetSettings with the MistType field set to MistWarm
            # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
            await self.send_SetSettingsCommand_expect_error(mistType=self.mistBitmap.kMistWarm, error=Status.CONSTRAINT_ERROR)
        else:
            self.skip_step(10)

        if not self.coldFeatureSupported:
            self.step(11)
            # TH sends command SetSettings with the MistType field set to MistCold
            # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
            await self.send_SetSettingsCommand_expect_error(mistType=self.mistBitmap.kMistCold, error=Status.CONSTRAINT_ERROR)
        else:
            self.skip_step(11)

        self.step(12)
        # TH sends command Off to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_off_cmd_expect_success()

        self.step(13)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains the NULL value.
        dut_MistType = await self.read_attribute_expect_success(attribute=self.attributes.MistType)
        asserts.assert_equal(dut_MistType, NullValue, "MistType is not NULL as expected")

if __name__ == '__main__':
    default_matter_test_main()
