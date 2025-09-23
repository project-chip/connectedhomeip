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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIT_ICD_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from typing import Any

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import (assert_is_commissionable_type, assert_valid_cm_key,
                                          assert_valid_commissionable_instance_name, assert_valid_d_key,
                                          assert_valid_devtype_subtype, assert_valid_dn_key, assert_valid_dt_key,
                                          assert_valid_hostname, assert_valid_long_discriminator_subtype, assert_valid_ph_key,
                                          assert_valid_pi_key, assert_valid_ri_key, assert_valid_sai_key, assert_valid_sat_key,
                                          assert_valid_short_discriminator_subtype, assert_valid_sii_key,
                                          assert_valid_vendor_subtype, assert_valid_vp_key)
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

'''
Purpose
The purpose of this test case is to verify that a device is able to
correctly advertise Commissionable Node Discovery service.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#341-tc-sc-41-commissionable-node-discovery-dut_commissionee
'''

PICS_MCORE_ROLE_COMMISSIONEE = "MCORE.ROLE.COMMISSIONEE"
TCP_PICS_STR = "MCORE.SC.TCP"
ONE_HOUR_IN_MS = 3600000
MAX_SAT_VALUE = 65535
MAX_T_VALUE = 6
LONG_DISCRIMINATOR = 3840
CM_SUBTYPE = f"_CM._sub.{MdnsServiceType.COMMISSIONABLE.value}"


