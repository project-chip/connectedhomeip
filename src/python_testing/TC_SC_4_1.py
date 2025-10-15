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
#     app: ${ALL_CLUSTERS_APP}
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
from time import sleep
from typing import Any, Optional

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import (assert_is_commissionable_type, assert_valid_cm_key,
                                          assert_valid_commissionable_instance_name, assert_valid_d_key,
                                          assert_valid_devtype_subtype, assert_valid_dn_key, assert_valid_dt_key,
                                          assert_valid_hostname, assert_valid_icd_key, assert_valid_ipv6_addresses,
                                          assert_valid_long_discriminator_subtype, assert_valid_ph_key, assert_valid_pi_key,
                                          assert_valid_ri_key, assert_valid_sai_key, assert_valid_sat_key,
                                          assert_valid_short_discriminator_subtype, assert_valid_sii_key, assert_valid_t_key,
                                          assert_valid_vendor_subtype, assert_valid_vp_key)
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.commissioning import get_setup_payload_info_config
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

'''
Purpose
The purpose of this test case is to verify that a device is able to
correctly advertise Commissionable Node Discovery service.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#341-tc-sc-41-commissionable-node-discovery-dut_commissionee
'''

TCP_PICS_STR = "MCORE.SC.TCP"
ROOT_NODE_ENDPOINT_ID = 0


