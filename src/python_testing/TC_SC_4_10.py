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
from mdns_helper import MdnsHelper
from mobly import asserts

'''
Category:
Functional conformance

Purpose:
The purpose of this test case is to verify that a Short Idle Time ICD node properly advertises SAI/SII
values. This verification is in addition any other operational/commissionable discovery test cases.

Test Plan:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#tc-sc-4-10-operational-discovery-sit-icd-node-dut_commissionee
'''

SESSION_IDLE_INTERVAL_MS = 500
ONE_HOUR_MS = 3600000


class TC_SC_4_10(MatterBaseTest):

    @async_test_body
    async def test_TC_SC_4_10(self):

        self.print_step(1, "DUT is instructed to advertise its service: already done")

        self.print_step(2, "TH scans for DNS-SD advertising, looks for SAI/SII values")

        mh = MdnsHelper(self)
        SAI_MS = int(await mh.getTxtRecord(key="SAI"))
        SII_MS = int(await mh.getTxtRecord(key="SII"))

        logging.info(f"SII: {SII_MS}ms")
        logging.info(f"SAI: {SAI_MS}ms")

        asserts.assert_greater(SII_MS, SESSION_IDLE_INTERVAL_MS,
                               f"SII value ({SII_MS}ms) must be greater than SESSION_IDLE_INTERVAL ({SESSION_IDLE_INTERVAL_MS} ms)")
        asserts.assert_less(SII_MS, ONE_HOUR_MS,
                            f"SII value ({SII_MS}ms) must be less than one hour ({ONE_HOUR_MS}ms)")
        asserts.assert_less(SAI_MS, ONE_HOUR_MS,
                            f"SAI value ({SAI_MS}ms) must be less than one hour ({ONE_HOUR_MS}ms)")


if __name__ == "__main__":
    default_matter_test_main()
