#
#    Copyright (c) 2026 Project CHIP Authors
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

from thread_meshcop_testing import (assert_common_diagnostic_fields, discriminator_from_config, establish_pase_over_thread_meshcop,
                                    get_setup_code)

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TC_SC_TC_2_1(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_TC_2_1(self):
        """[TC-SC-TC-2.1] Network Discovery and Selection [DUT - Commissionee]"""
        self.step(1, "TH initiates commissioning with the DUT's long discriminator."
                     "DUT provisionally selects the TH's network and sends DNS Announcement.")
        self.wait_for_user_input("Power on the factory-fresh DUT.")
        expected_discriminator = discriminator_from_config(self, use_short_discriminator=False)
        diagnostic = await establish_pase_over_thread_meshcop(self, get_setup_code(self, use_short_discriminator=False))
        assert_common_diagnostic_fields(diagnostic, expected_discriminator, use_short_discriminator=False)

        self.step(2, "TH initiates commissioning with the DUT's short discriminator."
                     "DUT provisionally selects the TH's network and sends DNS Announcement.")
        self.wait_for_user_input("Power cycle or factory reset the DUT so it can be discovered again.")
        expected_discriminator = discriminator_from_config(self, use_short_discriminator=True)
        diagnostic = await establish_pase_over_thread_meshcop(self, get_setup_code(self, use_short_discriminator=True))
        assert_common_diagnostic_fields(diagnostic, expected_discriminator, use_short_discriminator=True)


if __name__ == "__main__":
    default_matter_test_main()
