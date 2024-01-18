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

import logging

from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from operational_mdns_discovery import OperationalMdnsDiscovery
from mobly import asserts

'''
Category:
Functional conformance

Description:
The purpose of this test case is to verify that a Short Idle Time ICD node properly advertises SAI/SII
values.

Verifies:
  - SII key is higher than the SESSION_IDLE_INTERVAL default value (500 milliseconds)
  - SII key and SAI key is less than 1 hour (3600000 milliseconds)

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#tc-sc-4-10-operational-discovery-sit-icd-node-dut_commissionee
'''

SESSION_IDLE_INTERVAL_MS = 500
ONE_HOUR_MS = 3600000
SAI_KEY = "SAI"
SII_KEY = "SII"


class TC_SC_4_10(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_4_10(self):

        omd = OperationalMdnsDiscovery(tc=self)

        # *** Step 1 ***
        self.print_step(1, "DUT is instructed to advertise its service: already done")

        # *** Step 2 ***
        self.print_step(2, "TH scans for DNS-SD advertising, looks for SAI/SII values")

        # Get SAI value
        sai_value = await omd.getTxtRecord(key=SAI_KEY)
        asserts.assert_is_not_none(sai_value, f"Value for '{SAI_KEY}' not found")
        sai_ms = int(sai_value)

        # Get SII value
        sii_value = await omd.getTxtRecord(key=SII_KEY)
        asserts.assert_is_not_none(sii_value, f"Value for '{SII_KEY}' not found")
        sii_ms = int(sii_value)

        logging.info(f"{SAI_KEY}: {sai_ms}ms")
        logging.info(f"{SII_KEY}: {sii_ms}ms")

        asserts.assert_greater(sii_ms, SESSION_IDLE_INTERVAL_MS,
                               f"SII value ({sii_ms}ms) must be greater than SESSION_IDLE_INTERVAL ({SESSION_IDLE_INTERVAL_MS} ms)")
        asserts.assert_less(sii_ms, ONE_HOUR_MS,
                            f"SII value ({sii_ms}ms) must be less than one hour ({ONE_HOUR_MS}ms)")
        asserts.assert_less(sai_ms, ONE_HOUR_MS,
                            f"SAI value ({sai_ms}ms) must be less than one hour ({ONE_HOUR_MS}ms)")


if __name__ == "__main__":
    default_matter_test_main()
