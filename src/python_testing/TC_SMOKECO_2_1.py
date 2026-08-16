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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS /tmp/kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts
from support_modules.smokeco_support import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_1(SmokeCoBaseTest):

    def desc_TC_SMOKECO_2_1(self) -> str:
        return "[TC-SMOKECO-2.1] Attributes with DUT as Server"

    def steps_TC_SMOKECO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned to TH.", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the ExpressedState attribute.",
                     "Verify that the DUT response contains a value between 0 and 9"),
            TestStep(3, "TH reads from the DUT the SmokeState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(4, "TH reads from the DUT the COState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(5, "TH reads from the DUT the BatteryAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(6, "TH reads from the DUT the DeviceMuted attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(7, "TH reads from the DUT the TestInProgress attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(8, "TH reads from the DUT the HardwareFaultAlert attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(9, "TH reads from the DUT the EndOfServiceAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(10, "TH reads from the DUT the InterconnectSmokeAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(11, "TH reads from the DUT the InterconnectCOAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(12, "TH reads from the DUT the ContaminationState attribute.",
                     "Verify that the DUT response contains a value between 0 and 3"),
            TestStep(13, "TH reads from the DUT the SmokeSensitivityLevel attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(14, "TH reads from the DUT the ExpiryDate attribute.", "Verify that the DUT response contains epoch-s format"),
            TestStep(15, "TH reads from the DUT the Unmounted attribute.", "Verify that the DUT response contains a bool value"),
        ]

    def pics_TC_SMOKECO_2_1(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SmokeCoAlarm))
    async def test_TC_SMOKECO_2_1(self):

        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.ExpressedState, enum=self.smokeco_enums.ExpressedStateEnum)

        self.step(3)
        if await self.feature_guard(endpoint=self.get_endpoint(), cluster=Clusters.SmokeCoAlarm, feature_int=Clusters.SmokeCoAlarm.Bitmaps.Feature.kSmokeAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.SmokeState, enum=self.smokeco_enums.AlarmStateEnum)
        else:
            self.skip_step(3)

        self.step(4)
        if await self.feature_guard(endpoint=self.get_endpoint(), cluster=Clusters.SmokeCoAlarm, feature_int=Clusters.SmokeCoAlarm.Bitmaps.Feature.kCoAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.COState, enum=self.smokeco_enums.AlarmStateEnum)
        else:
            self.skip_step(4)

        self.step(5)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.BatteryAlert, enum=self.smokeco_enums.AlarmStateEnum)

        self.step(6)
        if await self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.DeviceMuted):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.DeviceMuted, enum=self.smokeco_enums.MuteStateEnum)

        self.step(7)
        await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.TestInProgress)

        self.step(8)
        await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.HardwareFaultAlert)

        self.step(9)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.EndOfServiceAlert, self.smokeco_enums.EndOfServiceEnum)

        self.step(10)
        if await self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.InterconnectSmokeAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.InterconnectSmokeAlarm, enum=self.smokeco_enums.AlarmStateEnum)

        self.step(11)
        if await self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.InterconnectCOAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.InterconnectCOAlarm, enum=self.smokeco_enums.AlarmStateEnum)

        if await self.feature_guard(endpoint=self.get_endpoint(), cluster=Clusters.SmokeCoAlarm, feature_int=Clusters.SmokeCoAlarm.Bitmaps.Feature.kSmokeAlarm):
            self.step(12)
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.ContaminationState, enum=self.smokeco_enums.ContaminationStateEnum)
        else:
            self.skip_step(12)

        self.step(13)
        if await self.feature_guard(endpoint=self.get_endpoint(), cluster=Clusters.SmokeCoAlarm, feature_int=Clusters.SmokeCoAlarm.Bitmaps.Feature.kSmokeAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.SmokeSensitivityLevel, enum=self.smokeco_enums.AlarmStateEnum)
        else:
            self.skip_step(13)

        self.step(14)
        if await self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.ExpiryDate):
            expiry_date = await self.read_attribute_check_epoch(self.smokeco_cluster.Attributes.ExpiryDate, check_expired=True)
            # Verify when is not running on CI the ExpiryDate date is no the default as default_ci_expiry_date
            if not self.is_pics_sdk_ci_only:
                asserts.assert_not_equal(expiry_date, self.default_ci_expiry_date,
                                         f"The current ExpiryDate is the same as the default ExpiryDate ({self.default_ci_expiry_date}).")

        self.step(15)
        if await self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.Unmounted):
            await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.Unmounted)


if __name__ == "__main__":
    default_matter_test_main()
