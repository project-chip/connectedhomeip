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

from mdns_discovery.data_classes.mdns_service_info import MdnsServiceInfo
from mdns_discovery.data_classes.ptr_record import PtrRecord
from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import (assert_is_commissionable_type, assert_valid_cm_key,
                                          assert_valid_commissionable_instance_name, assert_valid_d_key,
                                          assert_valid_devtype_subtype, assert_valid_dn_key, assert_valid_dt_key,
                                          assert_valid_hostname, assert_valid_icd_key, assert_valid_long_discriminator_subtype,
                                          assert_valid_ph_key, assert_valid_pi_key, assert_valid_ri_key, assert_valid_sai_key,
                                          assert_valid_sat_key, assert_valid_short_discriminator_subtype, assert_valid_sii_key,
                                          assert_valid_t_key, assert_valid_vendor_subtype, assert_valid_vp_key)
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

TCP_PICS_STR = "MCORE.SC.TCP"
ROOT_NODE_ENDPOINT_ID = 0


class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [TestStep(1, "DUT is commissioned.", is_commissioning=True),
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ", """
                                - If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
                                - If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold.                         
                                """),
                TestStep(3, "If DUT supports Open Basic Commissioning Window, put it in Commissioning Mode using Open Basic Commissioning Window command.", """
                                - DUT starts advertising Commissionable Node Discovery service through DNS-SD
                                - Verify DUT Commissionable Node Discovery service advertisements
                                - Close commissioning window
                                """),
                TestStep(4, "DUT is put in Commissioning Mode using Open Commissioning Window command.", """
                                - DUT starts advertising Commissionable Node Discovery service through DNS-SD
                                - Verify DUT Commissionable Node Discovery service advertisements
                                - Close commissioning window
                                """),
                TestStep(5, "Check if DUT is in Extended Discovery mode.", """
                                - Check if DUT is advertising Commissionable Node Discovery services through DNS-SD
                                - If so:
                                    - Verify DUT Commissionable Node Discovery service advertisements
                                    """),
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

    @staticmethod
    async def get_commissionable_service_ptr_record() -> Optional[PtrRecord]:
        mdns = MdnsDiscovery()

        # Browse for DUT's 'Commissionable Service'
        await mdns.discover(
            service_types=[MdnsServiceType.COMMISSIONABLE.value],
            log_output=True
        )

        # Check for the presence of DUT's comissionable service PTR record
        ptr_records = next(iter(mdns._discovered_services.values()), [])
        ptr_records_discovered = len(ptr_records) > 0

        if ptr_records_discovered:
            return ptr_records[0]

        return None

    async def close_commissioning_window(self) -> None:
        revoke_cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.default_controller.SendCommand(nodeid=self.dut_node_id,
                                                  endpoint=0,
                                                  payload=revoke_cmd,
                                                  timedRequestTimeoutMs=6000)
        sleep(1)  # Give some time for failsafe cleanup scheduling

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

    async def _get_verify_srv_record(self, service_name: str) -> MdnsServiceInfo:
        # TH performs a SRV record query against the 'Commissionable Service' service name
        srv_record = await MdnsDiscovery().get_srv_record(
            service_name=service_name,
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # Verify SRV record is returned
        srv_record_returned = srv_record is not None and srv_record.service_name == service_name
        asserts.assert_true(srv_record_returned, "SRV record was not returned")

        # Verify DUT's 'Commissionable Service' DNS-SD instance name is a 64-bit randomly
        # selected ID expressed as a sixteen-char hex string with capital letters
        assert_valid_commissionable_instance_name(srv_record.instance_name)

        # Verify DUT's 'Commissionable Service' service type is '_matterc._udp' and service domain '.local.'
        assert_is_commissionable_type(srv_record.service_type)

        # Verify target hostname is derived from the 48bit or 64bit MAC address
        # expressed as a twelve or sixteen capital letter hex string
        assert_valid_hostname(srv_record.hostname)

        return srv_record

    async def _verify_commissionable_subtypes(self, srv_service_name: str) -> None:
        # TH performs a browse for the 'Commissionable Service' subtypes
        subtypes = await MdnsDiscovery().get_commissionable_subtypes(log_output=True)

        # *** LONG DISCRIMINATOR SUBTYPE ***
        # Validate that the 'Long Discriminator' subtype is a 12-bit variable
        # length decimal number in ASCII text, omitting any leading zeros
        long_discriminator_subtype = next((s for s in subtypes if s.startswith('_L')), None)
        asserts.assert_is_not_none(long_discriminator_subtype, "Long discriminator must be present.")
        assert_valid_long_discriminator_subtype(long_discriminator_subtype)

        # Verify that the 'Long Discriminator' subtype PTR record is present
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[long_discriminator_subtype],
            log_output=True
        )
        asserts.assert_greater(len(ptr_records), 0, "Long Discriminator subtype PTR record must be present.")

        # Verify that the 'Long Discriminator' subtype PTR record's
        # 'service_name' is the same as the SRV record 'service_name'
        asserts.assert_equal(ptr_records[0].service_name, srv_service_name,
                             "Long Discriminator subtype PTR record service name must be equal to the SRV record service name.")

        # *** SHORT DISCRIMINATOR SUBTYPE ***
        # Validate that the 'Short Discriminator' subtype is a 4-bit variable
        # length decimal number in ASCII text, omitting any leading zeros
        short_discriminator_subtype = next((s for s in subtypes if s.startswith('_S')), None)
        asserts.assert_is_not_none(short_discriminator_subtype, "Short discriminator must be present.")
        assert_valid_short_discriminator_subtype(short_discriminator_subtype)

        # Verify that the 'Short Discriminator' subtype PTR record is present
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[short_discriminator_subtype],
            log_output=True
        )
        asserts.assert_greater(len(ptr_records), 0, "Short Discriminator subtype PTR record must be present.")

        # Verify that the 'Short Discriminator' subtype PTR record's
        # 'service_name' is the same as the SRV record's 'service_name'
        asserts.assert_equal(ptr_records[0].service_name, srv_service_name,
                             "Short Discriminator subtype PTR record service name must be equal to the SRV record service name.")

        # *** IN COMMISSIONING MODE SUBTYPE ***
        # Verify presence of the _CM subtype
        cm_subtype = f"_CM._sub.{MdnsServiceType.COMMISSIONABLE.value}"
        asserts.assert_in(cm_subtype, subtypes, f"'{cm_subtype}' subtype must be present.")

        # Verify that the 'In Commissioning Mode' _CM subtype PTR record is present
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[cm_subtype],
            log_output=True
        )
        asserts.assert_greater(len(ptr_records), 0, "'In Commissioning Mode' subtype PTR record must be present.")

        # Verify that the 'In Commissioning Mode' _CM subtype PTR record's
        # 'service_name' is the same as the SRV record's 'service_name'
        asserts.assert_equal(ptr_records[0].service_name, srv_service_name,
                             "'In Commissioning Mode' subtype PTR record service name must be equal to the SRV record service name.")

        # *** VENDOR SUBTYPE ***
        # If the Vendor subtype is present, validate it's a 16-bit variable
        # length decimal number in ASCII text, omitting any leading zeros
        vendor_subtype = next((s for s in subtypes if s.startswith('_V')), None)
        if vendor_subtype:
            assert_valid_vendor_subtype(vendor_subtype)

            # If the Vendor subtype PTR record is present, verify that its
            # 'service_name' is the same as the SRV record's 'service_name'
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[vendor_subtype],
                log_output=True
            )
            if len(ptr_records) > 0:
                asserts.assert_equal(ptr_records[0].service_name, srv_service_name,
                                     "Vendor subtype PTR record service name must be equal to the SRV record service name.")

        # *** DEVTYPE SUBTYPE ***
        # If the Devtype subtype is present, validate it's a 32-bit variable
        # length decimal number in ASCII text, omitting any leading zeros
        devtype_subtype = next((s for s in subtypes if s.startswith('_T')), None)
        if devtype_subtype:
            assert_valid_devtype_subtype(devtype_subtype)

            # If the Devtype subtype PTR record is present, verify that the its
            # 'service_name' is the same as the SRV record's 'service_name'
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[devtype_subtype],
                log_output=True
            )
            if len(ptr_records) > 0:
                asserts.assert_equal(ptr_records[0].service_name, srv_service_name,
                                     "Devtype subtype PTR record service name must be equal to the SRV record service name.")

    async def _verify_txt_record_keys(self, service_name: str, expected_cm: str) -> None:
        # TH performs a TXT record query against the 'Commissionable Service' service name
        # The device may omit the TXT record if there are no mandatory TXT keys
        txt_record = await MdnsDiscovery().get_txt_record(
            service_name=service_name,
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # Verify that the TXT record, when required, is returned and is non-empty
        txt_record_required = self.supports_icd or self.supports_tcp
        txt_record_returned = (txt_record is not None) and (len(txt_record.txt) > 0)
        asserts.assert_true((not txt_record_required) or txt_record_returned,
                            "TXT record is required and was not returned or contains no values")

        # If the TXT record is returned, verify the TXT keys
        if txt_record_returned:

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
                                     f"Expected 'CM' key value must be '0' when 'CM' key is not present in the TXT record.")

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

    async def verify_commissionable_node_advertisements(self, service_name: str, expected_cm: str) -> None:
        # Verify SRV record advertisements
        srv_record = await self._get_verify_srv_record(service_name)

        # Verify commissionable subtype advertisements
        await self._verify_commissionable_subtypes(srv_record.service_name)

        # Verify TXT record keys advertisements
        await self._verify_txt_record_keys(service_name, expected_cm)

        # *** AAAA RECORD CHECKS ***
        # **************************
        # TH performs a query for the AAAA record against the target
        # hostname listed in the Commissionable Service SRV record.
        quada_records = await MdnsDiscovery().get_quada_records(hostname=srv_record.hostname, log_output=True)

        # Verify that at least 1 AAAA record is returned for each IPv6 a address
        asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{srv_record.hostname}'")

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    @async_test_body
    async def test_TC_SC_4_1(self):
        self.endpoint = self.get_endpoint(default=1)
        self.supports_icd = False
        self.supports_lit = False
        self.supports_tcp = self.check_pics(TCP_PICS_STR)
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

        # *** STEP 4 ***
        # Check if the LITS feature is supported
        # If 'supports_icd' is True, TH reads the 'FeatureMap' attribute from the ICD Management cluster on EP0
        # If the LITS feature is set, set 'supports_lit' to True, otherwise, set 'supports_lit' to False
        self.step(3)
        if self.supports_icd:
            feature_map = await self.get_icd_feature_map()
            LITS = Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport
            self.supports_lit = bool(feature_map & LITS == LITS)
            logging.info(f"\n\n\t** supports_lit: {self.supports_lit}\n")

        # *** STEP 3 ***
        # If DUT supports Open Basic Commissioning Window, put it in Commissioning Mode using
        # Open Basic Commissioning Window command.
        #   - DUT starts advertising Commissionable Node Discovery service through DNS-SD.
        # self.step(3)
        # Check if DUT supports Open Basic Commissioning Window
        supports_obcw = await self.feature_guard(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            cluster=Clusters.AdministratorCommissioning,
            feature_int=Clusters.AdministratorCommissioning.Bitmaps.Feature.kBasic)        

        if supports_obcw:
            logging.info("\n\n\t ** Open Basic Commissioning Window supported\n")
            await self.default_controller.SendCommand(
                nodeid=self.dut_node_id,
                endpoint=0,
                payload=obcw_cmd,
                timedRequestTimeoutMs=6000
            )

            # TH performs a browse for the 'Commissionable Service' PTR record of type '_matterc._udp.local.'
            commissionable_service_ptr = await self.get_commissionable_service_ptr_record()

            # Verify PTR record is returned
            asserts.assert_is_not_none(commissionable_service_ptr, "DUT's 'Commissionable Service' must be present.")

            # Verify DUT Commissionable Node Discovery service advertisements
            await self.verify_commissionable_node_advertisements(service_name=commissionable_service_ptr.service_name, expected_cm="1")

            # Close commissioning window
            await self.close_commissioning_window()
        else:
            logging.info("\n\n\t ** Open Basic Commissioning Window unsupported, skipping step.\n")

        # *** STEP 4 ***
        # DUT is put in Commissioning Mode using Open Commissioning Window command and
        # starts advertising Commissionable Node Discovery service through DNS-SD
        self.step(4)
        await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id,
            timeout=180,
            iteration=10000,
            discriminator=3840,
            option=1
        )

        # TH performs a browse for the 'Commissionable Service' PTR record of type '_matterc._udp.local.'
        commissionable_service_ptr = await self.get_commissionable_service_ptr_record()

        # Verify PTR record is returned
        asserts.assert_is_not_none(commissionable_service_ptr, "DUT's commissionable node services not present")

        # Verify DUT Commissionable Node Discovery service advertisements
        await self.verify_commissionable_node_advertisements(service_name=commissionable_service_ptr.service_name, expected_cm="2")

        # Close commissioning window
        await self.close_commissioning_window()

        # *** STEP 5 ***
        # Check if DUT Extended Discovery mode is active
        self.step(5)
        # TH performs a browse for the 'Commissionable Service' PTR record of type '_matterc._udp.local.'
        commissionable_service_ptr = await self.get_commissionable_service_ptr_record()

        # If DUT's 'Commissionable Service' is present, Extended Discovery mode is active'
        extended_discovery_mode = commissionable_service_ptr is not None
        logging.info(f"DUT Extended Discovery mode active: {extended_discovery_mode}")

        if extended_discovery_mode:
            # Verify DUT Commissionable Node Discovery service advertisements
            await self.verify_commissionable_node_advertisements(service_name=commissionable_service_ptr.service_name, expected_cm="0")


if __name__ == "__main__":
    default_matter_test_main()
