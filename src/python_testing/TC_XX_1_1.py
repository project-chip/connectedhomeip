#
#    Copyright (c) 2022 Project CHIP Authors
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

from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
import logging
from mobly import asserts


class TC_XX_1_1(MatterBaseTest):
    @async_test_body
    async def test_xx_1_1(self):
        standard_range_prefix = 0x0000_0000
        manufacturer_code_min_prefix = 0x0001_0000
        manufacturer_code_max_prefix = 0xFFF0_0000
        test_vendor_min_prefix = 0xFFF1_0000
        test_vendor_max_prefix = 0xFFF4_0000
        invalid_vendor_min_prefix = 0xFFF5_0000
        invalid_vendor_max_prefix = 0xFFFF_0000

        attribute_id_range_min = 0x0000
        attribute_id_range_max = 0x4FFF
        attribute_global_range_min = 0xF000
        # attribute_global_range_max = 0xFFFE
        attribute_id_invalid_max = 0xFFFF

        # event_id_range_min = 0x00
        # event_id_range_max = 0xFF
        # event_id_invalid_max = 0xFFFF

        command_id_range_min = 0x00
        command_id_range_max = 0xFF
        command_id_invalid_max = 0xFFFF

        ATTRIBUTE_LIST_ID = 0xFFFB
        # EVENT_LIST_ID = 0xFFFA
        ACCEPTED_COMMAND_LIST_ID = 0xFFF9
        GENERATED_COMMAND_LIST_ID = 0xFFF8

        dev_ctrl = self.default_controller

        self.print_step(1, "Perform a wildcard read of attributes on all endpoints")
        wildcard_read = (await dev_ctrl.Read(self.dut_node_id, [()]))
        endpoints_tlv = wildcard_read.tlvAttributes

        self.print_step(2, "Verify check of invalid elements on all clusters, across all endpoints")
        for endpoint_id, endpoint in endpoints_tlv.items():
            logging.info(f"Verifying Endpoint: {endpoint_id}")

            for cluster_id, cluster in endpoint.items():
                logging.info(f"Verifying ClusterID: {cluster_id:#06x}")
                logging.info(f"AttributeList: {cluster[ATTRIBUTE_LIST_ID]}")
                logging.info(f"AcceptedCommandList: {cluster[ACCEPTED_COMMAND_LIST_ID]}")
                logging.info(f"GeneratedCommandList: {cluster[GENERATED_COMMAND_LIST_ID]}")

                attribute_list = cluster[ATTRIBUTE_LIST_ID]
                # event_list = cluster[EVENT_LIST_ID]
                accepted_command_list = cluster[ACCEPTED_COMMAND_LIST_ID]
                generated_command_list = cluster[GENERATED_COMMAND_LIST_ID]

                attribute_list.append(0x1000_4F00)

                # Check for invalid elements in standard range
                # -- Attributes
                # print(f"Attribute invalid range: {(standard_range_prefix + attribute_id_range_max + 1):#010x} - {(standard_range_prefix + attribute_global_range_min):#010x} and {(standard_range_prefix + attribute_id_invalid_max):#010x}")

                if any(((attribute_id > (standard_range_prefix + attribute_id_range_max) and attribute_id < (standard_range_prefix + attribute_global_range_min)) or attribute_id == attribute_id_invalid_max) for attribute_id in attribute_list):
                    asserts.fail(f'Invalid attribute in range [{(standard_range_prefix + attribute_id_range_max + 1):#010x} - {(standard_range_prefix + attribute_global_range_min):#010x}] or [{attribute_id_invalid_max:#010x}]')

                '''
                # -- Events
                # Currently disabled, since event list is not supported
                # print(f"EventList invalid range: {(standard_range_prefix + event_id_range_max + 1):#010x} - {(standard_range_prefix + event_id_invalid_max):#010x}")

                if any((event_id > (standard_range_prefix + event_id_range_max) and event_id <= (standard_range_prefix + event_id_invalid_max)) for event_id in event_list):
                    asserts.fail(f'Invalid event in range [{(standard_range_prefix + event_id_range_max + 1):#010x} - {(standard_range_prefix + event_id_invalid_max):#010x}]')
                '''

                # -- Commands
                # print(f"CommandLists invalid range: {(standard_range_prefix + command_id_range_max + 1):#010x} - {(standard_range_prefix + command_id_invalid_max):#010x}")

                if any((accepted_command_id > (standard_range_prefix + command_id_range_max) and accepted_command_id <= (standard_range_prefix + command_id_invalid_max)) for accepted_command_id in accepted_command_list):
                    asserts.fail(f'Invalid accepted command in range [{(standard_range_prefix + command_id_range_max + 1):#010x} - {(standard_range_prefix + command_id_invalid_max):#010x}]')

                if any((generated_command_id > (standard_range_prefix + command_id_range_max) and generated_command_id <= (standard_range_prefix + command_id_invalid_max)) for generated_command_id in generated_command_list):
                    asserts.fail(f'Invalid generated command in range [{(standard_range_prefix + command_id_range_max + 1):#010x} - {(standard_range_prefix + command_id_invalid_max):#010x}]')

                # Check for invalid attributes in MEI ranges
                for prefix_value in range(manufacturer_code_min_prefix, manufacturer_code_max_prefix + 1, 0x0001_0000):
                    # -- Attributes
                    # print(f"Attribute invalid range: {(prefix_value + attribute_id_range_max + 1):#010x} - {(prefix_value + attribute_id_invalid_max):#010x}")

                    if any((attribute_id > (prefix_value + attribute_id_range_max) and attribute_id <= (prefix_value + attribute_id_invalid_max)) for attribute_id in attribute_list):
                        asserts.fail(f'Invalid attribute in range [{(prefix_value + attribute_id_range_max + 1):#010x} - {(prefix_value + attribute_id_invalid_max):#010x}]')

                    '''
                    # -- Events
                    # Currently disabled, since event list is not supported
                    # print(f"EventList invalid range: {(prefix_value + event_id_range_max + 1):#010x} - {(prefix_value + event_id_invalid_max):#010x}")

                    if any((event_id > (prefix_value + event_id_range_max) and event_id <= (prefix_value + event_id_invalid_max)) for event_id in event_list):
                        asserts.fail(f'Invalid event in range [{(prefix_value + event_id_range_max + 1):#010x} - {(prefix_value + event_id_invalid_max):#010x}]')
                    '''

                    # -- Commands
                    # print(f"CommandLists invalid range: {(prefix_value + command_id_range_max + 1):#010x} - {(prefix_value + command_id_invalid_max):#010x}")

                    if any((accepted_command_id > (prefix_value + command_id_range_max) and accepted_command_id <= (prefix_value + command_id_invalid_max)) for accepted_command_id in accepted_command_list):
                        asserts.fail(f'Invalid accepted command in range [{(prefix_value + command_id_range_max + 1):#010x} - {(prefix_value + command_id_invalid_max):#010x}]')

                    if any((generated_command_id > (prefix_value + command_id_range_max) and generated_command_id <= (prefix_value + command_id_invalid_max)) for generated_command_id in generated_command_list):
                        asserts.fail(f'Invalid generated command in range [{(prefix_value + command_id_range_max + 1):#010x} - {(prefix_value + command_id_invalid_max):#010x}]')

                # Check for invalid attributes in test vendor ranges
                # -- Attributes
                # print(f"Attribute invalid range: {(test_vendor_min_prefix + attribute_id_range_min):#x} - {(test_vendor_max_prefix + attribute_id_invalid_max):#x}")

                if any((attribute_id >= (test_vendor_min_prefix + attribute_id_range_min) and attribute_id <= (test_vendor_max_prefix + attribute_id_invalid_max)) for attribute_id in attribute_list):
                    asserts.fail(f'Invalid attribute in range [{(test_vendor_min_prefix + attribute_id_range_min):#010x} - {(test_vendor_max_prefix + attribute_id_invalid_max):#010x}]')

                '''
                # -- Events
                # Currently disabled, since event list is not supported
                print(f"EventList invalid range: {(test_vendor_min_prefix + event_id_range_min):#010x} - {(test_vendor_max_prefix + event_id_invalid_max):#010x}")

                if any((event_id >= (test_vendor_min_prefix + event_id_range_min) and event_id <= (test_vendor_max_prefix + event_id_invalid_max)) for event_id in event_list):
                    asserts.fail(f'Invalid event in range [{(test_vendor_min_prefix + event_id_range_min):#010x} - {(test_vendor_max_prefix + event_id_invalid_max):#010x}]')
                '''

                # -- Commands
                # print(f"CommandLists invalid range: {(test_vendor_min_prefix + command_id_range_min):#010x} - {(test_vendor_max_prefix + command_id_invalid_max):#010x}")

                if any((accepted_command_id >= (test_vendor_min_prefix + command_id_range_min) and accepted_command_id <= (test_vendor_max_prefix + command_id_invalid_max)) for accepted_command_id in accepted_command_list):
                    asserts.fail(f'Invalid accepted command in range [{(test_vendor_min_prefix + command_id_range_min):#010x} - {(test_vendor_max_prefix + command_id_invalid_max):#010x}]')

                # -- Generated Commands
                if any((generated_command_id >= (test_vendor_min_prefix + command_id_range_min) and generated_command_id <= (test_vendor_max_prefix + command_id_invalid_max)) for generated_command_id in generated_command_list):
                    asserts.fail(f'Invalid generated command in range [{(test_vendor_min_prefix + command_id_range_min):#010x} - {(test_vendor_max_prefix + command_id_invalid_max):#010x}]')

                # Check for invalid attributes in non-specified ranges
                # print(f"Attribute invalid range: {(invalid_vendor_min_prefix + attribute_id_range_min):#x} - {(invalid_vendor_max_prefix + attribute_id_invalid_max):#x}")

                if any((attribute_id >= (invalid_vendor_min_prefix + attribute_id_range_min) and attribute_id <= (invalid_vendor_max_prefix + attribute_id_invalid_max)) for attribute_id in attribute_list):
                    asserts.fail(f'Invalid attribute in range [{(invalid_vendor_min_prefix + attribute_id_range_min):#010x} - {(invalid_vendor_max_prefix + attribute_id_invalid_max):#010x}]')

                '''
                # -- Events
                # Currently disabled, since event list is not supported
                print(f"EventList invalid range: {(invalid_vendor_min_prefix + event_id_range_min):#010x} - {(invalid_vendor_max_prefix + event_id_invalid_max):#010x}")

                if any((event_id >= (invalid_vendor_min_prefix + event_id_range_min) and event_id <= (invalid_vendor_max_prefix + event_id_invalid_max)) for event_id in event_list):
                    asserts.fail(f'Invalid event in range [{(invalid_vendor_min_prefix + event_id_range_min):#010x} - {(invalid_vendor_max_prefix + event_id_invalid_max):#010x}]')
                '''

                # -- Commands
                # print(f"CommandLists invalid range: {(invalid_vendor_min_prefix + command_id_range_min):#010x} - {(invalid_vendor_max_prefix + command_id_invalid_max):#010x}")

                if any((accepted_command_id >= (invalid_vendor_min_prefix + command_id_range_min) and accepted_command_id <= (invalid_vendor_max_prefix + command_id_invalid_max)) for accepted_command_id in accepted_command_list):
                    asserts.fail(f'Invalid accepted command in range [{(invalid_vendor_min_prefix + command_id_range_min):#010x} - {(invalid_vendor_max_prefix + command_id_invalid_max):#010x}]')

                # -- Generated Commands
                if any((generated_command_id >= (invalid_vendor_min_prefix + command_id_range_min) and generated_command_id <= (invalid_vendor_max_prefix + command_id_invalid_max)) for generated_command_id in generated_command_list):
                    asserts.fail(f'Invalid generated command in range [{(invalid_vendor_min_prefix + command_id_range_min):#010x} - {(invalid_vendor_max_prefix + command_id_invalid_max):#010x}]')


if __name__ == "__main__":
    default_matter_test_main()
