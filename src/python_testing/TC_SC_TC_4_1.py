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

# N/A - Test requires Thread MeshCoP hardware or simulation and will not run on CI.

from thread_meshcop_testing import establish_pase_over_thread_meshcop, get_setup_code

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TC_SC_TC_4_1(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_TC_4_1(self):
        """[TC-SC-TC-4.1] Message Framing and PASE Session Establishment [DUT – Commissionee]"""
        self.step(1, "DUT is ready to be discovered", is_commissioning=False)
        self.wait_for_user_input("Power on the DUT")
        self.step(2, "TH establishes PASE session over Thread Meshcop with DUT", is_commissioning=False)
        await establish_pase_over_thread_meshcop(self, get_setup_code(self, use_short_discriminator=False))


if __name__ == "__main__":
    default_matter_test_main()
