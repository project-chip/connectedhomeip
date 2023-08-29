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


standard_range_prefix = 0x0000
manufacturer_code_min_prefix = 0x0001
manufacturer_code_max_prefix = 0xFFF0
test_vendor_min_prefix = 0xFFF1
test_vendor_max_prefix = 0xFFF4
invalid_vendor_min_prefix = 0xFFF5
invalid_vendor_max_prefix = 0xFFFF

attribute_id_range_min = 0x0000
attribute_id_range_max = 0x4FFF
attribute_global_range_min = 0xF000
attribute_global_range_max = 0xFFFE
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


def get_element_prefix_suffix(element):
    prefix_mask = ~0xffff
    suffix_mask = 0xffff

    prefix = (element & prefix_mask) >> 16
    suffix = (element & suffix_mask)

    return prefix, suffix


def is_attribute_id_in_valid_range(attribute_id):
    prefix, suffix = get_element_prefix_suffix(attribute_id)

    if prefix == standard_range_prefix:
        if suffix > attribute_id_range_max and suffix < attribute_global_range_min or suffix == attribute_id_invalid_max:
            asserts.fail(f"Invalid attribute id {attribute_id:010x} in standard range")

    elif prefix >= manufacturer_code_min_prefix and prefix <= manufacturer_code_max_prefix:
        if suffix > attribute_id_range_max:
            asserts.fail(f"Invalid attribute id {attribute_id:010x} in MC range")

    elif prefix >= test_vendor_min_prefix and prefix <= test_vendor_max_prefix:
        if suffix >= attribute_id_range_min:
            asserts.fail(f"Invalid attribute id {attribute_id:010x} in Test Vendor MC range")

    else:  # prefix >= invalid_vendor_min_prefix and prefix >= invalid_vendor_max_prefix
        if suffix >= attribute_id_range_min:
            asserts.fail(f"Invalid attribute id {attribute_id:010x} in invalid range")


def is_command_id_in_valid_range(command_id):
    prefix, suffix = get_element_prefix_suffix(command_id)

    if prefix == standard_range_prefix:
        if suffix > command_id_range_max:
            asserts.fail(f"Invalid command id {command_id:010x} in standard range")

    elif prefix >= manufacturer_code_min_prefix and prefix <= manufacturer_code_max_prefix:
        if suffix > command_id_range_max:
            asserts.fail(f"Invalid command id {command_id:010x} in MC range")

    elif prefix >= test_vendor_min_prefix and prefix <= test_vendor_max_prefix:
        if suffix >= command_id_range_min:
            asserts.fail(f"Invalid command id {command_id:010x} in Test Vendor MC range")

    else:  # prefix >= invalid_vendor_min_prefix and prefix >= invalid_vendor_max_prefix
        if suffix >= command_id_range_min:
            asserts.fail(f"Invalid command id {command_id:010x} in invalid range")


class TC_XX_1_1(MatterBaseTest):
    @async_test_body
    async def test_xx_1_1(self):
        
        dev_ctrl = self.default_controller

        self.print_step(1, "Perform a wildcard read of attributes on all endpoints")
        wildcard_read = (await dev_ctrl.Read(self.dut_node_id, [()]))
        endpoints_tlv = wildcard_read.tlvAttributes

        self.print_step(2, "Check invalid elements on all clusters, across all endpoints")
        for endpoint_id, endpoint in endpoints_tlv.items():
            logging.info(f"Verifying Endpoint: {endpoint_id}")

            for cluster_id, cluster in endpoint.items():
                logging.info(f"Verifying ClusterID: {cluster_id:#06x}")

                attribute_list = cluster[ATTRIBUTE_LIST_ID]
                accepted_command_list = cluster[ACCEPTED_COMMAND_LIST_ID]
                generated_command_list = cluster[GENERATED_COMMAND_LIST_ID]

                # -- Attributes
                logging.info(f"AttributeList: {attribute_list}")
                for attribute_id in attribute_list:
                    is_attribute_id_in_valid_range(attribute_id)

                # -- Commands
                logging.info(f"AcceptedCommandList: {accepted_command_list}")
                for command_id in accepted_command_list:
                    is_command_id_in_valid_range(command_id)

                logging.info(f"GeneratedCommandList: {generated_command_list}")
                for command_id in generated_command_list:
                    is_command_id_in_valid_range(command_id)


if __name__ == "__main__":
    default_matter_test_main()
