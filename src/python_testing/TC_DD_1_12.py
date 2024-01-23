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


from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import MdnsDiscovery


'''
Category:
Functional conformance

Description:
This test case verifies that the interactions defined by the Custom Flow field are reflected by the DUT supporting Standard Commissioning Flow.

Verifies:
  - The DUT Commissionee is advertising to be commissioned to a Matter network.

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/devicediscovery.adoc#tc-dd-1-12-onboarding-payload-verification-custom-flow-0-dut-commissionee
'''


class TC_DD_1_12(MatterBaseTest):

    @async_test_body
    async def test_TC_DD_1_12(self):
        print("\n" * 10)

        mdns = MdnsDiscovery(tc=self)
        await mdns.discover()

        print(f"getCommisionerServiceInfo: {mdns.getCommisionerServiceInfo()}\n")
        print(f"getCommissionableServiceInfo: {mdns.getCommissionableServiceInfo()}\n")
        print(f"getOperationalServiceInfo: {mdns.getOperationalServiceInfo()}\n")
        print(f"getBorderRouterServiceInfo: {mdns.getBorderRouterServiceInfo()}\n")
        print(f"txtsai: {mdns.getOperationalServiceInfo()[0]['txt_record']['SAI']}")

        print("\n" * 10)


if __name__ == "__main__":
    default_matter_test_main()
