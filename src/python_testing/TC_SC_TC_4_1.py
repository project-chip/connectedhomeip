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

from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_SC_TC_4_1(MatterBaseTest):
    def desc_TC_SC_TC_4_1(self) -> str:
        return "[TC-SC-TC-4.1] Message Framing and PASE Session Establishment [DUT – Commissionee]"

    def steps_TC_SC_TC_4_1(self) -> list[TestStep]:
        return [
            TestStep(1, "DUT petitions the Thread network to become the Thread Commissioner with a 12-bit discriminator"),
            TestStep(2, 'Validate the discriminator and perform the commissioning')
        ]


    @async_test_body
    async def test_TC_SC_TC_4_1(self):
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        self.step(1)
        commissioner.SetSkipCommissioningComplete(True)
        self.step(2)
        await self.commission_devices()


if __name__ == "__main__":
    default_matter_test_main()
