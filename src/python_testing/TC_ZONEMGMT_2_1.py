#
#    Copyright (c) 2025 Project CHIP Authors
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
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ZONEMGMT_2_1(MatterBaseTest):
    def desc_TC_ZONEMGMT_2_1(self) -> str:
        return "[TC-ZONEMGMT-2.1] Attributes with Server as DUT"

    def pics_TC_ZONEMGMT_2_1(self):
        return ["ZONEMGMT.S"]

    def steps_TC_ZONEMGMT_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads MaxUserDefinedZones attribute.",
                     "If DUT supports UserDefined feature, verify that the DUT response contains a value greater than or equal to 5."),
            TestStep(3, "TH reads MaxZones attribute.",
                     "If DUT supports UserDefined feature, verify that the DUT response contains a value greater than or equal to MaxUserDefinedZones ."),
            TestStep(
                4, "TH reads Zones attribute.", "Verify that the list has between a range of 0 to MaxZones."
            ),
            TestStep(5, "TH reads Triggers attribute.",
                     "Verify that the list has between a range of 0 to MaxZones."),
            TestStep(6, "TH reads SensitivityMax attribute.",
                     "Verify that the DUT response contains a value has between a range of 2 to 10."),
            TestStep(7, "TH reads Sensitivity attribute.",
                     "Verify that the DUT response contains a value has between a range of 1 to SensitivityMax.",
                     ),
            TestStep(8, "TH reads TwoDCartesianMax attribute.",
                     "Verify that the DUT response contains value of SensorWidth - 1 and SensorHeight - 1 from the VideoSensorParams attribute (in Camera AV Stream Management Cluster)."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.ZoneManagement) and
                             has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_ZONEMGMT_2_1(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.ZoneManagement
        attr = Clusters.ZoneManagement.Attributes

        self.step(1)
        # Commission DUT - already done
        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.twoDCartSupported = aFeatureMap & cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone
        self.userDefinedSupported = aFeatureMap & cluster.Bitmaps.Feature.kUserDefined
        self.perZoneSenseSupported = aFeatureMap & cluster.Bitmaps.Feature.kPerZoneSensitivity

        if self.userDefinedSupported:
            self.step(2)
            maxUserDefinedZones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxUserDefinedZones
            )
            log.info(f"Rx'd MaxUserDefinedZones: {maxUserDefinedZones}")
            asserts.assert_true(maxUserDefinedZones >= 5,
                                "Expected Max value of UserDefinedZones to be least 5")
        else:
            log.info("UserDefinedZones Feature not supported. Test steps skipped")
            self.skip_step(2)

        self.step(3)
        maxZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MaxZones
        )
        log.info(f"Rx'd MaxZones: {maxZones}")
        zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Zones
        )
        log.info(f"Rx'd Zones: {zones}")
        if self.userDefinedSupported:
            asserts.assert_true(maxZones >= maxUserDefinedZones,
                                "Expected Max value of Zones should be >= UserDefinedZones")
            mfgCount = 0
            for zone in zones:
                if zone.zoneSource == Clusters.ZoneManagement.Enums.ZoneSourceEnum.kMfg:
                    mfgCount += 1
            asserts.assert_equal(mfgCount, (maxZones - maxUserDefinedZones),
                                 "Mfg zones + MaxUserDefinedZones count is expected to be equal to MaxZones count")
        else:
            asserts.assert_true(maxZones >= 1,
                                "Expected Max value of Zones should be >= 1")

        self.step(4)
        asserts.assert_true(len(zones) <= maxZones, f"The number of zones in the list should at most be {maxZones}")

        self.step(5)
        triggers = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.Triggers
        )
        log.info(f"Rx'd Triggers: {triggers}")
        asserts.assert_true(len(triggers) <= maxZones, f"The number of triggers in the list should at most be {maxZones}")

        self.step(6)
        sensitivityMax = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SensitivityMax
        )
        log.info(f"Rx'd SensitivityMax: {sensitivityMax}")
        asserts.assert_true(sensitivityMax >= 2 and sensitivityMax <= 10, "SensitivityMax should be between 2 and 10")

        if not self.perZoneSenseSupported:
            self.step(7)
            sensitivity = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Sensitivity
            )
            log.info(f"Rx'd Sensitivity: {sensitivity}")
            asserts.assert_true(sensitivity >= 1 and sensitivity <= sensitivityMax,
                                f"Sensitivity should be between 1 and {sensitivityMax}")
        else:
            log.info("PerZoneSensitivity Feature supported. Test steps skipped")
            self.skip_step(7)

        if self.twoDCartSupported:
            self.step(8)
            twoDCartesianMax = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.TwoDCartesianMax
            )
            log.info(f"Rx'd TwoDCartesianMax: {twoDCartesianMax}")

            videoSensorParams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.VideoSensorParams
            )
            log.info(f"Rx'd VideoSensorParams: {videoSensorParams}")
            asserts.assert_true(twoDCartesianMax.x == videoSensorParams.sensorWidth - 1 and twoDCartesianMax.y ==
                                videoSensorParams.sensorHeight - 1, "TwoDCartesianMax should be within the VideoSensorParams dimensions")
        else:
            log.info("TWoDCart Feature not supported. Test steps skipped")
            self.skip_step(8)


if __name__ == "__main__":
    default_matter_test_main()
