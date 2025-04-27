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
#       --endpoint 1
# === END CI TEST ARGUMENTS ===

import logging
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# These below variables are used to test the AccessControl clusters Extension attribute and come from the test plan here:
# https://github.com/CHIP-Specifications/chip-test-plans/blob/59e8c45b8e7c24d5ce130b166520ff4f7bd935b6/src/cluster/AccessControl.adoc#tc-acl-2-6-accesscontrolentrychanged-event:~:text=D_OK_EMPTY%3A%20%221718%22%20which%20is%20an%20octstr%20of%20length%202%20containing%20valid%20TLV%3A
D_OK_EMPTY = bytes.fromhex('1718')
D_OK_SINGLE = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')
D_OK_FULL = bytes.fromhex('17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E6700D00000F1FF02003148656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E7420616761696E2E2E2E2E2E0018')
D_BAD_LENGTH = bytes.fromhex('17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E6700D00000F1FF02003248656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E7420616761696E2E2E2E2E2E2E0018')
D_BAD_STRUCT = bytes.fromhex('1518')
D_BAD_LIST = bytes.fromhex(
    '3701D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')
D_BAD_ELEM = bytes.fromhex(
    '17103D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')
D_BAD_OVERFLOW = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018FF')
D_BAD_UNDERFLOW = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E6700')
D_BAD_NONE = bytes.fromhex('')


