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
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from matter_testing_support import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, default_matter_test_main,
                                    has_cluster, run_if_endpoint_matches)
from mobly import asserts
from test_plan_support import commission_if_required, read_attribute, verify_success


class TC_CC_2_3(MatterBaseTest):

    # Test includes several long waits, adjust timeout to accommodate.
    @property
    def default_timeout(self) -> int:
        return 180

    def steps_TC_CC_2_2(self):
        THcommand = "Test Harness sends the"

        def store_values(reportList: str, attr: str) -> str:
            return f"TH stores the reported values of _{attr}_ in all incoming reports for _{attr}_ attribute, that contains data in _{reportList}_, over a period of 20 seconds."

        def verify_entry_count(reportList: str, attr: str) -> str:
            return f'TH verifies that _{reportList}_ does not contain more than 12 entries for _{attr}_'

        def entry_count_verification(reportList: str) -> str:
            return f'_{reportList}_ has 12 or less entries in the list'

        return [TestStep(1, commission_if_required(), is_commissioning=True),
                TestStep(2, read_attribute('FeatureMap')),
                TestStep(3, read_attribute('AttributeList')),
                TestStep(4, read_attribute('ServerList', 'Descriptor')),
                TestStep(
                    5, f"If OnOff cluster is present in _ServerList_, {THcommand} On command on OnOff cluster", verify_success()),
                TestStep(6, 'Set up a subscription wildcard subscription for the Color Control Cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false',
                         'Subscription successfully established'),
                TestStep(7, 'If the CT feature is not supported, skip step 8 to 12'),
                TestStep(
                    8, f'{THcommand} MoveColorTemperature with _MoveMode_ field set to Down, _Rate_ field set to 65535 and remaining fields set to 0', verify_success()),
                TestStep(
                    9, 'TH reads from the DUT the ColorTempPhysicalMaxMireds and stores the returned value as colorTempPhysicalMaxMireds', verify_success()),
                TestStep(10, f'{THcommand} MoveToColorTemperature with ColorTemperatureMireds field set to the value of colorTempPhysicalMaxMireds, TransitionTime field set to 100, remaining fields set to 0', verify_success()),
                TestStep(11, store_values('reportedColorTemperatureMiredsValuesList', 'ColorTemperatureMireds')),
                TestStep(12, verify_entry_count('reportedColorTemperatureMiredsValuesList', 'ColorTemperatureMireds'),
                         entry_count_verification('reportedColorTemperatureMiredsValuesList')),
                TestStep(13, 'If the HS feature is not supported, skip step 14 to 21'),
                TestStep(
                    14, f'{THcommand} MoveHue with _MoveMode_ field set to Down, _Rate_ field set to 255 and remaining fields set to 0', verify_success()),
                TestStep(
                    15, f'{THcommand} MoveSaturation with _MoveMode_ field set to Down, _Rate_ field set to 255 and remaining fields set to 0', verify_success()),
                TestStep(16, f'{THcommand} MoveToHue with _Hue_ field set to 254, _TransitionTime_ field set to 100, _Direction_ field set to Shortest and remaining fields set to 0', verify_success()),
                TestStep(17, store_values('reportedCurrentHueValuesList', 'CurrentHue')),
                TestStep(18, verify_entry_count('reportedCurrentHueValuesList', 'CurrentHue'),
                         entry_count_verification('reportedCurrentHueValuesList')),
                TestStep(
                    19, f"{THcommand} MoveToSaturation with _Saturation_ field set to 254, _TransitionTime_ field set to 100 and remaining fields set to 0"),
                TestStep(20, store_values('reportedCurrentSaturationValuesList', 'CurrentSaturation')),
                TestStep(21, verify_entry_count('reportedCurrentSaturationValuesList',
                         'CurrentSaturation'), entry_count_verification('reportedCurrentSaturationValuesList')),
                TestStep(22, 'If XY feature is not supported, skip steps 23-28'),
                TestStep(
                    23, f"{THcommand} MoveToColor with _ColorX_ field set to 32768, _ColorY_ set to 19660, _TransitionTime_ field set to 0 and remaining fields set to 0"),
                TestStep(
                    24, f"{THcommand} MoveToColor with _ColorX_ field set to 13107, _ColorY_ set to 13107, _TransitionTime_ field set to 100 and remaining fields set to 0"),
                TestStep(25, store_values('reportedCurrentXValuesList', 'CurrentX')),
                TestStep(26, store_values('reportedCurrentYValuesList', 'CurrentY')),
                TestStep(27, verify_entry_count('reportedCurrentXValuesList', 'CurrentX'),
                         entry_count_verification('reportedCurrentXValuesList')),
                TestStep(28, verify_entry_count('reportedCurrentYValuesList', 'CurrentY'),
                         entry_count_verification('reportedCurrentYValuesList')),
                TestStep(29, "If the EHUE feature is not supported, skip steps 30 to 32"),
                TestStep(30, f"{THcommand} EnhancedMoveToHue with _EnhancedHue_ field set to 0, _TransitionTime_ field set to 100, _Direction_ field set to Shortest and remaining fields set to 0", verify_success()),
                TestStep(31, store_values('reportedEnhancedCurrentHueValuesList', 'EnhancedCurrentHue')),
                TestStep(32, verify_entry_count('reportedEnhancedCurrentHueValuesList',
                         'EnhancedCurrentHue'), entry_count_verification('reportedEnhancedCurrentHueValuesList')),
                TestStep(
                    33, 'If the RemainingTime attribute is not supported or the CT feature is not supported, skip the remaining steps and end test case'),
                TestStep(
                    34, f'{THcommand} MoveColorTemperature with MoveMode field set to Down, Rate field set to 65535 and remaining fields set to 0', verify_success()),
                TestStep(35, 'TH stores the reported values of RemainingTime in all incoming reports for RemainingTime attribute, for steps 36 to 39 that contains data in reportedRemainingTimeValuesList.'),
                TestStep(
                    36, f'{THcommand} MoveToColorTemperature with ColorTemperatureMireds field set to the value of colorTempPhysicalMaxMireds / 2, TransitionTime field set to 100, remaining fields set to 0', verify_success()),
                TestStep(37, "Wait for 5 seconds"),
                TestStep(
                    38, f'{THcommand} MoveToColorTemperature with ColorTemperatureMireds field set to the value of colorTempPhysicalMaxMireds, TransitionTime field set to 150, remaining fields set to 0', verify_success()),
                TestStep(39, "Wait for 20 seconds"),
                TestStep(40, "TH verifies _reportedRemainingTimeValuesList_ contains three entries",
                         "_reportedRemainingTimeValuesList_ has 3 entries in the list"),
                TestStep(41, "TH verifies the first entry in _reportedRemainingTimeValuesList_ is 100",
                         "The first entry in reportedRemainingTimeValuesList is in the range of 95 to 100"),
                TestStep(42, "TH verifies the second entry in _reportedRemainingTimeValuesList_ is approximately 150",
                         "The second entry in reportedRemainingTimeValuesList is in the range of 145 to 150"),
                TestStep(43, "TH verifies the third entry in _reportedRemainingTimeValuesList_ is 0",
                         "The third entry in _reportedRemainingTimeValuesList_ is equal to 0")
                ]

    @run_if_endpoint_matches(has_cluster(Clusters.ColorControl))
    async def test_TC_CC_2_2(self):
        gather_time = 20

        # commissioning - already done
        self.step(1)

        cc = Clusters.ColorControl

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(cluster=cc, attribute=cc.Attributes.FeatureMap)
        supports_ct = (feature_map & cc.Bitmaps.Feature.kColorTemperature) != 0
        supports_hs = (feature_map & cc.Bitmaps.Feature.kHueAndSaturation) != 0
        supports_xy = (feature_map & cc.Bitmaps.Feature.kXy) != 0
        supports_ehue = (feature_map & cc.Bitmaps.Feature.kEnhancedHue) != 0

        self.step(3)
        attribute_list = await self.read_single_attribute_check_success(cluster=cc, attribute=cc.Attributes.AttributeList)

        self.step(4)
        server_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(5)
        if Clusters.OnOff.id in server_list:
            cmd = Clusters.OnOff.Commands.On()
            await self.send_single_cmd(cmd)
        else:
            self.mark_current_step_skipped()

        self.step(6)
        sub_handler = ClusterAttributeChangeAccumulator(cc)
        await sub_handler.start(self.default_controller, self.dut_node_id, self.matter_test_config.endpoint)

        def accumulate_reports():
            sub_handler.reset()
            logging.info(f"Test will now wait {gather_time} seconds to accumulate reports")
            time.sleep(gather_time)

        def check_report_counts(attr: ClusterObjects.ClusterAttributeDescriptor):
            count = sub_handler.attribute_report_counts[attr]
            asserts.assert_less_equal(count, 12, "More than 12 reports received")
            asserts.assert_less_equal(count, gather_time, f"More than {gather_time} reports received")

        self.step(7)
        if not supports_ct:
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)
        else:
            self.step(8)
            cmd = cc.Commands.MoveColorTemperature(moveMode=cc.Enums.MoveModeEnum.kDown, rate=65535)
            await self.send_single_cmd(cmd)

            self.step(9)
            colorTempPhysicalMaxMireds = await self.read_single_attribute_check_success(cluster=cc, attribute=cc.Attributes.ColorTempPhysicalMaxMireds)

            self.step(10)
            cmd = cc.Commands.MoveToColorTemperature(colorTemperatureMireds=colorTempPhysicalMaxMireds, transitionTime=100)
            await self.send_single_cmd(cmd)

            self.step(11)
            accumulate_reports()

            self.step(12)
            check_report_counts(cc.Attributes.ColorTemperatureMireds)

        self.step(13)
        if not supports_hs:
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)
            self.skip_step(20)
            self.skip_step(21)
        else:
            self.step(14)
            cmd = cc.Commands.MoveHue(moveMode=cc.Enums.MoveModeEnum.kDown, rate=225)
            await self.send_single_cmd(cmd)

            self.step(15)
            cmd = cc.Commands.MoveSaturation(moveMode=cc.Enums.MoveModeEnum.kDown, rate=225)
            await self.send_single_cmd(cmd)

            self.step(16)
            cmd = cc.Commands.MoveToHue(hue=254, transitionTime=100, direction=cc.Enums.DirectionEnum.kShortest)
            await self.send_single_cmd(cmd)

            self.step(17)
            accumulate_reports()

            self.step(18)
            check_report_counts(cc.Attributes.CurrentHue)

            self.step(19)
            cmd = cc.Commands.MoveToSaturation(saturation=254, transitionTime=100)
            await self.send_single_cmd(cmd)

            self.step(20)
            accumulate_reports()

            self.step(21)
            check_report_counts(cc.Attributes.CurrentSaturation)

        self.step(22)
        if not supports_xy:
            self.skip_step(23)
            self.skip_step(24)
            self.skip_step(25)
            self.skip_step(26)
            self.skip_step(27)
            self.skip_step(28)
        else:
            self.step(23)
            cmd = cc.Commands.MoveToColor(colorX=32768, colorY=19660, transitionTime=0)
            await self.send_single_cmd(cmd)

            self.step(24)
            cmd = cc.Commands.MoveToColor(colorX=13107, colorY=13107, transitionTime=100)
            await self.send_single_cmd(cmd)

            self.step(25)
            accumulate_reports()

            self.step(26)
            # reports for x and y are both accumulated in a dict - done above

            self.step(27)
            check_report_counts(cc.Attributes.CurrentX)

            self.step(28)
            check_report_counts(cc.Attributes.CurrentY)

        self.step(29)
        if not supports_ehue:
            self.skip_step(30)
            self.skip_step(31)
            self.skip_step(32)
        else:
            self.step(30)
            cmd = cc.Commands.EnhancedMoveToHue(enhancedHue=0, transitionTime=100,
                                                direction=cc.Enums.DirectionEnum.kShortest)
            await self.send_single_cmd(cmd)

            self.step(31)
            accumulate_reports()

            self.step(32)
            check_report_counts(cc.Attributes.EnhancedCurrentHue)

        self.step(33)
        if cc.Attributes.RemainingTime.attribute_id not in attribute_list or not supports_ct:
            self.skip_all_remaining_steps(34)
            return

        self.step(34)
        cmd = cc.Commands.MoveColorTemperature(moveMode=cc.Enums.MoveModeEnum.kDown, rate=65535)
        await self.send_single_cmd(cmd)

        self.step(35)
        accumulate_reports()

        self.step(36)
        cmd = cc.Commands.MoveToColorTemperature(colorTemperatureMireds=colorTempPhysicalMaxMireds/2, transitionTime=100)
        await self.send_single_cmd(cmd)

        self.step(37)
        logging.info("Test will now wait for 5 seconds")
        time.sleep(5)

        self.step(38)
        cmd = cc.Commands.MoveToColorTemperature(colorTemperatureMireds=colorTempPhysicalMaxMireds, transitionTime=150)
        await self.send_single_cmd(cmd)

        self.step(39)
        logging.info("Test will now wait for 20 seconds")
        time.sleep(20)

        self.step(40)
        logging.info(f'received reports: {sub_handler.attribute_reports[cc.Attributes.RemainingTime]}')
        count = sub_handler.attribute_report_counts[cc.Attributes.RemainingTime]
        asserts.assert_equal(count, 3, "Unexpected number of reports received")

        self.step(41)
        asserts.assert_less_equal(
            sub_handler.attribute_reports[cc.Attributes.RemainingTime][0].value, 100, "Unexpected first report")
        asserts.assert_almost_equal(
            sub_handler.attribute_reports[cc.Attributes.RemainingTime][0].value, 100, delta=10, msg="Unexpected first report")

        self.step(42)
        asserts.assert_less_equal(
            sub_handler.attribute_reports[cc.Attributes.RemainingTime][1].value, 150, "Unexpected second report")
        asserts.assert_almost_equal(
            sub_handler.attribute_reports[cc.Attributes.RemainingTime][1].value, 150, delta=10, msg="Unexpected second report")

        self.step(43)
        asserts.assert_equal(sub_handler.attribute_reports[cc.Attributes.RemainingTime][-1].value, 0, "Unexpected last report")


if __name__ == "__main__":
    default_matter_test_main()
