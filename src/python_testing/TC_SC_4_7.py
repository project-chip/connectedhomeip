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
#     factory-reset: true
#     quiet: true
#     script-args: >
#       --string-arg th_server_app_path:${TV_APP}
#       --storage-path admin_storage.json
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import os
import tempfile

from mdns_discovery.mdns_discovery import MdnsServiceType
from mdns_discovery.utils.support import (get_verify_commissioner_service, get_verify_srv_record, verify_aaaa_records,
                                          verify_commissioner_txt_record_keys, verify_devtype_subtype)
from mobly import asserts

from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

'''
Purpose
The purpose of this test case is to verify that a device that is already
connected to an IP network is able to discover a commissioner.

The DUT is the commissionee that scans for commissioners; the DUT is never
commissioned and no PASE or CASE session is established. The TH plays the
commissioner by launching a commissioner reference app (the TV reference app,
provided via --string-arg th_server_app_path:<path>) that advertises the
'_matterd._udp' service the DUT must discover. The TH first verifies the
advertised commissioner service records are conformant, then the DUT is
instructed to scan for commissioners and the discovery is confirmed on the
DUT. The test expects the TH-side commissioner app to be the only commissioner
advertising on the test network.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#tc-sc-4-7commissioner-discovery-dut_commissionee
'''

# The TH-side commissioner app is not commissioned by this test; its
# discriminator and passcode are irrelevant to the '_matterd._udp'
# advertisement under verification, but the app requires them to boot.
TH_SERVER_DISCRIMINATOR = 3840
TH_SERVER_PASSCODE = 20202021


class TC_SC_4_7(MatterBaseTest):
    # The DUT is never commissioned in this test (the TH-side commissioner app
    # advertises, the DUT only scans DNS-SD), so there is no DUT fabric state
    # to capture or clean up.
    requires_dut = False
    # The TH's controller stack advertises its own '_matterd._udp' commissioner
    # service by default, which this test would discover alongside the TH-side
    # commissioner app's; disable it so only the app's advertisement is on the
    # network.
    enable_server_interactions = False

    def setup_class(self):
        super().setup_class()
        self.th_server = None
        self.storage = None

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH-side commissioner app (TV reference app). "
                         "Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")
        self.th_server_app = th_server_app

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def _start_th_server(self):
        # Temporary storage directory for the TH-side commissioner app's KVS,
        # so the app boots factory-fresh and leaves nothing behind.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        log.info("Temporary storage directory: %s", self.storage.name)

        self.th_server = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage.name,
            discriminator=TH_SERVER_DISCRIMINATOR,
            passcode=TH_SERVER_PASSCODE
        )

        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

    def steps_TC_SC_4_7(self) -> list[TestStep]:
        return [
            TestStep(1, "TH acts as a commissioner and starts advertising a 'Commissioner Service' (_matterd._udp) on the network"),

            TestStep(2, """TH confirms its commissioner advertisement by browsing for the 'Commissioner Service' (_matterd._udp) through DNS-SD""",
                     """- Verify that there is one, and only one, commissioner service advertised
                        - Verify that the DNS-SD instance name is a 64-bit ID expressed as a
                          sixteen-char hex string with capital letters
                        - Verify that the service type is _matterd._udp and the service domain is .local"""),

            TestStep(3, """TH confirms its commissioner advertisement's SRV record by querying it against the instance name""",
                     """- Verify that the SRV record is returned and its instance name is equal to the browsed instance name
                        - Verify that the target hostname is expressed as a twelve or sixteen capital letter hex string."""),

            TestStep(4, """TH confirms its commissioner advertisement's TXT record by querying it against the instance name""",
                     """- If the VP key is present, verify that it is non-empty and contains at least Vendor ID, and if
                          Product ID is present, values must be separated by a + sign
                        - If the DT key is present, verify that it is non-empty and contains the device type identifier
                          from Data Model Device Types encoded as a variable length decimal ASCII number without leading zeros
                        - If the DN key is present, verify that it is non-empty and is a UTF-8 encoded string with a
                          maximum length of 32B"""),

            TestStep(5, """If the DT key is present, TH confirms its commissioner advertisement's 'Devtype Subtype' (_T<ddd>) PTR record constructed from the DT key""",
                     """- Verify that there is one, and only one, 'Devtype Subtype' PTR record, where <ddd> represents the
                          device type from Data Model represented as a variable length decimal number in ASCII without
                          leading zeros
                        - Verify that the 'Devtype Subtype' PTR record's instance name is equal to the commissioner service
                          instance name"""),

            TestStep(6, """TH confirms its commissioner advertisement's addressing by querying the AAAA record against the target hostname in the SRV record""",
                     """- Verify that at least 1 AAAA record is returned for each IPv6 address
                        - Verify that each AAAA record contains a valid IPv6 address"""),

            TestStep(7, """DUT is instructed to scan for DNS-SD commissioner advertisements""",
                     """- Verify that the DUT is able to discover the TH's commissioner service"""),
        ]

    def pics_TC_SC_4_7(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "MCORE.DD.COMM_DISCOVERY"]

    def desc_TC_SC_4_7(self) -> str:
        return "[TC-SC-4.7] Commissioner Discovery [DUT as Commissionee]"

    @async_test_body
    async def test_TC_SC_4_7(self):
        # *** STEP 1 ***
        # TH acts as a commissioner and starts advertising a 'Commissioner Service'
        # (_matterd._udp) on the network: the TH launches the commissioner reference
        # app, which advertises the service on boot.
        self.step(1)
        self._start_th_server()

        # *** STEP 2 ***
        # TH confirms its commissioner advertisement by browsing for the
        # 'Commissioner Service' (_matterd._udp) through DNS-SD
        self.step(2)
        service = await get_verify_commissioner_service()

        # *** STEP 3 ***
        # TH confirms its commissioner advertisement's SRV record by querying it against the instance name
        self.step(3)
        srv_hostname = await get_verify_srv_record(service.instance_name, MdnsServiceType.COMMISSIONER.value)

        # *** STEP 4 ***
        # TH confirms its commissioner advertisement's TXT record by querying it against the instance name
        self.step(4)
        dt_key = await verify_commissioner_txt_record_keys(service.instance_name)

        # *** STEP 5 ***
        # If the DT key is present, TH confirms its commissioner advertisement's
        # 'Devtype Subtype' (_T<ddd>) PTR record constructed from the DT key
        if dt_key is not None:
            self.step(5)
            await verify_devtype_subtype(service.instance_name, dt_key, MdnsServiceType.COMMISSIONER.value)
        else:
            self.skip_step(5)

        # *** STEP 6 ***
        # TH confirms its commissioner advertisement's addressing by querying the AAAA record against the target hostname in the SRV record
        self.step(6)
        await verify_aaaa_records(srv_hostname)

        # *** STEP 7 ***
        # DUT is instructed to scan for DNS-SD commissioner advertisements.
        #   - Discovery happens on the DUT and is confirmed there by the operator;
        #     in CI there is no DUT and no DUT-side observable for the TH to check,
        #     so the step is skipped rather than reported as executed, and CI covers
        #     the TH-side advertisement (steps 1-6).
        if self.is_pics_sdk_ci_only:
            self.skip_step(7)
        else:
            self.step(7)
            self.wait_for_user_input(
                prompt_msg=f"Instruct the DUT to scan for DNS-SD commissioner advertisements, "
                f"verify on the DUT that it discovered the TH's commissioner service "
                f"(instance name '{service.instance_name}', hostname '{srv_hostname}'), "
                f"then press Enter.")


if __name__ == "__main__":
    default_matter_test_main()
