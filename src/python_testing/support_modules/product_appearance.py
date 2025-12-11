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
#

from mobly import asserts

from matter.clusters.ClusterObjects import Cluster
from matter.testing.matter_testing import MatterBaseTest, TestStep


class ProductAppearanceBase(MatterBaseTest):
    def steps(self) -> list[TestStep]:
        return [
            TestStep(0, "DUT commissioned if not already done", is_commissioning=True),
            TestStep(1, "TH reads ProductAppearance attribute from the DUT.",
                     "Verify the finish is a valid ProductFinishEnum and the PrimaryColor is a valid ColorEnum")
        ]

    def pics(self, cluster_pics) -> list[str]:
        return [f"{cluster_pics}.S.A0014"]

    async def implementation(self, cluster: Cluster):
        self.step(0)  # commissioning already done

        self.step(1)
        ret = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.ProductAppearance)
        asserts.assert_in(ret.finish, cluster.Enums.ProductFinishEnum, "Product finish enum value is unknown")
        asserts.assert_not_equal(ret.finish, cluster.Enums.ProductFinishEnum.kUnknownEnumValue,
                                 "Product finish enum value is unknown")
        asserts.assert_in(ret.primaryColor, cluster.Enums.ColorEnum, "Primary color enum value is unknown")
        asserts.assert_not_equal(ret.primaryColor, cluster.Enums.ColorEnum.kUnknownEnumValue, "Primary color enum value is unknown")
