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
from itertools import count

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

class TC_HSTAT_2_2(MatterBaseTest):
    def pics_TC_HSTAT_2_2(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_1(self) -> str:
        return "[TC-HSTAT-2.1] Primary functionality with DUT as Server"

    def steps_TC_HSTAT_2_1(self):
        step = count(1)
        return [TestStep(next(step), "Commissioning already done.", is_commissioning=True),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Humidifier or Dehumidifier",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to false",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),

        ]

    async def read_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_2(self):
        step = count(1)

        endpoint = self.get_endpoint()

        self.step(next(step))
        # Commissioning already done.
        cluster = Clusters.Humidistat
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        mistBitmap = cluster.Bitmaps.Mist
        supported_attributes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        feature_map = await self.read_setting(attributes.FeatureMap)
        supports_humidifier = bool(feature_map & features.kHumidifier)
        supports_dehumidifier = bool(feature_map & features.kDehumidifier)
        supports_continuous = bool(feature_map & features.kContinuous)
        supports_sensor = bool(feature_map & features.kSensor)
        supports_auto = bool(feature_map & features.kAuto)
        supports_fan = bool(feature_map & features.kFan)
        supports_optimal = bool(feature_map & features.kOptimal)
        supports_warm = bool(feature_map & features.kWarmMist)
        supports_cold = bool(feature_map & features.kColdMist)

        # some convenience definions
        modeOff = cluster.Enums.ModeEnum.kOff
        modeHumidifier = cluster.Enums.ModeEnum.kHumidifier
        modeDehumidifier = cluster.Enums.ModeEnum.kDeumidifier
        modeAuto = cluster.Enums.ModeEnum.kAuto
        modeFanOnly = cluster.Enums.ModeEnum.kFanOnly
        stateOff = cluster.Enums.SystemStateEnum.kOff
        stateHumidifying = cluster.Enums.SystemStateEnum.kHumidifying
        stateDehumidifying = cluster.Enums.SystemStateEnum.kDehumidifying
        stateFan = cluster.Enums.SystemStateEnum.kFan
        stateIdle = cluster.Enums.SystemStateEnum.kIdle

        self.step(next(step))  # Write Mode to Humidifier or Dehumidifier
        if supports_humidifier:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeHumidifier), endpoint_id=endpoint)
        else:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeDehumidifier), endpoint_id=endpoint)

        self.step(next(step))  # Set Continuous, Sleep, and Optimal to false
        self.send_single_cmd(cmd=cluster.Commands.SetSettings(continuous=False, sleep=False, optimal=False), endpoint=endpoint)


if __name__ == "__main__":
    default_matter_test_main()
