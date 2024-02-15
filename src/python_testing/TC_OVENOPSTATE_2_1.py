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
from TC_OpstateCommon import TC_OPSTATE_BASE, TestInfo


class TC_OVENOPSTATE_2_1(MatterBaseTest, TC_OPSTATE_BASE):
    def __init__(self, *args):
        super().__init__(*args)

        test_info = TestInfo(
            pics_code="OVENOPSTATE",
            cluster=Clusters.OvenCavityOperationalState
        )

        super().setup_base(test_info=test_info)

    def steps_TC_OVENOPSTATE_2_1(self) -> list[TestStep]:
        return self.STEPS_TC_OPSTATE_BASE_2_1()

    def pics_TC_OVENOPSTATE_2_1(self) -> list[str]:
        return ["OVENOPSTATE.S"]

    @async_test_body
    async def test_TC_OVENOPSTATE_2_1(self):
        endpoint = self.matter_test_config.endpoint
        await self.TEST_TC_OPSTATE_BASE_2_1(endpoint)


if __name__ == "__main__":
    default_matter_test_main()
