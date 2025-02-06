# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
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
# === END CI TEST ARGUMENTS ===

import logging
from copy import deepcopy
from enum import Enum, auto
from typing import Optional

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.matter_testing import (AttributePathLocation, ClusterPathLocation, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from chip.testing.spec_parsing import XmlCluster
from chip.tlv import uint


class AccessTestType(Enum):
    READ = auto()
    WRITE = auto()


def step_number_with_privilege(step: int, substep: str, privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum) -> str:
    return f'{step}{substep}_{privilege.name}'


def operation_allowed(spec_requires: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum,
                      acl_set_to: Optional[Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum]) -> bool:
    ''' Determines if the action is allowed on the device based on the spec_requirements and the current ACL privilege granted.

        The spec parsing uses kUnknownEnumValue to indicate that NO access is allowed for this attribute
        or command (ex. command with no write access).
        ACL uses None to indicate that no access has been granted to this controller.
        In both of these cases, the action is disallowed. In all other cases, access is allowed if the ACL
        grants a privilege at or above the privilege required in the spec.
    '''
    if spec_requires == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue or acl_set_to is None:
        return False
    return spec_requires <= acl_set_to


def checkable_attributes(cluster_id, cluster, xml_cluster) -> list[uint]:
    all_attrs = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]

    def known_cluster_attribute(attribute_id) -> bool:
        ''' Returns true if this is a non-manufacturer specific attribute that has information in the XML and has python codegen data'''
        return attribute_id <= 0xFFFF and attribute_id in xml_cluster.attributes and attribute_id in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]
    return [x for x in all_attrs if known_cluster_attribute(x)]


