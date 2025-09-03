# Copyright (c) 2025 Project CHIP Authors
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

class TC_DD_3_23(MatterBaseTest):
    def steps_TC_DD_3_23(self):
        return [
            TestStep(0, "Display Hello World", "Hello World is displayed")
        ]

    def pics_TC_DD_3_23(self):
        return ["HELLO_WORLD.S"]

    @async_test_body
    async def test_TC_DD_3_23(self):
        self.step(0)
        print("Hello World")

if __name__ == "__main__":
    default_matter_test_main()