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
import time

import chip.clusters as Clusters
import test_plan_support
from matter_testing_support import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, default_matter_test_main,
                                    has_cluster, per_endpoint_test)
from mobly import asserts


class TC_LVL_2_3(MatterBaseTest):

    def steps_TC_LVL_2_3(self) -> list[TestStep]:
        THRead = "TH reads"
        THcommand = "TH sends the command"
        return [TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
                TestStep(2, f"{THRead} FeatureMap attribute."),
                TestStep(3, f"{THRead} MaxLevel attribute and store value as maxLevel", test_plan_support.verify_success()),
                TestStep(4, f"{THcommand} MoveWithOnOff with MoveMode field set to Down and remaining fields set to 0",
                         test_plan_support.verify_success()),
                TestStep(5, f"{THRead} CurrentLevel attribute and store value as startCurrentLevel",
                         test_plan_support.verify_success()),
                TestStep(6, "Set up a subscription wildcard subscription for the Level Control Cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false",
                         "Subscription successfully established"),
                TestStep(7, f"{THcommand} MoveToLevel with Level field set to maxLevel, TransitionTime field set to 100 (10s) and remaining fields set to 0",
                         test_plan_support.verify_success()),
                TestStep(8, "TH stores the reported values of CurrentLevel in all incoming reports for CurrentLevel attribute, that contains data in reportedCurrentLevelValuesList, over a period of 30 seconds."),
                TestStep(9, "TH verifies that reportedCurrentLevelValuesList does not contain more than 10 entries for CurrentLevel",
                         "reportedCurrentLevelValuesList has 10 or less entries in the list"),
                TestStep(10, "If reportedCurrentLevelValuesList only contain a single entry, TH verifies the value of the entry is equal to maxLevel",
                         "The entry in reportedCurrentLevelValuesList is equal to maxLevel"),
                TestStep(11, "If reportedCurrentLevelValuesList contains two or more entries, TH verifies the value of the first entry is larger than startCurrentLevel",
                         "The first entry in reportedCurrentLevelValuesList is equal to or larger than to startCurrentLevel"),
                TestStep(12, "If reportedCurrentLevelValuesList contains two or more entries, TH verifies the value of the last entry is equal to maxLevel",
                         "The last entry in reportedCurrentLevelValuesList is equal to maxLevel"),
                TestStep(13, "If the LT feature is not supported, skip remaining steps and end test case"),
                # 14 is missing in the test plan
                TestStep(15, "TH stores the reported values of RemainingTime in all incoming reports for RemainingTime attribute, that contains data in reportedRemainingTimeValuesList."),
                TestStep(16, f"	{THcommand} MoveToLevel with Level field set to startCurrentLevel, TransitionTime field set to 100 (10s) and remaining fields set to 0",
                         test_plan_support.verify_success()),
                TestStep(17, "Wait for 5 seconds"),
                TestStep(18, f"{THcommand} MoveToLevel with Level field set to startCurrentLevel, TransitionTime field set to 150 (15s) and remaining fields set to 0",
                         test_plan_support.verify_success()),
                TestStep(19, "Wait for 20 seconds"),
                TestStep(20, "TH verifies reportedRemainingTimeValuesList contains three entries",
                         "reportedRemainingTimeValuesList has 3 entries in the list"),
                TestStep(21, "TH verifies the first entry in reportedRemainingTimeValuesList is approximately 100 (10s)",
                         "The first entry in reportedRemainingTimeValuesList is approximately equal to 100"),
                TestStep(22, "TH verifies the second entry in reportedRemainingTimeValuesList is approximately 150",
                         "The second entry in reportedRemainingTimeValuesList is approximately equal to 150"),
                TestStep(23, "TH verifies the third entry in reportedRemainingTimeValuesList is 0",
                         "The third entry in reportedRemainingTimeValuesList is equal to 0")
                ]

    @per_endpoint_test(has_cluster(Clusters.LevelControl))
    async def test_TC_LVL_2_3(self):
        # Commissioning - already done
        self.step(1)

        lvl = Clusters.LevelControl

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(cluster=lvl, attribute=lvl.Attributes.FeatureMap)

        self.step(3)
        max_level = await self.read_single_attribute_check_success(cluster=lvl, attribute=lvl.Attributes.MaxLevel)

        self.step(4)
        cmd = Clusters.LevelControl.Commands.MoveWithOnOff(moveMode=lvl.Enums.MoveModeEnum.kDown)
        await self.send_single_cmd(cmd)
        # NOTE: added this sleep to let the DUT have some time to move
        logging.info("Test waits for 5 seconds")
        time.sleep(5)

        self.step(5)
        start_current_level = await self.read_single_attribute_check_success(cluster=lvl, attribute=lvl.Attributes.CurrentLevel)

        self.step(6)
        sub_handler = ClusterAttributeChangeAccumulator(lvl)
        await sub_handler.start(self.default_controller, self.dut_node_id, self.matter_test_config.endpoint)

        self.step(7)
        # NOTE: had to use the WithOnOff version of this command because the dut is off at this point thanks to the above command
        # TODO: Need to check above and here that the on/off cluster is actually implemented.
        cmd = lvl.Commands.MoveToLevelWithOnOff(level=max_level, transitionTime=100, optionsMask=0, optionsOverride=0)
        await self.send_single_cmd(cmd)

        self.step(8)
        logging.info('Test will now collect data for 30 seconds')
        time.sleep(30)

        self.step(9)
        count = sub_handler.attribute_report_counts[lvl.Attributes.CurrentLevel]
        asserts.assert_less_equal(count, 10, "Received more than 10 reports for CurrentLevel")
        asserts.assert_greater(count, 0, "Did not receive any reports for CurrentLevel")

        self.step(10)
        if count == 1:
            entry = sub_handler.attribute_reports[lvl.Attributes.CurrentLevel][-1]
            asserts.assert_equal(entry.value, max_level, "Entry is not equal to max level")
        else:
            self.mark_current_step_skipped()

        if count > 1:
            self.step(11)
            last_value = start_current_level
            for e in sub_handler.attribute_reports[lvl.Attributes.CurrentLevel]:
                asserts.assert_greater_equal(e.value, last_value, "Values are not increasing")

            self.step(12)
            asserts.assert_equal(e.value, max_level, "Last entry is not max value")
        else:
            self.skip_step(11)
            self.skip_step(12)

        self.step(13)
        if (lvl.Bitmaps.Feature.kLighting & feature_map) == 0:
            self.skip_all_remaining_steps(15)

        self.step(15)
        # reports are stored by the handler, so just reset so we get a clean look
        sub_handler.reset()

        self.step(16)
        cmd = Clusters.LevelControl.Commands.MoveToLevel(
            level=start_current_level, transitionTime=100, optionsMask=0, optionsOverride=0)
        await self.send_single_cmd(cmd)

        self.step(17)
        logging.info("Test waits for 5 seconds")
        time.sleep(5)

        self.step(18)
        cmd = Clusters.LevelControl.Commands.MoveToLevel(
            level=start_current_level, transitionTime=150, optionsMask=0, optionsOverride=0)
        await self.send_single_cmd(cmd)

        self.step(19)
        logging.info("Test waits for 20 seconds")
        time.sleep(20)

        self.step(20)
        count = sub_handler.attribute_report_counts[lvl.Attributes.RemainingTime]
        asserts.assert_equal(count, 3, "Unexpected number of remaining time reports")

        self.step(21)
        remaining_time = sub_handler.attribute_reports[lvl.Attributes.RemainingTime]
        logging.info(f'Reamining time reports: {remaining_time}')
        asserts.assert_almost_equal(remaining_time[0].value, 100, delta=10, msg="Unexpected first RemainingTime report")

        self.step(22)
        asserts.assert_almost_equal(remaining_time[1].value, 150, delta=10, msg="Unexpected second RemainingTime report")

        self.step(23)
        asserts.assert_equal(remaining_time[2].value, 0, "Unexpected last RemainingTime report")


if __name__ == "__main__":
    default_matter_test_main()
