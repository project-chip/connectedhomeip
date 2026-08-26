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

from mobly import asserts

import matter.clusters as Clusters

class AVANALYTestBase:
    SPEC_MAX_COUNT_SUPPORTEDAMBIENTCONTEXTS = 50
    SPEC_MAX_COUNT_ANALYSIS_STREAMS = 255

    has_feature_lclcondetect = False
    has_feature_remcondetect = False
    has_feature_perzonedetect = False

    async def read_avanaly_attribute_expect_success(self, endpoint, attribute):
        """Read the provided attribute from the cluster instance on the provided endpoint.

        Args:
            attribute: The attribute to be read, must be a valid AvAnalysis cluster attribute
        """
        cluster = Clusters.Objects.AvAnalysis
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def check_avanaly_attribute(self, attribute, expected_value, endpoint):
        """Validate that the current instance of an attribute has the value provided

        Args:
            attribute:      The attribute to be read and validated, must be a valid AvAnalysis cluster attribute.
            expected_value: The value to be matched, assert failure if this is not equal to the current attribute value
        """
        value = await self.read_avanaly_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")
