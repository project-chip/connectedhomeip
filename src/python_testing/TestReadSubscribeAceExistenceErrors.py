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
#     app: ${ALL_CLUSTERS_APP}
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

import chip.clusters as Clusters
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

ROOT_NODE_ENDPOINT_ID = 0
UNIT_TESTING_ENDPOINT_ID = 1
INVALID_ACTION_ERROR_CODE = 0x580


class TestReadSubscribeAceExistenceErrors(MatterBaseTest):

    async def get_dut_acl(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )

        acl_list = sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        return acl_list

    async def add_ace_to_dut_acl(self, ctrl, ace, dut_acl_original):
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ROOT_NODE_ENDPOINT_ID, Clusters.AccessControl.Attributes.Acl(dut_acl))])
        asserts.assert_equal(result[ROOT_NODE_ENDPOINT_ID].Status, Status.Success, "ACL write failed")

    @staticmethod
    def verify_attribute_exists(res, cluster, attribute, ep=ROOT_NODE_ENDPOINT_ID):
        attrs = res
        if isinstance(attrs, Clusters.Attribute.SubscriptionTransaction):
            attrs = attrs.GetAttributes()

        asserts.assert_true(ep in attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in attrs[ep], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in attrs[ep][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

    @staticmethod
    def verify_event_exists(res, cluster, event, ep=ROOT_NODE_ENDPOINT_ID):
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

    @async_test_body
    async def setup_class(self):

        self.print_step("precondition", "Commissioning - already done")
        self.TH1 = self.default_controller

        self.print_step("precondition", "Create Second Controller")

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        CR2_nodeid = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(
            nodeId=CR2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        self.print_step(
            "precondition", "Enforce Access Control for 2nd Controller: 2nd Controller ONLY has Access to the BasicInformation Cluster")

        CR2_limited_ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.BasicInformation.id)],
            subjects=[CR2_nodeid])

        # Original DUT ACL used for resetting the ACL
        dut_acl_original = await self.get_dut_acl(self.TH1)

        await self.add_ace_to_dut_acl(self.TH1, CR2_limited_ace, dut_acl_original)

    @async_test_body
    async def test_subscribe_access_existence(self):

        aclAttr = Clusters.AccessControl.Attributes.Acl
        UnsupportedReadAttr = Clusters.UnitTesting.Attributes.WriteOnlyInt8u

        validAclAttrPath = (ROOT_NODE_ENDPOINT_ID, aclAttr)
        UnsupportedEndpointAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 80, aclAttr)
        UnsupportedClusterAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 1, aclAttr)
        UnsupportedReadAttrPath = (UNIT_TESTING_ENDPOINT_ID, UnsupportedReadAttr)

        #######################################    Step 1: Subscribe to Single Attribute on Valid Path with No Access    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #

        self.print_step(1, "Subscribe Request from Controller 2 To Valid Concrete Attribute Path  it has NO Access to")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=[validAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 2: Subscribe to Single Attribute on Unsupported Cluster (Controller HAS Access)    ################################################
        #
        #
        # Attribute Paths:
        #   1.Attribute Path on an Unsupported Cluster (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #

        self.print_step(2, "Subscribe Request from Controller 1 to Unsupported Cluster (It has Access to)")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH1.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=[UnsupportedClusterAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 3: Subscribe to Single Attribute on Unsupported Cluster (Controller has NO Access)   ################################################
        #
        #
        # Attribute Paths:
        #   1.Attribute Path on an Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. INVALID_ACTION
        #

        self.print_step(3, "Subscribe Request from Controller 2 (NO Access) to Unsupported Cluster")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=[UnsupportedClusterAclAttrPath],
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        # We receive an Invalid Action rather than Unsupported Access, since no Report Data was sent
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 4: Subscribe to Attributes on Mixed Valid and Invalid Paths (WITH ACCESS)    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Concrete Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #   4. Unsupported Read (HAVE Access)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedEndpoint
        #   3. UnsupportedCluster
        #   4. UnsupportedRead
        #
        self.print_step(
            4, "Subscribe Request from Controller 1 to Mixed Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster + Unsupported Read ) - Controller 1 has Access to all paths")

        AttrPathsStep4 = [validAclAttrPath,
                          UnsupportedEndpointAclAttrPath,
                          UnsupportedClusterAclAttrPath,
                          UnsupportedReadAttrPath]

        sub_th1_step4 = await self.TH1.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=AttrPathsStep4,
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Attribute Subscription came back
        self.verify_attribute_exists(
            res=sub_th1_step4,
            cluster=Clusters.AccessControl,
            attribute=aclAttr
        )

        # Assert Error StatusIBs
        sub_attrs_step4 = sub_th1_step4.GetAttributes()

        asserts.assert_equal(Status.UnsupportedEndpoint,
                             sub_attrs_step4[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")

        asserts.assert_equal(Status.UnsupportedCluster,
                             sub_attrs_step4[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")

        asserts.assert_equal(Status.UnsupportedRead, sub_attrs_step4[UNIT_TESTING_ENDPOINT_ID]
                             [Clusters.UnitTesting][UnsupportedReadAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedRead")

        #######################################    Step 5: Subscribe Attributes Mixed Valid and Invalid Paths (Controller has NO ACCESS to None of the paths)    ################################################
        #
        #
        # Attribute Paths:
        #   1.Valid Attribute Concrete Path (NO ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #   4. Unsupported Read (NO Access)
        #
        # Expected Response: Only INVALID_ACTION
        #
        self.print_step(
            5, "Subscribe Request from Controller 2 to Mixed Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster + Unsupported Read)  - Controller 2 does NOT have Access to none of the paths")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=AttrPathsStep4,
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )
        # All Paths will be discarded, and INVALID_ACTION is returned
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #######################################    Step 6: Subscribe to Attributes Mixed Valid and Invalid Paths (Controller ONLY has Access to Valid Attribute Path)  ################################################
        #
        #
        # Two Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #   4. Unsupported Read (NO Access)
        #
        # Expected Responses:
        #   1. Subscription Success
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #   4. UnsupportedAccess
        #
        self.print_step(
            6, "Subscribe Request from Controller 2 to Mixed Attributes Paths (Valid Concrete Path + Unsupported Endpoint + Unsupported Cluster + Unsupported Read) - Controller 2 only has Access to Valid Concrete Attribute Path")

        attrPathsStep6 = [
            (ROOT_NODE_ENDPOINT_ID, Clusters.BasicInformation.Attributes.VendorID),
            UnsupportedEndpointAclAttrPath,
            UnsupportedClusterAclAttrPath,
            UnsupportedReadAttrPath]

        sub_th2_step6 = await self.TH2.ReadAttribute(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            attributes=attrPathsStep6,
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Attribute Subscription came back
        self.verify_attribute_exists(
            res=sub_th2_step6,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        # Verify we got Unsupported Access instead of Other Existence Errors
        sub_step6_attrs = sub_th2_step6.GetAttributes()

        asserts.assert_equal(Status.UnsupportedAccess,
                             sub_step6_attrs[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess instead of UnsupportedEndpoint")

        asserts.assert_equal(Status.UnsupportedAccess,
                             sub_step6_attrs[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess instead of UnsupportedCluster")

        asserts.assert_equal(Status.UnsupportedAccess, sub_step6_attrs[UNIT_TESTING_ENDPOINT_ID]
                             [Clusters.UnitTesting][UnsupportedReadAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess instead of UnsupportedRead")

        #######################################    Step 7: Subscribe to Events on Mixed valid and Invalid Paths (WITH ACCESS to all paths)   ################################################
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
            7, "Subscribe Request from Controller 1 to Mixed Events Path (Valid Event + Unsupported Endpoint + Unsupported Cluster) - Controller 1 has Access to all paths")

        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged

        validAclEventPath = (ROOT_NODE_ENDPOINT_ID, aclChangedEvent)
        unsupportedEndpointAclEventPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        unsupportedClusterAclEventPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        EventPathsStep7 = [validAclEventPath,
                           unsupportedEndpointAclEventPath,
                           unsupportedClusterAclEventPath]

        sub_th1_step7 = await self.TH1.ReadEvent(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            events=EventPathsStep7,
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Event Subscription is received
        self.verify_event_exists(
            res=sub_th1_step7,
            cluster=Clusters.AccessControl,
            event=aclChangedEvent
        )

        sub_step7_events = sub_th1_step7.GetEvents()

        UnsupportedEndpointList = [e for e in sub_step7_events if e.Status == Status.UnsupportedEndpoint]
        asserts.assert_equal(
            len(UnsupportedEndpointList),
            1,
            f"Expected 1 UnsupportedEndpoint EventStatusIB, but found {len(UnsupportedEndpointList)}"
        )
        unsupportedClusterList = [e for e in sub_step7_events if e.Status == Status.UnsupportedCluster]
        asserts.assert_equal(
            len(unsupportedClusterList),
            1,
            f"Expected 1 UnsupportedCluster EventStatusIB, but found {len(unsupportedClusterList)}"
        )

        #######################################    Step 8: Subscribe to Events on Mixed valid and Invalid Paths (WITH NO ACCESS to none of the paths)   ################################################
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
            8, "Subscribe Request from Controller 2 to Mixed valid and invalid Concrete Event Paths (Valid Event + Unsupported Endpoint + Unsupported Cluster) - Controller 2 does NOT have Access to none of the paths ")

        with asserts.assert_raises(ChipStackError) as cm:
            await self.TH2.ReadEvent(
                nodeid=self.dut_node_id,
                events=EventPathsStep7,
                keepSubscriptions=False,
                reportInterval=(1, 5),
                autoResubscribe=False
            )

        # All Paths will be discarded, and INVALID_ACTION is returned
        asserts.assert_equal(cm.exception.err, INVALID_ACTION_ERROR_CODE,
                             "Expected Invalid_Action since all paths are discarded")

        #####################################    Step 9: Subscribe to Events on Mixed valid and Invalid Paths (WITH ACCESS to the Valid Event Path Only)    ################################################
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
            9, "Subscribe Request from Controller 2 to Mixed valid and invalid Events Paths (Valid Event + Unsupported Endpoint + Unsupported Cluster) - Controller 2 ONLY has access to the valid concrete event paths")

        basicInfoStartUpEvent = Clusters.BasicInformation.Events.StartUp
        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged

        validBasicInfoEventPath = (ROOT_NODE_ENDPOINT_ID, basicInfoStartUpEvent)
        unsupportedEndpointAclEventPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        unsupportedClusterAclEventPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        event_paths_step9 = [
            validBasicInfoEventPath,
            unsupportedEndpointAclEventPath,
            unsupportedClusterAclEventPath]

        sub_th2_step9 = await self.TH2.ReadEvent(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            events=event_paths_step9,
            keepSubscriptions=False,
            reportInterval=(1, 5),
            autoResubscribe=False
        )

        # Verify Valid Event Subscription is received
        self.verify_event_exists(
            res=sub_th2_step9,
            cluster=Clusters.BasicInformation,
            event=basicInfoStartUpEvent
        )

        sub_step9_events = sub_th2_step9.GetEvents()

        unsupportedAccessList = 0

        # Verify Two Unsupported Access statuses are received rather than Unsupported Cluster and Unsupported Endpoint
        unsupportedAccessList = [e for e in sub_step9_events if e.Status == Status.UnsupportedAccess]
        asserts.assert_equal(
            len(unsupportedAccessList),
            2,
            f"Expected 2 UnsupportedAccess EventStatusIBs, but found {len(unsupportedAccessList)}"
        )

    @async_test_body
    async def test_read_access_existence(self):

        aclAttr = Clusters.AccessControl.Attributes.Acl
        UnsupportedReadAttr = Clusters.UnitTesting.Attributes.WriteOnlyInt8u

        validAclAttrPath = (ROOT_NODE_ENDPOINT_ID, aclAttr)
        UnsupportedEndpointAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 80, aclAttr)
        UnsupportedClusterAclAttrPath = (ROOT_NODE_ENDPOINT_ID + 1, aclAttr)
        UnsupportedReadAttrPath = (UNIT_TESTING_ENDPOINT_ID, UnsupportedReadAttr)

        #######################################    Step 1: Read Attributes with Access    ################################################
        #
        #
        #  Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #   4. Unsupported Read (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. Read Success
        #   2. UnsupportedEndpoint
        #   3. UnsupportedCluster
        #   4. UnsupportedRead
        #
        self.print_step(
            1, "Read Request from Controller 1 to Mixed Path (Valid + Unsupported Endpoint + Unsupported Cluster + Unsupported Read ) - Controller 1 has Access to all paths")

        AttrPaths = [validAclAttrPath,
                     UnsupportedEndpointAclAttrPath,
                     UnsupportedClusterAclAttrPath,
                     UnsupportedReadAttrPath]

        read_th1_step1 = await self.TH1.ReadAttribute(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            attributes=AttrPaths,

        )

        # Verify Valid Acl Attribute was read
        self.verify_attribute_exists(
            res=read_th1_step1,
            cluster=Clusters.AccessControl,
            attribute=aclAttr
        )

        asserts.assert_equal(Status.UnsupportedEndpoint,
                             read_th1_step1[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedEndpoint")

        asserts.assert_equal(Status.UnsupportedCluster,
                             read_th1_step1[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedCluster")

        asserts.assert_equal(Status.UnsupportedRead, read_th1_step1[UNIT_TESTING_ENDPOINT_ID]
                             [Clusters.UnitTesting][UnsupportedReadAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedRead")

        #######################################    Step 2: Read Attributes WITHOUT Access     ################################################
        #
        #
        #  Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #   4. Unsupported Read (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. UnsupportedAccess
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #   4. UnsupportedAccess
        #
        self.print_step(
            2, "Read Request from Controller 2 to Mixed Attribute Paths (Valid + Unsupported Endpoint + Unsupported Cluster + Unsupported Read ) - Controller 2 does NOT have Access to none of the paths")

        read_th2_step2 = await self.TH2.ReadAttribute(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            attributes=AttrPaths,

        )

        asserts.assert_equal(Status.UnsupportedAccess,
                             read_th2_step2[ROOT_NODE_ENDPOINT_ID][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Valid Attribute Path")

        asserts.assert_equal(Status.UnsupportedAccess,
                             read_th2_step2[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Concrete Path with Unsupported Endpoint ID")

        asserts.assert_equal(Status.UnsupportedAccess,
                             read_th2_step2[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Concrete Path with Unsupported Cluster")

        asserts.assert_equal(Status.UnsupportedAccess, read_th2_step2[UNIT_TESTING_ENDPOINT_ID]
                             [Clusters.UnitTesting][UnsupportedReadAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Concrete Path with non-readable Attribute")

        #######################################    Step 3: Read Attributes with Mixed Access   ################################################
        #
        #
        # Two Attribute Paths:
        #   1.Valid Attribute Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. Read Success
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #
        self.print_step(
            3, "Read Request from Controller 2 to Mixed Attribute Paths (Valid + Unsupported Endpoint + Unsupported Cluster ) - Controller 2 ONLY has access to the Valid Attribute Path")

        attrPathsMixedAccess = [
            (ROOT_NODE_ENDPOINT_ID, Clusters.BasicInformation.Attributes.VendorID),
            UnsupportedEndpointAclAttrPath,
            UnsupportedClusterAclAttrPath
        ]

        read_th2_step3 = await self.TH2.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=attrPathsMixedAccess,
        )

        # Verify Valid Attribute was read
        self.verify_attribute_exists(
            res=read_th2_step3,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        # Verify we got Unsupported Access instead of Unsupported Endpoint and Unsupported Access
        asserts.assert_equal(Status.UnsupportedAccess,
                             read_th2_step3[ROOT_NODE_ENDPOINT_ID + 80][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Concrete Path with Unsupported Endpoint")

        asserts.assert_equal(Status.UnsupportedAccess,
                             read_th2_step3[ROOT_NODE_ENDPOINT_ID + 1][Clusters.AccessControl][aclAttr].Reason.status, "Expected Attribute StatusIB with UnsupportedAccess for Concrete Path with Unsupported Cluster")

        #######################################    Step 4: Read Events with Access   ################################################
        #
        #
        # Event Paths:
        #   1.Valid Event Path (HAVE ACCESS)
        #   2. Unsupported Endpoint (HAVE ACCESS)
        #   3. Unsupported Cluster (HAVE ACCESS)
        #
        # Expected Responses:
        #   1. Read Success
        #   2. UnsupportedEndpoint
        #   3. UnsupportedCluster
        #
        self.print_step(
            4, "Read Request from Controller 1 to Mixed Concrete Event Paths (Valid Event Paths + Unsupported Endpoint + Unsupported Cluster) - Controller 1 has Access to all paths")

        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged

        validAclEventPath = (ROOT_NODE_ENDPOINT_ID, aclChangedEvent)
        UnsupportedEndpointAclEventPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        unsupportedClusterAclEventPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        eventPaths = [validAclEventPath,
                      UnsupportedEndpointAclEventPath,
                      unsupportedClusterAclEventPath]

        read_th2_step4 = await self.TH1.ReadEvent(
            nodeid=self.dut_node_id,
            events=eventPaths,
        )

        # Verify Valid Event was read
        self.verify_event_exists(
            res=read_th2_step4,
            cluster=Clusters.AccessControl,
            event=aclChangedEvent
        )

        UnsupportedEndpointList = [e for e in read_th2_step4 if e.Status == Status.UnsupportedEndpoint]
        asserts.assert_equal(
            len(UnsupportedEndpointList),
            1,
            f"Expected 1 UnsupportedEndpoint EventStatusIB, but found {len(UnsupportedEndpointList)}"
        )
        unsupportedClusterList = [e for e in read_th2_step4 if e.Status == Status.UnsupportedCluster]
        asserts.assert_equal(
            len(unsupportedClusterList),
            1,
            f"Expected 1 UnsupportedCluster EventStatusIB, but found {len(unsupportedClusterList)}"
        )

        #######################################    Step 5: Read Events WITHOUT Access   ################################################
        #
        #
        # Event Paths:
        #   1.Valid Event Path (NO ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. UnsupportedAccess
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #
        self.print_step(
            5, "Read Request from Controller 2 to Mixed Event Paths (Valid Event + Unsupported Endpoint + Unsupported Cluster)  - Controller 2 does NOT have Access to none of the paths")

        read_th2_step5 = await self.TH2.ReadEvent(
            nodeid=self.dut_node_id,
            events=eventPaths,
        )

        unsupportedAccessList = [e for e in read_th2_step5 if e.Status == Status.UnsupportedAccess]
        asserts.assert_equal(
            len(unsupportedAccessList),
            3,
            f"Expected 3 UnsupportedAccess EventStatusIB, but found {len(unsupportedAccessList)}"
        )
        #####################################    Step 6: Read Events with MIXED Access    ################################################
        #
        #
        # Event Paths:
        #   1. Valid Event Path (HAS ACCESS)
        #   2. Unsupported Endpoint (NO ACCESS)
        #   3. Unsupported Cluster (NO ACCESS)
        #
        # Expected Responses:
        #   1. Read Success for the Valid Concrete Path
        #   2. UnsupportedAccess
        #   3. UnsupportedAccess
        #
        self.print_step(
            6, "Read Request from Controller 2 to Mixed Event Paths (Valid Event + Unsupported Endpoint + Unsupported Cluster)  - Controller 2 ONLY HAS Access to the Valid Event Path")

        basicInfoStartUpEvent = Clusters.BasicInformation.Events.StartUp
        aclChangedEvent = Clusters.AccessControl.Events.AccessControlEntryChanged

        validBasicInfoEventPath = (ROOT_NODE_ENDPOINT_ID, basicInfoStartUpEvent)
        UnsupportedEndpointAclEventPath = (ROOT_NODE_ENDPOINT_ID + 80, aclChangedEvent)
        unsupportedClusterAclEventPath = (ROOT_NODE_ENDPOINT_ID + 1, aclChangedEvent)

        eventPathsStep6 = [
            validBasicInfoEventPath,
            UnsupportedEndpointAclEventPath,
            unsupportedClusterAclEventPath]

        read_th2_step6 = await self.TH2.ReadEvent(
            nodeid=self.dut_node_id,
            # Attribute from a cluster controller 2 has no access to
            events=eventPathsStep6,

        )

        # Verify Valid Event is read for the valid Path
        self.verify_event_exists(
            res=read_th2_step6,
            cluster=Clusters.BasicInformation,
            event=basicInfoStartUpEvent
        )

        unsupportedAccessList = 0

        unsupportedAccessList = [e for e in read_th2_step6 if e.Status == Status.UnsupportedAccess]
        asserts.assert_equal(
            len(unsupportedAccessList),
            2,
            f"Expected 2 UnsupportedAccess EventStatusIB, but found {len(unsupportedAccessList)}"
        )


if __name__ == "__main__":
    default_matter_test_main()
