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
from typing import Any

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.ClusterObjects import ClusterAttributeDescriptor
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest

log = logging.getLogger(__name__)


class HSTATBase(MatterBaseTest):
    """Base class for Humidistat cluster test cases.

    Provides common setup logic, feature support detection, attribute read/write
    helpers, and command wrappers for testing the Humidistat cluster.
    """

    async def setup(self) -> None:
        """Sets up cluster references, reads supported attributes and feature map.

        Initializes references to cluster definitions, queries the FeatureMap and
        AttributeList to determine supported features, and sets up convenience
        attributes and enum constants for test cases.
        """
        self.endpoint = self.get_endpoint()
        self.cluster = Clusters.Humidistat
        self.attributes = self.cluster.Attributes
        self.features = self.cluster.Bitmaps.Feature
        self.MistTypeBitmap = self.cluster.Bitmaps.MistTypeBitmap
        self.SetSettings = self.cluster.Commands.SetSettings

        self.supported_attributes = await self.read_attribute_expect_success(attribute=self.attributes.AttributeList)

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

        # some convenience definitions
        self.modeHumidifier = self.cluster.Enums.ModeEnum.kHumidifier
        self.modeDehumidifier = self.cluster.Enums.ModeEnum.kDehumidifier
        self.modeAuto = self.cluster.Enums.ModeEnum.kAuto
        self.modeFanOnly = self.cluster.Enums.ModeEnum.kFanOnly
        self.stateIdle = self.cluster.Enums.SystemStateEnum.kIdle
        self.ModeEnum = self.cluster.Enums.ModeEnum
        self.SystemStateEnum = self.cluster.Enums.SystemStateEnum

    async def read_attribute_expect_success(self, attribute: type[ClusterAttributeDescriptor]) -> Any:
        """Reads a single Humidistat attribute from the DUT and asserts success.

        Args:
            attribute: The Humidistat cluster attribute descriptor class to read.

        Returns:
            The decoded value of the attribute read from the DUT.
        """
        return await self.read_single_attribute_check_success(endpoint=self.get_endpoint(), cluster=self.cluster, attribute=attribute)

    async def send_SetSettingsCommand_expect_success(self, **kwargs: Any) -> None:
        """Sends the SetSettings command to the DUT and asserts success.

        Args:
            **kwargs: Keyword arguments passed to the SetSettings command.
        """
        await self.send_single_cmd(cmd=self.SetSettings(**kwargs), endpoint=self.get_endpoint(), timedRequestTimeoutMs=1000)

    async def send_SetSettingsCommand_expect_error(self, error: Status, **kwargs: Any) -> None:
        """Sends the SetSettings command to the DUT and asserts that an expected error is returned.

        Args:
            error: The expected InteractionModel Status error code.
            **kwargs: Keyword arguments passed to the SetSettings command.
        """
        try:
            await self.send_single_cmd(cmd=self.SetSettings(**kwargs), endpoint=self.get_endpoint(), timedRequestTimeoutMs=1000)
            asserts.assert_fail("Unexpected command success, for command SetSettings")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    async def send_onoff_on_cmd_expect_success(self) -> None:
        """Sends the On command to the OnOff cluster on the DUT endpoint and asserts success."""
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On(), endpoint=self.get_endpoint(), timedRequestTimeoutMs=1000)

    async def send_onoff_off_cmd_expect_success(self) -> None:
        """Sends the Off command to the OnOff cluster on the DUT endpoint and asserts success."""
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off(), endpoint=self.get_endpoint(), timedRequestTimeoutMs=1000)
