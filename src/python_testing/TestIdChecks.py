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

from global_attribute_ids import (AttributeIdType, ClusterIdType, DeviceTypeIdType, attribute_id_type, cluster_id_type,
                                  device_type_id_type, is_valid_attribute_id, is_valid_cluster_id, is_valid_device_type_id)
from matter_testing_support import MatterBaseTest, default_matter_test_main
from mobly import asserts


class TestIdChecks(MatterBaseTest):
    def test_device_type_ids(self):
        standard_good = [0x0000_0000, 0x0000_BFFF]
        standard_bad = [0x0000_C000]

        manufacturer_good = [0x0001_0000, 0x0001_BFFF, 0xFFF0_0000, 0xFFF0_BFFF]
        manufacturer_bad = [0x0001_C000, 0xFFF0_C000]

        test_good = [0xFFF1_0000, 0xFFF1_BFFF, 0xFFF4_0000, 0xFFF4_BFFF]
        test_bad = [0xFFF1_C000, 0xFFF4_C000]

        prefix_bad = [0xFFF5_0000, 0xFFF5_BFFFF, 0xFFF5_C000]

        def check_standard(id):
            id_type = device_type_id_type(id)
            msg = f"Incorrect device type range assessment, expecting standard {id:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(id), DeviceTypeIdType.kStandard, msg)
            asserts.assert_true(is_valid_device_type_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_device_type_id(id_type, allow_test=False), msg)

        def check_manufacturer(id):
            id_type = device_type_id_type(id)
            msg = f"Incorrect device type range assessment, expecting manufacturer {id:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(id), DeviceTypeIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_device_type_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_device_type_id(id_type, allow_test=False), msg)

        def check_test(id):
            id_type = device_type_id_type(id)
            msg = f"Incorrect device type range assessment, expecting test {id:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(id), DeviceTypeIdType.kTest, msg)
            asserts.assert_true(is_valid_device_type_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_device_type_id(id_type, allow_test=False), msg)

        def check_all_bad(id):
            id_type = device_type_id_type(id)
            msg = f"Incorrect device type range assessment, expecting invalid {id:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(id), DeviceTypeIdType.kInvalid, msg)
            asserts.assert_false(is_valid_device_type_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_device_type_id(id_type, allow_test=False), msg)

        for id in standard_good:
            check_standard(id)

        for id in standard_bad:
            check_all_bad(id)

        for id in manufacturer_good:
            check_manufacturer(id)

        for id in manufacturer_bad:
            check_all_bad(id)

        for id in test_good:
            check_test(id)

        for id in test_bad:
            check_all_bad(id)

        for id in prefix_bad:
            check_all_bad(id)

    def test_cluster_ids(self):
        standard_good = [0x0000_0000, 0x0000_7FFF]
        standard_bad = [0x0000_8000]

        manufacturer_good = [0x0001_FC00, 0x0001_FFFE, 0xFFF0_FC00, 0xFFF0_FFFE]
        manufacturer_bad = [0x0001_0000, 0x0001_7FFF, 0x0001_FFFF, 0xFFF0_0000, 0xFFF0_7FFF, 0xFFF0_FFFF]

        test_good = [0xFFF1_FC00, 0xFFF1_FFFE, 0xFFF4_FC00, 0xFFF4_FFFE]
        test_bad = [0xFFF1_0000, 0xFFF1_7FFF, 0xFFF1_FFFF, 0xFFF4_0000, 0xFFF4_7FFF, 0xFFF4_FFFF]

        prefix_bad = [0xFFF5_0000, 0xFFF5_FC00, 0xFFF5_FFFF]

        def check_standard(id):
            id_type = cluster_id_type(id)
            msg = f"Incorrect cluster range assessment, expecting standard {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kStandard, msg)
            asserts.assert_true(is_valid_cluster_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_cluster_id(id_type, allow_test=False), msg)

        def check_manufacturer(id):
            id_type = cluster_id_type(id)
            msg = f"Incorrect cluster range assessment, expecting manufacturer {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_cluster_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_cluster_id(id_type, allow_test=False), msg)

        def check_test(id):
            id_type = cluster_id_type(id)
            msg = f"Incorrect cluster range assessment, expecting test {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kTest, msg)
            asserts.assert_true(is_valid_cluster_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_cluster_id(id_type, allow_test=False), msg)

        def check_all_bad(id):
            id_type = cluster_id_type(id)
            msg = f"Incorrect cluster range assessment, expecting invalid {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kInvalid, msg)
            asserts.assert_false(is_valid_cluster_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_cluster_id(id_type, allow_test=False), msg)

        for id in standard_good:
            check_standard(id)

        for id in standard_bad:
            check_all_bad(id)

        for id in manufacturer_good:
            check_manufacturer(id)

        for id in manufacturer_bad:
            check_all_bad(id)

        for id in test_good:
            check_test(id)

        for id in test_bad:
            check_all_bad(id)

        for id in prefix_bad:
            check_all_bad(id)

    def test_attribute_ids(self):
        standard_global_good = [0x0000_F000, 0x0000_FFFE]
        standard_global_bad = [0x0000_FFFF]
        standard_non_global_good = [0x0000_0000, 0x0000_4FFF]
        standard_non_global_bad = [0x0000_5000]
        manufacturer_good = [0x0001_0000, 0x0001_4FFF, 0xFFF0_0000, 0xFFF0_4FFF]
        manufacturer_bad = [0x0001_5000, 0x0001_F000, 0x0001_FFFFF, 0xFFF0_5000, 0xFFF0_F000, 0xFFF0_FFFF]
        test_good = [0xFFF1_0000, 0xFFF1_4FFF, 0xFFF4_0000, 0xFFF4_4FFF]
        test_bad = [0xFFF1_5000, 0xFFF1_F000, 0xFFF1_FFFFF, 0xFFF4_5000, 0xFFF4_F000, 0xFFF4_FFFF]
        prefix_bad = [0xFFF5_0000, 0xFFF5_4FFF, 0xFFF5_5000, 0xFFF5_F000, 0xFFF5_FFFF]

        def check_standard_global(id):
            id_type = attribute_id_type(id)
            msg = f"Incorrect attribute range assessment, expecting standard global {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kStandardGlobal, msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=False), msg)

        def check_standard_non_global(id):
            id_type = attribute_id_type(id)
            msg = f"Incorrect attribute range assessment, expecting standard non-global {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kStandardNonGlobal, msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=False), msg)

        def check_manufacturer(id):
            id_type = attribute_id_type(id)
            msg = f"Incorrect attribute range assessment, expecting manufacturer {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=False), msg)

        def check_test(id):
            id_type = attribute_id_type(id)
            msg = f"Incorrect attribute range assessment, expecting test {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kTest, msg)
            asserts.assert_true(is_valid_attribute_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_attribute_id(id_type, allow_test=False), msg)

        def check_all_bad(id):
            id_type = attribute_id_type(id)
            msg = f"Incorrect attribute range assessment, expecting invalid {id:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kInvalid, msg)
            asserts.assert_false(is_valid_attribute_id(id_type, allow_test=True), msg)
            asserts.assert_false(is_valid_attribute_id(id_type, allow_test=False), msg)

        for id in standard_global_good:
            check_standard_global(id)
        for id in standard_global_bad:
            check_all_bad(id)
        for id in standard_non_global_good:
            check_standard_non_global(id)
        for id in standard_non_global_bad:
            check_all_bad(id)
        for id in manufacturer_good:
            check_manufacturer(id)
        for id in manufacturer_bad:
            check_all_bad(id)
        for id in test_good:
            check_test(id)
        for id in test_bad:
            check_all_bad(id)
        for id in prefix_bad:
            check_all_bad(id)


if __name__ == "__main__":
    default_matter_test_main()
