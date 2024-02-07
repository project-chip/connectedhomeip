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

import logging
import re

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType


'''
Category
Functional conformance

Purpose
The purpose of this test case is to verify that a Matter node is discoverable
and can advertise its services in a Matter network.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#343-tc-sc-43-discovery-dut_commissionee
'''

class TC_SC_4_3(MatterBaseTest):

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
    def verify_decimal_value(input_value, comparison_value: int):
        try:
            input_float = float(input_value)
            input_int = int(input_float)

            if str(input_value).startswith("0") and input_int != 0:
                return (False, f"Input ({input_value}) has leading zeros.")

            if input_float != input_int:
                return (False, f"Input ({input_value}) is not an integer.")

            if input_int <= comparison_value:
                return (True, f"Input ({input_value}) is valid.")
            else:
                return (False, f"Input ({input_value}) exceeds the allowed value {comparison_value}.")
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
        except ValueError:
            return False, "T value ({t_value}) is not a valid decimal number."
    @staticmethod
    def contains_ipv6_address(addresses):
        # IPv6 pattern for basic validation
        ipv6_pattern = re.compile(r'(?:(?:[0-9a-fA-F]{1,4}:){7}(?:[0-9a-fA-F]{1,4}|:))|(?:[0-9a-fA-F]{1,4}:){6}(?::[0-9a-fA-F]{1,4}|(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?:[0-9a-fA-F]{1,4}:){5}(?:(?::[0-9a-fA-F]{1,4}){1,2}|:((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?:[0-9a-fA-F]{1,4}:){4}(?:(?::[0-9a-fA-F]{1,4}){1,3}|:((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?:[0-9a-fA-F]{1,4}:){3}(?:(?::[0-9a-fA-F]{1,4}){1,4}|:((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?:[0-9a-fA-F]{1,4}:){2}(?:(?::[0-9a-fA-F]{1,4}){1,5}|:((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?:[0-9a-fA-F]{1,4}:){1}(?:(?::[0-9a-fA-F]{1,4}){1,6}|:((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|:)|(?::(?::[0-9a-fA-F]{1,4}){1,7}|:)', re.VERBOSE)

        for address in addresses:
            if ipv6_pattern.match(address):
                return True, "At least one IPv6 address is present."

        return False, "No IPv6 addresses found."
    
    @async_test_body
    async def test_TC_SC_4_3(self):
        print(f"\n"*10)

        supports_icd = None
        supports_lit = None
        active_mode_threshold_ms = None
        instance_name = None
        icd_value = None
        sit_mode = None

        # *** STEP 1 ***
        self.print_step("1", "DUT is commissioned on the same fabric as TH.")
        
        # *** STEP 2 ***
        self.print_step("2", "TH reads ServerList attribute from the Descriptor cluster on EP0. If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.")
        ep0_servers = await self.get_descriptor_server_list()

        # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        supports_icd = Clusters.IcdManagement.id in ep0_servers
        logging.info(f"\n\n\tsupports_icd: {supports_icd}\n\n")

        # *** STEP 3 ***
        self.print_step("3", "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold.")
        if supports_icd:
            active_mode_threshold_ms = await self.get_idle_mode_threshhold_ms()
        logging.info(f"\n\n\tactive_mode_threshold_ms: {active_mode_threshold_ms}\n\n")

        # *** STEP 4 ***
        self.print_step("4", "If supports_icd is true, TH reads FeatureMap from the ICD Management cluster on EP0. If the LITS feature is set, set supports_lit to true. Otherwise set supports_lit to false.")
        if supports_icd:
            feature_map = await self.get_icd_feature_map()
            LITS = Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport
            supports_lit = bool(feature_map & LITS == LITS)
            logging.info(f"\n\n\tkLongIdleTimeSupport set: {supports_lit}\n\n")

        # *** STEP 5 ***
        self.print_step("5", "TH constructs the instance name for the DUT as the 64-bit compressed Fabric identifier, and the assigned 64-bit Node identifier, each expressed as a fixed-length sixteen-character hexadecimal string, encoded as ASCII (UTF-8) text using capital letters, separated by a hyphen.")
        instance_name = self.get_dut_instance_name()



        # PENDING STEPS 6-8


        mdns = MdnsDiscovery()
        operational = await mdns.get_operational_service(
            service_name=f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}",
            service_type=MdnsServiceType.OPERATIONAL.value,
            log_output=True
        )

        # *** STEP 9 ***
        self.print_step("9", "TH verifies ICD, SII, SAI, SAT, and T TXT record keys/vales of the returned record.")

        # ICD TXT KEY
        if supports_lit:
            logging.info(f"supports_lit is true, verify the ICD key IS present in the TXT record, and it has the value of 0 or 1 (ASCII).")

            # Verify the ICD key IS present
            asserts.assert_in('ICD', operational.txt_record, "ICD key is NOT present in the TXT record.")

            # Verify it has the value of 0 or 1 (ASCII)
            icd_value = int(operational.txt_record['ICD'])
            asserts.assert_true(icd_value == 0 or icd_value == 1, "ICD value is different than 0 or 1 (ASCII).")
        else:
            logging.info(f"supports_lit is false, verify that the ICD key is NOT present in the TXT record.")
            asserts.assert_not_in('ICD', operational.txt_record, "ICD key is present in the TXT record.")

        # SII TXT KEY
        if supports_icd and not supports_lit:
            sit_mode = True

        if supports_lit and supports_lit:
            if icd_value == 0:
                sit_mode = True
            else:
                sit_mode = False

        if not supports_icd:
            sit_mode = False

        if sit_mode:
            logging.info(f"sit_mode is True, verify the SII key IS present.")
            asserts.assert_in('SII', operational.txt_record, "SII key is NOT present in the TXT record.")

            logging.info(f"Verify SII value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            sii_value = operational.txt_record['SII']
            result, message = self.verify_decimal_value(sii_value, self.ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # SAI TXT KEY
        if supports_icd:
            logging.info(f"supports_icd is True, verify the SAI key IS present.")
            asserts.assert_in('SAI', operational.txt_record, "SAI key is NOT present in the TXT record.")

            logging.info(f"Verify SAI value is a decimal with no leading zeros and is less than or equal to 3600000 (1h in ms).")
            sai_value = operational.txt_record['SAI']
            result, message = self.verify_decimal_value(sai_value, self.ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # SAT TXT KEY
        if 'SAT' in operational.txt_record:
            logging.info(f"SAT key is present in TXT record, verify that it is a decimal value with no leading zeros and is less than or equal to 65535.")
            sat_value = operational.txt_record['SAT']
            result, message = self.verify_decimal_value(sat_value, self.MAX_SAT_VALUE)
            asserts.assert_true(result, message)

            if supports_icd:
                logging.info(f"supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                asserts.assert_equal(int(sat_value), active_mode_threshold_ms)

        # # T TXT KEY
        # if 'T' in operational.txt_record:
        #     logging.info(f"T key is present in TXT record, verify if that it is a decimal value with no leading zeros and is less than or equal to 6. Convert the value to a bitmap and verify bit 0 is clear.")
        #     t_value = operational.txt_record['T']
        #     result, message = self.verify_t_value(t_value)
        #     asserts.assert_true(result, message)

        # AAAA
        logging.info(f"Verify the AAAA record contains at least one IPv6 address")
        result, message = self.contains_ipv6_address(operational.addresses)
        asserts.assert_true(result, message)

        # *** STEP 10 ***
        self.print_step("10", "Verify DUT returns a PTR record with DNS-SD instance name set instance_name.")
        service_types = await mdns.get_service_types(log_output=True)
        op_sub_type = self.get_operational_subtype()
        asserts.assert_in(op_sub_type, service_types, f"No PTR record with DNS-SD instance name '{op_sub_type}'")

        print(f"\n"*10)


if __name__ == "__main__":
    default_matter_test_main()
