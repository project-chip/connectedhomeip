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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factoryreset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --int-arg PIXIT.ACE.APPENDPOINT:1 PIXIT.ACE.APPDEVTYPEID:0x0100
#       --string-arg PIXIT.ACE.APPCLUSTER:OnOff PIXIT.ACE.APPATTRIBUTE:OnOff
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import sys

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT.ACE.ENDPOINT:<endpoint> PIXIT.ACE.APPDEVTYPE:<device_type_id>
# --string-arg PIXIT.ACE.APPCLUSTER:<cluster_name> PIXIT.ACE.APPATTRIBUTE:<attribute_name>


def str_to_cluster(str):
    return getattr(sys.modules["chip.clusters.Objects"], str)


def str_to_attribute(cluster, str):
    return getattr(cluster.Attributes, str)


class TC_ACE_1_4(MatterBaseTest):

    async def write_acl(self, acl):
        # This returns an attribute status
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    async def read_descriptor_expect_success(self, endpoint: int) -> None:
        cluster = Clusters.Objects.Descriptor
        attribute = Clusters.Descriptor.Attributes.DeviceTypeList
        await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_descriptor_expect_unsupported_access(self, endpoint: int) -> None:
        cluster = Clusters.Objects.Descriptor
        attribute = Clusters.Descriptor.Attributes.DeviceTypeList
        await self.read_single_attribute_expect_error(
            endpoint=endpoint, cluster=cluster, attribute=attribute, error=Status.UnsupportedAccess)

    async def read_appcluster_expect_success(self) -> None:
        await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.attribute)

    async def read_appcluster_expect_unsupported_access(self) -> None:
        await self.read_single_attribute_expect_error(
            endpoint=self.endpoint, cluster=self.cluster, attribute=self.attribute, error=Status.UnsupportedAccess)

    async def read_wildcard_endpoint(self, attribute: object) -> object:
        return await self.default_controller.ReadAttribute(self.dut_node_id, [(attribute)])

    def check_read_success(self, results: object, endpoint: int, cluster: object, attribute: object) -> None:
        err_msg = "Results not returned for ep {}".format(str(endpoint))
        asserts.assert_true(endpoint in results, err_msg)
        err_msg = "Results not returned for cluster {} on ep {}".format(str(cluster), str(endpoint))
        asserts.assert_true(cluster in results[endpoint], err_msg)
        err_msg = "Results not returned for attribute {} on ep {}".format(str(attribute), str(endpoint))
        asserts.assert_true(attribute in results[endpoint][cluster], err_msg)
        attr_ret = results[endpoint][cluster][attribute]
        err_msg = "Error reading {}:{}".format(str(cluster), str(attribute))
        asserts.assert_true(attr_ret is not None, err_msg)
        asserts.assert_false(isinstance(attr_ret, Clusters.Attribute.ValueDecodeFailure), err_msg)

    @async_test_body
    async def test_TC_ACE_1_4(self):
        # TODO: Guard these on the PICS
        asserts.assert_true('PIXIT.ACE.APPENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT.ACE.APPENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT.ACE.APPENDPOINT:<endpoint>")
        asserts.assert_true('PIXIT.ACE.APPCLUSTER' in self.matter_test_config.global_test_params,
                            "PIXIT.ACE.APPCLUSTER must be included on the command line in "
                            "the --string-arg flag as PIXIT.ACE.APPCLUSTER:<cluster_name>")
        asserts.assert_true('PIXIT.ACE.APPATTRIBUTE' in self.matter_test_config.global_test_params,
                            "PIXIT.ACE.APPATTRIBUTE must be included on the command line in "
                            "the --string-arg flag as PIXIT.ACE.APPATTRIBUTE:<attribute_name>")
        asserts.assert_true('PIXIT.ACE.APPDEVTYPEID' in self.matter_test_config.global_test_params,
                            "PIXIT.ACE.APPDEVTYPEID must be included on the command line in "
                            "the --int-arg flag as PIXIT.ACE.APPDEVTYPEID:<device_type_id>")

        cluster_str = self.matter_test_config.global_test_params['PIXIT.ACE.APPCLUSTER']
        attribute_str = self.matter_test_config.global_test_params['PIXIT.ACE.APPATTRIBUTE']

        self.cluster = str_to_cluster(cluster_str)
        self.attribute = str_to_attribute(self.cluster, attribute_str)
        self.devtype = self.matter_test_config.global_test_params['PIXIT.ACE.APPDEVTYPEID']
        self.endpoint = self.matter_test_config.global_test_params['PIXIT.ACE.APPENDPOINT']

        asserts.assert_true(self.cluster is not None, "Invalid cluster name")
        asserts.assert_true(self.attribute is not None, "Invalue attribute name")

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "TH1 writes ACL all clusters view on all endpoints")
        admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])
        all_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[])
        acl = [admin_acl, all_view]
        await self.write_acl(acl)

        self.print_step(3, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(0)

        self.print_step(4, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(5, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(6, "TH1 writes ACL descriptor view all endpoints")
        descriptor_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.Descriptor.id)])

        acl = [admin_acl, descriptor_view]
        await self.write_acl(acl)

        self.print_step(7, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(0)

        self.print_step(8, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(9, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect UNSUPPORTED_ACCESS")
        await self.read_appcluster_expect_unsupported_access()

        self.print_step(10, "TH1 writes ACL PIXIT.ACE.APPCLUSTER view on all endpoints")
        appcluster_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=self.cluster.id)])

        acl = [admin_acl, appcluster_view]
        await self.write_acl(acl)

        self.print_step(11, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(12, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(self.endpoint)

        self.print_step(13, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(14, "TH1 writes ACL descriptor view on PIXIT.ACE.APPENDPOINT")
        descriptor_appendpoint_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=self.endpoint, cluster=Clusters.Descriptor.id)])

        acl = [admin_acl, descriptor_appendpoint_view]
        await self.write_acl(acl)

        self.print_step(15, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(16, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(17, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect UNSUPPORTED_ACCESS")
        await self.read_appcluster_expect_unsupported_access()

        self.print_step(18, "TH1 writes ACL appcluster view on PIXIT.ACE.APPENDPOINT")
        appcluster_appendpoint_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=self.endpoint, cluster=self.cluster.id)])

        acl = [admin_acl, appcluster_appendpoint_view]
        await self.write_acl(acl)

        self.print_step(19, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(20, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(self.endpoint)

        self.print_step(21, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(22, "TH1 writes ACL all clusters view on PIXIT.ACE.APPENDPOINT")
        allclusters_appendpoint_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=self.endpoint)])

        acl = [admin_acl, allclusters_appendpoint_view]
        await self.write_acl(acl)

        self.print_step(23, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(24, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(25, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(26, "TH1 writes ACL rootnode device type view on all endpoints")
        rootnode_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(deviceType=0x0016)])

        acl = [admin_acl, rootnode_view]
        await self.write_acl(acl)

        self.print_step(27, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(0)

        self.print_step(28, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(self.endpoint)

        self.print_step(29, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect UNSUPPORTED_ACCESS")
        await self.read_appcluster_expect_unsupported_access()

        self.print_step(30, "TH1 writes ACL PIXIT.ACE.APPDEVTYPE view on all endpoints")
        appdevtype_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(deviceType=self.devtype)])

        acl = [admin_acl, appdevtype_view]
        await self.write_acl(acl)

        self.print_step(31, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(32, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(33, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(34, "TH1 writes ACL descriptor cluster on appdevtype on all endpoints")
        descriptor_appdevtype_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=0x001d, deviceType=self.devtype)])

        acl = [admin_acl, descriptor_appdevtype_view]
        await self.write_acl(acl)

        self.print_step(35, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(36, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(self.endpoint)

        self.print_step(37, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect UNSUPPORTED_ACCESS")
        await self.read_appcluster_expect_unsupported_access()

        self.print_step(38, "TH1 writes ACL PIXIT.ACE.APPCLUSTER view on PIXIT.ACE.APPDEVTYPE")
        appcluster_appdevtype_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=self.cluster.id, deviceType=self.devtype)])

        acl = [admin_acl, appcluster_appdevtype_view]
        await self.write_acl(acl)

        self.print_step(39, "TH1 reads EP0 descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(0)

        self.print_step(40, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(self.endpoint)

        self.print_step(41, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(42, "TH1 writes ACL multi-target view")
        multitarget_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.Descriptor.id, endpoint=0),
                     Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=self.cluster.id, endpoint=self.endpoint)])

        acl = [admin_acl, multitarget_view]
        await self.write_acl(acl)

        self.print_step(43, "TH1 reads EP0 descriptor - expect SUCCESS")
        await self.read_descriptor_expect_success(0)

        self.print_step(44, "TH1 reads PIXIT.ACE.APPENDPOINT descriptor - expect UNSUPPORTED_ACCESS")
        await self.read_descriptor_expect_unsupported_access(self.endpoint)

        self.print_step(45, "TH1 reads PIXIT.ACE.APPENDPOINT PIXIT.ACE.APPCLUSTER - expect SUCCESS")
        await self.read_appcluster_expect_success()

        self.print_step(46, "TH1 reads wildcard descriptor - expect SUCCESS on EP0, no response on PIXIT.ACE.APPENDPOINT")
        descriptor_wildcard = await self.read_wildcard_endpoint(Clusters.Descriptor.Attributes.DeviceTypeList)
        # Ensure we get a success callback for ep 0
        self.check_read_success(descriptor_wildcard, 0, Clusters.Objects.Descriptor, Clusters.Descriptor.Attributes.DeviceTypeList)

        # Ensure we get NO results back for ep PIXIT.ACE.APPENDPOINT
        asserts.assert_false(self.endpoint in descriptor_wildcard, "Received unexpected results on PIXIT.ACE.APPENDPOINT")

        self.print_step(47, "TH1 reads wildcard PIXIT.ACE.APPCLUSTER - expect no response on EP0, success on PIXIT.ACE.APPENDPOINT")
        appcluster_wildcard = await self.read_wildcard_endpoint(self.attribute)
        # Ensure we get no response on EP 0
        asserts.assert_false(0 in appcluster_wildcard, "Received unexpected results on EP 0")

        # Ensure we get a success callback for PIXIT.ACE.APPENDPOINT
        self.check_read_success(appcluster_wildcard, self.endpoint, self.cluster, self.attribute)

        self.print_step(48, "TH1 resets ACL")
        full_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[self.matter_test_config.controller_node_id],
            targets=[])

        acl = [full_acl]
        await self.write_acl(acl)


if __name__ == "__main__":
    default_matter_test_main()