class AccessChecker(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        # TODO: Make this into a proper default in the class so we're not overriding the command lines
        self.user_params["use_pase_only"] = False
        super().setup_class()
        await self.setup_class_helper()
        self.build_spec_xmls()

        acl_attr = Clusters.AccessControl.Attributes.Acl
        self.default_acl = await self.read_single_attribute_check_success(cluster=Clusters.AccessControl, attribute=acl_attr)
        self._record_errors()
        # We need to run this test from two controllers so we can test access to the ACL cluster while retaining access to the ACL cluster
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        self.TH2_nodeid = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(nodeId=self.TH2_nodeid)

    # Both the tests in this suite are potentially long-running if there are a large number of attributes on the DUT
    # and the network is slow. Set the default to 3 minutes to account for this.
    @property
    def default_timeout(self) -> int:
        return 180

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.success = True

    @async_test_body
    async def teardown_test(self):
        await self.default_controller.WriteAttribute(self.dut_node_id, attributes=[
            (0, Clusters.AccessControl.Attributes.Acl(self.default_acl))])

    async def _setup_acl(self, privilege: Optional[Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum]):
        if privilege is None:
            return
        new_acl = deepcopy(self.default_acl)
        new_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=privilege, authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase, subjects=[self.TH2_nodeid])
        new_acl.append(new_entry)
        await self.default_controller.WriteAttribute(self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl(new_acl))])

    def _record_errors(self):
        ''' Checks through all the endpoints and records all the spec warnings in one go so we don't get repeats'''
        all_clusters = set()
        attrs: dict[uint, set()] = {}

        for endpoint_id, endpoint in self.endpoints_tlv.items():
            all_clusters |= set(endpoint.keys())
            for cluster_id, device_cluster_data in endpoint.items():
                # Find all the attributes for this cluster across all endpoint
                if cluster_id not in attrs:
                    attrs[cluster_id] = set()
                # discard MEI attributes as we do not have access information for them.
                attrs[cluster_id].update(
                    set([id for id in device_cluster_data[GlobalAttributeIds.ATTRIBUTE_LIST_ID] if id <= 0xFFFF]))

        # Remove MEI clusters - we don't have information available to check these.
        all_clusters = [id for id in all_clusters if id <= 0x7FFF]
        for cluster_id in all_clusters:
            location = ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id)
            if cluster_id not in self.xml_clusters:
                # TODO: Upgrade from warning when the spec XML stabilizes
                self.record_warning(test_name="Access Checker", location=location, problem="Cluster not present in spec data")
                continue
            if cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                self.record_error(test_name="Access Checker", location=location, problem="Unknown cluster")
                self.success = False
                continue

            # check that we have information for all the required attributes
            xml_cluster = self.xml_clusters[cluster_id]
            for attribute_id in attrs[cluster_id]:
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                if attribute_id not in xml_cluster.attributes:
                    self.record_warning(test_name="Access Checker", location=location,
                                        problem="Cluster attribute not found in spec XML")
                    continue
                if attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                    self.record_error(test_name="Access Checker", location=location,
                                      problem="Unknown attribute")
                    self.success = False
                    continue

    async def _run_read_access_test_for_cluster_privilege(self, endpoint_id, cluster_id, device_cluster_data, xml_cluster: XmlCluster, privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum):
        # TODO: This assumes all attributes are readable. Which they are currently. But we don't have a general way to mark otherwise.
        for attribute_id in checkable_attributes(cluster_id, device_cluster_data, xml_cluster):
            spec_requires = xml_cluster.attributes[attribute_id].read_access
            attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]
            cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

            if operation_allowed(spec_requires, privilege):
                ret = await self.read_single_attribute_check_success(dev_ctrl=self.TH2, endpoint=endpoint_id, cluster=cluster_class, attribute=attribute, assert_on_error=False, test_name=f"Read access Checker - {privilege}")
                if ret is None:
                    self.success = False
            else:
                ret = await self.read_single_attribute_expect_error(dev_ctrl=self.TH2, endpoint=endpoint_id, cluster=cluster_class, attribute=attribute, error=Status.UnsupportedAccess, assert_on_error=False, test_name=f"Read access Checker - {privilege}")
                if ret is None:
                    self.success = False

    async def _run_write_access_test_for_cluster_privilege(self, endpoint_id, cluster_id, cluster, xml_cluster: XmlCluster, privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum, wildcard_read):
        for attribute_id in checkable_attributes(cluster_id, cluster, xml_cluster):
            spec_requires = xml_cluster.attributes[attribute_id].write_access
            is_optional_write = xml_cluster.attributes[attribute_id].write_optional

            attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]
            cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]
            location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
            test_name = f'Write access checker - {privilege}'
            logging.info(f"Testing attribute {attribute} on endpoint {endpoint_id}")
            if attribute == Clusters.AccessControl.Attributes.Acl and privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister:
                logging.info("Skipping ACL attribute check for admin privilege as this is known to be writeable and is being used for this test")
                continue

            # Because we read everything with admin, we should have this in the wildcard read
            # This will only not work if we end up with write-only attributes. We do not currently have any of these.
            val = wildcard_read.attributes[endpoint_id][cluster_class][attribute]
            if isinstance(val, list):
                # Use an empty list for writes in case the list is large and does not fit
                val = []

            resp = await self.TH2.WriteAttribute(nodeid=self.dut_node_id, attributes=[(endpoint_id, attribute(val))])
            if spec_requires == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                # not writeable - expect an unsupported write response
                if resp[0].Status != Status.UnsupportedWrite:
                    self.record_error(test_name=test_name, location=location,
                                      problem=f"Unexpected error writing non-writeable attribute - expected Unsupported Write, got {resp[0].Status}")
                    self.success = False
            elif is_optional_write and resp[0].Status == Status.UnsupportedWrite:
                # unsupported optional writeable attribute - this is fine, no error
                continue
            elif operation_allowed(spec_requires, privilege):
                # Write the default attribute. We don't care if this fails, as long as it fails with a DIFFERENT error than the access
                # This is OK because access is required to be checked BEFORE any other thing to avoid leaking device information.
                # For example, because we don't have any range information, we might be writing an out of range value, but this will
                # get rejected by the ACL check before the range check.
                # See section: 8.4.3.2
                if resp[0].Status == Status.UnsupportedAccess:
                    self.record_error(test_name=test_name, location=location,
                                      problem="Unexpected UnsupportedAccess writing attribute")
                    self.success = False
            else:
                if resp[0].Status != Status.UnsupportedAccess:
                    self.record_error(test_name=test_name, location=location,
                                      problem=f"Unexpected error writing attribute - expected Unsupported Access, got {resp[0].Status}")
                    self.success = False

            if resp[0].Status == Status.Success and isinstance(val, list):
                # Reset the value to the original if we managed to write an empty list
                val = wildcard_read.attributes[endpoint_id][cluster_class][attribute]
                await self.TH2.WriteAttribute(nodeid=self.dut_node_id, attributes=[(endpoint_id, attribute(val))])

    async def run_access_test(self, test_type: AccessTestType):
        # Step precondition, 1 and 2 are handled in the class setup, but need to be marked for every test
        self.step("precondition")
        self.step(1)
        self.step(2)
        # Read all the attributes on TH2 using admin access
        check_step = 3
        if test_type == AccessTestType.WRITE:
            self.step(3)
            await self._setup_acl(privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister)
            self.step(4)
            wildcard_read = await self.TH2.Read(self.dut_node_id, [()])
            check_step = 5

        self.step(check_step)
        enum = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
        privilege_enum = [p for p in enum if p != enum.kUnknownEnumValue]
        for privilege in privilege_enum:
            logging.info(f"Testing for {privilege}")
            self.step(step_number_with_privilege(check_step, 'a', privilege))
            await self._setup_acl(privilege=privilege)
            self.step(step_number_with_privilege(check_step, 'b', privilege))
            for endpoint_id, endpoint in self.endpoints_tlv.items():
                for cluster_id, device_cluster_data in endpoint.items():
                    if cluster_id > 0x7FFF or cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                        # These cases have already been recorded by the _record_errors function
                        continue
                    xml_cluster = self.xml_clusters[cluster_id]
                    if test_type == AccessTestType.READ:
                        await self._run_read_access_test_for_cluster_privilege(endpoint_id, cluster_id, device_cluster_data, xml_cluster, privilege)
                    elif test_type == AccessTestType.WRITE:
                        await self._run_write_access_test_for_cluster_privilege(endpoint_id, cluster_id, device_cluster_data, xml_cluster, privilege, wildcard_read)
                    else:
                        self.fail_current_test("Unsupported test type")
        if not self.success:
            self.fail_current_test("One or more access violations was found")

    def steps_TC_ACE_2_1(self):
        steps = [TestStep("precondition", "DUT is commissioned", is_commissioning=True),
                 TestStep(1, "TH_commissioner performs a wildcard read"),
                 TestStep(2, "TH_commissioner reads the ACL attribute"),
                 TestStep(3, "Repeat steps 3a and 3b for each permission level")]
        enum = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
        privilege_enum = [p for p in enum if p != enum.kUnknownEnumValue]
        for p in privilege_enum:
            steps.append(TestStep(step_number_with_privilege(3, 'a', p),
                         "TH_commissioner gives TH_second_commissioner the specified privilege"))
            steps.append(TestStep(step_number_with_privilege(3, 'b', p),
                         "TH_second_controller reads all the attributes and checks for appropriate permission errors"))
        return steps

    def desc_TC_ACE_2_1(self):
        return "[TC-ACE-2.1] Attribute read privilege enforcement - [DUT as Server]"

    @async_test_body
    async def test_TC_ACE_2_1(self):
        await self.run_access_test(AccessTestType.READ)

    def steps_TC_ACE_2_2(self):
        steps = [TestStep("precondition", "DUT is commissioned", is_commissioning=True),
                 TestStep(1, "TH_commissioner performs a wildcard read"),
                 TestStep(2, "TH_commissioner reads the ACL attribute"),
                 TestStep(3, "TH_commissioner grants TH_second_controller admin permission"),
                 TestStep(4, "TH_second_controller performs a wildcard read"),
                 TestStep(5, "Repeat steps 5a and 5b for each permission level")]
        enum = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
        privilege_enum = [p for p in enum if p != enum.kUnknownEnumValue]
        for p in privilege_enum:
            steps.append(TestStep(step_number_with_privilege(5, 'a', p),
                         "TH_commissioner gives TH_second_commissioner the specified privilege"))
            steps.append(TestStep(step_number_with_privilege(5, 'b', p),
                         "TH_second_commissioner writes all the attributes and checks for appropriate permission errors"))
        return steps

    def desc_TC_ACE_2_2(self):
        return "[TC-ACE-2.2] Attribute write privilege enforcement - [DUT as Server]"

    @async_test_body
    async def test_TC_ACE_2_2(self):
        await self.run_access_test(AccessTestType.WRITE)


if __name__ == "__main__":
    default_matter_test_main()
