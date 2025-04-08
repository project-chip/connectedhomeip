#
#    Copyright (c) 2023 Project CHIP Authors
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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts

logger = logging.getLogger(__name__)


class AVSUMTestBase:

    async def read_avsum_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        full_attr = getattr(cluster.Attributes, attribute)
        logging.info(f"endpoint {endpoint} full_attr {full_attr}")
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_avsum_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def validate_feature_map(self, must_have_features, must_not_have_features):
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")
        for must_have_feature in must_have_features:
            asserts.assert_true(feature_map & must_have_feature,
                                f"{must_have_feature.name} must be set but is not. feature_map 0x{feature_map:x}")

        for must_not_have_feature in must_not_have_features:
            asserts.assert_false(feature_map & must_not_have_feature,
                                 f"{must_not_have_feature.name} is not allowed to be set. feature_map 0x{feature_map:x}")

    async def send_save_presets_command(self, endpoint, name: str, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.CameraAvSettingsUserLevelManagement.Commands.MPTZSavePreset(
                name=name),
                endpoint=endpoint)

            asserts.assert_equal(expected_status, Status.Success)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

 
