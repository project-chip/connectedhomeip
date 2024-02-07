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
#

import logging

import chip.clusters as Clusters
from matter_testing_support import TestStep, MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from TC_OpstateCommon import TC_OVENOPSTATE_BASE, TestInfo


class TC_OVENOPSTATE_1_1(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)

        test_info = TestInfo(
            pics_code="OVENOPSTATE",
            cluster=Clusters.OvenCavityOperationalState
        )

        self.TC_BASE = TC_OVENOPSTATE_BASE(
                            implementer=self,
                            test_info=test_info)

    def steps_TC_OVENOPSTATE_1_1(self) -> list[TestStep]:
        return self.TC_BASE.steps_TC_OPSTATE_BASE_1_1()

    def pics_TC_OVENOPSTATE_1_1(self) -> list[str]:
        return ["OVENOPSTATE.S"]

    @async_test_body
    async def test_TC_OVENOPSTATE_1_1(self):
        endpoint = self.matter_test_config.endpoint
        cluster_revision = 1
        feature_map=0

        await self.TC_BASE.test_TC_OPSTATE_BASE_1_1(endpoint=endpoint,
                                                    cluster_revision=cluster_revision,
                                                    feature_map=feature_map)


if __name__ == "__main__":
    default_matter_test_main()
