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

from mobly import asserts

from matter.testing.global_attribute_ids import (AttributeIdType, ClusterIdType, CommandIdType, DeviceTypeIdType, attribute_id_type,
                                                 cluster_id_type, command_id_type, device_type_id_type, is_standard_attribute_id,
                                                 is_standard_cluster_id, is_standard_command_id, is_standard_device_type_id,
                                                 is_valid_attribute_id, is_valid_cluster_id, is_valid_command_id,
                                                 is_valid_device_type_id)
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TestIdChecks(MatterBaseTest):
    def test_device_type_ids(self):
        standard_good = [0x0000_0000, 0x0000_BFFF]
        standard_bad = [0x0000_C000]

        manufacturer_good = [0x0001_0000, 0x0001_BFFF, 0xFFF0_0000, 0xFFF0_BFFF]
        manufacturer_bad = [0x0001_C000, 0xFFF0_C000]

        test_good = [0xFFF1_0000, 0xFFF1_BFFF, 0xFFF4_0000, 0xFFF4_BFFF]
        test_bad = [0xFFF1_C000, 0xFFF4_C000]

        prefix_bad = [0xFFF5_0000, 0xFFF5_BFFFF, 0xFFF5_C000]

        def check_standard(tid):
            id_type = device_type_id_type(tid)
            msg = f"Incorrect device type range assessment, expecting standard {tid:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(tid), DeviceTypeIdType.kStandard, msg)
            asserts.assert_true(is_valid_device_type_id(tid, allow_test=True), msg)
            asserts.assert_true(is_valid_device_type_id(tid, allow_test=False), msg)
            asserts.assert_true(is_standard_device_type_id(tid), msg)

        def check_manufacturer(tid):
            id_type = device_type_id_type(tid)
            msg = f"Incorrect device type range assessment, expecting manufacturer {tid:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(tid), DeviceTypeIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_device_type_id(tid, allow_test=True), msg)
            asserts.assert_true(is_valid_device_type_id(tid, allow_test=False), msg)
            asserts.assert_false(is_standard_device_type_id(tid), msg)

        def check_test(tid):
            id_type = device_type_id_type(tid)
            msg = f"Incorrect device type range assessment, expecting test {tid:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(tid), DeviceTypeIdType.kTest, msg)
            asserts.assert_true(is_valid_device_type_id(tid, allow_test=True), msg)
            asserts.assert_false(is_valid_device_type_id(tid, allow_test=False), msg)
            asserts.assert_false(is_standard_device_type_id(tid), msg)

        def check_all_bad(tid):
            id_type = device_type_id_type(tid)
            msg = f"Incorrect device type range assessment, expecting invalid {tid:08x}, type = {id_type}"
            asserts.assert_equal(device_type_id_type(tid), DeviceTypeIdType.kInvalid, msg)
            asserts.assert_false(is_valid_device_type_id(tid, allow_test=True), msg)
            asserts.assert_false(is_valid_device_type_id(tid, allow_test=False), msg)
            asserts.assert_false(is_standard_device_type_id(tid), msg)

        for tid in standard_good:
            check_standard(tid)

        for tid in standard_bad:
            check_all_bad(tid)

        for tid in manufacturer_good:
            check_manufacturer(tid)

        for tid in manufacturer_bad:
            check_all_bad(tid)

        for tid in test_good:
            check_test(tid)

        for tid in test_bad:
            check_all_bad(tid)

        for tid in prefix_bad:
            check_all_bad(tid)

    def test_cluster_ids(self):
        standard_good = [0x0000_0000, 0x0000_7FFF]
        standard_bad = [0x0000_8000]

        manufacturer_good = [0x0001_FC00, 0x0001_FFFE, 0xFFF0_FC00, 0xFFF0_FFFE]
        manufacturer_bad = [0x0001_0000, 0x0001_7FFF, 0x0001_FFFF, 0xFFF0_0000, 0xFFF0_7FFF, 0xFFF0_FFFF]

        test_good = [0xFFF1_FC00, 0xFFF1_FFFE, 0xFFF4_FC00, 0xFFF4_FFFE]
        test_bad = [0xFFF1_0000, 0xFFF1_7FFF, 0xFFF1_FFFF, 0xFFF4_0000, 0xFFF4_7FFF, 0xFFF4_FFFF]

        prefix_bad = [0xFFF5_0000, 0xFFF5_FC00, 0xFFF5_FFFF]

        def check_standard(cid):
            id_type = cluster_id_type(cid)
            msg = f"Incorrect cluster range assessment, expecting standard {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kStandard, msg)
            asserts.assert_true(is_valid_cluster_id(cid, allow_test=True), msg)
            asserts.assert_true(is_valid_cluster_id(cid, allow_test=False), msg)
            asserts.assert_true(is_standard_cluster_id(cid), msg)

        def check_manufacturer(cid):
            id_type = cluster_id_type(cid)
            msg = f"Incorrect cluster range assessment, expecting manufacturer {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_cluster_id(cid, allow_test=True), msg)
            asserts.assert_true(is_valid_cluster_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_cluster_id(cid), msg)

        def check_test(cid):
            id_type = cluster_id_type(cid)
            msg = f"Incorrect cluster range assessment, expecting test {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kTest, msg)
            asserts.assert_true(is_valid_cluster_id(cid, allow_test=True), msg)
            asserts.assert_false(is_valid_cluster_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_cluster_id(cid), msg)

        def check_all_bad(cid):
            id_type = cluster_id_type(cid)
            msg = f"Incorrect cluster range assessment, expecting invalid {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, ClusterIdType.kInvalid, msg)
            asserts.assert_false(is_valid_cluster_id(cid, allow_test=True), msg)
            asserts.assert_false(is_valid_cluster_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_cluster_id(cid), msg)

        for cid in standard_good:
            check_standard(cid)

        for cid in standard_bad:
            check_all_bad(cid)

        for cid in manufacturer_good:
            check_manufacturer(cid)

        for cid in manufacturer_bad:
            check_all_bad(cid)

        for cid in test_good:
            check_test(cid)

        for cid in test_bad:
            check_all_bad(cid)

        for cid in prefix_bad:
            check_all_bad(cid)

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

        def check_standard_global(aid):
            id_type = attribute_id_type(aid)
            msg = f"Incorrect attribute range assessment, expecting standard global {aid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kStandardGlobal, msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=False), msg)
            asserts.assert_true(is_standard_attribute_id(aid), msg)

        def check_standard_non_global(aid):
            id_type = attribute_id_type(aid)
            msg = f"Incorrect attribute range assessment, expecting standard non-global {aid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kStandardNonGlobal, msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=False), msg)
            asserts.assert_true(is_standard_attribute_id(aid), msg)

        def check_manufacturer(aid):
            id_type = attribute_id_type(aid)
            msg = f"Incorrect attribute range assessment, expecting manufacturer {aid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=True), msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=False), msg)
            asserts.assert_false(is_standard_attribute_id(aid), msg)

        def check_test(aid):
            id_type = attribute_id_type(aid)
            msg = f"Incorrect attribute range assessment, expecting test {aid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kTest, msg)
            asserts.assert_true(is_valid_attribute_id(aid, allow_test=True), msg)
            asserts.assert_false(is_valid_attribute_id(aid, allow_test=False), msg)
            asserts.assert_false(is_standard_attribute_id(aid), msg)

        def check_all_bad(aid):
            id_type = attribute_id_type(aid)
            msg = f"Incorrect attribute range assessment, expecting invalid {aid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, AttributeIdType.kInvalid, msg)
            asserts.assert_false(is_valid_attribute_id(aid, allow_test=True), msg)
            asserts.assert_false(is_valid_attribute_id(aid, allow_test=False), msg)
            asserts.assert_false(is_standard_attribute_id(aid), msg)

        for aid in standard_global_good:
            check_standard_global(aid)
        for aid in standard_global_bad:
            check_all_bad(aid)
        for aid in standard_non_global_good:
            check_standard_non_global(aid)
        for aid in standard_non_global_bad:
            check_all_bad(aid)
        for aid in manufacturer_good:
            check_manufacturer(aid)
        for aid in manufacturer_bad:
            check_all_bad(aid)
        for aid in test_good:
            check_test(aid)
        for aid in test_bad:
            check_all_bad(aid)
        for aid in prefix_bad:
            check_all_bad(aid)

    def test_command_ids(self):
        standard_global_good = [0x0000_00E0, 0x0000_00FF, 0x0000_00E1, 0x0000_00FE]
        standard_global_bad = [0x0000_01E0, 0x0000_0FFF, 0x0000_AAE1, 0x0000_BBFE, 0x0000_FFFF]
        scoped_non_global_good = [0x0000_0000, 0x0000_00DF, 0x0000_0001]
        scoped_non_global_bad = [0x0000_0F00, 0x0000_01DF, 0x0000_0F01]
        manufacturer_good = [0x0001_0000, 0x0001_00FF, 0xFFF0_0000, 0xFFF0_00FF, 0x0001_00FE]
        manufacturer_bad = [0x0001_0A00, 0x0001_0BFF, 0x0001_FFFF, 0xFFF0_0C00, 0xFFF0_D0FF, 0x0001_F0FE]
        test_good = [0xFFF1_0000, 0xFFF1_00E0, 0xFFF1_00FF, 0xFFF4_0000, 0xFFF4_00E0, 0xFFF4_00FF]
        test_bad = [0xFFF1_5000, 0xFFF1_F000, 0xFFF1_FFFF, 0xFFF4_5000, 0xFFF4_F000, 0xFFF4_FFFF]
        prefix_bad = [0xFFF5_0000, 0xFFF5_4FFF, 0xFFF5_5000, 0xFFF5_F000, 0xFFF5_FFFF]

        def check_standard_global(cid):
            id_type = command_id_type(cid)
            msg = f"Incorrect command range assessment, expecting standard global {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, CommandIdType.kStandardGlobal, msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=True), msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=False), msg)
            asserts.assert_true(is_standard_command_id(cid), msg)

        def check_scoped_non_global(cid):
            id_type = command_id_type(cid)
            msg = f"Incorrect command range assessment, expecting scoped non-global {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, CommandIdType.kScopedNonGlobal, msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=True), msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=False), msg)
            asserts.assert_true(is_standard_command_id(cid), msg)

        def check_manufacturer(cid):
            id_type = command_id_type(cid)
            msg = f"Incorrect command range assessment, expecting manufacturer {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, CommandIdType.kManufacturer, msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=True), msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_command_id(cid), msg)

        def check_test(cid):
            id_type = command_id_type(cid)
            msg = f"Incorrect command range assessment, expecting test {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, CommandIdType.kTest, msg)
            asserts.assert_true(is_valid_command_id(cid, allow_test=True), msg)
            asserts.assert_false(is_valid_command_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_command_id(cid), msg)

        def check_all_bad(cid):
            id_type = command_id_type(cid)
            msg = f"Incorrect command range assessment, expecting invalid {cid:08x}, type = {id_type}"
            asserts.assert_equal(id_type, CommandIdType.kInvalid, msg)
            asserts.assert_false(is_valid_command_id(cid, allow_test=True), msg)
            asserts.assert_false(is_valid_command_id(cid, allow_test=False), msg)
            asserts.assert_false(is_standard_command_id(cid), msg)

        for cid in standard_global_good:
            check_standard_global(cid)
        for cid in standard_global_bad:
            check_all_bad(cid)
        for cid in scoped_non_global_good:
            check_scoped_non_global(cid)
        for cid in scoped_non_global_bad:
            check_all_bad(cid)
        for cid in manufacturer_good:
            check_manufacturer(cid)
        for cid in manufacturer_bad:
            check_all_bad(cid)
        for cid in test_good:
            check_test(cid)
        for cid in test_bad:
            check_all_bad(cid)
        for cid in prefix_bad:
            check_all_bad(cid)


if __name__ == "__main__":
    default_matter_test_main()
