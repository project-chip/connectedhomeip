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
from typing import Any, Optional

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.data_classes.ptr_record import PtrRecord
from mdns_discovery.utils.asserts import (assert_is_commissionable_type, assert_valid_cm_key,
                                          assert_valid_commissionable_instance_name, assert_valid_d_key,
                                          assert_valid_devtype_subtype, assert_valid_dn_key, assert_valid_dt_key,
                                          assert_valid_hostname, assert_valid_long_discriminator_subtype, assert_valid_ph_key,
                                          assert_valid_pi_key, assert_valid_ri_key, assert_valid_sai_key, assert_valid_sat_key,
                                          assert_valid_short_discriminator_subtype, assert_valid_sii_key, assert_valid_t_key,
                                          assert_valid_vendor_subtype, assert_valid_vp_key, assert_valid_icd_key)
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from time import sleep
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
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ","""
                                - If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
                                - If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold.                         
                                """),
                TestStep(3, "DUT is put in Commissioning Mode using Open Basic Commissioning Window command.", """
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

        # Browse for DUT's commissionable service
        await mdns.discover(
            service_types=[MdnsServiceType.COMMISSIONABLE.value],
            log_output=True
        )

        # Filter for commissionable service type
        commissionable_services_ptr = mdns._discovered_services.get(MdnsServiceType.COMMISSIONABLE.value, [])

        # Verify presence of DUT's comissionable service PTR record and return if present
        if len(commissionable_services_ptr) > 0:
            return commissionable_services_ptr[0]

        return None

    async def close_commissioning_window(self) -> None:
        revoke_cmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.default_controller.SendCommand(nodeid=self.dut_node_id,
                                                  endpoint=0,
                                                  payload=revoke_cmd,
                                                  timedRequestTimeoutMs=6000)
        sleep(1) # Give some time for failsafe cleanup scheduling

    def verify_t_value(self, record):
        has_t = record and record.txt and 'T' in record.txt
        if not has_t:
            asserts.assert_false(self.check_pics(TCP_PICS_STR),
                                 f"T key must be included if TCP is supported - returned TXT record: {record}")
            return True, 'T is not provided or required'

        t_value = record.txt['T']
        logging.info("T key is present in TXT record, verify if that it is a decimal value with no leading zeros and is less than or equal to 6. Convert the value to a bitmap and verify bit 0 is clear.")
        # Verify t_value is a decimal number without leading zeros and less than or equal to 6
        try:
            assert_valid_t_key(t_value, enforce_provisional=False)

            # Convert to bitmap and verify bit 0 is clear
            T_int = int(t_value)
            if T_int & 1 == 0:
                return True, f"T value ({t_value}) is valid and bit 0 is clear."
            else:
                return False, f"Bit 0 is not clear. T value ({t_value})"

            # Check that the value can be either 2, 4 or 6 depending on whether
            # DUT is a TCPClient, TCPServer or both.
            if self.check_pics(TCP_PICS_STR):
                if (T_int & 0x04 != 0):
                    return True, f"T value ({t_value}) represents valid TCP support info."
                else:
                    return False, f"T value ({t_value}) does not have TCP bits set even though the MCORE.SC.TCP PICS indicates it is required."
            else:
                if (T_int & 0x04 != 0):
                    return False, f"T value ({t_value}) has the TCP bits set even though the MCORE.SC.TCP PICS is not set."
                else:
                    return True, f"T value ({t_value}) is valid."
        except ValueError:
            return False, f"T value ({t_value}) is not a valid integer"

    async def verify_commissionable_node_advertisements(self, service_name: str, expected_cm: str) -> None:
        mdns = MdnsDiscovery()

        # *** SRV RECORD CHECKS ***
        # *************************
        # TH performs a query for the SRV record against the commissionable service service name.
        srv_record = await mdns.get_srv_record(
            service_name=service_name,
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # Verify SRV record is returned
        srv_record_returned = srv_record is not None and srv_record.service_name == service_name
        asserts.assert_true(srv_record_returned, "SRV record was not returned")        

        # Verify DUT's commissionable service is a valid DNS-SD instance name
        # (64-bit randomly selected ID expressed as a sixteen-char hex string with capital letters)
        assert_valid_commissionable_instance_name(srv_record.instance_name)

        # Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        assert_is_commissionable_type(srv_record.service_type)

        # Verify target hostname is derived from the 48bit or 64bit MAC address
        # expressed as a twelve or sixteen capital letter hex string. If the MAC
        # is randomized for privacy, the randomized version must be used each time.
        assert_valid_hostname(srv_record.hostname)


        # *** COMMISSIONABLE SUBTYPES CHECKS ***
        # **************************************
        # Get commissionable subtypes
        subtypes = await MdnsDiscovery().get_commissionable_subtypes(log_output=True)

        # *** LONG DISCRIMINATOR SUBTYPE ***
        # Validate that the commissionable long discriminator subtype is a 12-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        long_discriminator_subtype = next((s for s in subtypes if s.startswith('_L')), None)
        asserts.assert_is_not_none(long_discriminator_subtype, "Long discriminator must be present.")
        assert_valid_long_discriminator_subtype(long_discriminator_subtype)

        # Get Long Discriminator subtype PTR record
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[long_discriminator_subtype],
            log_output=True
        )

        # Verify Long Discriminator subtype PTR record is present
        asserts.assert_greater(len(ptr_records), 0, "Long Discriminator subtype PTR record must be present.")
        
        # Verify that the Long Discriminator subtype PTR record's
        # 'service_name'is the same as the SRV record 'service_name'
        if len(ptr_records) > 0:
            asserts.assert_equal(ptr_records[0].service_name, srv_record.service_name,
                                "Long Discriminator subtype PTR record service name must be equal to the SRV record service name.")

        # *** SHORT DISCRIMINATOR SUBTYPE ***
        # Validate that the short commissionable discriminator subtype is a 4-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        short_discriminator_subtype = next((s for s in subtypes if s.startswith('_S')), None)
        asserts.assert_is_not_none(short_discriminator_subtype, "Short discriminator must be present.")
        assert_valid_short_discriminator_subtype(short_discriminator_subtype)

        # Get Short Discriminator subtype PTR record
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[short_discriminator_subtype],
            log_output=True
        )

        # Verify Short Discriminator subtype PTR record is present
        asserts.assert_greater(len(ptr_records), 0, "Short Discriminator subtype PTR record must be present.")

        # Verify that the Short Discriminator subtype PTR record's
        # 'service_name'is the same as the SRV record 'service_name'
        if len(ptr_records) > 0:
            asserts.assert_equal(ptr_records[0].service_name, srv_record.service_name,
                                "Short Discriminator subtype PTR record service name must be equal to the SRV record service name.")

        # *** IN COMMISSIONING MODE SUBTYPE ***
        # Verify presence of the _CM subtype
        cm_subtype = f"_CM._sub.{MdnsServiceType.COMMISSIONABLE.value}"
        asserts.assert_in(cm_subtype, subtypes, f"'{cm_subtype}' subtype must be present.")

        # Get In Commissioning Mode subtype PTR record
        ptr_records = await MdnsDiscovery().get_ptr_records(
            service_types=[cm_subtype],
            log_output=True
        )

        # Verify In Commissioning Mode subtype PTR record is present
        asserts.assert_greater(len(ptr_records), 0, "In Commissioning Mode subtype PTR record must be present.")

        # *** VENDOR SUBTYPE ***
        # If the commissionable vendor subtype is present, validate it's a
        # 16-bit vendor id, encoded as a variable-length decimal number in
        # ASCII text, omitting any leading zeros
        vendor_subtype = next((s for s in subtypes if s.startswith('_V')), None)
        if vendor_subtype:
            assert_valid_vendor_subtype(vendor_subtype)

            # Get Vendor subtype PTR record
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[vendor_subtype],
                log_output=True
            )

            # If Vendor subtype PTR record is present, verify the its
            # 'service_name'is the same as the SRV record 'service_name'
            if len(ptr_records) > 0:
                asserts.assert_equal(ptr_records[0].service_name, srv_record.service_name,
                                    "Vendor subtype PTR record service name must be equal to the SRV record service name.")

        # *** DEVTYPE SUBTYPE ***
        # If the commissionable devtype subtype is present, validate it's a
        # 32-bit variable length decimal number in ASCII without leading zeros
        devtype_subtype = next((s for s in subtypes if s.startswith('_T')), None)
        if devtype_subtype:
            assert_valid_devtype_subtype(devtype_subtype)

            # Get Devtype subtype PTR record
            ptr_records = await MdnsDiscovery().get_ptr_records(
                service_types=[devtype_subtype],
                log_output=True
            )

            # If Devtype subtype PTR record is present, verify the its
            # 'service_name'is the same as the SRV record 'service_name'
            if len(ptr_records) > 0:
                asserts.assert_equal(ptr_records[0].service_name, srv_record.service_name,
                                    "Devtype subtype PTR record service name must be equal to the SRV record service name.")


        # *** TXT RECORD CHECKS ***
        # *************************
        # TH performs a query for the TXT record against the commissionable service service name.
        txt_record = await mdns.get_txt_record(
            service_name=service_name,
            service_type=MdnsServiceType.COMMISSIONABLE.value,
            log_output=True
        )

        # The device may omit the TXT record if there are no mandatory TXT keys
        txt_record_required = self.supports_icd or self.check_pics(TCP_PICS_STR)
        txt_record_returned = (txt_record is not None) and (len(txt_record.txt) > 0)

        # Verify that the TXT record, when required, is returned and is non-empty
        asserts.assert_true((not txt_record_required) or txt_record_returned, "TXT record is required and was not returned or contains no values")

        if txt_record_returned:

            # *** ICD KEY ***
            icd_key: str | None = None
            if self.supports_lit:
                # Verify that TXT key 'ICD' exists
                asserts.assert_true('ICD' in txt_record.txt, "TXT key 'ICD' must be present.")

                # Get the value
                icd_key = txt_record.txt.get('ICD')

                # Verify it’s not empty
                asserts.assert_true(icd_key, "TXT key 'ICD' is present but has no value.")

                # Verify it has the value of 0 or 1 (ASCII)
                assert_valid_icd_key(icd_key)
            else:
                asserts.assert_not_in('ICD', txt_record.txt, "TXT key 'ICD' must NOT be present.")

            # Set sit_mode = True when:
            #   - supports_icd is True and supports_lit is False.
            #   - supports_icd is True and supports_lit is True and ICD == '0'.
            # Set sit_mode = False when:
            #   - supports_icd is False.
            #   - supports_icd is True and supports_lit is True and ICD == '1'.
            sit_mode = self.supports_icd and (not self.supports_lit or icd_key == '0')
            logging.info(f"\n\n\t** sit_mode: {sit_mode}\n")

            # *** SII KEY ***
            if sit_mode:
                # Verify that TXT key 'SII' exists if Session Idle Timeout mode is True
                asserts.assert_true('SII' in txt_record.txt, "TXT key 'SII' must be present.")

                # Get the value
                sii_key = txt_record.txt['SII']

                # Verify it’s not empty
                asserts.assert_true(sii_key, "TXT key 'SII' is present but has no value.")

                # Verify SII key is an unsigned integer with units of milliseconds
                # and shall be encoded as a variable length decimal number in ASCII,
                # omitting leading zeros. Shall not exceed 3600000.
                assert_valid_sii_key(sii_key)

            # *** SAI KEY ***
            if self.supports_icd:
                # Verify that TXT key 'SAI' exists if 'supports_icd' is True
                asserts.assert_true('SAI' in txt_record.txt, "TXT key 'SAI' must be present.")

                # Get the value
                sai_key = txt_record.txt['SAI']

                # Verify it’s not empty
                asserts.assert_true(sai_key, "TXT key 'SAI' is present but has no value.")

                # Verify SAI key is an unsigned integer with units of milliseconds
                # and shall be encoded as a variable length decimal number in ASCII,
                # omitting leading zeros. Shall not exceed 3600000.
                assert_valid_sai_key(sai_key)

            # *** SAT KEY ***
            # - If the SAT key is present, verify that it is a decimal value with
            #   no leading zeros and is less than or equal to 65535
            if 'SAT' in txt_record.txt:
                sat_key = txt_record.txt['SAT']
                if sat_key:
                    assert_valid_sat_key(sat_key)

                # - If the SAT key is present and supports_icd is true, verify that
                #   the value is equal to active_mode_threshold
                if self.supports_icd:
                    logging.info("supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                    asserts.assert_equal(int(sat_key), self.active_mode_threshold_ms)

            # Verify that the SAI key is present if the SAT key is present
            asserts.assert_true(
                'SAT' not in txt_record.txt
                or 'SAI' in txt_record.txt,
                "SAI key must be present if SAT key is present."
            )

            # *** D KEY ***
            # Verify D key is present, which represents the discriminator
            # and must be encoded as a variable-length decimal value with up to 4
            # digits omitting any leading zeros
            d_key = txt_record.txt.get('D')
            asserts.assert_is_not_none(d_key, "D key must be present.")
            assert_valid_d_key(d_key)

            # *** VP KEY ***
            # If VP key is present, verify it contain at least Vendor ID
            # and if Product ID is present, values must be separated by a + sign
            if 'VP' in txt_record.txt:
                vp_key = txt_record.txt['VP']
                if vp_key:
                    assert_valid_vp_key(vp_key)

            # *** T KEY ***
            result, message = self.verify_t_value(txt_record)
            asserts.assert_true(result, message)

            # Verify that the CM key is present and is equal to the expected
            # CM value or ommitted if DUT is in Extended Discovery mode
            if 'CM' in txt_record.txt:
                cm_key = txt_record.txt['CM']
                assert_valid_cm_key(cm_key)
                asserts.assert_true(cm_key == expected_cm, f"CM key must be '{expected_cm}', got '{cm_key}'")
            else:
                # When the DUT is in Extended Discovery mode and the
                # CM key is not present, it's equivalent to CM=0
                extended_discovery_mode = expected_cm == "0"

                # Fail test only when CM key is not present
                # and DUT is not in Extended Discovery mode
                if not extended_discovery_mode:
                    asserts.fail(f"CM key not present, was expecting CM='{expected_cm}'")

            # If the DT key is present, it must contain the device type identifier from
            # Data Model Device Types and must be encoded as a variable length decimal
            # ASCII number without leading zeros
            if 'DT' in txt_record.txt:
                dt_key = txt_record.txt['DT']
                assert_valid_dt_key(dt_key)

            # If the DN key is present, DN key must be a UTF-8 encoded string with a maximum length of 32B
            if 'DN' in txt_record.txt:
                dn_key = txt_record.txt['DN']
                assert_valid_dn_key(dn_key)

            # If the RI key is present, key RI must include the Rotating Device Identifier
            # encoded as an uppercase string with a maximum length of 100 chars (each octet
            # encoded as a 2-digit hex number, max 50 octets)
            if 'RI' in txt_record.txt:
                ri_key = txt_record.txt['RI']
                assert_valid_ri_key(ri_key)

            # If the PH key is present, key PH must be encoded as a variable-length decimal number
            # in ASCII text, omitting any leading zeros. If present value must be different of 0
            if 'PH' in txt_record.txt:
                ph_key = txt_record.txt['PH']
                assert_valid_ph_key(ph_key)

            # TODO: Fix PI key present but null/None ??
            # If the PI key is present, key PI must be encoded as a valid UTF-8 string
            # with a maximum length of 128 bytes
            # if 'PI' in txt_record.txt:
            #     pi_key = txt_record.txt['PI']
                # assert_valid_pi_key(pi_key)
        else:
            logging.info("TXT record NOT required.")


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
        obcw_cmd = Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180)

        # *** STEP 1 ***
        # DUT is Commissioned.
        self.step(1)

        # Check if DUT supports Open Basic Commissioning Window
        supports_obcw = await self.feature_guard(
            endpoint=ROOT_NODE_ENDPOINT_ID,
            cluster=Clusters.AdministratorCommissioning,
            feature_int=Clusters.AdministratorCommissioning.Bitmaps.Feature.kBasic)

        # *** STEP 2 ***
        # TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0.
        #   - If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
        #   - If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold_ms.
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
        # If supports_icd is true, TH reads FeatureMap from the ICD Management cluster on EP0. If the LITS feature
        # is set, set supports_lit to true. Otherwise set supports_lit to false.
        # self.step(4)
        if self.supports_icd:
            feature_map = await self.get_icd_feature_map()
            LITS = Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport
            self.supports_lit = bool(feature_map & LITS == LITS)
            logging.info(f"\n\n\t** supports_lit: {self.supports_lit}\n")

        # *** STEP 3 ***
        # If DUT supports Open Basic Commissioning Window, put it in Commissioning Mode using
        # Open Basic Commissioning Window command.
        #   - DUT starts advertising Commissionable Node Discovery service through DNS-SD.
        self.step(3)
        if supports_obcw:
            logging.info(f"\n\n\t ** Open Basic Commissioning Window supported\n")
            await self.default_controller.SendCommand(
                nodeid=self.dut_node_id,
                endpoint=0,
                payload=obcw_cmd,
                timedRequestTimeoutMs=6000
            )

            # TH performs a browse for the commissionable service PTR record of type '_matterc._udp.local.'
            commissionable_service_ptr = await self.get_commissionable_service_ptr_record()

            # Verify PTR record is returned
            asserts.assert_is_not_none(commissionable_service_ptr, "DUT's commissionable service not present")

            # Verify DUT Commissionable Node Discovery service advertisements
            await self.verify_commissionable_node_advertisements(service_name=commissionable_service_ptr.service_name, expected_cm="1")

            # Close commissioning window
            await self.close_commissioning_window()
        else:
            logging.info(f"\n\n\t ** Open Basic Commissioning Window unsupported, skipping step.\n")

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

        # TH performs a browse for the commissionable service PTR record of type '_matterc._udp.local.'
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
        # TH performs a browse for the commissionable service PTR record of type '_matterc._udp.local.'
        commissionable_service_ptr = await self.get_commissionable_service_ptr_record()

        # If DUT's commissionable service is present, Extended Discovery mode is active
        extended_discovery_mode = commissionable_service_ptr is not None
        logging.info(f"DUT Extended Discovery mode active: {extended_discovery_mode}")

        if extended_discovery_mode:
            # Verify DUT Commissionable Node Discovery service advertisements
            await self.verify_commissionable_node_advertisements(service_name=commissionable_service_ptr.service_name, expected_cm="0")


if __name__ == "__main__":
    default_matter_test_main()
