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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${all-clusters}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import copy
import logging
from typing import Type, Union

from mobly import asserts  # type: ignore

import matter.clusters as Clusters
from matter.exceptions import ChipStackError
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

ROOT_NODE_ENDPOINT_ID = 0
UNIT_TESTING_ENDPOINT_ID = 1
INVALID_ACTION_ERROR_CODE = 0x580


class TestReadSubscribeAceExistenceErrors(MatterBaseTest):

    async def get_dut_acl(self, ctrl):
        sub = await ctrl.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(ROOT_NODE_ENDPOINT_ID, Clusters.AccessControl.Attributes.Acl)],
            fabricFiltered=True
        )
        return sub[ROOT_NODE_ENDPOINT_ID][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

    async def write_acl(self, ctrl, acl):
        result = await ctrl.WriteAttribute(
            self.dut_node_id,
            [(ROOT_NODE_ENDPOINT_ID, Clusters.AccessControl.Attributes.Acl(acl))]
        )
        asserts.assert_equal(result[ROOT_NODE_ENDPOINT_ID].Status, Status.Success, "ACL write failed")

    async def grant_privilege_to_cluster(self, privilege, cluster, subject):
        ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=privilege,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=cluster)],
            subjects=[subject]
        )
        updated_acl = copy.deepcopy(self.dut_acl_original)
        updated_acl.append(ace)
        await self.write_acl(self.TH1, updated_acl)

    async def restore_acls_to_th1_only(self):
        """
        Restores the DUT ACL to its original state which only includes Access for TH1
        """
        await self.write_acl(self.TH1, self.dut_acl_original)

    @staticmethod
    def verify_attribute_exists(res: Union[Clusters.Attribute.SubscriptionTransaction, dict],
                                cluster:  Type[Clusters.ClusterObjects.Cluster],
                                attribute: Type[Clusters.ClusterObjects.ClusterAttributeDescriptor],
                                ep: int = ROOT_NODE_ENDPOINT_ID):
        '''
        This method can be used with the Response of Read Request and Subscribe Requests.

        res: the response of ReadAttribute when used with Subscription or Read Requests
        '''
        if isinstance(res, Clusters.Attribute.SubscriptionTransaction):
            attrs = res.GetAttributes()
        else:
            attrs = res

        asserts.assert_true(ep in attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in attrs[ep], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in attrs[ep][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

    @staticmethod
    def assert_event_exists(res, cluster, event, ep=ROOT_NODE_ENDPOINT_ID):
        res_events = res
        if isinstance(res, Clusters.Attribute.SubscriptionTransaction):
            res_events = res.GetEvents()

        asserts.assert_true(
            any(
                e.Header and e.Header.EndpointId == ep
                and e.Header.ClusterId == cluster.id
                and isinstance(e.Data, event)
                for e in res_events
            ),
            "Must have read back event %s at endpoint %s" % (event.__name__, ep)
        )

    @staticmethod
    def assert_expected_event_status_count(status, events, expected_count):
        actual_count = len([e for e in events if e.Status == status])
        asserts.assert_equal(
            actual_count,
            expected_count,
            f"Expected {expected_count} {status} EventStatusIB, but found {actual_count}"
        )

    @async_test_body
    async def setup_class(self):

        self.print_step("precondition", "Commissioning - already done")

        self.TH1 = self.default_controller
        # Save original DUT ACL used for resetting the ACLs
        self.dut_acl_original = await self.get_dut_acl(self.TH1)

        self.print_step("precondition", "Create Second Controller")

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        self.TH2_nodeid = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(
            nodeId=self.TH2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

    @async_test_body
    async def test_read_attribute_access_existence(self):

        #######################################    Step 1: Attribute exists; View privilege required to read.    ################################################
        #
        #

        self.print_step(
            "1a",
            "Attribute exists; View privilege required to read. "
            "No privileges granted to cluster under test."
        )

        AttrViewExists = Clusters.BasicInformation.Attributes.VendorID
        AttrViewPrivilegePath = (ROOT_NODE_ENDPOINT_ID, AttrViewExists)

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        read_step1a = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrViewPrivilegePath],
        )
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step1a[ROOT_NODE_ENDPOINT_ID][Clusters.BasicInformation][AttrViewExists].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "1b",
            "Attribute exists; View privilege required to read. "
            "View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to BasicInformation Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.BasicInformation.id,
            subject=self.TH2_nodeid
        )

        read_step1b = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrViewPrivilegePath],
        )

        # Verify Valid Attribute was read
        self.verify_attribute_exists(
            res=read_step1b,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        ####################### Step2: Attribute does not exist; View privilege required to read. ######################################################
        #
        #

        self.print_step(
            "2a",
            "Attribute does not exist; View privilege required to read. "
            "No privileges granted to cluster under test. Non-Existence should NOT be leaked"
        )

        AttrViewDoesNotExist = Clusters.UnitTesting.Attributes.Unsupported

        UnsupportedEndpointPath = (UNIT_TESTING_ENDPOINT_ID + 80, AttrViewDoesNotExist)
        UnsupportedClusterPath = (UNIT_TESTING_ENDPOINT_ID + 1, AttrViewDoesNotExist)
        UnsupportedAttributePath = (UNIT_TESTING_ENDPOINT_ID, AttrViewDoesNotExist)

        TestPaths = [UnsupportedEndpointPath, UnsupportedClusterPath, UnsupportedAttributePath]

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        read_step2a = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=TestPaths,
        )
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step2a[UNIT_TESTING_ENDPOINT_ID + 80][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step2a[UNIT_TESTING_ENDPOINT_ID + 1][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step2a[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "2b",
            "Attribute does not exist; View privilege required to read. "
            "At least View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to UnitTesting Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.UnitTesting.id,
            subject=self.TH2_nodeid
        )

        read_step2_granted = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=TestPaths,
        )

        asserts.assert_equal(Status.UnsupportedEndpoint,
                             read_step2_granted[UNIT_TESTING_ENDPOINT_ID + 80][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")

        asserts.assert_equal(Status.UnsupportedCluster,
                             read_step2_granted[UNIT_TESTING_ENDPOINT_ID + 1][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")

        asserts.assert_equal(Status.UnsupportedAttribute,
                             read_step2_granted[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrViewDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAttribute")

        ############################### Step3: Attribute exists; higher-than-view privilege required to read. ##############################################
        #
        #

        self.print_step(
            "3a",
            "Attribute exists; higher-than-view (Admin) privilege required to read. "
            "No privileges granted to cluster under test."
        )

        AttrNeedsAdminAndItExists = Clusters.AccessControl.Attributes.Acl
        AttrNeedsAdminAndItExistsPath = (ROOT_NODE_ENDPOINT_ID, AttrNeedsAdminAndItExists)

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        read_step3a = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrNeedsAdminAndItExistsPath],
        )
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step3a[ROOT_NODE_ENDPOINT_ID][Clusters.AccessControl][AttrNeedsAdminAndItExists].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "3b",
            "Attribute exists; higher-than-view (Admin) privilege required to read. "
            "Only View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step3b = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrNeedsAdminAndItExistsPath],
        )
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step3b[ROOT_NODE_ENDPOINT_ID][Clusters.AccessControl][AttrNeedsAdminAndItExists].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "3c",
            "Attribute exists; higher-than-view (Admin) privilege required to read. "
            "Admin privileges granted to cluster under test."

        )

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step3c = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrNeedsAdminAndItExistsPath],
        )

        # Verify Valid Attribute was read
        self.verify_attribute_exists(
            res=read_step3c,
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.Acl
        )

        ############################### Step4: Attribute does NOT exist; higher-than-view privilege required to read. ##############################################
        #
        #
        self.print_step(
            "4a",
            "Attribute does NOT exist; higher-than-view privilege required to read. "
            "No privileges granted to cluster under test. Non-Existence should NOT be leaked"
        )

        AttrNeedsAdminDoesNotExist = Clusters.UnitTesting.Attributes.UnsupportedAttributeRequiringAdminPrivilege

        UnsupportedEndpointPath = (UNIT_TESTING_ENDPOINT_ID + 80, AttrNeedsAdminDoesNotExist)
        UnsupportedClusterPath = (UNIT_TESTING_ENDPOINT_ID + 1, AttrNeedsAdminDoesNotExist)
        UnsupportedAttributePath = (UNIT_TESTING_ENDPOINT_ID, AttrNeedsAdminDoesNotExist)

        TestPaths = [UnsupportedEndpointPath, UnsupportedClusterPath, UnsupportedAttributePath]

        await self.restore_acls_to_th1_only()

        read_step4a = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=TestPaths,
        )

        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step4a[UNIT_TESTING_ENDPOINT_ID + 80][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step4a[UNIT_TESTING_ENDPOINT_ID + 1][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step4a[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "4b",
            "Attribute does NOT exist; higher-than-view privilege required to read. "
            "Only View privilege granted to cluster under test. It's acceptable for Existence to be leaked"
        )

        # Grant TH2 Only View Access to UnitTesting Cluster (View-or-Higher Access granted, but is less than the Required Admin Access)
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.UnitTesting.id,
            subject=self.TH2_nodeid
        )

        read_step4b = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=TestPaths,
        )
        asserts.assert_equal(Status.UnsupportedEndpoint,
                             read_step4b[UNIT_TESTING_ENDPOINT_ID + 80][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")
        asserts.assert_equal(Status.UnsupportedCluster,
                             read_step4b[UNIT_TESTING_ENDPOINT_ID + 1][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")
        asserts.assert_equal(Status.UnsupportedAttribute,
                             read_step4b[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAttribute")

        self.print_step(
            "4c",
            "Attribute does NOT exist; higher-than-view privilege required to read. "
            "Admin privileges granted to cluster under test."
        )

        # Grant TH2 Admin Privileges to UnitTesting Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.UnitTesting.id,
            subject=self.TH2_nodeid
        )

        read_step4c = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=TestPaths,
        )

        asserts.assert_equal(Status.UnsupportedEndpoint,
                             read_step4c[UNIT_TESTING_ENDPOINT_ID + 80][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")

        asserts.assert_equal(Status.UnsupportedCluster,
                             read_step4c[UNIT_TESTING_ENDPOINT_ID + 1][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")

        asserts.assert_equal(Status.UnsupportedAttribute,
                             read_step4c[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrNeedsAdminDoesNotExist].Reason.status, "Expected Attribute StatusIB with UnsupportedAttribute")

        ############################### Step5: Attribute exists; Attribute is Write-Only. ##############################################
        #
        #
        self.print_step(
            "5a",
            "Attribute exists; Attribute is Write-Only. "
            "No privileges granted to cluster under test. Non-Existence should NOT be leaked"
        )

        AttrWriteOnlyExists = Clusters.UnitTesting.Attributes.WriteOnlyInt8u
        AttrWriteOnlyExistsPath = (UNIT_TESTING_ENDPOINT_ID, AttrWriteOnlyExists)

        await self.restore_acls_to_th1_only()

        read_step5a = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrWriteOnlyExistsPath],
        )
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_step5a[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrWriteOnlyExists].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess")

        self.print_step(
            "5b",
            "Attribute exists; Attribute is Write-Only. "
            "View Privilege granted to cluster under test. UnsupportedRead will be Returned"
        )

        # Grant TH2 View Privileges to UnitTesting Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.UnitTesting.id,
            subject=self.TH2_nodeid
        )

        read_step5b = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[AttrWriteOnlyExistsPath],
        )

        asserts.assert_equal(Status.UnsupportedRead,
                             read_step5b[UNIT_TESTING_ENDPOINT_ID][Clusters.UnitTesting][AttrWriteOnlyExists].Reason.status, "Expected Attribute StatusIB with UnsupportedRead")

    @async_test_body
    async def test_read_event_access_existence(self):

        #######################################    Step 1: Event exists; View privilege required to read.    ################################################
        #
        #

        self.print_step(
            "1a",
            "Event exists; View privilege required to read. "
            "No privileges granted to cluster under test."
        )

        basicInformationStartUpEvent = Clusters.BasicInformation.Events.StartUp

        EventViewPrivilegePath = (ROOT_NODE_ENDPOINT_ID, basicInformationStartUpEvent)

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        read_step1a = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[EventViewPrivilegePath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=read_step1a,
                                                expected_count=1
                                                )

        self.print_step(
            "1b",
            "Event exists; View privilege required to read. "
            "View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to BasicInformation Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.BasicInformation.id,
            subject=self.TH2_nodeid
        )

        read_step1b = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[EventViewPrivilegePath],
        )

        # Verify Valid Event was read
        self.assert_event_exists(
            res=read_step1b,
            cluster=Clusters.BasicInformation,
            event=basicInformationStartUpEvent
        )

        ####################### Step2: Event does not exist; View privilege required to read. ######################################################
        #
        #

        self.print_step(
            "2a",
            "Event does not exist; View privilege required to read. "
            "No privileges granted to cluster under test. Non-Existence should NOT be leaked"
        )

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        UnsupportedEndpointPath = (ROOT_NODE_ENDPOINT_ID + 80, basicInformationStartUpEvent)
        UnsupportedClusterPath = (ROOT_NODE_ENDPOINT_ID + 1, basicInformationStartUpEvent)

        read_step2a = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[UnsupportedEndpointPath, UnsupportedClusterPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=read_step2a,
                                                expected_count=2
                                                )

        self.print_step(
            "2b",
            "Event does not exist; View privilege required to read. "
            "At least View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to UnitTesting Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.BasicInformation.id,
            subject=self.TH2_nodeid
        )

        read_step2b = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[UnsupportedEndpointPath, UnsupportedClusterPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedEndpoint,
                                                events=read_step2b,
                                                expected_count=1
                                                )
        self.assert_expected_event_status_count(status=Status.UnsupportedCluster,
                                                events=read_step2b,
                                                expected_count=1
                                                )

        ############################### Step3: Event exists; higher-than-view privilege required to read. ##############################################
        #
        #

        self.print_step(
            "3a",
            "Event exists; higher-than-view (Admin) privilege required to read. "
            "No privileges granted to cluster under test."
        )

        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged
        EventNeedsAdminAndItExistsPath = (ROOT_NODE_ENDPOINT_ID, aclChangedEvent)

        # Ensure TH2 has No ACCESS
        await self.restore_acls_to_th1_only()

        read_step3a = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[EventNeedsAdminAndItExistsPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=read_step3a,
                                                expected_count=1
                                                )

        self.print_step(
            "3b",
            "Event exists; higher-than-view (Admin) privilege required to read. "
            "Only View privilege granted to cluster under test."
        )

        # Grant TH2 View Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step3b = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[EventNeedsAdminAndItExistsPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=read_step3b,
                                                expected_count=1
                                                )

        self.print_step(
            "3c",
            "Event exists; higher-than-view (Admin) privilege required to read. "
            "Admin privileges granted to cluster under test."

        )

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step3c = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[EventNeedsAdminAndItExistsPath],
        )

        # Verify Valid Event is read for the valid Path
        self.assert_event_exists(
            res=read_step3c,
            cluster=Clusters.AccessControl,
            event=aclChangedEvent
        )

        ############################### Step4: Event does NOT exist; higher-than-view privilege required to read. ##############################################
        #
        #
        self.print_step(
            "4a",
            "Event does NOT exist; higher-than-view privilege required to read. "
            "No privileges granted to cluster under test. Non-Existence should NOT be leaked"
        )

        await self.restore_acls_to_th1_only()

        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged

        UnsupportedEndpointPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        UnsupportedClusterPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        read_step4a = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[UnsupportedEndpointPath, UnsupportedClusterPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=read_step4a,
                                                expected_count=2
                                                )

        self.print_step(
            "4b",
            "Event does NOT exist; higher-than-view privilege required to read. "
            "Only View privilege granted to cluster under test. It's acceptable for Existence to be leaked"
        )

        # Grant TH2 Only View Access to Access Control Cluster (View-or-Higher Access granted, but is less than the Required Admin Access)
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step4b = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[UnsupportedEndpointPath, UnsupportedClusterPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedEndpoint,
                                                events=read_step4b,
                                                expected_count=1
                                                )

        self.assert_expected_event_status_count(status=Status.UnsupportedCluster,
                                                events=read_step4b,
                                                expected_count=1
                                                )

        self.print_step(
            "4c",
            "Event does NOT exist; higher-than-view privilege required to read. "
            "Admin privileges granted to cluster under test."
        )

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        read_step4c = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[UnsupportedEndpointPath, UnsupportedClusterPath],
        )

        self.assert_expected_event_status_count(status=Status.UnsupportedEndpoint,
                                                events=read_step4c,
                                                expected_count=1
                                                )

        self.assert_expected_event_status_count(status=Status.UnsupportedCluster,
                                                events=read_step4c,
                                                expected_count=1
                                                )

    @async_test_body
    async def test_subscribe_access_existence(self):

        aclAttr = Clusters.AccessControl.Attributes.Acl
        validAclAttrPath = (ROOT_NODE_ENDPOINT_ID, aclAttr)

        vendorIdAttr = Clusters.BasicInformation.Attributes.VendorID
        validBasicInformationVendorIDPath = (ROOT_NODE_ENDPOINT_ID, vendorIdAttr)

        UnsupportedEndpointAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 80, aclAttr)
        UnsupportedClusterAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 1, aclAttr)
        #######################################    Step 1: Subscribe to Single Attribute on Valid Path with No Access    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Path (NO ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #
        self.print_step(1, "Subscribe Request to valid Concrete Attribute Path; No privileges granted to cluster under test")

        await self.restore_acls_to_th1_only()

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[validAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 2: Subscribe to Single Attribute on Unsupported Cluster with No Access   ################################################
        #
        #
        # Attribute Paths:
        #   1.Attribute Path on an Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #
        self.print_step(2, "Subscribe Request to Attribute with Unsupported Cluster; No privileges granted to cluster under test")

        await self.restore_acls_to_th1_only()

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[UnsupportedClusterAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )

        # We receive an Invalid Action rather than Unsupported Access, since no Report Data was sent
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 3: Subscribe to Single Attribute on Unsupported Cluster with Access   ################################################
        #
        #
        # Attribute Paths:
        #   1.Attribute Path on an Unsupported Cluster (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #
        self.print_step(3, "Subscribe Request to Attribute with Unsupported Cluster; Admin Privileges granted to cluster under test")

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[UnsupportedClusterAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 4: Subscribe Request to Mixed Valid and Invalid Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster0); No privileges granted to cluster under test    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Concrete Path (NO ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Response: Only INVALID_ACTION
        #
        self.print_step(
            4, "Subscribe Request to Mixed Valid and Invalid Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "No privileges granted to cluster under test")

        await self.restore_acls_to_th1_only()

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[validAclAttrPath,
                            UnsupportedEndpointAclAttrPath,
                            UnsupportedClusterAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        # All Paths will be discarded, and INVALID_ACTION is returned
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step5: Subscribe to Attributes Mixed Valid and Invalid Paths (Controller ONLY has Access to Valid Attribute Path)  ################################################
        #
        #
        # Two Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #
        self.print_step(
            5, "Subscribe Request to Mixed Valid and Invalid Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "Admin privileges granted to Valid Attribute Path ONLY")

        # Grant TH2 Admin Privileges to BasicInformation Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.BasicInformation.id,
            subject=self.TH2_nodeid
        )

        sub_step5 = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[validBasicInformationVendorIDPath,
                        UnsupportedEndpointAclAttrPath,
                        UnsupportedClusterAclAttrPath],
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Attribute Subscription came back
        self.verify_attribute_exists(
            res=sub_step5,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        # Verify we got Unsupported Access instead of Other Existence Errors
        sub_step5_attrs = sub_step5.GetAttributes()
        asserts.assert_equal(Status.UnsupportedAccess,
                             sub_step5_attrs[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess instead of UnsupportedEndpoint")
        asserts.assert_equal(Status.UnsupportedAccess,
                             sub_step5_attrs[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess instead of UnsupportedCluster")

        #######################################    Step 6: Subscribe to Attributes on Mixed Valid and Invalid Paths (WITH ACCESS)    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Concrete Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedEndpoint
        #   3. UnsupportedCluster
        #
        self.print_step(
            6, "Subscribe Request to Mixed Valid and Invalid Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "Admin privileges granted to all Paths")

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        sub_step6 = await self.TH2.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[validAclAttrPath,
                        UnsupportedEndpointAclAttrPath,
                        UnsupportedClusterAclAttrPath],
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )
        # Verify Valid Attribute Subscription came back
        self.verify_attribute_exists(
            res=sub_step6,
            cluster=Clusters.AccessControl,
            attribute=aclAttr
        )
        # Assert Error StatusIBs
        sub_attrs_step6 = sub_step6.GetAttributes()
        asserts.assert_equal(Status.UnsupportedEndpoint,
                             sub_attrs_step6[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")
        asserts.assert_equal(Status.UnsupportedCluster,
                             sub_attrs_step6[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")

        #################### Subscribe to Events ##############

        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged
        validAclEventPath = (ROOT_NODE_ENDPOINT_ID, aclChangedEvent)

        unsupportedEndpointAclEventPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        unsupportedClusterAclEventPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        basicInfoStartUpEvent = Clusters.BasicInformation.Events.StartUp
        validBasicInfoEventPath = (ROOT_NODE_ENDPOINT_ID, basicInfoStartUpEvent)

        #######################################    Step 7: Subscribe to Events on Mixed valid and Invalid Paths (WITH NO ACCESS to none of the paths)   ################################################
        #
        #
        # Two Event Paths:
        #   1.Valid Event Path (NO ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Response: Only Invalid Action
        #

        self.print_step(
            7, "Subscribe Request to Mixed Valid and Invalid Concrete Event Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "No privileges granted to none of the paths")

        await self.restore_acls_to_th1_only()

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadEvent(
                nodeId=self.dut_node_id,
                events=[validAclEventPath,
                        unsupportedEndpointAclEventPath,
                        unsupportedClusterAclEventPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        # All Paths will be discarded, and INVALID_ACTION is returned
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #####################################    Step 8: Subscribe to Events on Mixed valid and Invalid Paths (WITH ACCESS to the Valid Event Path Only)    ################################################
        #
        #
        #  Event Paths:
        #   1. Valid Event Path (HAS ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #

        self.print_step(
            8, "Subscribe Request to Mixed Valid and Invalid Concrete Event Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "Admin privileges granted to Valid Event Path ONLY")

        # Grant TH2 Admin Privileges to BasicInformation Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.BasicInformation.id,
            subject=self.TH2_nodeid
        )

        sub_step8 = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[validBasicInfoEventPath,
                    unsupportedEndpointAclEventPath,
                    unsupportedClusterAclEventPath],
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Event Subscription is received
        self.assert_event_exists(
            res=sub_step8,
            cluster=Clusters.BasicInformation,
            event=basicInfoStartUpEvent
        )

        # Verify Two Unsupported Access statuses are received rather than Unsupported Cluster and Unsupported Endpoint
        sub_step8_events = sub_step8.GetEvents()
        self.assert_expected_event_status_count(status=Status.UnsupportedAccess,
                                                events=sub_step8_events,
                                                expected_count=2)

        #######################################    Step 9: Subscribe to Events on Mixed valid and Invalid Paths (WITH ACCESS to all paths)   ################################################
        #
        #
        # Event Paths:
        #   1.Valid Event Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedEndpoint
        #   3. UnsupportedCluster
        #

        self.print_step(
            9, "Subscribe Request to Mixed Valid and Invalid Concrete Event Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster)"
            "Admin privileges granted to all Paths")

        # Grant TH2 Admin Privileges to AccessControl Cluster
        await self.grant_privilege_to_cluster(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            cluster=Clusters.AccessControl.id,
            subject=self.TH2_nodeid
        )

        sub_step9 = await self.TH2.ReadEvent(
            nodeId=self.dut_node_id,
            events=[validAclEventPath,
                    unsupportedEndpointAclEventPath,
                    unsupportedClusterAclEventPath],
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )
        # Verify Valid Event Subscription is received
        self.assert_event_exists(
            res=sub_step9,
            cluster=Clusters.AccessControl,
            event=aclChangedEvent
        )
        sub_step9_events = sub_step9.GetEvents()
        self.assert_expected_event_status_count(status=Status.UnsupportedEndpoint,
                                                events=sub_step9_events,
                                                expected_count=1
                                                )
        self.assert_expected_event_status_count(status=Status.UnsupportedCluster,
                                                events=sub_step9_events,
                                                expected_count=1
                                                )


if __name__ == "__main__":
    default_matter_test_main()
