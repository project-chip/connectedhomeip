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
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_MICROWAVE_OVEN_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --endpoint endpoint


class TC_MWOCTRL_2_4(MatterBaseTest):

    async def read_mwoctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_MWOCTRL_2_4(self) -> str:
        return "[TC-MWOCTRL-2.4] WATTS functionality with DUT as Server"

    def steps_TC_MWOCTRL_2_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the SupportedWatts attribute"),
            TestStep(3, "Read the SelectedWattIndex attribute"),
            TestStep(4, "Send the SetCookingParameters command"),
            TestStep(5, "Read and verify the SelectedWattIndex attribute"),
        ]
        return steps

    def pics_TC_MWOCTRL_2_4(self) -> list[str]:
        pics = [
            "MWOCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOCTRL_2_4(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.MicrowaveOvenControl.Attributes
        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        features = Clusters.MicrowaveOvenControl.Bitmaps.Feature
        commands = Clusters.Objects.MicrowaveOvenControl.Commands

        feature_map = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        only_watts_supported = feature_map == features.kPowerInWatts

        if not only_watts_supported:
            logging.info("PowerInWatts is not supported so skipping the rest of the tests.")
            self.skip_all_remaining_steps(2)
            return

        self.step(2)
        supportedWattsList = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedWatts)
        asserts.assert_true(len(supportedWattsList) > 0, "SupportedWatts list is empty")

        self.step(3)
        selectedWattIndex = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.SelectedWattIndex)
        logging.info("SelectedWattIndex is %s" % selectedWattIndex)
        asserts.assert_true(selectedWattIndex >= 0 and selectedWattIndex < len(
            supportedWattsList), "SelectedWattIndex is out of range")

        self.step(4)
        newWattIndex = (selectedWattIndex+1) % (len(supportedWattsList)-1)
        try:
            await self.send_single_cmd(cmd=commands.SetCookingParameters(wattSettingIndex=newWattIndex), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        selectedWattIndex = await self.read_mwoctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.SelectedWattIndex)
        asserts.assert_true(selectedWattIndex == newWattIndex, "SelectedWattIndex was not correctly set")


if __name__ == "__main__":
    default_matter_test_main()
