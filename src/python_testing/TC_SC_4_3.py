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

import ipaddress
import logging
import re

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import DNSRecordType, MdnsDiscovery, MdnsServiceType
from mobly import asserts
from zeroconf.const import _TYPE_AAAA, _TYPES

'''
Purpose
The purpose of this test case is to verify that a Matter node is discoverable
and can advertise its services in a Matter network.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#343-tc-sc-43-discovery-dut_commissionee
'''


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
                         "Verify TXT record is returned"),
                TestStep(8, "TH performs a query for the AAAA record against the target listed in the SRV record",
                         "Verify AAAA record is returned"),
                TestStep(9, "TH verifies the following from the returned records:",
                         "TH verifies the following from the returned records: The hostname must be a fixed-length twelve-character (or sixteen-character) hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.. ICD TXT key: • If supports_lit is false, verify that the ICD key is NOT present in the TXT record • If supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII) SII TXT key: • If supports_icd is true and supports_lit is false, set sit_mode to true • If supports_icd is true and supports_lit is true, set sit_mode to true if ICD=0 otherwise set sit_mode to false • If supports_icd is false, set sit_mode to false • If sit_mode is true, verify that the SII key IS present in the TXT record • if the SII key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms) SAI TXT key: • if supports_icd is true, verify that the SAI key is present in the TXT record • If the SAI key is present, verify it is a decimal value with no leading zeros and is less than or equal to 3600000 (1h in ms)"),
                TestStep(10, "TH performs a DNS-SD browse for _I<hhhh>._sub._matter._tcp.local, where <hhhh> is the 64-bit compressed Fabric identifier, expressed as a fixed-length, sixteencharacter hexadecimal string, encoded as ASCII (UTF-8) text using capital letters.",
                         "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                TestStep(11, "TH performs a DNS-SD browse for _matter._tcp.local",
                         "Verify DUT returns a PTR record with DNS-SD instance name set to instance_name"),
                ]

    ONE_HOUR_IN_MS = 3600000
    MAX_SAT_VALUE = 65535
    MAX_T_VALUE = 6

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

    def get_dut_instance_name(self) -> str:
        node_id = self.dut_node_id
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'
        return instance_name

    def get_operational_subtype(self) -> str:
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        service_name = f'_I{compressed_fabric_id:016X}._sub.{MdnsServiceType.OPERATIONAL.value}'
        return service_name

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
            else:
                return (False, f"Input ({input_value}) exceeds the allowed value {max_value}.")
        except ValueError:
            return (False, f"Input ({input_value}) is not a valid decimal number.")

    def verify_t_value(self, t_value):
        # Verify t_value is a decimal number without leading zeros and less than or equal to 6
        try:
            T_int = int(t_value)
            if T_int < 0 or T_int > self.MAX_T_VALUE:
                return False, f"T value ({t_value}) is not in the range 0 to 6. ({t_value})"
            if str(t_value).startswith("0") and T_int != 0:
                return False, f"T value ({t_value}) has leading zeros."
            if T_int != float(t_value):
                return False, f"T value ({t_value}) is not an integer."

            # Convert to bitmap and verify bit 0 is clear
            if T_int & 1 == 0:
                return True, f"T value ({t_value}) is valid and bit 0 is clear."
            else:
                return False, f"Bit 0 is not clear. T value ({t_value})"

            # Check that the value can be either 2, 4 or 6 depending on whether
            # DUT is a TCPClient, TCPServer or both.
            if self.check_pics("MCORE.SC.TCP"):
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

    @staticmethod
    def is_ipv6_address(addresses):
        if isinstance(addresses, str):
            addresses = [addresses]

        for address in addresses:
            try:
                # Attempt to create an IPv6 address object. If successful, this is an IPv6 address.
                ipaddress.IPv6Address(address)
                return True, "At least one IPv6 address is present."
            except ipaddress.AddressValueError:
                # If an AddressValueError is raised, the current address is not a valid IPv6 address.
                return False, f"Invalid IPv6 address encountered: {address}, provided addresses: {addresses}"
        return False, "No IPv6 addresses found."

    @staticmethod
    def extract_ipv6_address(text):
        items = text.split(',')
        return items[-1]

    @staticmethod
    def verify_hostname(hostname: str) -> bool:
        # Remove any trailing dot
        if hostname.endswith('.'):
            hostname = hostname[:-1]

        # Remove '.local' suffix if present
        if hostname.endswith('.local'):
            hostname = hostname[:-6]

        # Regular expression to match an uppercase hexadecimal string of 12 or 16 characters
        pattern = re.compile(r'^[0-9A-F]{12}$|^[0-9A-F]{16}$')
        return bool(pattern.match(hostname))

    @async_test_body
    async def test_TC_SC_4_3(self):
        supports_icd = None
        supports_lit = None
        active_mode_threshold_ms = None
        instance_name = None

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
        instance_name = self.get_dut_instance_name()
        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"

        # *** STEP 6 ***
        # TH performs a query for the SRV record against the qname instance_qname.
        # Verify SRV record is returned
        self.step(6)
        mdns = MdnsDiscovery()
        operational_record = await mdns.get_service_by_record_type(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            record_type=DNSRecordType.SRV,
            log_output=True
        )

        # Will be used in Step 8 and 11
        # This is the hostname
        hostname = operational_record.server

        # Verify SRV record is returned
        srv_record_returned = operational_record is not None and operational_record.service_name == instance_qname
        asserts.assert_true(srv_record_returned, "SRV record was not returned")

        # *** STEP 7 ***
        # TH performs a query for the TXT record against the qname instance_qname.
        # Verify TXT record is returned
        self.step(7)
        operational_record = await mdns.get_service_by_record_type(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            record_type=DNSRecordType.TXT,
            log_output=True
        )

        # Verify TXT record is returned and it contains values
        txt_record_returned = operational_record.txt_record is not None and bool(operational_record.txt_record)
        asserts.assert_true(txt_record_returned, "TXT record not returned or contains no values")

        # *** STEP 8 ***
        # TH performs a query for the AAAA record against the target listed in the SRV record.
        # Verify AAAA record is returned
        self.step(8)

        quada_record = await mdns.get_service_by_record_type(
            service_name=hostname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            record_type=DNSRecordType.AAAA,
            log_output=True
        )

        answer_record_type = quada_record.get_type(quada_record.type)
        quada_record_type = _TYPES[_TYPE_AAAA]

        # Verify AAAA record is returned
        asserts.assert_equal(hostname, quada_record.name, f"Server name mismatch: {hostname} vs {quada_record.name}")
        asserts.assert_equal(quada_record_type, answer_record_type,
                             f"Record type should be {quada_record_type} but got {answer_record_type}")

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

        # Verify hostname character length (12 or 16)
        asserts.assert_true(self.verify_hostname(hostname=hostname),
                            f"Hostname for '{hostname}' is not a 12 or 16 character uppercase hexadecimal string")

        # ICD TXT KEY
        if supports_lit:
            logging.info("supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII).")

            # Verify the ICD key IS present
            asserts.assert_in('ICD', operational_record.txt_record, "ICD key is NOT present in the TXT record.")

            # Verify it has the value of 0 or 1 (ASCII)
            icd_value = int(operational_record.txt_record['ICD'])
            asserts.assert_true(icd_value == 0 or icd_value == 1, "ICD value is different than 0 or 1 (ASCII).")
        else:
            logging.info("supports_lit is false, verify that the ICD key is NOT present in the TXT record.")
            asserts.assert_not_in('ICD', operational_record.txt_record, "ICD key is present in the TXT record.")

        # SII TXT KEY
        if supports_icd and not supports_lit:
            sit_mode = True

        if supports_icd and supports_lit:
            if icd_value == 0:
                sit_mode = True
            else:
                sit_mode = False

        if not supports_icd:
            sit_mode = False

        if sit_mode:
            logging.info("sit_mode is True, verify the SII key IS present.")
            asserts.assert_in('SII', operational_record.txt_record, "SII key is NOT present in the TXT record.")

            logging.info("Verify SII value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            sii_value = operational_record.txt_record['SII']
            result, message = self.verify_decimal_value(sii_value, self.ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # SAI TXT KEY
        if supports_icd:
            logging.info("supports_icd is True, verify the SAI key IS present.")
            asserts.assert_in('SAI', operational_record.txt_record, "SAI key is NOT present in the TXT record.")

            logging.info("Verify SAI value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            sai_value = operational_record.txt_record['SAI']
            result, message = self.verify_decimal_value(sai_value, self.ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # SAT TXT KEY
        if 'SAT' in operational_record.txt_record:
            logging.info(
                "SAT key is present in TXT record, verify that it is a decimal value with no leading zeros and is less than or equal to 65535.")
            sat_value = operational_record.txt_record['SAT']
            result, message = self.verify_decimal_value(sat_value, self.MAX_SAT_VALUE)
            asserts.assert_true(result, message)

            if supports_icd:
                logging.info("supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                asserts.assert_equal(int(sat_value), active_mode_threshold_ms)

        # T TXT KEY
        if 'T' in operational_record.txt_record:
            logging.info("T key is present in TXT record, verify if that it is a decimal value with no leading zeros and is less than or equal to 6. Convert the value to a bitmap and verify bit 0 is clear.")
            t_value = operational_record.txt_record['T']
            result, message = self.verify_t_value(t_value)
            asserts.assert_true(result, message)

        # AAAA
        logging.info("Verify the AAAA record contains at least one IPv6 address")
        ipv6_address = self.extract_ipv6_address(str(quada_record))
        result, message = self.is_ipv6_address(ipv6_address)
        asserts.assert_true(result, message)

        # # *** STEP 10 ***
        # TH performs a DNS-SD browse for _I<hhhh>._sub._matter._tcp.local, where <hhhh> is the 64-bit compressed
        # Fabric identifier, expressed as a fixed-length, sixteencharacter hexadecimal string, encoded as ASCII (UTF-8)
        # text using capital letters. Verify DUT returns a PTR record with DNS-SD instance name set to instance_name
        self.step(10)
        service_types = await mdns.get_service_types(log_output=True)
        op_sub_type = self.get_operational_subtype()
        asserts.assert_in(op_sub_type, service_types, f"No PTR record with DNS-SD instance name '{op_sub_type}' wsa found.")

        # # *** STEP 11 ***
        # TH performs a DNS-SD browse for _matter._tcp.local
        # Verify DUT returns a PTR record with DNS-SD instance name set to instance_name
        self.step(11)
        op_service_info = await mdns._get_service(
            service_type=MdnsServiceType.OPERATIONAL,
            log_output=True,
            discovery_timeout_sec=15
        )

        # Verify DUT returns a PTR record with DNS-SD instance name set instance_name
        asserts.assert_equal(op_service_info.server, hostname,
                             f"No PTR record with DNS-SD instance name '{MdnsServiceType.OPERATIONAL.value}'")
        asserts.assert_equal(instance_name, op_service_info.instance_name, "Instance name mismatch")


if __name__ == "__main__":
    default_matter_test_main()
