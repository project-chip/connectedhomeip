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


class TC_SC_TC_3_1(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_TC_3_1(self):
        """[TC-SC-TC-3.1] Matter Discovery - Advertisement [DUT - Commissionee]"""
        self.step(1, "DUT successfully selects a Thread network provisionally."
                     "DUT sends a DNS Announcement containing commissionable node discovery information.")
        self.wait_for_user_input("Power on the factory-fresh DUT.")
        expected_discriminator = discriminator_from_config(self, use_short_discriminator=False)
        diagnostic = await establish_pase_over_thread_meshcop(self, get_setup_code(self, use_short_discriminator=False))
        assert_common_diagnostic_fields(diagnostic, expected_discriminator, use_short_discriminator=False)

        announcement = diagnostic["dns_announcement"]
        asserts.assert_greater(announcement["commissioning_mode"], 0,
                               "DNS announcement must indicate that the DUT is commissionable.")

        self.step(2, "TH sends incoming commissioning messages to establish the secure session."
                     "The DUT receives the commissioning messages and starts establishing a secure session with TH.")
        # Step 1 establishes PASE over the Matter UDP port advertised in the DNS announcement.
        # Returning from EstablishPASESessionThreadMeshcop without an exception verifies this step.


if __name__ == "__main__":
    default_matter_test_main()
