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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TV_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 3840 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     app-stdin-pipe: dut-tv-app-stdin
#     script-args: >
#       --storage-path admin_storage.json
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

from mdns_discovery.mdns_discovery import MdnsServiceType
from mdns_discovery.utils.support import (get_verify_commissioner_service, get_verify_srv_record, verify_aaaa_records,
                                          verify_commissioner_txt_record_keys, verify_devtype_subtype)

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

'''
Purpose
The purpose of this test case is to verify that a device that supports
Commissioner Discovery is able to advertise its commissioner service.

The DUT acts as the commissioner (advertiser of the '_matterd._udp' service)
and the TH is a passive DNS-SD scanner; the DUT is never commissioned and no
PASE or CASE session is established. The test expects the DUT to be the only
commissioner advertising on the test network.
'''


class TC_SC_4_6(MatterBaseTest):
    # The DUT is never commissioned in this test (it advertises, the TH only
    # scans DNS-SD), so there is no DUT fabric state to capture or clean up.
    requires_dut = False
    # The TH's controller stack advertises its own '_matterd._udp' commissioner
    # service by default, which this test would discover alongside the DUT's;
    # disable it so only the DUT's advertisement is on the network.
    enable_server_interactions = False

    def steps_TC_SC_4_6(self) -> list[TestStep]:
        return [
            TestStep(1, "DUT is instructed to start advertising its presence as a commissioner in the network"),

            TestStep(2, """TH browses for the 'Commissioner Service' (_matterd._udp) through DNS-SD""",
                     """- Verify that there is one, and only one, commissioner service advertised
                        - Verify that the DNS-SD instance name is a 64-bit ID expressed as a
                          sixteen-char hex string with capital letters
                        - Verify that the service type is _matterd._udp and the service domain is .local"""),

            TestStep(3, """TH performs a 'Commissioner Service' SRV record query against the instance name""",
                     """- Verify that the SRV record is returned and its instance name is equal to the browsed instance name
                        - Verify that the target hostname is expressed as a twelve or sixteen capital letter hex string."""),

            TestStep(4, """TH performs a 'Commissioner Service' TXT record query against the instance name""",
                     """- If the VP key is present, verify that it is non-empty and contains at least Vendor ID, and if
                          Product ID is present, values must be separated by a + sign
                        - If the DT key is present, verify that it is non-empty and contains the device type identifier
                          from Data Model Device Types encoded as a variable length decimal ASCII number without leading zeros
                        - If the DN key is present, verify that it is non-empty and is a UTF-8 encoded string with a
                          maximum length of 32B"""),

            TestStep(5, """If the DT key is present, TH performs a PTR record query against the 'Devtype Subtype' (_T<ddd>) constructed from the DT key""",
                     """- Verify that there is one, and only one, 'Devtype Subtype' PTR record, where <ddd> represents the
                          device type from Data Model represented as a variable length decimal number in ASCII without
                          leading zeros
                        - Verify that the 'Devtype Subtype' PTR record's instance name is equal to the commissioner service
                          instance name"""),

            TestStep(6, """TH performs a AAAA record query against the target hostname listed in the SRV record""",
                     """- Verify that at least 1 AAAA record is returned for each IPv6 address
                        - Verify that each AAAA record contains a valid IPv6 address"""),
        ]

    def pics_TC_SC_4_6(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONER", "MCORE.DD.COMM_DISCOVERY"]

    def desc_TC_SC_4_6(self) -> str:
        return "[TC-SC-4.6] Commissioner Discovery [DUT as Commissioner]"

    @async_test_body
    async def test_TC_SC_4_6(self):
        # *** STEP 1 ***
        # DUT is instructed to start advertising its presence as a commissioner in the network
        self.step(1)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                prompt_msg="Instruct the DUT to start advertising its presence as a commissioner in the network, then press Enter.")
        # In CI the TV reference app advertises the commissioner service on boot,
        # no instruction is needed.

        # *** STEP 2 ***
        # TH browses for the 'Commissioner Service' (_matterd._udp) through DNS-SD
        self.step(2)
        service = await get_verify_commissioner_service()

        # *** STEP 3 ***
        # TH performs a 'Commissioner Service' SRV record query against the instance name
        self.step(3)
        srv_hostname = await get_verify_srv_record(service.instance_name, MdnsServiceType.COMMISSIONER.value)

        # *** STEP 4 ***
        # TH performs a 'Commissioner Service' TXT record query against the instance name
        self.step(4)
        dt_key = await verify_commissioner_txt_record_keys(service.instance_name)

        # *** STEP 5 ***
        # If the DT key is present, TH performs a PTR record query against the
        # 'Devtype Subtype' (_T<ddd>) constructed from the DT key
        if dt_key is not None:
            self.step(5)
            await verify_devtype_subtype(service.instance_name, dt_key, MdnsServiceType.COMMISSIONER.value)
        else:
            self.skip_step(5)

        # *** STEP 6 ***
        # TH performs a AAAA record query against the target hostname listed in the SRV record
        self.step(6)
        await verify_aaaa_records(srv_hostname)


if __name__ == "__main__":
    default_matter_test_main()
