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


import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status

log = logging.getLogger(__name__)

class HSTATBase:
    """
    This is a base class for Humidistat cluster tests.
    """

    async def async_hstat_setup(self):
        self.cluster = Clusters.Humidistat
        self.attributes = self.cluster.Attributes
        self.features = self.cluster.Bitmaps.Feature
        self.mistBitmap = self.cluster.Bitmaps.MistTypeBitmap
        self.SetSettings = self.cluster.Commands.SetSettings
        self.SystemStatus = self.cluster.Enums.SystemStateEnum

        self.supported_attributes = await self.read_hstat_attribute_expect_success(endpoint=self.get_endpoint(), attribute=self.attributes.AttributeList)

        feature_map = await self.read_single_attribute_check_success(
            endpoint=self.get_endpoint(), cluster=self.cluster, attribute=self.attributes.FeatureMap)
        self.humidifierFeatureSupported = bool(feature_map & self.features.kHumidifier)
        self.dehumidifierFeatureSupported = bool(feature_map & self.features.kDehumidifier)
        self.continuousFeatureSupported = bool(feature_map & self.features.kContinuous)
        self.sensorFeatureSupported = bool(feature_map & self.features.kSensor)
        self.autoFeatureSupported = bool(feature_map & self.features.kAuto)
        self.fanOnlyFeatureSupported = bool(feature_map & self.features.kFanOnly)
        self.optimalFeatureSupported = bool(feature_map & self.features.kOptimal)
        self.warmFeatureSupported = bool(feature_map & self.features.kWarmMist)
        self.coldFeatureSupported = bool(feature_map & self.features.kColdMist)
        self.condPumpFeatureSupported = bool(feature_map & self.features.kCondPump)
        log.info("DUT supports the Humidifier feature: %s", self.humidifierFeatureSupported)
        log.info("DUT supports the Dehumidifier feature: %s", self.dehumidifierFeatureSupported)
        log.info("DUT supports the Continuous feature: %s", self.continuousFeatureSupported)
        log.info("DUT supports the Sensor feature: %s", self.sensorFeatureSupported)
        log.info("DUT supports the Auto feature: %s", self.autoFeatureSupported)
        log.info("DUT supports the Fan feature: %s", self.fanOnlyFeatureSupported)
        log.info("DUT supports the Optimal feature: %s", self.optimalFeatureSupported)
        log.info("DUT supports the Warm feature: %s", self.warmFeatureSupported)
        log.info("DUT supports the Cold feature: %s", self.coldFeatureSupported)
        log.info("DUT supports the CondPump feature: %s", self.condPumpFeatureSupported)

        # some convenience definions
        self.modeHumidifier = self.cluster.Enums.ModeEnum.kHumidifier
        self.modeDehumidifier = self.cluster.Enums.ModeEnum.kDehumidifier
        self.modeAuto = self.cluster.Enums.ModeEnum.kAuto
        self.modeFanOnly = self.cluster.Enums.ModeEnum.kFanOnly
        self.stateIdle = self.cluster.Enums.SystemStateEnum.kIdle
        self.ModeEnum = self.cluster.Enums.ModeEnum

    async def read_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attribute)])
        err_msg = "Received error status {} when writing {}:{}".format(str(result[0].Status), str(cluster), str(attribute))
        asserts.assert_equal(result[0].Status, Status.Success, err_msg)

    async def send_hstat_cmd_expect_success(self, endpoint, command) -> None:
        await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=1000)

    async def send_hstat_cmd_expect_error(self, endpoint, command, error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=1000)
            asserts.assert_true(False, "Unexpected command success, command=%s", command)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    async def send_onoff_on_cmd_expect_success(self, endpoint) -> None:
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On(), endpoint=endpoint, timedRequestTimeoutMs=1000)

    async def send_onoff_off_cmd_expect_success(self, endpoint) -> None:
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off(), endpoint=endpoint, timedRequestTimeoutMs=1000)
