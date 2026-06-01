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

from mobly import asserts
from thread_meshcop_testing import (assert_common_diagnostic_fields, discriminator_from_config, establish_pase_over_thread_meshcop,
                                    get_setup_code)

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TC_SC_TC_2_2(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_TC_2_2(self):
        """[TC-SC-TC-2.2] MAC Extended Address Generation [DUT - Commissionee]"""
        setup_code = get_setup_code(self, use_short_discriminator=False)
        expected_discriminator = discriminator_from_config(self, use_short_discriminator=False)

        self.step(1, "TH sets the steering data to match the DUT's full 12-bit discriminator."
                     "DUT sends a Discovery Request and TH sends a Discovery Response back to the address of DUT.")

        self.step(2, "Power on the factory-fresh DUT."
                     "DUT sends a Discovery Request and TH sends a Discovery Response back to the address of DUT.")
        self.wait_for_user_input("Power on the factory-fresh DUT.")
        first_diagnostic = await establish_pase_over_thread_meshcop(self, setup_code)
        assert_common_diagnostic_fields(first_diagnostic, expected_discriminator, use_short_discriminator=False)
        first_joiner_id = first_diagnostic["joiner_id"]

        self.step(3, "Power cycle the DUT."
                     "DUT sends a Discovery Request and TH sends a Discovery Response back to a different address.")
        self.wait_for_user_input("Power cycle the DUT.")
        second_diagnostic = await establish_pase_over_thread_meshcop(self, setup_code)
        assert_common_diagnostic_fields(second_diagnostic, expected_discriminator, use_short_discriminator=False)
        second_joiner_id = second_diagnostic["joiner_id"]

        asserts.assert_not_equal(second_joiner_id, first_joiner_id,
                                 "The Thread MeshCoP source identifier must change after a DUT power cycle.")


if __name__ == "__main__":
    default_matter_test_main()
