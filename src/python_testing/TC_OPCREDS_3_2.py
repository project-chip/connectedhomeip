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
from matter_testing_support import MatterBaseTest, TestStep, async_test_body
from test_plan_support import *


def verify_fabric(controller: str) -> str:
    return (f"- Verify there is one entry returned. Verify FabricIndex matches `fabric_index_{controller}`.\n"
            f"- Verify the RootPublicKey matches the public key for rcac_{controller}.\n"
            f"- Verify the VendorID matches the vendor ID for {controller}.\n"
            f"- Verify the FabricID matches the fabricID for {controller}")


class TC_OPCREDS_3_2(MatterBaseTest):
    def desc_TC_OPCREDS_3_2(self):
        return " Attribute-CurrentFabricIndex validation [DUTServer]"

    def steps_TC_OPCREDS_3_2(self):
        return [TestStep(0, commission_if_required('CR1'), is_commissioning=True),
                TestStep(1, f"{commission_from_existing('CR1', 'CR2')}\n. Save the FabricIndex from the NOCResponse as `fabric_index_CR2`.", verify_commissioning_successful()),
                TestStep(2, f"{commission_from_existing('CR1', 'CR3')}\n. Save the FabricIndex from the NOCResponse as `fabric_index_CR3`.", verify_commissioning_successful()),
                TestStep(3, f"CR2 {read_attribute('CurrentFabricIndex')}", "Verify the returned value is `fabric_index_CR2`"),
                TestStep(4, f"CR3 {read_attribute('CurrentFabricIndex')}", "Verify the returned value is `fabric_index_CR3`"),
                TestStep(5, f"CR2 {read_attribute('Fabrics')} using a fabric-filtered read", verify_fabric('CR2')),
                TestStep(6, f"CR3 {read_attribute('Fabrics')} using a fabric-filtered read", verify_fabric('CR3')),
                TestStep(7, remove_fabric('fabric_index_CR2', 'CR1'), verify_success()),
                TestStep(8, remove_fabric('fabric_index_CR3', 'CR1'), verify_success()),
                ]

    @async_test_body
    async def test_TC_OPCREDS_3_2(self):
        # TODO: implement
        pass


if __name__ == "__main__":
    default_matter_test_main()
