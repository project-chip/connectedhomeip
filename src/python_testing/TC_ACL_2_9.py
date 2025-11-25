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
#       --endpoint 0
# === END CI TEST ARGUMENTS ===

import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_ACL_2_9(MatterBaseTest):
    async def read_and_check_min_value(self, attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor, min_value: int):
        result = await self.th2.ReadAttribute(self.dut_node_id, [(0, attribute)])
        logging.info(f"Result: {result}")
        value = result[0][Clusters.Objects.AccessControl][attribute]
        asserts.assert_greater_equal(
            value,
            min_value,
            f"{attribute.__name__} attribute should be {min_value} or greater, but got {value}"
        )

    async def read_event_expect_unsupported_access(self, event: Clusters.ClusterObjects.ClusterEvent):
        result = await self.th2.ReadEvent(self.dut_node_id, [(0, event)])
        asserts.assert_equal(result[0].Status, Status.UnsupportedAccess, f"Expected UnsupportedAccess but got {result[0].Status}")

    def desc_TC_ACL_2_9(self) -> str:
        return "[TC-ACL-2.9] Cluster access"

    def pics_TC_ACL_2_9(self) -> list[str]:
        return ['ACL.S.A0001']

    def steps_TC_ACL_2_9(self) -> list[TestStep]:
        steps = [
            TestStep(
                1,
                "TH1 commissions DUT using admin node ID N1",
                "DUT is commissioned on TH1 fabric",
                is_commissioning=True),
            TestStep(
                2,
                "TH1 opens commissioning window on DUT, TH2 commissions DUT using admin node ID N2",
                "DUT is commissioned on TH2 fabric"),
            TestStep(
                3,
                "TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element giving TH2 Manage access to the node",
                "Result is SUCCESS"),
            TestStep(
                4,
                "TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                5,
                "TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                6,
                "TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                7,
                "TH2 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                8,
                "TH2 reads DUT Endpoint 0 AccessControl cluster SubjectsPerAccessControlEntry attribute",
                "Result is SUCCESS,value is an integer with value 4 or greater."),
            TestStep(
                9,
                "TH2 reads DUT Endpoint 0 AccessControl cluster TargetsPerAccessControlEntry attribute",
                "Result is SUCCESS,value is an integer with value 3 or greater."),
            TestStep(
                10,
                "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntriesPerFabric attribute",
                "Result is SUCCESS, value is an integer with value 4 or greater."),
            TestStep(
                11,
                "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                12,
                "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                "Result is UNSUPPORTED_ACCESS (0x7e)"),
            TestStep(
                13,
                "TH2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx",
                "th2_idx set to value for CurrentFabricIndex attribute from TH2"),
            TestStep(
                14,
                "TH1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx",
                "TH1 removes TH2 fabric using th2_idx"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_9(self):
        self.step(1)
        # Create TH1 controller
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        self.endpoint = self.get_endpoint()
        extension_attribute = Clusters.AccessControl.Attributes.Extension

        self.step(2)
        # Create TH2 controller
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2)

        # Get the max window duration
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        # Open commissioning window on TH1
        params = await self.th1.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=self.max_window_duration,
            iteration=1000,
            discriminator=self.discriminator,
            option=1
        )

        # Commission TH2
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator
        )

        self.step(3)
        # TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is
        # list of AccessControlEntryStruct containing 1 element
        acl_attribute = Clusters.AccessControl.Attributes.Acl
        new_acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th2.nodeId],
                targets=NullValue,
            ),
        ]
        result = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=new_acl))]
        )
        # Add explicit check for Success
        if result[0].Status != Status.Success:
            asserts.fail(f"Expected Success but got {result[0].Status}")

        self.step(4)
        # TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute
        acl_attribute = Clusters.AccessControl.Attributes.Acl
        await self.read_single_attribute_expect_error(
            dev_ctrl=self.th2,
            cluster=Clusters.Objects.AccessControl,
            attribute=acl_attribute,
            error=Status.UnsupportedAccess,
            endpoint=0
        )

        self.step(5)
        # TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is
        # list of AccessControlEntryStruct containing 1 element
        new_acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th2.nodeId],
                targets=NullValue,
            ),
        ]
        result2 = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=new_acl))]
        )
        # Add explicit check for UnsupportedAccess error
        if result2[0].Status != Status.UnsupportedAccess:
            asserts.fail(f"Expected UnsupportedAccess but got {result2[0].Status}")

        self.step(6)
        # TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attribute):
            await self.read_single_attribute_expect_error(
                dev_ctrl=self.th2,
                cluster=Clusters.Objects.AccessControl,
                attribute=extension_attribute,
                error=Status.UnsupportedAccess,
                endpoint=0
            )

        self.step(7)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attribute):
            # TH2 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list
            new_extension = []
            result3 = await self.th2.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attribute(value=new_extension))]
            )
            # Add explicit check for UnsupportedAccess error
            if result3[0].Status != Status.UnsupportedAccess:
                asserts.fail(f"Expected UnsupportedAccess but got {result3[0].Status}")

        self.step(8)
        # TH2 reads DUT Endpoint 0 AccessControl cluster SubjectsPerAccessControlEntry attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.SubjectsPerAccessControlEntry, 4)

        self.step(9)
        # TH2 reads DUT Endpoint 0 AccessControl cluster TargetsPerAccessControlEntry attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.TargetsPerAccessControlEntry, 3)

        self.step(10)
        # TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntriesPerFabric attribute
        await self.read_and_check_min_value(Clusters.AccessControl.Attributes.AccessControlEntriesPerFabric, 4)

        self.step(11)
        # TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event
        await self.read_event_expect_unsupported_access(Clusters.AccessControl.Events.AccessControlEntryChanged)

        self.step(12)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attribute):
            # TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event
            await self.read_event_expect_unsupported_access(Clusters.AccessControl.Events.AccessControlExtensionChanged)

        self.step(13)
        # Read the CurrentFabricIndex attribute from the Operational Credentials cluster
        cluster = Clusters.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        th2_idx = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2,
            endpoint=0,
            cluster=cluster,
            attribute=attribute
        )

        self.step(14)
        # TH1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx)
        await self.th1.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=removeFabricCmd)


if __name__ == "__main__":
    default_matter_test_main()
