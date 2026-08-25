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


import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status

log = logging.getLogger(__name__)


class AVANALYTestBase:
    SPEC_MAX_COUNT_SUPPORTEDAMBIENTCONTEXTS = 50

    has_feature_lclcondetect = False
    has_feature_remcondetect = False
    has_feature_perzonedetect = False

    async def read_avanaly_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.AvAnalysis
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def check_avanaly_attribute(self, attribute, expected_value, endpoint):
        value = await self.read_avanal_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")
