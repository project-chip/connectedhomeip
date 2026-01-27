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

# No CI on this test because the bridge device app doesn't implement this feature
# but this test is a DIRECT copy of the BINFO test, which does run in the CI.

from support_modules.product_appearance import ProductAppearanceBase

import matter.clusters as Clusters
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.matter_testing import TestStep, default_matter_test_main


class TC_BRBINFO_3_1(ProductAppearanceBase):

    def steps_TC_BRBINFO_3_1(self) -> list[TestStep]:
        return self.steps()

    def pics_TC_BRBINFO_3_1(self) -> list[str]:
        return self.steps('BRBINFO')

    @run_if_endpoint_matches(has_attribute(Clusters.BridgedDeviceBasicInformation.Attributes.ProductAppearance))
    async def test_TC_BRBINFO_3_1(self):
        return await self.implementation(Clusters.BridgedDeviceBasicInformation)


if __name__ == "__main__":
    default_matter_test_main()