class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [
            # DUT DETAILS
            #
            TestStep(1, "DUT is commissioned.", is_commissioning=True),

            TestStep(2, """Check if the ICD Management cluster is present""",
                        """TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0
                            - If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to True, otherwise set supports_icd to False
                            - If supports_icd is true
                                - TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold_ms"""),

            TestStep(3, """Check if the LITS (Long Idle Time Support) feature is supported""",
                        """If supports_icd is True:
                            - TH reads the FeatureMap attribute from the ICD Management cluster on EP0
                                - If the LITS feature is set:
                                    - Set supports_lit to True, otherwise, set supports_lit to False"""),

            TestStep(4, """Check if the DUT supports TCP""",
                        """If supported:
                            - Set supports_tcp to True, otherwise, set supports_tcp to False"""),

            TestStep(5, """Check if DUT supports Open Basic Commissioning Window""",
                        """If supported:
                            - Set supports_obcw to True, otherwise, set supports_obcw to False"""),

            TestStep(6, """Get the 'Long Discriminator Subtype'""",
                        """TH gets the 'Long Discriminator' from the DUT and constructs the 'Long Discriminator Subtype'
                            - Verify that the 'Long Discriminator Subtype' discriminator value is a valid 12-bit variable length decimal
                            number in ASCII text, omitting any leading zeros. Save as 'long_discriminator_subtype'
                            - Save the 'Long Discriminator' value as 'long_discriminator'"""),

            # OPEN BASIC COMMISSIONING WINDOW
            #
            TestStep(7, """If supports_obcw is True, DUT is put in Commissioning Mode using Open Basic Commissioning Window command""",
                        """DUT starts advertising Commissionable Node Discovery service through DNS-SD"""),

            TestStep(8, """Get the 'Long Discriminator Subtype' PTR record's instance name""",
                        """TH performs a PTR record query against the 'Long Discriminator Subtype' _L
                            - Verify that there is one, and only one, 'Long Discriminator Subtype' PTR record
                            - Save the 'Long Discriminator Subtype' PTR record's instance name as 'long_discriminator_subtype_ptr_instance_name'"""),

            TestStep(9, """Verify commissionable subtype advertisements""",
                        """See the 'Commissionable Subtypes Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(10, """VVerify SRV record advertisements""",
                     """See the 'SRV Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(11, """Verify TXT record advertisements""",
                     """See the 'TXT Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed. Expected CM TXT key value = 1"""),

            TestStep(12, """Verify AAAA records""",
                     """See the 'AAAA Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(13, """Close commissioning window""",
                     """DUT stops advertising Commissionable Node Discovery services"""),

            # OPEN COMISSIONING WINDOW
            #
            TestStep(14, """DUT is put in Commissioning Mode using Open Commissioning Window command""",
                     """DUT starts advertising Commissionable Node Discovery service through DNS-SD"""),

            TestStep(15, """Get the 'Long Discriminator Subtype' PTR record's instance name""",
                     """TH performs a PTR record query against the 'Long Discriminator Subtype' _L
                            - Verify that there is one, and only one, 'Long Discriminator Subtype' PTR record
                            - Save the 'Long Discriminator Subtype' PTR record's instance name as 'long_discriminator_subtype_ptr_instance_name'"""),

            TestStep(16, """Verify commissionable subtype advertisements""",
                     """See the 'Commissionable Subtypes Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(17, """VVerify SRV record advertisements""",
                     """See the 'SRV Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(18, """Verify TXT record advertisements""",
                     """See the 'TXT Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed. Expected CM TXT key value = 2"""),

            TestStep(19, """Verify AAAA records""",
                     """See the 'AAAA Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(20, """Close commissioning window""",
                     """DUT stops advertising Commissionable Node Discovery services"""),

            # EXTENDED DISCOVERY MODE
            #
            TestStep(21, """Check if DUT Extended Discovery mode is active""",
                     """Get the 'Long Discriminator Subtype' PTR record's instance name
                            - If the DUT's 'Long Discriminator Subtype' PTR record's instance name is present,
                            Extended Discovery mode is active, if so, save the 'Long Discriminator Subtype'
                            PTR record's instance name as 'long_discriminator_subtype_ptr_instance_name'"""),

            TestStep(22, """Verify commissionable subtype advertisements""",
                     """See the 'Commissionable Subtypes Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(23, """VVerify SRV record advertisements""",
                     """See the 'SRV Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),

            TestStep(24, """Verify TXT record advertisements""",
                     """See the 'TXT Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed. Expected
                        CM TXT key value = 0 or omitted key"""),

            TestStep(25, """Verify AAAA records""",
                     """See the 'AAAA Record Verifications' table in the Notes/Testing considerations
                        section of the Test Plan for the list of verifications to be performed"""),
        ]

    async def read_attribute(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def get_descriptor_server_list(self):
        return await self.read_single_attribute_check_success(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            dev_ctrl=self.default_controller,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_active_mode_threshhold_ms(self):
        return await self.read_single_attribute_check_success(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.ActiveModeThreshold
        )

    async def get_icd_feature_map(self):
        return await self.read_single_attribute_check_success(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.FeatureMap
        )

    def get_long_discriminator_subtype(self) -> Optional[tuple[str, str]]:
        # TH constructs the 'Long Discriminator Subtype'
        # using the DUT's 'Long Discriminator'
        long_discriminator: str | None = None
        long_discriminator_subtype: str | None = None
        setup_payload_info = get_setup_payload_info_config(self.matter_test_config)
        if setup_payload_info:
            long_discriminator = setup_payload_info[0].filter_value
            long_discriminator_subtype = f"_L{long_discriminator}._sub.{MdnsServiceType.COMMISSIONABLE.value}"
            logging.info(f"\n\n\t** long_discriminator: {long_discriminator}\n")

            # Verify that it contains a valid 12-bit variable length decimal number
            # in ASCII text, omitting any leading zeros 'Long Discriminator' value
            assert_valid_long_discriminator_subtype(long_discriminator_subtype)

            return long_discriminator_subtype, long_discriminator
        else:
            asserts.fail("Failed to get the 'Long Discriminator' value from the setup payload info.")

        return None

    async def _get_verify_long_discriminator_subtype_ptr_instance_name(self, long_discriminator_subtype: str, must_be_present: bool = True) -> Optional[str]:

        # TH performs a PTR record query against the 'Long Discriminator Subtype' _L
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[long_discriminator_subtype],
            log_output=True
        )

        if must_be_present or ptr_records:
            # Verify that there is one, and only one, 'Long Discriminator Subtype' PTR record
            asserts.assert_equal(len(ptr_records), 1,
                                 f"There must only be one 'Long Discriminator Subtype' ({long_discriminator_subtype}) PTR record, found {len(ptr_records)}.")

            return ptr_records[0].instance_name

        return None

    async def _verify_commissionable_subtypes(self, long_discriminator_ptr_instance_name: str) -> None:
        # TH performs a browse for the rest of the 'Commissionable Service' subtypes
        subtypes = await MdnsDiscovery().get_commissionable_subtypes(log_output=True)

        # *** SHORT DISCRIMINATOR SUBTYPE ***
        # Verify that the 'Short Discriminator Subtype' _S is present
        short_discriminator_subtype = next((s for s in subtypes if s.startswith('_S')), None)
        asserts.assert_is_not_none(short_discriminator_subtype, "'Short Discriminator Subtype' must be present.")

        # Verify that it contains a valid 4-bit variable length decimal number
        # in ASCII text, omitting any leading zeros 'Short Discriminator' value
        assert_valid_short_discriminator_subtype(short_discriminator_subtype)

        # TH performs a PTR record query against the 'Short Discriminator Subtype'
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[short_discriminator_subtype],
            log_output=True
        )

        # Verify that there is one, and only one, 'Short Discriminator Subtype' PTR record
        asserts.assert_equal(len(ptr_records), 1, "There must only be one 'Short Discriminator Subtype' PTR record.")
        short_discriminator_ptr = ptr_records[0]

        # Verify that the 'Short Discriminator Subtype' PTR record's instance name
        # is equal to the 'Long Discriminator Subtype' PTR record's instance name
        asserts.assert_equal(short_discriminator_ptr.instance_name, long_discriminator_ptr_instance_name,
                             "'Short Discriminator Subtype' PTR record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

        # *** IN COMMISSIONING MODE SUBTYPE ***
        # Verify that the 'In Commissioning Mode Subtype' _CM is present
        cm_subtype = f"_CM._sub.{MdnsServiceType.COMMISSIONABLE.value}"
        asserts.assert_in(cm_subtype, subtypes, f"In Commissioning Mode Subtype '{cm_subtype}' must be present.")

        # TH performs a PTR record query against the 'In Commissioning Mode Subtype'
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[cm_subtype],
            log_output=True
        )

        # Verify that there is one, and only one, 'In Commissioning Mode Subtype' PTR record
        asserts.assert_equal(len(ptr_records), 1, "There must only be one 'In Commissioning Mode Subtype' subtype PTR record.")
        cm_ptr = ptr_records[0]

        # Verify that the 'In Commissioning Mode Subtype' PTR record's instance name
        # is equal to the 'Long Discriminator Subtype' PTR record's instance name
        asserts.assert_equal(cm_ptr.instance_name, long_discriminator_ptr_instance_name,
                             "'In Commissioning Mode Subtype' PTR record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

        # *** VENDOR SUBTYPE ***
        # Check for the presence of the 'Vendor Subtype' _V
        vendor_subtype = next((s for s in subtypes if s.startswith('_V')), None)

        # If present:
        if vendor_subtype:
            # Verify that it contains a valid 16-bit variable length decimal
            # number in ASCII text, omitting any leading zeros 'Vendor Subtype' value
            assert_valid_vendor_subtype(vendor_subtype)

            # TH performs a PTR record query against the 'Vendor Subtype'
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[vendor_subtype],
                log_output=True
            )

            # If present:
            if len(ptr_records) > 0:
                # Verify that the 'Vendor Subtype' PTR record's instance name
                # is equal to the 'Long Discriminator Subtype' PTR record's instance name
                vendor_subtype_ptr = ptr_records[0]
                asserts.assert_equal(vendor_subtype_ptr.instance_name, long_discriminator_ptr_instance_name,
                                     "'Vendor Subtype' PTR record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

        # *** DEVTYPE SUBTYPE ***
        # Check for the presence of the 'Devtype Subtype' _T
        devtype_subtype = next((s for s in subtypes if s.startswith('_T')), None)

        # If present:
        if devtype_subtype:
            # Verify that it contains a valid 32-bit variable length decimal
            # number in ASCII text, omitting any leading zeros 'Devtype Subtype' value
            assert_valid_devtype_subtype(devtype_subtype)

            # TH performs a PTR record query against the 'Devtype Subtype'
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[devtype_subtype],
                log_output=True
            )

            # If present:
            if len(ptr_records) > 0:
                # Verify that the 'Devtype Subtype' PTR record's instance name
                # is equal to the 'Long Discriminator Subtype' PTR record's instance name.
                devtype_subtype_ptr = ptr_records[0]
                asserts.assert_equal(devtype_subtype_ptr.instance_name, long_discriminator_ptr_instance_name,
                                     "'Devtype Subtype' PTR record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

    async def _get_verify_srv_record(self, long_discriminator_ptr_instance_name: str) -> str:
        # TH performs a 'Commissionable Service' SRV record query
        # against 'long_discriminator_subtype_ptr_instance_name'
        srv_record = await MdnsDiscovery().get_srv_record(
            service_name=f"{long_discriminator_ptr_instance_name}.{MdnsServiceType.COMMISSIONABLE.value}",
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # Verify SRV record is returned
        asserts.assert_true(srv_record is not None, "SRV record was not returned")

        # Verify that the SRV record's instance name is equal
        # to 'long_discriminator_subtype_ptr_instance_name'
        asserts.assert_equal(srv_record.instance_name, long_discriminator_ptr_instance_name,
                             "SRV record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

        # Verify that the DUT's SRV record's instance name is a 64-bit randomly
        # selected ID expressed as a sixteen-char hex string with capital letters
        assert_valid_commissionable_instance_name(srv_record.instance_name)

        # Verify that the DUT's SRV record's service type
        # is '_matterc._udp' and service domain '.local.'
        assert_is_commissionable_type(srv_record.service_type)

        # Verify that the target hostname is derived from the 48bit or 64bit MAC
        # address expressed as a twelve or sixteen capital letter hex string
        assert_valid_hostname(srv_record.hostname)

        return srv_record.hostname

    async def _verify_txt_record_keys(self, long_discriminator_ptr_instance_name: str, expected_cm: str) -> None:
        # TH performs a 'Commissionable Service' TXT record query against 'long_discriminator_subtype_ptr_instance_name'
        # The device may omit the TXT record if there are no mandatory TXT keys
        txt_record = await MdnsDiscovery().get_txt_record(
            service_name=f"{long_discriminator_ptr_instance_name}.{MdnsServiceType.COMMISSIONABLE.value}",
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # Verify that the TXT record, when required, is returned and is non-empty
        txt_record_required = self.supports_icd or self.supports_tcp
        txt_record_returned = (txt_record is not None) and (len(txt_record.txt) > 0)
        asserts.assert_true((not txt_record_required) or txt_record_returned,
                            "TXT record is required and was not returned or contains no values")

        # If the TXT record is returned, verify its TXT keys
        if txt_record_returned:

            # Verify that the TXT record's instance name is equal to 'long_discriminator_subtype_ptr_instance_name'
            asserts.assert_equal(txt_record.instance_name, long_discriminator_ptr_instance_name,
                                 "TXT record's instance name must be equal to the 'Long Discriminator Subtype' PTR record's instance name.")

            # *** ICD KEY ***
            icd_key: str | None = None
            if self.supports_lit:
                # Verify that the 'ICD' key is present and non-empty
                asserts.assert_in('ICD', txt_record.txt, "'ICD' key must be present.")
                icd_key = txt_record.txt.get('ICD')
                asserts.assert_true(icd_key, "'ICD' key is present but has no value.")

                # Verify that the 'ICD' key has the value of 0 or 1 encoded
                # as a decimal number in ASCII text ommiting any leading zeros
                assert_valid_icd_key(icd_key)
            else:
                # Verify that the 'ICD' TXT key is NOT present
                asserts.assert_not_in('ICD', txt_record.txt, "'ICD' key must NOT be present.")

            # Set sit_mode = True when:
            #   - supports_icd is True and supports_lit is False.
            #   - supports_icd is True and supports_lit is True and ICD == '0'
            # Set sit_mode = False when:
            #   - supports_icd is False.
            #   - supports_icd is True and supports_lit is True and ICD == '1'
            sit_mode = self.supports_icd and (not self.supports_lit or icd_key == '0')
            logging.info(f"\n\n\t** sit_mode: {sit_mode}\n")

            # *** SII KEY ***
            if sit_mode:
                # Verify that the 'SII' key is present and non-empty if 'sit_mode' is True
                asserts.assert_in('SII', txt_record.txt, "'SII' key must be present.")
                sii_key = txt_record.txt['SII']
                asserts.assert_true(sii_key, "'SII' key is present but has no value.")

                # Verify that the 'SII' key is an unsigned integer with units of milliseconds
                # encoded as a variable length decimal number in ASCII text, omitting any
                # leading zeros, and shall not exceed 3600000
                assert_valid_sii_key(sii_key)

            # *** SAI KEY ***
            if self.supports_icd:
                # Verify that the 'SAI' key is present and non-empty if 'supports_icd' is True
                asserts.assert_in('SAI', txt_record.txt, "'SAI' key must be present.")
                sai_key = txt_record.txt['SAI']
                asserts.assert_true(sai_key, "'SAI' key is present but has no value.")

                # Verify that the 'SAI' key is an unsigned integer with units of milliseconds
                # encoded as a variable length decimal number in ASCII text, omitting any
                # leading zeros, and shall not exceed 3600000
                assert_valid_sai_key(sai_key)

            # *** SAT KEY ***
            # If the 'SAT' key is present:
            if 'SAT' in txt_record.txt:
                sat_key = txt_record.txt['SAT']

                # Verify that it is non-empty
                asserts.assert_true(sat_key, "'SAT' key is present but has no value.")

                # Verify that it is an unsigned integer with units of milliseconds encoded as
                # a variable length decimal number in ASCII text, omitting any leading zeros,
                # and shall be less than or equal to 65535
                assert_valid_sat_key(sat_key)

                # If the 'SAT' key is present and supports_icd is true, verify
                # that the value is equal to 'active_mode_threshold_ms'
                if self.supports_icd:
                    asserts.assert_equal(int(sat_key), self.active_mode_threshold_ms,
                                         f"'SAT' key value ({sat_key}) must be equal to 'active_mode_threshold_ms' ({self.active_mode_threshold_ms})")

            # Verify that the 'SAI' key is present if the 'SAT' key is present
            asserts.assert_true(
                'SAT' not in txt_record.txt
                or 'SAI' in txt_record.txt,
                "SAI key must be present if SAT key is present."
            )

            # *** D KEY ***
            # Verify that the 'D' key is present and non-empty
            asserts.assert_in('D', txt_record.txt, "'D' key must be present.")
            d_key = txt_record.txt['D']
            asserts.assert_true(d_key, "'D' key is present but has no value.")

            # Verify that it is a full 12-bit discriminator encoded as a variable length
            # decimal number in ASCII text, omitting any leading zeros, and up to 4 digits
            assert_valid_d_key(d_key)

            # *** VP KEY ***
            # If the VP key is present
            if 'VP' in txt_record.txt:
                # Verify that it is non-empty
                vp_key = txt_record.txt['VP']
                asserts.assert_true(d_key, "'D' key is present but has no value.")

                # Verify that it contain at least Vendor ID, and if Product ID
                # is present, both values must be separated by a + sign
                assert_valid_vp_key(vp_key)

            # *** T KEY ***
            self.verify_t_key(txt_record)

            # *** CM KEY ***
            # If the 'CM' key is present
            if 'CM' in txt_record.txt:
                # Verify that it is non-empty
                cm_key = txt_record.txt['CM']
                asserts.assert_true(cm_key, "'CM' key is present but has no value.")

                # Verify value is a decimal number in ASCII text with allowed values (0, 1, 2, 3)
                assert_valid_cm_key(cm_key)

                # Verify that the 'CM' key value is equal to the expected value
                asserts.assert_true(cm_key == expected_cm, f"'CM' key must be '{expected_cm}', got '{cm_key}'")
            else:
                # When the 'CM' key is not present, or present and equal to '0', the DUT is in Extended Discovery mode
                # Verify that the expected 'CM' value is '0' (Extended Discovery mode)
                asserts.assert_equal(expected_cm, "0",
                                     "Expected 'CM' key value must be '0' when 'CM' key is not present in the TXT record.")

            # *** DT KEY ***
            # If the DT key is present
            if 'DT' in txt_record.txt:
                # Verify that it is non-empty
                dt_key = txt_record.txt['DT']
                asserts.assert_true(dt_key, "'DT' key is present but has no value.")

                # Verify that it contains the device type identifier encoded as a
                # variable length decimal number in ASCII text without leading zeros
                assert_valid_dt_key(dt_key)

            # *** DN KEY ***
            # If the DN key is present
            if 'DN' in txt_record.txt:
                # Verify that it is non-empty
                dn_key = txt_record.txt['DN']
                asserts.assert_true(dn_key, "'DN' key is present but has no value.")

                # Verify that it is a valid UTF-8 encoded string of maximum length of 32 bytes
                assert_valid_dn_key(dn_key)

            # *** RI KEY ***
            # If the RI key is present
            if 'RI' in txt_record.txt:
                # Verify that it is non-empty
                ri_key = txt_record.txt['RI']
                asserts.assert_true(ri_key, "'RI' key is present but has no value.")

                # Verify that it is encoded as an uppercase string with a maximum length of
                # 100 chars (each octet encoded as a 2-digit hex number, max 50 octets)
                assert_valid_ri_key(ri_key)

            # *** PH KEY ***
            # If the PH key is present
            if 'PH' in txt_record.txt:
                # Verify that it is non-empty
                ph_key = txt_record.txt['PH']
                asserts.assert_true(ph_key, "'PH' key is present but has no value.")

                # Verify that it is encoded as a variable-length decimal number in ASCII
                # text, omitting any leading zeros, and value different than '0'
                assert_valid_ph_key(ph_key)

            # TODO: Fix PI key present but null/None ??
            # *** PI KEY ***
            # # If the PI key is present
            # if 'PI' in txt_record.txt:
            #     # Verify that it is non-empty
            #     pi_key = txt_record.txt['PI']
            #     asserts.assert_true(pi_key, "'PH' key is present but has no value.")

            #     # Verify it is encoded as a valid UTF-8 string
            #     # with a maximum length of 128 bytes
            #     assert_valid_pi_key(pi_key)

            assert_valid_pi_key("for-lint...")  # To de removed
        else:
            logging.info("TXT record NOT required.")

    def verify_t_key(self, txt_record) -> None:
        # If 'supports_tcp' is False and T key is not present, nothing to check
        if (not self.supports_tcp) and ('T' not in txt_record.txt):
            return

        # Verify that if 'supports_tcp' is True, the 'T' key is present
        if self.supports_tcp:
            asserts.assert_in('T', txt_record.txt, "'T' key must be present.")

        # Verify that the 'T' key is non-empty
        t_key = txt_record.txt.get('T')
        asserts.assert_true(t_key, "'T' key is present but has no value.")

        # Verify that the 'T' key is a decimal number encoded as ASCII
        # text without any leading zeros, and less than or equal to 6
        assert_valid_t_key(t_key, enforce_provisional=False)

        # Convert to bitmap
        T_int = int(t_key)

        # Verify that bit 0 is clear
        asserts.assert_true((T_int & 0x01) == 0, f"T key ({t_key}) bit 0 must be clear.")

        # Verify that TCP bit (0x04) matches the PICS exactly if 'supports_tcp' is True
        tcp_bit_set = (T_int & 0x04) != 0
        asserts.assert_true(tcp_bit_set == self.supports_tcp,
                            "TCP bit (0x04) must be set if MCORE.SC.TCP PICS is set.")

        # Verify that the value encodes TCP capability per PICS:
        #   - If 'supports_tcp' is True, T key allowed values are (4, 6)
        #   - If 'supports_tcp' is False, T key allowed values are (0)
        allowed = {4, 6} if self.supports_tcp else {0}
        asserts.assert_true(T_int in allowed,
                            f"T value ({t_key}) is not in allowed set {sorted(allowed)}.")

    @staticmethod
    async def _verify_aaaa_records(srv_hostname: str) -> None:
        # TH performs a AAAA record query against the target 'hostname'
        # listed in the 'Commissionable Service' SRV record
        quada_records = await MdnsDiscovery().get_quada_records(hostname=srv_hostname, log_output=True)

        # Verify that at least 1 AAAA record is returned for each IPv6 a address
        asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{srv_hostname}'")

        # Verify that each AAAA record contains a valid IPv6 address
        ipv6_addresses = [f"{r.address}%{r.interface}" for r in quada_records]
        assert_valid_ipv6_addresses(ipv6_addresses)

    async def close_commissioning_window(self) -> None:
        revoke_cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.default_controller.SendCommand(nodeid=self.dut_node_id,
                                                  endpoint=0,
                                                  payload=revoke_cmd,
                                                  timedRequestTimeoutMs=6000)
        sleep(1)  # Give some time for failsafe cleanup scheduling

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    @async_test_body
    async def test_TC_SC_4_1(self):
        self.endpoint = self.get_endpoint(default=1)
        self.supports_icd = False
        self.supports_lit = False
        long_discriminator = None
        obcw_cmd = Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180)

        # *** STEP 1 ***
        # DUT is Commissioned.
        self.step(1)

        # *** STEP 2 ***
        # Check if the ICD Management cluster is present
        # TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0
        #   - If the ICD Management cluster ID (70,0x46) is present in the list, set 'supports_icd' to True, otherwise set 'supports_icd' to False
        #   - If 'supports_icd' is True, TH reads 'ActiveModeThreshold' from the ICD Management cluster on EP0 and saves as 'active_mode_threshold_ms'
        self.step(2)
        ep0_servers = await self.get_descriptor_server_list()

        # Check if ep0_servers contain the ICD Management cluster ID (0x0046)
        self.supports_icd = Clusters.IcdManagement.id in ep0_servers
        logging.info(f"\n\n\t** supports_icd: {self.supports_icd}\n")

        # Read the ActiveModeThreshold attribute if ICD is supported
        if self.supports_icd:
            self.active_mode_threshold_ms = await self.get_active_mode_threshhold_ms()
            logging.info(f"\n\n\t** active_mode_threshold_ms: {self.active_mode_threshold_ms}\n")

        # *** STEP 3 ***
        # Check if the LITS feature is supported
        # If 'supports_icd' is True, TH reads the 'FeatureMap' attribute from the ICD Management cluster on EP0
        # If the LITS feature is set, set 'supports_lit' to True, otherwise, set 'supports_lit' to False
        self.step(3)
        if self.supports_icd:
            feature_map = await self.get_icd_feature_map()
            LITS = Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport
            self.supports_lit = bool(feature_map & LITS == LITS)
            logging.info(f"\n\n\t** supports_lit: {self.supports_lit}\n")

        # *** STEP 4 ***
        # Check if the DUT supports TCP
        self.step(4)
        self.supports_tcp = self.check_pics(TCP_PICS_STR)

        # *** STEP 5 ***
        # Check if the DUT supports the Open Basic Commissioning Window command (BC feature)
        self.step(5)
        supports_obcw = await self.feature_guard(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            cluster=Clusters.AdministratorCommissioning,
            feature_int=Clusters.AdministratorCommissioning.Bitmaps.Feature.kBasic)

        # *** STEP 6 ***
        # Get the 'Long Discriminator Subtype'
        self.step(6)
        long_discriminator_sybtype, long_discriminator = self.get_long_discriminator_subtype()

        # *** STEP 7 ***
        # If 'supports_obcw' is True DUT is put in Commissioning
        # Mode using **Open Basic Commissioning Window** command
        #   - DUT starts advertising Commissionable Node Discovery services
        self.step(7)
        if supports_obcw:
            logging.info("\n\n\t ** Open Basic Commissioning Window supported\n")
            await self.default_controller.SendCommand(
                nodeid=self.dut_node_id,
                endpoint=0,
                payload=obcw_cmd,
                timedRequestTimeoutMs=6000
            )

            # *** STEP 8 ***
            # Get the 'Long Discriminator Subtype' PTR record's instance name
            self.step(8)
            long_discriminator_ptr_instance_name = await self._get_verify_long_discriminator_subtype_ptr_instance_name(long_discriminator_sybtype)

            # *** STEP 9 ***
            # Verify commissionable subtype advertisements
            self.step(9)
            await self._verify_commissionable_subtypes(long_discriminator_ptr_instance_name)

            # *** STEP 10 ***
            # Verify SRV record advertisements
            self.step(10)
            srv_hostname = await self._get_verify_srv_record(long_discriminator_ptr_instance_name)

            # *** STEP 11 ***
            # Verify TXT record advertisements
            self.step(11)
            await self._verify_txt_record_keys(long_discriminator_ptr_instance_name, expected_cm="1")

            # *** STEP 12 ***
            # Verify AAAA records
            self.step(12)
            await self._verify_aaaa_records(srv_hostname)

            # *** STEP 13 ***
            # Close commissioning window
            self.step(13)
            await self.close_commissioning_window()
        else:
            logging.info("\n\n\t ** Open Basic Commissioning Window unsupported, skipping step.\n")
            self.mark_step_range_skipped(8, 13)

        # *** STEP 14 ***
        # DUT is put in Commissioning Mode using Open Commissioning Window command
        #   - DUT starts advertising Commissionable Node Discovery services
        self.step(14)
        await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id,
            timeout=180,
            iteration=10000,
            discriminator=long_discriminator,
            option=1
        )

        # *** STEP 15 ***
        # Get the 'Long Discriminator Subtype' PTR record's instance name
        self.step(15)
        long_discriminator_ptr_instance_name = await self._get_verify_long_discriminator_subtype_ptr_instance_name(long_discriminator_sybtype)

        # *** STEP 16 ***
        # Verify commissionable subtype advertisements
        self.step(16)
        await self._verify_commissionable_subtypes(long_discriminator_ptr_instance_name)

        # *** STEP 17 ***
        # Verify SRV record advertisements
        self.step(17)
        srv_hostname = await self._get_verify_srv_record(long_discriminator_ptr_instance_name)

        # *** STEP 18 ***
        # Verify TXT record keys advertisements
        self.step(18)
        await self._verify_txt_record_keys(long_discriminator_ptr_instance_name, expected_cm="2")

        # *** STEP 19 ***
        # Verify AAAA records
        self.step(19)
        await self._verify_aaaa_records(srv_hostname)

        # *** STEP 20 ***
        # Close commissioning window
        self.step(20)
        await self.close_commissioning_window()

        # *** STEP 21 ***
        # Check if DUT Extended Discovery mode is active
        self.step(21)
        # Get the 'Long Discriminator Subtype' PTR record's instance name
        long_discriminator_ptr_instance_name = await self._get_verify_long_discriminator_subtype_ptr_instance_name(long_discriminator_sybtype, must_be_present=False)

        # If the DUT's 'Long Discriminator Subtype' PTR record's instance name is present, Extended Discovery mode is active
        extended_discovery_mode = long_discriminator_ptr_instance_name is not None
        logging.info(f"DUT Extended Discovery mode active: {extended_discovery_mode}")

        if extended_discovery_mode:
            # *** STEP 22 ***
            # Verify commissionable subtype advertisements
            self.step(22)
            await self._verify_commissionable_subtypes(long_discriminator_ptr_instance_name)

            # *** STEP 23 ***
            # Verify SRV record advertisements
            self.step(23)
            srv_hostname = await self._get_verify_srv_record(long_discriminator_ptr_instance_name)

            # *** STEP 24 ***
            # Verify TXT record keys advertisements
            self.step(24)
            await self._verify_txt_record_keys(long_discriminator_ptr_instance_name, expected_cm="0")

            # *** STEP 25 ***
            # Verify AAAA records
            self.step(25)
            await self._verify_aaaa_records(srv_hostname)
        else:
            self.mark_step_range_skipped(22, 25)


if __name__ == "__main__":
    default_matter_test_main()