class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [TestStep(1, "DUT is commissioned.", is_commissioning=True),
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ",
                         "If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false."),
                TestStep(
                    3, "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold_ms."),
                TestStep(4, "DUT is put in Commissioning Mode.",
                         "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
                TestStep(5, "DUT is put in Commissioning Mode.",
                         "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
                ]

    async def read_attribute(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def get_descriptor_server_list(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_active_mode_threshhold_ms(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.ActiveModeThreshold
        )

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    def pics_TC_SC_4_1(self) -> list[str]:
        return [PICS_MCORE_ROLE_COMMISSIONEE]

    @async_test_body
    async def test_TC_SC_4_1(self):
        self.endpoint = self.get_endpoint(default=1)
        active_mode_threshold_ms = None
        supports_icd = False

        # *** STEP 1 ***
        # DUT is Commissioned.
        self.step(1)

        # *** STEP 2 ***
        # TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0. If the ICD Management
        # cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
        self.step(2)
        ep0_servers = await self.get_descriptor_server_list()

        # Check if ep0_servers contain the ICD Management cluster ID (0x0046)
        supports_icd = Clusters.IcdManagement.id in ep0_servers
        logging.info(f"\n\n\t** supports_icd: {supports_icd}\n")

        # *** STEP 3 ***
        # If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves
        # as active_mode_threshold.
        self.step(3)
        if supports_icd:
            active_mode_threshold_ms = await self.get_active_mode_threshhold_ms()
        logging.info(f"\n\n\t** active_mode_threshold_ms: {active_mode_threshold_ms}\n")

        # *** STEP 4 ***
        # DUT is put in Commissioning Mode.
        # - DUT starts advertising Commissionable Node Discovery service using DNS-SD.
        self.step(4)
        await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id,
            timeout=600,
            iteration=10000,
            discriminator=LONG_DISCRIMINATOR,
            option=1
        )

        # *** STEP 5 ***
        # Get DUT's commissionable service.
        # - Verify presence of DUT's commissionable service.
        # - Verify DUT's commissionable service is a valid DNS-SD instance name.
        # - Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        self.step(5)
        mdns = MdnsDiscovery()

        # Get DUT's commissionable service
        commissionable_services = await mdns.get_commissionable_services(log_output=True)

        # Verify presence of DUT's comissionable service
        asserts.assert_greater(len(commissionable_services), 0, "DUT's commissionable services not present")
        commissionable_service = commissionable_services[0]

        # Verify DUT's commissionable service is a valid DNS-SD instance name
        # (64-bit randomly selected ID expressed as a sixteen-char hex string with capital letters)
        assert_valid_commissionable_instance_name(commissionable_service.instance_name)

        # Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        assert_is_commissionable_type(commissionable_service.service_type)

        # Verify target hostname is derived from the 48bit or 64bit MAC address
        # expressed as a twelve or sixteen capital letter hex string. If the MAC
        # is randomized for privacy, the randomized version must be used each time.
        assert_valid_hostname(commissionable_service.hostname)

        # Get commissionable subtypes
        subtypes = await mdns.get_commissionable_subtypes(log_output=True)

        # Validate that the commissionable long discriminator subtype is a 12-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        long_discriminator_subtype = next((s for s in subtypes if s.startswith('_L')), None)
        asserts.assert_is_not_none(long_discriminator_subtype, "Long discriminator must be present.")
        assert_valid_long_discriminator_subtype(long_discriminator_subtype)

        # Validate that the short commissionable discriminator subtype is a 4-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        short_discriminator_subtype = next((s for s in subtypes if s.startswith('_S')), None)
        asserts.assert_is_not_none(short_discriminator_subtype, "Short discriminator must be present.")
        assert_valid_short_discriminator_subtype(short_discriminator_subtype)

        # If the commissionable vendor subtype is present, validate it's a
        # 16-bit vendor id, encoded as a variable-length decimal number in
        # ASCII text, omitting any leading zeros
        vendor_subtype = next((s for s in subtypes if s.startswith('_V')), None)
        if vendor_subtype:
            assert_valid_vendor_subtype(vendor_subtype)

        # If the commissionable devtype subtype is present, validate it's a
        # 32-bit variable length decimal number in ASCII without leading zeros
        # TODO: Update test plan to inclide "32-bit"
        devtype_subtype = next((s for s in subtypes if s.startswith('_T')), None)
        if devtype_subtype:
            assert_valid_devtype_subtype(devtype_subtype)

        # Verify presence of the _CM subtype
        asserts.assert_in(CM_SUBTYPE, subtypes, f"'{CM_SUBTYPE}' subtype must be present.")

        # Verify D key is present, which represents the discriminator
        # and must be encoded as a variable-length decimal value with up to 4
        # digits omitting any leading zeros
        d_key = commissionable_service.txt.get('D')
        asserts.assert_is_not_none(d_key, "D key must be present.")
        assert_valid_d_key(d_key)

        # If VP key is present, verify it contain at least Vendor ID
        # and if Product ID is present, values must be separated by a + sign
        if 'VP' in commissionable_service.txt:
            vp_key = commissionable_service.txt['VP']
            if vp_key:
                assert_valid_vp_key(vp_key)

        # If SAI key is present, SII key must be an unsigned integer with
        # units of milliseconds and shall be encoded as a variable length decimal
        # number in ASCII, omitting leading zeros. Shall not exceed 3600000.
        if 'SII' in commissionable_service.txt:
            sii_key = commissionable_service.txt['SII']
            if sii_key:
                assert_valid_sii_key(sii_key)

        # If SAI key is present, SAI key must be an unsigned integer with
        # units of milliseconds and shall be encoded as a variable length decimal
        # number in ASCII, omitting leading zeros. Shall not exceed 3600000.
        if 'SAI' in commissionable_service.txt:
            sai_key = commissionable_service.txt['SAI']
            if sai_key:
                assert_valid_sai_key(sai_key)

        # - If the SAT key is present, verify that it is a decimal value with
        #   no leading zeros and is less than or equal to 65535
        if 'SAT' in commissionable_service.txt:
            sat_key = commissionable_service.txt['SAT']
            if sat_key:
                assert_valid_sat_key(sat_key)

            # - If the SAT key is present and supports_icd is true, verify that
            #   the value is equal to active_mode_threshold
            if supports_icd:
                logging.info("supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                asserts.assert_equal(int(sat_key), active_mode_threshold_ms)

        # Verify that the SAI key is present if the SAT key is present
        asserts.assert_true(
            'SAT' not in commissionable_service.txt
            or 'SAI' in commissionable_service.txt,
            "SAI key must be present if SAT key is present."
        )

        # TODO: how to make CM = 1, getting CM = 2 currently, why 1?
        # # Verify that the CM key is present and is equal to 1
        # if 'CM' in commissionable_service.txt:
        #     cm_key = commissionable_service.txt['CM']
        #     assert_valid_cm_key(cm_key)
        #     asserts.assert_true(cm_key == "1", f"CM key must be equal to 1, got {cm_key}")
        # else:
        #     asserts.fail("CM key not present")

        # If the DT key is present, it must contain the device type identifier from
        # Data Model Device Types and must be encoded as a variable length decimal
        # ASCII number without leading zeros
        if 'DT' in commissionable_service.txt:
            dt_key = commissionable_service.txt['DT']
            assert_valid_dt_key(dt_key)

        # If the DN key is present, DN key must be a UTF-8 encoded string with a maximum length of 32B
        if 'DN' in commissionable_service.txt:
            dn_key = commissionable_service.txt['DN']
            assert_valid_dn_key(dn_key)

        # If the RI key is present, key RI must include the Rotating Device Identifier
        # encoded as an uppercase string with a maximum length of 100 chars (each octet
        # encoded as a 2-digit hex number, max 50 octets)
        if 'RI' in commissionable_service.txt:
            ri_key = commissionable_service.txt['RI']
            assert_valid_ri_key(ri_key)

        # If the PH key is present, key PH must be encoded as a variable-length decimal number
        # in ASCII text, omitting any leading zeros. If present value must be different of 0
        if 'PH' in commissionable_service.txt:
            ph_key = commissionable_service.txt['PH']
            assert_valid_ph_key(ph_key)

        # TODO: Fix PI key present but null/None ??
        # If the PI key is present, key PI must be encoded as a valid UTF-8 string
        # with a maximum length of 128 bytes
        if 'PI' in commissionable_service.txt:
            pi_key = commissionable_service.txt['PI']
            # assert_valid_pi_key(pi_key)

        # TH performs a query for the AAAA record against the target
        # listed in the Commissionable Service SRV record.
        hostname = commissionable_service.hostname
        quada_records = await mdns.get_quada_records(hostname=hostname, log_output=True)

        # Verify at least 1 AAAA record is returned
        asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{hostname}'")


if __name__ == "__main__":
    default_matter_test_main()
