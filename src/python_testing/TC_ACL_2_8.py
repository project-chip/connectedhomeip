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
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_8(MatterBaseTest):
    def _verify_acl_event(
            self,
            event,
            admin_node_id,
            admin_passcode_id,
            change_type,
            subjects,
            fabric_index):
        """Verifies ACL event contents"""
        data = event.Data

        # Verify admin IDs
        asserts.assert_equal(data.adminNodeID, admin_node_id)
        asserts.assert_equal(data.adminPasscodeID, admin_passcode_id)

        # Verify event data
        asserts.assert_equal(data.changeType, change_type)
        asserts.assert_equal(data.latestValue.privilege, 5)
        asserts.assert_equal(data.latestValue.authMode, 2)
        asserts.assert_equal(
            data.latestValue.subjects,
            [subjects] if not isinstance(
                subjects,
                list) else subjects)
        asserts.assert_equal(data.latestValue.targets, NullValue)
        asserts.assert_equal(data.fabricIndex, fabric_index)

    def desc_TC_ACL_2_8(self) -> str:
        return "[TC-ACL-2.8] ACL multi-fabric"

    def _validate_events_th1(self, events, f1, f2, is_filtered):
        """Helper method to validate events for TH1"""
        logging.info("Found %d events", len(events))

        # For TH1, we expect to see two events with f1 as fabric index
        expected_events_count = 2

        found_valid_events = 0
        found_th2_event = False

        for event in events:
            logging.info("Examining event: %s", str(event))
            if hasattr(event, 'Data') and hasattr(event.Data, 'fabricIndex'):
                # If this is a TH1 event
                if event.Data.fabricIndex == f1:
                    # Check for expected field values
                    if ((event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event.Data.adminNodeID == NullValue and
                        event.Data.adminPasscodeID == 0) or
                        (event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
                        event.Data.adminNodeID == self.th1.nodeId and
                            event.Data.adminPasscodeID == NullValue)):
                        found_valid_events += 1

                # If this is a TH2 event
                if event.Data.fabricIndex == f2:
                    found_th2_event = True

        asserts.assert_equal(found_valid_events, expected_events_count,
                             f"Expected {expected_events_count} valid events for TH1, found {found_valid_events}")

        if is_filtered:
            asserts.assert_false(found_th2_event, "TH1 should not see any events from TH2's fabric when fabric filtered")

    def _validate_events_th2(self, events, f1, f2, is_filtered):
        """Helper method to validate events for TH2"""
        logging.info("Found %d events", len(events))

        # For TH2, we expect to see two events with f2 as fabric index
        expected_events_count = 2

        found_valid_events = 0
        found_th1_event = False

        for event in events:
            logging.info("Examining event: %s", str(event))
            if hasattr(event, 'Data') and hasattr(event.Data, 'fabricIndex'):
                # If this is a TH2 event
                if event.Data.fabricIndex == f2:
                    # Check for expected field values
                    if ((event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event.Data.adminNodeID == NullValue and
                        event.Data.adminPasscodeID == 0) or
                        (event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
                        event.Data.adminNodeID == self.th2.nodeId and
                            event.Data.adminPasscodeID == NullValue)):
                        found_valid_events += 1

                # If this is a TH1 event
                if event.Data.fabricIndex == f1:
                    found_th1_event = True

        asserts.assert_equal(found_valid_events, expected_events_count,
                             f"Expected {expected_events_count} valid events for TH2, found {found_valid_events}")

        if is_filtered:
            asserts.assert_false(found_th1_event, "TH2 should not see any events from TH1's fabric when fabric filtered")

    def steps_TC_ACL_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID N1",
                     "DUT is commissioned on TH1 fabric", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 puts DUT into commissioning mode, TH2 commissions DUT using admin node ID N2",
                     "DUT is commissioned on TH2 fabric"),
            TestStep(4, "TH2 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F2"),
            TestStep(5, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element",
                     "Result is SUCCESS"),
            TestStep(6, "TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute value is list of AccessControlEntryStruct containing 1 element",
                     "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 1 element when fabric filtered, and the list MUST NOT contain an element with FabricIndex F2"),
            TestStep(8, "TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 1 element when fabric filtered, and the list MUST NOT contain an element with FabricIndex F1"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements when fabric filtered, and the list MUST NOT contain any element with FabricIndex F2"),
            TestStep(10, "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements when fabric filtered, and the list MUST NOT contain any element with FabricIndex F1"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_8(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F1 {str(f1)}")

        self.step(3)
        # Commission TH2
        params = await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(
            nodeId=2, useTestCommissioner=True)
        setupPinCode = params.setupPinCode
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

        self.step(4)
        # Read CurrentFabricIndex for TH2
        f2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F2 {str(f2)}")

        self.step(5)
        # TH1 writes ACL attribute
        acl_struct = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=5,
            authMode=2,
            subjects=[self.th1.nodeId, 1111]
        )
        acl_attr = Clusters.AccessControl.Attributes.Acl
        acl_list = [acl_struct]
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))]
        )
        asserts.assert_equal(
            result[0].Status,
            Status.Success,
            "Write should have succeeded")

        self.step(6)
        # TH2 writes ACL attribute
        acl_struct_th2 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=5,
            authMode=2,
            subjects=[self.th2.nodeId, 2222]
        )
        acl_list = [acl_struct_th2]
        result = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))]
        )
        asserts.assert_equal(
            result[0].Status,
            Status.Success,
            "Write should have succeeded")

        self.step(7)
        # TH1 reads ACL attribute with fabricFiltered=True (default)
        ac_cluster = Clusters.AccessControl
        acl_attr = Clusters.AccessControl.Attributes.Acl

        # Read with fabric_filtered=True (default)
        acl_list_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=acl_attr
        )
        logging.info("TH1 read ACL result (fabricFiltered=True): %s", str(acl_list_filtered))

        asserts.assert_equal(
            len(acl_list_filtered), 1, "Should have exactly one ACL entry when fabric filtered")
        entry = acl_list_filtered[0]

        asserts.assert_equal(
            entry.privilege, 5, "Privilege should be Administer (5)")
        asserts.assert_equal(
            entry.authMode, 2, "AuthMode should be CASE (2)")
        asserts.assert_equal(entry.subjects, [self.th1.nodeId, 1111])
        asserts.assert_equal(
            entry.targets,
            NullValue,
            "Targets should be NullValue")
        asserts.assert_equal(entry.fabricIndex, f1)

        for entry in acl_list_filtered:
            asserts.assert_not_equal(
                entry.fabricIndex, f2, "Should not contain entry with FabricIndex F2")

        # Read with fabric_filtered=False
        acl_list_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=acl_attr,
            fabric_filtered=False
        )
        logging.info("TH1 read ACL result (fabric_filtered=False): %s", str(acl_list_unfiltered))
        asserts.assert_equal(len(acl_list_unfiltered), 2, "Should have two ACL entries when not fabric filtered")
        # Check non-accessing fabric entry is empty because data leaks are bad
        for entry in acl_list_unfiltered:
            if entry.fabricIndex == f2:
                asserts.assert_equal(entry.privilege, 0)
                asserts.assert_equal(entry.authMode, 0)
                asserts.assert_true(
                    entry.subjects is None or entry.subjects == NullValue or entry.subjects == [],
                    "Non-accessing fabric: subjects should be empty"
                )
                asserts.assert_true(
                    entry.targets is None or entry.targets == NullValue,
                    "Non-accessing fabric: targets should be empty"
                )

        self.step(8)
        # TH2 reads ACL attribute with fabricFiltered=True (default)
        acl_list_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=acl_attr
        )
        logging.info("TH2 read ACL result (fabric_filtered=True): %s", str(acl_list_filtered))

        asserts.assert_equal(
            len(acl_list_filtered), 1, "Should have exactly one ACL entry when fabric filtered")
        entry = acl_list_filtered[0]

        # Verify entry contents
        asserts.assert_equal(
            entry.privilege, 5, "Privilege should be Administer (5)")
        asserts.assert_equal(
            entry.authMode, 2, "AuthMode should be CASE (2)")
        asserts.assert_equal(entry.subjects, [self.th2.nodeId, 2222])
        asserts.assert_equal(
            entry.targets,
            NullValue,
            "Targets should be NullValue")
        asserts.assert_equal(entry.fabricIndex, f2)

        for entry in acl_list_filtered:
            asserts.assert_not_equal(
                entry.fabricIndex, f1, "Should not contain entry with FabricIndex F1")

        # Read with fabric_filtered=False
        acl_list_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=acl_attr,
            fabric_filtered=False
        )
        logging.info("TH2 read ACL result (fabric_filtered=False): %s", str(acl_list_unfiltered))
        asserts.assert_equal(len(acl_list_unfiltered), 2, "Should have two ACL entries when not fabric filtered")
        # Check non-accessing fabric entry is empty because data leaks are bad
        for entry in acl_list_unfiltered:
            if entry.fabricIndex == f1:
                asserts.assert_equal(entry.privilege, 0)
                asserts.assert_equal(entry.authMode, 0)
                asserts.assert_true(
                    entry.subjects is None or entry.subjects == NullValue or entry.subjects == [],
                    "Non-accessing fabric: subjects should be empty"
                )
                asserts.assert_true(
                    entry.targets is None or entry.targets == NullValue,
                    "Non-accessing fabric: targets should be empty"
                )

        self.step(9)
        # TH1 reads AccessControlEntryChanged events with fabricFiltered=True
        events_filtered = await self.th1.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )
        self._validate_events_th1(events_filtered, f1, f2, True)

        # Read with fabricFiltered=False
        events_unfiltered = await self.th1.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=False
        )
        self._validate_events_th1(events_unfiltered, f1, f2, False)

        self.step(10)
        # TH2 reads AccessControlEntryChanged events with fabricFiltered=True
        events_filtered = await self.th2.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )
        self._validate_events_th2(events_filtered, f1, f2, True)

        # Read with fabricFiltered=False
        events_unfiltered = await self.th2.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=False
        )
        self._validate_events_th2(events_unfiltered, f1, f2, False)


if __name__ == "__main__":
    default_matter_test_main()