class TC_ACL_2_3(MatterBaseTest):
    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    def desc_TC_ACL_2_3(self) -> str:
        return "[TC-ACL-2.3] Multiple fabrics test"

    def steps_TC_ACL_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID",
                     is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is an empty list"),
            TestStep(4, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_EMPTY",
                     "Result is SUCCESS"),
            TestStep(5, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_EMPTY, fabric index is F1"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_SINGLE",
                     "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_SINGLE, fabric index is F1"),
            TestStep(8, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_FULL",
                     "Result is SUCCESS"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_OK_FULL, fabric index is F1"),
            TestStep(10, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_LENGTH",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(11, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_STRUCT",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(12, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_LIST",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(13, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_ELEM",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(14, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_OVERFLOW",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(15, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_UNDERFLOW",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(16, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element struct data field: D_BAD_NONE",
                     "Result is CONSTRAINT_ERROR (0x87)"),
            TestStep(17, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 2 elements struct data field: D_OK_EMPTY, D_OK_SINGLE",
                     "Result is CONSTRAINT_ERROR (0x87) for the entire list, even the first valid struct should be rejected"),
            TestStep(18, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is Success, AccessControlExtensionStruct containing 1 element; which is the last successfully written extension from Test Step 8; value is struct with data field: D_OK_FULL, fabric index is F1"),
            TestStep(19, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list",
                     "Result is SUCCESS"),
            TestStep(20, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is Success, value is an empty list"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_3(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F1: {str(f1)}")

        self.step(3)
        # Read AccessControlExtension attribute for TH1 value is empty list
        ac_cluster = Clusters.AccessControl
        ac_extension_attr = ac_cluster.Attributes.Extension
        ac_extension_value0 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value0)}")
        asserts.assert_equal(ac_extension_value0, [], "AccessControlExtension is empty list")

        self.step(4)
        # Write AccessControlExtension attribute for TH1 value is D_OK_EMPTY
        extension1 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)
        extensions_list1 = [extension1]
        result1 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list1))]
        )
        logging.info(f"Write result: {str(result1)}")
        asserts.assert_equal(
            result1[0].Status, Status.Success, "Write should have succeeded")

        self.step(5)
        # Read AccessControlExtension attribute for TH1 value is D_OK_EMPTY
        ac_extension_value1 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value1)}")
        asserts.assert_equal(ac_extension_value1[0].data, D_OK_EMPTY, "AccessControlExtension is D_OK_EMPTY")

        self.step(6)
        # Write AccessControlExtension attribute for TH1 value is D_OK_SINGLE
        extension2 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_SINGLE)
        extensions_list2 = [extension2]
        result2 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list2))]
        )
        logging.info(f"Write result: {str(result2)}")
        asserts.assert_equal(
            result2[0].Status, Status.Success, "Write should have succeeded")

        self.step(7)
        # Read AccessControlExtension attribute for TH1 value is D_OK_SINGLE
        ac_extension_value2 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value2)}")
        asserts.assert_equal(ac_extension_value2[0].data, D_OK_SINGLE, "AccessControlExtension is D_OK_SINGLE")

        self.step(8)
        # Write AccessControlExtension attribute for TH1 value is D_OK_FULL
        extension3 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_FULL)
        extensions_list3 = [extension3]
        result3 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list3))]
        )
        logging.info(f"Write result: {str(result3)}")
        asserts.assert_equal(
            result3[0].Status, Status.Success, "Write should have succeeded")

        self.step(9)
        # Read AccessControlExtension attribute for TH1 value is D_OK_FULL
        ac_extension_value3 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value3)}")
        asserts.assert_equal(ac_extension_value3[0].data, D_OK_FULL, "AccessControlExtension is D_OK_FULL")

        self.step(10)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_LENGTH
        extension4 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_LENGTH)
        extensions_list4 = [extension4]
        result4 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list4))]
        )
        logging.info(f"Write result: {str(result4)}")
        asserts.assert_equal(
            result4[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(11)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_STRUCT
        extension5 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_STRUCT)
        extensions_list5 = [extension5]
        result5 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list5))]
        )
        logging.info(f"Write result: {str(result5)}")
        asserts.assert_equal(
            result5[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(12)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_LIST
        extension6 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_LIST)
        extensions_list6 = [extension6]
        result6 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list6))]
        )
        logging.info(f"Write result: {str(result6)}")
        asserts.assert_equal(
            result6[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(13)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_ELEM
        extension7 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_ELEM)
        extensions_list7 = [extension7]
        result7 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list7))]
        )
        logging.info(f"Write result: {str(result7)}")
        asserts.assert_equal(
            result7[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(14)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_OVERFLOW
        extension8 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_OVERFLOW)
        extensions_list8 = [extension8]
        result8 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list8))]
        )
        logging.info(f"Write result: {str(result8)}")
        asserts.assert_equal(
            result8[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(15)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_UNDERFLOW
        extension9 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_UNDERFLOW)
        extensions_list9 = [extension9]
        result9 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list9))]
        )
        logging.info(f"Write result: {str(result9)}")
        asserts.assert_equal(
            result9[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(16)
        # Write AccessControlExtension attribute for TH1 value is D_BAD_NONE
        extension10 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_BAD_NONE)
        extensions_list10 = [extension10]
        result10 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list10))]
        )
        logging.info(f"Write result: {str(result10)}")
        asserts.assert_equal(
            result10[0].Status, Status.ConstraintError, "Write should have returned a CONSTRAINT_ERROR")

        self.step(17)
        # Write AccessControlExtension attribute for TH1 value is D_OK_EMPTY, D_OK_SINGLE
        extension11 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)
        extension12 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_SINGLE)
        extensions_list11 = [extension11, extension12]
        result11 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list11))]
        )
        logging.info(f"Write result: {str(result11)}")
        asserts.assert_equal(
            result11[0].Status,
            Status.ConstraintError,
            "Write should have returned a CONSTRAINT_ERROR for the entire list, even the first valid D_OK_EMPTY should be rejected ")

        self.step(18)
        # Read AccessControlExtension attribute for TH1 value is D_OK_FULL
        ac_extension_value4 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value4)}")
        asserts.assert_equal(
            ac_extension_value4[0].data,
            D_OK_FULL,
            "AccessControlExtension is D_OK_FULL from test step 8 as last successfully written extension")

        self.step(19)
        # Write AccessControlExtension attribute for TH1 value is an empty list
        extensions_list12 = []
        result12 = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, ac_extension_attr(value=extensions_list12))]
        )
        logging.info(f"Write result: {str(result12)}")
        asserts.assert_equal(
            result12[0].Status, Status.Success, "Write should have returned a SUCCESS")

        self.step(20)
        # Read AccessControlExtension attribute for TH1 value is an empty list
        ac_extension_value5 = await self.read_single_attribute_check_success(endpoint=0, cluster=ac_cluster, attribute=ac_extension_attr)
        logging.info(f"AccessControlExtension: {str(ac_extension_value5)}")
        asserts.assert_equal(ac_extension_value5, [], "AccessControlExtension is empty list")


if __name__ == "__main__":
    default_matter_test_main()
