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

import logging
import random
import string
from typing import Optional

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_CNET_4_13(MatterBaseTest):
    def steps_TC_CNET_4_13(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH reads MaxNetworks attribute from DUT and is saved as MaxNetworksValue for future use')
                ]

    def def_TC_CNET_4_13(self):
        return '[TC-CNET-4.13] [Wi-Fi] Verification for ReorderNetwork command [DUT-Server]'

    def pics_TC_CNET_4_13(self):
        return ['CNET.S']

    @async_test_body
    async def test_TC_CNET_4_13(self):
        # Commissioning is already done
        self.step("precondition")

        self.step(1)


if __name__ == "__main__":
    default_matter_test_main()
