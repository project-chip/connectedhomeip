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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${lit-icd}
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
#   run2:
#     app: ${all-clusters}
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

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import (assert_valid_hostname, assert_valid_icd_key, assert_valid_ipv6_addresses,
                                          assert_valid_sai_key, assert_valid_sat_key, assert_valid_sii_key, assert_valid_t_key)
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

'''
Purpose
The purpose of this test case is to verify that a Matter node is discoverable
and can advertise its services in a Matter network.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#343-tc-sc-43-discovery-dut_commissionee
'''

TCP_PICS_STR = "MCORE.SC.TCP"
ONE_HOUR_IN_MS = 3600000
MAX_SAT_VALUE = 65535
MAX_T_VALUE = 6


class TC_SC_4_3(MatterBaseTest):

    def steps_TC_SC_4_3(self):
        return [TestStep(1, "DUT is commissioned on the same fabric as TH.", is_commissioning=True),
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ",
                         "If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false."),
                TestStep(3,
                         "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold.", ""),
                TestStep(4, "If supports_icd is true, TH reads FeatureMap from the ICD Management cluster on EP0. If the LITS feature is set, set supports_lit to true. Otherwise set supports_lit to false.", ""),
                TestStep(5, "TH constructs the instance name for the DUT as the 64-bit compressed Fabric identifier, and the assigned 64-bit Node identifier, each expressed as a fixed-length sixteen-character hexadecimal string, encoded as ASCII (UTF-8) text using capital letters, separated by a hyphen.", ""),
                TestStep(6, "TH performs a query for the SRV record against the qname instance_qname.",
                         "Verify SRV record is returned"),
                TestStep(7, "TH performs a query for the TXT record against the qname instance_qname.",
                         "Verify TXT record is returned if the device supports ICD or TCP. The TXT record MAY be returned if these are not supported, but it is not required."),
                TestStep(8, "TH performs a query for the AAAA record against the target listed in the SRV record",
                         "Verify AAAA record is returned"),
                TestStep(9, "TH verifies the following from the returned records:",
                         "The hostname must be a fixed-length twelve-character (or sixteen-character) hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.. ICD TXT key: • If supports_lit is false, verify that the ICD key is NOT present in the TXT record • If supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII) SII TXT key: • If supports_icd is true and supports_lit is false, set sit_mode to true • If supports_icd is true and supports_lit is true, set sit_mode to true if ICD=0 otherwise set sit_mode to false • If supports_icd is false, set sit_mode to false • If sit_mode is true, verify that the SII key IS present in the TXT record • if the SII key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms) SAI TXT key: • if supports_icd is true, verify that the SAI key is present in the TXT record • If the SAI key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms)"),
                TestStep(10, "TH performs a DNS-SD browse for _I<hhhh>._sub._matter._tcp.local, where <hhhh> is the 64-bit compressed Fabric identifier, expressed as a fixed-length, sixteencharacter hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.",
                         "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                TestStep(11, "TH performs a DNS-SD browse for _matter._tcp.local",
                         "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                ]

    async def get_descriptor_server_list(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_idle_mode_threshhold_ms(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.ActiveModeThreshold
        )

    async def get_icd_feature_map(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.FeatureMap
        )

    def get_dut_instance_name(self, log_result: bool = False) -> str:
        node_id = self.dut_node_id
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'
        if log_result:
            logging.info(f"\n\n\tDUT Instance Name: {instance_name}\n")
        return instance_name

    def get_operational_subtype(self, log_result: bool = False) -> str:
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        operational_subtype = f'_I{compressed_fabric_id:016X}._sub.{MdnsServiceType.OPERATIONAL.value}'
        if log_result:
            logging.info(f"\n\n\tOperational Subtype: {operational_subtype}\n")
        return operational_subtype

    @staticmethod
    def verify_decimal_value(input_value, max_value: int):
        try:
            input_float = float(input_value)
            input_int = int(input_float)

            if str(input_value).startswith("0") and input_int != 0:
                return (False, f"Input ({input_value}) has leading zeros.")

            if input_float != input_int:
                return (False, f"Input ({input_value}) is not an integer.")

            if input_int <= max_value:
                return (True, f"Input ({input_value}) is valid.")
            return (False, f"Input ({input_value}) exceeds the allowed value {max_value}.")
        except ValueError:
            return (False, f"Input ({input_value}) is not a valid decimal number.")

    def verify_t_value(self, operational_record):
        has_t = operational_record and operational_record.txt and 'T' in operational_record.txt
        if not has_t:
            asserts.assert_false(self.check_pics(TCP_PICS_STR),
                                 f"T key must be included if TCP is supported - returned TXT record: {operational_record}")
            return True, 'T is not provided or required'

        t_value = operational_record.txt['T']
        logging.info("T key is present in TXT record, verify if that it is a decimal value with no leading zeros and is less than or equal to 6. Convert the value to a bitmap and verify bit 0 is clear.")
        # Verify t_value is a decimal number without leading zeros and less than or equal to 6
        try:
            assert_valid_t_key(t_value, enforce_provisional=False)

            # Convert to bitmap and verify bit 0 is clear
            T_int = int(t_value)
            if T_int & 1 == 0:
                return True, f"T value ({t_value}) is valid and bit 0 is clear."
            return False, f"Bit 0 is not clear. T value ({t_value})"

            # Check that the value can be either 2, 4 or 6 depending on whether
            # DUT is a TCPClient, TCPServer or both.
            if self.check_pics(TCP_PICS_STR):
                if (T_int & 0x04 != 0):
                    return True, f"T value ({t_value}) represents valid TCP support info."
                return False, f"T value ({t_value}) does not have TCP bits set even though the MCORE.SC.TCP PICS indicates it is required."
            if (T_int & 0x04 != 0):
                return False, f"T value ({t_value}) has the TCP bits set even though the MCORE.SC.TCP PICS is not set."
            return True, f"T value ({t_value}) is valid."
        except ValueError:
            return False, f"T value ({t_value}) is not a valid integer"

    @async_test_body
    async def test_TC_SC_4_3(self):
        supports_icd = None
        supports_lit = None
        active_mode_threshold_ms = None

        # *** STEP 1 ***
        # DUT is commissioned on the same fabric as TH.
        self.step(1)

        # *** STEP 2 ***
        # TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0. If the ICD Management
        # cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
        self.step(2)
        ep0_servers = await self.get_descriptor_server_list()

        # Check if ep0_servers contain the ICD Management cluster ID (0x0046)
        supports_icd = Clusters.IcdManagement.id in ep0_servers
        logging.info(f"supports_icd: {supports_icd}")

        # *** STEP 3 ***
        # If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves
        # as active_mode_threshold.
        self.step(3)
        if supports_icd:
            active_mode_threshold_ms = await self.get_idle_mode_threshhold_ms()
        logging.info(f"active_mode_threshold_ms: {active_mode_threshold_ms}")

        # *** STEP 4 ***
        # If supports_icd is true, TH reads FeatureMap from the ICD Management cluster on EP0. If the LITS feature
        # is set, set supports_lit to true. Otherwise set supports_lit to false.
        self.step(4)
        if supports_icd:
            feature_map = await self.get_icd_feature_map()
            LITS = Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport
            supports_lit = bool(feature_map & LITS == LITS)
            logging.info(f"kLongIdleTimeSupport set: {supports_lit}")

        # *** STEP 5 ***
        # TH constructs the instance name for the DUT as the 64-bit compressed Fabric identifier, and the
        # assigned 64-bit Node identifier, each expressed as a fixed-length sixteen-character hexadecimal
        # string, encoded as ASCII (UTF-8) text using capital letters, separated by a hyphen.
        self.step(5)
        instance_name = self.get_dut_instance_name(log_result=True)
        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"

        # *** STEP 6 ***
        # TH performs a query for the SRV record against the qname instance_qname.
        self.step(6)
        mdns = MdnsDiscovery()
        srv_record = await mdns.get_srv_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            log_output=True
        )

        # Verify SRV record is returned
        srv_record_returned = srv_record is not None and srv_record.service_name == instance_qname
        asserts.assert_true(srv_record_returned, "SRV record was not returned")

        # *** STEP 7 ***
        # TH performs a query for the TXT record against the qname instance_qname.
        # Verify TXT record is returned
        self.step(7)
        txt_record = await mdns.get_txt_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            log_output=True
        )

        # Request the TXT record. The device may opt not to return a TXT record if there are no mandatory TXT keys
        txt_record_returned = txt_record is not None and txt_record.txt is not None and bool(
            txt_record.txt)
        txt_record_required = supports_icd or self.check_pics(TCP_PICS_STR)

        if txt_record_required:
            asserts.assert_true(txt_record_returned, "TXT record is required and was not returned or contains no values")

        # *** STEP 8 ***
        # TH performs a query for the AAAA record against the target listed in the SRV record.
        self.step(8)
        quada_records = await mdns.get_quada_records(
            hostname=srv_record.hostname,
            log_output=True
        )

        # Verify AAAA record is returned
        asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{srv_record.hostname}'")

        # # *** STEP 9 ***
        # TH verifies the following from the returned records: The hostname must be a fixed-length twelve-character (or sixteen-character)
        # hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.. ICD TXT key: • If supports_lit is false, verify that the
        # ICD key is NOT present in the TXT record • If supports_lit is true, verify the ICD key IS present in the TXT record, and it has the
        # value of 0 or 1 (ASCII) SII TXT key: • If supports_icd is true and supports_lit is false, set sit_mode to true • If supports_icd is
        # true and supports_lit is true, set sit_mode to true if ICD=0 otherwise set sit_mode to false • If supports_icd is false, set
        # sit_mode to false • If sit_mode is true, verify that the SII key IS present in the TXT record • if the SII key is present, verify
        # it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms) SAI TXT key: • if supports_icd is true,
        # verify that the SAI key is present in the TXT record • If the SAI key is present, verify it is a decimal value with no leading
        # zeros and is less than or equal to 3600000 (1h in ms)
        self.step(9)

        def txt_has_key(key: str):
            return txt_record_returned and key in txt_record.txt

        # Verify hostname character length (12 or 16)
        assert_valid_hostname(srv_record.hostname)

        # ICD TXT KEY
        if supports_lit:
            logging.info("supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII).")

            # Verify the ICD key IS present
            asserts.assert_true(txt_has_key('ICD'), "ICD key is NOT present in the TXT record.")

            # Verify it has the value of 0 or 1 (ASCII)
            assert_valid_icd_key(txt_record.txt['ICD'])
        else:
            logging.info("supports_lit is false, verify that the ICD key is NOT present in the TXT record.")
            if txt_record_returned:
                asserts.assert_not_in('ICD', txt_record.txt, "ICD key is present in the TXT record.")

        # SII TXT KEY
        if supports_icd and not supports_lit:
            sit_mode = True

        if supports_icd and supports_lit:
            if int(txt_record.txt['ICD']) == 0:
                sit_mode = True
            else:
                sit_mode = False

        if not supports_icd:
            sit_mode = False

        if sit_mode:
            logging.info("sit_mode is True, verify the SII key IS present.")
            asserts.assert_true(txt_has_key('SII'), "SII key is NOT present in the TXT record.")

            logging.info("Verify SII value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            assert_valid_sii_key(txt_record.txt['SII'])

        # SAI TXT KEY
        if supports_icd:
            logging.info("supports_icd is True, verify the SAI key IS present.")
            asserts.assert_true(txt_has_key('SAI'), "SAI key is NOT present in the TXT record.")

            logging.info("Verify SAI value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            assert_valid_sai_key(txt_record.txt['SAI'])

        # SAT TXT KEY
        if txt_has_key('SAT'):
            logging.info(
                "SAT key is present in TXT record, verify that it is a decimal value with no leading zeros and is less than or equal to 65535.")
            assert_valid_sat_key(txt_record.txt['SAT'])

            if supports_icd:
                logging.info("supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                asserts.assert_equal(int(txt_record.txt['SAT']), active_mode_threshold_ms)

        # T TXT KEY
        result, message = self.verify_t_value(txt_record)
        asserts.assert_true(result, message)

        # Verify the AAAA records contain a valid IPv6 address
        logging.info("Verify the AAAA record contains a valid IPv6 address")
        ipv6_addresses = [f"{r.address}%{r.interface}" for r in quada_records]
        assert_valid_ipv6_addresses(ipv6_addresses)

        # # *** STEP 10 ***
        # TH performs a DNS-SD browse for _I<hhhh>._sub._matter._tcp.local, where <hhhh> is the 64-bit compressed
        # Fabric identifier, expressed as a fixed-length, sixteencharacter hexadecimal string, encoded as ASCII (UTF-8)
        # text using capital letters.
        self.step(10)
        op_sub_type = self.get_operational_subtype(log_result=True)
        ptr_records = await mdns.get_ptr_records(
            service_types=[op_sub_type],
            log_output=True,
        )

        # Verify DUT returns a PTR record with DNS-SD instance name set to instance_name
        asserts.assert_true(
            any(r.instance_name == instance_name for r in ptr_records),
            f"No PTR record with DNS-SD instance name '{instance_name}' was found."
        )

        # # *** STEP 11 ***
        # TH performs a DNS-SD browse for _matter._tcp.local
        self.step(11)
        ptr_records = await mdns.get_ptr_records(
            service_types=[MdnsServiceType.OPERATIONAL.value],
            log_output=True,
        )

        # Verify DUT returns a PTR record with DNS-SD instance name set to instance_name
        asserts.assert_true(
            any(r.instance_name == instance_name for r in ptr_records),
            f"No PTR record with DNS-SD instance name '{instance_name}' was found."
        )


if __name__ == "__main__":
    default_matter_test_main()
