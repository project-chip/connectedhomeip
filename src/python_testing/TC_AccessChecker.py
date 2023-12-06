import logging
from copy import deepcopy

import chip.clusters as Clusters
from basic_composition_support import BasicCompositionTests
from chip.interaction_model import Status
from chip.tlv import uint
from enum import Enum, auto
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import (AttributePathLocation, ClusterPathLocation, MatterBaseTest,
                                    async_test_body, default_matter_test_main)
from spec_parsing_support import XmlCluster, build_xml_clusters


class AccessTestType(Enum):
    READ = auto()
    WRITE = auto()


def operation_allowed(spec_requires: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum, acl_set_to: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum) -> bool:
    if spec_requires == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue or acl_set_to is None:
        return False
    return spec_requires <= acl_set_to


def checkable_attributes(cluster_id, cluster, xml_cluster) -> list[uint]:
    all_attrs = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]

    def attr_ok(attribute_id):
        return attribute_id <= 0xFFFF and attribute_id in xml_cluster.attributes and attribute_id in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]
    return [x for x in all_attrs if attr_ok(x)]


class AccessChecker(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        # TODO: Make this into a proper default in the class so we're not overriding the command lines
        self.user_params["use_pase_only"] = False
        super().setup_class()
        await self.setup_class_helper()
        self.xml_clusters, self.problems = build_xml_clusters()
        acl_attr = Clusters.AccessControl.Attributes.Acl
        self.default_acl = await self.read_single_attribute_check_success(cluster=Clusters.AccessControl, attribute=acl_attr)
        self._record_errors()
        # We need to run this test from two controllers so we can test access to the ACL cluster while retaining access to the ACL cluster
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        self.TH2_nodeid = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(nodeId=self.TH2_nodeid)

    @async_test_body
    async def setup_test(self):
        self.success = True

    @async_test_body
    async def teardown_test(self):
        await self._cleanup_acl(default_acl=self.default_acl)

    async def _setup_acl(self, default_acl: list[Clusters.AccessControl.Structs.AccessControlEntryStruct], privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum):
        if privilege is not None:
            new_acl = deepcopy(default_acl)
            new_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=privilege, authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase, subjects=[self.TH2_nodeid])
            new_acl.append(new_entry)
            await self.default_controller.WriteAttribute(self.dut_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl(new_acl))])

    async def _cleanup_acl(self, default_acl: list[Clusters.AccessControl.Structs.AccessControlEntryStruct]):
        await self.default_controller.WriteAttribute(self.dut_node_id, attributes=[
            (0, Clusters.AccessControl.Attributes.Acl(default_acl))])

    def _record_errors(self):
        ''' Checks through all the endpoints and records all the spec warnings in one go so we don't get repeats'''
        all_clusters = set()
        attrs: dict[uint, set()] = {}

        for endpoint_id, endpoint in self.endpoints_tlv.items():
            all_clusters |= set(endpoint.keys())
            for cluster_id, cluster in endpoint.items():
                # Find all the attributes for this cluster across all endpoint
                if cluster_id not in attrs:
                    attrs[cluster_id] = set()
                attrs[cluster_id].update(set(cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]))

        for cluster_id in all_clusters:
            location = ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id)
            if cluster_id > 0x7FFF:
                # We cannot check access on MEI clusters
                continue
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
                if attribute_id > 0xFFFF:
                    # We don't have any understanding of access requirements on MEI attributes, so skip these for now
                    continue
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

    async def _run_read_access_test_for_cluster_privilege(self, endpoint_id, cluster_id, cluster, xml_cluster: XmlCluster, privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum):
        # TODO: This assumes all attributes are readable. Which they are currently. But we don't have a general way to mark otherwise.
        for attribute_id in checkable_attributes(cluster_id, cluster, xml_cluster):
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

    async def _run_write_access_test_for_cluster_privilege(self, endpoint_id, cluster_id, cluster, xml_cluster: XmlCluster, privilege: Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum):
        for attribute_id in checkable_attributes(cluster_id, cluster, xml_cluster):
            spec_requires = xml_cluster.attributes[attribute_id].write_access
            if spec_requires == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                # not writeable
                continue
            attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]
            cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]
            location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
            test_name = f'Write access checker - {privilege}'

            # TODO: I'm an idiot. This returns the error.

            if operation_allowed(spec_requires, privilege):
                # Write the default attribute. We don't care if this fails, as long as it fails with a DIFFERENT error than the access
                # This is OK because access is required to be checked BEFORE any other thing to avoid leaking device information.
                try:
                    # TODO: Do we have any attributes that require a timed invoke (or whatever it's called on write)
                    self.TH2.WriteAttribute(nodeid=self.dut_node_id, attributes=[(endpoint_id, attribute())])
                    # success is OK, no fail here
                except InteractionModelError as e:
                    if e.status == Status.UnsupportedAccess:
                        self.record_error(test_name=test_name, location=location, problem="Unexpected UnsupportedAccess")
                        self.success = False
            else:
                try:
                    # TODO: Do we have any attributes that require a timed invoke (or whatever it's called on write)
                    self.TH2.WriteAttribute(nodeid=self.dut_node_id, attributes=[(endpoint_id, attribute())])
                    # The above write should fail, so if we're here, that's an error
                    self.record_error(test_name=test_name, location=location, problem="Unexpected success writing attribute")
                    self.success = False
                except InteractionModelError as e:
                    if e.status != Status.UnsupportedAccess:
                        self.record_error(test_name=test_name, location=location,
                                          problem=f"Unexpected error writing attribute - expected Unsupported Access, got {e.status}")
                        self.success = False

    async def run_access_test(self, test_type: AccessTestType):
        privilege_enum = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum
        for privilege in privilege_enum:
            logging.info(f"Testing for {privilege}")
            await self._setup_acl(default_acl=self.default_acl, privilege=privilege)
            for endpoint_id, endpoint in self.endpoints_tlv.items():
                for cluster_id, cluster in endpoint.items():
                    if cluster_id > 0x7FFF or cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                        # These cases have already been recorded by the _record_errors function
                        continue
                    xml_cluster = self.xml_clusters[cluster_id]
                    if test_type == AccessTestType.READ:
                        await self._run_read_access_test_for_cluster_privilege(endpoint_id, cluster_id, cluster, xml_cluster, privilege)
                    elif test_type == AccessTestType.WRITE:
                        await self._run_write_access_test_for_cluster_privilege(endpoint_id, cluster_id, cluster, xml_cluster, privilege)
                    else:
                        self.fail_current_test("Unsupported test type")
        if not self.success:
            self.fail_current_test("One or more access violations was found")

    @async_test_body
    async def test_read_access(self):
        await self.run_access_test(AccessTestType.READ)

    @async_test_body
    async def test_write_access(self):
        await self.run_access_test(AccessTestType.WRITE)


if __name__ == "__main__":
    default_matter_test_main()
