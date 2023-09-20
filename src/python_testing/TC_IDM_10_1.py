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

import logging

from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

STANDARD_PREFIX = 0x0000
MANUFACTURER_CODE_RANGE = range(0x0001, 0xFFF0 + 1)
TEST_VENDOR_RANGE = range(0xFFF1, 0xFFF4 + 1)
INVALID_VENDOR_RANGE = range(0xFFF5, 0xFFFF + 1)

ATTRIBUTE_ID_RANGE = range(0x0000, 0x4FFF + 1)
GLOBAL_ATTRIBUTE_ID_RANGE = range(0xF000, 0xFFFE + 1)
COMMAND_ID_RANGE = range(0x00, 0xFF + 1)

ATTRIBUTE_LIST_ID = 0xFFFB
ACCEPTED_COMMAND_LIST_ID = 0xFFF9
GENERATED_COMMAND_LIST_ID = 0xFFF8


def split_element_into_prefix_suffix(element):
    return (element >> 16), (element & 0xFFFF)


def validate_attribute_id_range(attribute_id):
    prefix, suffix = split_element_into_prefix_suffix(attribute_id)

    if prefix == STANDARD_PREFIX:
        if suffix not in ATTRIBUTE_ID_RANGE and suffix not in GLOBAL_ATTRIBUTE_ID_RANGE:
            asserts.fail(f"Invalid attribute id (0x{attribute_id:08X}) in standard range")

    elif prefix in MANUFACTURER_CODE_RANGE:
        if suffix not in ATTRIBUTE_ID_RANGE:
            asserts.fail(f"Invalid attribute id (0x{attribute_id:08X}) in MC range")

    elif prefix in TEST_VENDOR_RANGE:
        if suffix in ATTRIBUTE_ID_RANGE:
            logging.info(f"Warning: Attribute id (0x{attribute_id:08X}) found in test vendor range")
        else:
            asserts.fail(f"Invalid attribute id (0x{attribute_id:08X}) in Test Vendor MC range")

    else:  # prefix in INVALID_VENDOR_RANGE
        asserts.fail(f"Invalid attribute id (0x{attribute_id:08X}) in invalid range")


def validate_command_id_range(command_id):
    prefix, suffix = split_element_into_prefix_suffix(command_id)

    if prefix == STANDARD_PREFIX:
        if suffix not in COMMAND_ID_RANGE:
            asserts.fail(f"Invalid command id (0x{command_id:08X}) in standard range")

    elif prefix in MANUFACTURER_CODE_RANGE:
        if suffix not in COMMAND_ID_RANGE:
            asserts.fail(f"Invalid command id (0x{command_id:08X}) in MC range")

    elif prefix in TEST_VENDOR_RANGE:
        if suffix in COMMAND_ID_RANGE:
            logging.info(f"Warning: Command id (0x{command_id:08X}) found in test vendor range")
        else:
            asserts.fail(f"Invalid command id (0x{command_id:08X}) in Test Vendor MC range")

    else:  # prefix in INVALID_VENDOR_RANGE
        asserts.fail(f"Invalid command id (0x{command_id:08X}) in invalid range")


class TC_IDM_10_1(MatterBaseTest):
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
                logging.info(f"Verifying ClusterID: 0x{cluster_id:04X}")

                attribute_list = cluster[ATTRIBUTE_LIST_ID]
                accepted_command_list = cluster[ACCEPTED_COMMAND_LIST_ID]
                generated_command_list = cluster[GENERATED_COMMAND_LIST_ID]

                # -- Attributes
                logging.info(f"AttributeList: {[f'0x{attribute_id:04X}' for attribute_id in attribute_list]}")

                for attribute_id in attribute_list:
                    validate_attribute_id_range(attribute_id)

                # -- Commands
                logging.info(f"AcceptedCommandList: {[f'0x{command_id:02X}' for command_id in accepted_command_list]}")

                for command_id in accepted_command_list:
                    validate_command_id_range(command_id)

                logging.info(f"GeneratedCommandList: {[f'0x{command_id:02X}' for command_id in generated_command_list]}")
                for command_id in generated_command_list:
                    validate_command_id_range(command_id)


if __name__ == "__main__":
    default_matter_test_main()
