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

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGGEN_3_2(MatterBaseTest):
    def steps_TC_DGGEN_3_2(self):
        return [TestStep(0, "Commission DUT (already done)", is_commissioning=True),
                TestStep(1, "TH reads the MaxPathsPerInvoke attribute from the Basic Information Cluster from DUT. Save the value as `max_paths_per_invoke",
                         "Read is successful"),
                TestStep(2, "TH reads FeatureMap attribute from the General Diagnostics Cluster from DUT",
                         "Verify that the FeatureMap value has the DMTEST feature bit (0) set to 1 if `max_path_per_invoke` > 1")
                ]

    @async_test_body
    async def test_TC_DGGEN_3_2(self):
        # commissioning - already done
        self.step(0)

        self.step(1)
        max_paths_per_invoke = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.MaxPathsPerInvoke)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(cluster=Clusters.GeneralDiagnostics, attribute=Clusters.GeneralDiagnostics.Attributes.FeatureMap)
        if max_paths_per_invoke > 1:
            asserts.assert_true(feature_map & Clusters.GeneralDiagnostics.Bitmaps.Feature.kDataModelTest,
                                "DMTEST feature must be set if MaxPathsPerInvoke > 1")


if __name__ == "__main__":
    default_matter_test_main()
