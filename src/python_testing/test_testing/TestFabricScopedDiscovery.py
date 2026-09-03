#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Unit tests for the TC-IDM-8.1 fabric-scoped discovery helpers.

TC-IDM-8.1 sweeps every fabric-scoped attribute and fabric-sensitive event a DUT
exposes, so the discovery walk and the cross-fabric masking assertion decide what
the test actually covers. These tests exercise both without a DUT, using a
synthetic composition and the real spec XML.
"""

import asyncio
import sys
from pathlib import Path

# The module under test lives in src/python_testing, one level above this directory.
sys.path.append(str(Path(__file__).resolve().parent.parent))

from mobly import asserts, signals  # noqa: E402 - import must follow the sys.path setup above
# Imported as a module, not by name: the runner requires exactly one MatterBaseTest
# subclass among this module's members, and IDMBaseTest would count as a second.
from support_modules import idm_support  # noqa: E402

import matter.clusters as Clusters  # noqa: E402
from matter.clusters.Types import NullValue  # noqa: E402
from matter.testing.global_attribute_ids import GlobalAttributeIds  # noqa: E402
from matter.testing.matter_testing import CertificationUnitTestNoDevice  # noqa: E402
from matter.testing.runner import default_matter_test_main  # noqa: E402
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters  # noqa: E402

# Parsed once: build_xml_clusters walks the whole data model directory.
XML_CLUSTERS, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_6_1)


def make_dut(composition: dict) -> idm_support.IDMBaseTest:
    """Build an IDMBaseTest carrying only the state the discovery walk reads.

    Instantiated without __init__ because discovery depends on nothing but
    endpoints_tlv and xml_clusters; running the mobly constructor would require a
    full test configuration and a DUT connection that these tests deliberately
    avoid.
    """
    dut = idm_support.IDMBaseTest.__new__(idm_support.IDMBaseTest)
    dut.endpoints_tlv = composition
    dut.xml_clusters = XML_CLUSTERS
    return dut


def attribute_list(*attribute_ids: int) -> dict:
    return {GlobalAttributeIds.ATTRIBUTE_LIST_ID: list(attribute_ids)}


ACL_ID = Clusters.AccessControl.Attributes.Acl.attribute_id
EXTENSION_ID = Clusters.AccessControl.Attributes.Extension.attribute_id
SUBJECTS_PER_ENTRY_ID = Clusters.AccessControl.Attributes.SubjectsPerAccessControlEntry.attribute_id
BINDING_ID = Clusters.Binding.Attributes.Binding.attribute_id
PROVISIONED_ENDPOINTS_ID = Clusters.TlsClientManagement.Attributes.ProvisionedEndpoints.attribute_id
PROVISIONED_ROOT_CERTIFICATES_ID = Clusters.TlsCertificateManagement.Attributes.ProvisionedRootCertificates.attribute_id
GROUP_TABLE_ID = Clusters.GroupKeyManagement.Attributes.GroupTable.attribute_id


class TestFabricScopedDiscovery(CertificationUnitTestNoDevice):

    def find(self, infos: list, cluster_id: int, attribute_id: int) -> idm_support.FabricScopedAttributeInfo:
        matches = [i for i in infos if i.cluster_id == cluster_id and i.attribute_id == attribute_id]
        asserts.assert_equal(len(matches), 1, f"Expected exactly one entry for 0x{cluster_id:04X}/0x{attribute_id:04X}")
        return matches[0]

    def test_entry_type_unwrapping(self):
        # ACL is List[AccessControlEntryStruct]; Binding is
        # Union[Nullable, List[TargetStruct]]; a scalar attribute is neither.
        asserts.assert_equal(
            idm_support.fabric_scoped_entry_type(Clusters.AccessControl.Attributes.Acl),
            Clusters.AccessControl.Structs.AccessControlEntryStruct,
            "ACL entry type should unwrap to AccessControlEntryStruct")
        asserts.assert_equal(
            idm_support.fabric_scoped_entry_type(Clusters.Binding.Attributes.Binding),
            Clusters.Binding.Structs.TargetStruct,
            "Binding entry type should unwrap through the nullable wrapper")
        asserts.assert_is_none(
            idm_support.fabric_scoped_entry_type(Clusters.BasicInformation.Attributes.NodeLabel),
            "A scalar attribute has no list entry type")

    def test_struct_fabric_scoping_from_codegen(self):
        asserts.assert_true(
            idm_support.struct_is_fabric_scoped(Clusters.AccessControl.Structs.AccessControlEntryStruct),
            "AccessControlEntryStruct carries a FabricIndex field")
        asserts.assert_false(
            idm_support.struct_is_fabric_scoped(Clusters.AccessControl.Structs.AccessControlTargetStruct),
            "AccessControlTargetStruct carries no FabricIndex field")
        asserts.assert_false(idm_support.struct_is_fabric_scoped(None),
                             "A missing struct type is not fabric scoped")

    def test_fabric_sensitive_field_labels(self):
        # The spec marks Privilege/AuthMode/Subjects/Targets/AuxiliaryType of
        # AccessControlEntryStruct fabric sensitive, but not FabricIndex.
        xml_struct = XML_CLUSTERS[Clusters.AccessControl.id].structs["AccessControlEntryStruct"]
        labels = idm_support.fabric_sensitive_field_labels(
            Clusters.AccessControl.Structs.AccessControlEntryStruct, xml_struct)
        asserts.assert_equal(labels, frozenset({"privilege", "authMode", "subjects", "targets", "auxiliaryType"}),
                             "Unexpected fabric-sensitive field labels for AccessControlEntryStruct")
        asserts.assert_not_in("fabricIndex", labels, "FabricIndex is not fabric sensitive")

        asserts.assert_equal(
            idm_support.fabric_sensitive_field_labels(Clusters.AccessControl.Structs.AccessControlEntryStruct, None),
            frozenset(), "A struct with no spec XML entry yields no labels")

    def test_discovery_finds_fabric_scoped_attributes_only(self):
        dut = make_dut({
            0: {Clusters.AccessControl.id: attribute_list(ACL_ID, EXTENSION_ID, SUBJECTS_PER_ENTRY_ID)},
            1: {Clusters.Binding.id: attribute_list(BINDING_ID)},
        })
        infos = dut.discover_fabric_scoped_attributes()
        discovered = {(i.endpoint_id, i.cluster_id, i.attribute_id) for i in infos}
        asserts.assert_equal(
            discovered,
            {(0, Clusters.AccessControl.id, ACL_ID),
             (0, Clusters.AccessControl.id, EXTENSION_ID),
             (1, Clusters.Binding.id, BINDING_ID)},
            "Discovery should return every fabric-scoped attribute and nothing else")

        acl = self.find(infos, Clusters.AccessControl.id, ACL_ID)
        asserts.assert_true(acl.from_codegen and acl.from_spec_xml,
                            "Both signals should identify ACL as fabric scoped")
        asserts.assert_equal(acl.struct_class, Clusters.AccessControl.Structs.AccessControlEntryStruct,
                             "Unexpected ACL entry struct")
        asserts.assert_in("privilege", acl.fabric_sensitive_labels,
                          "ACL entries carry fabric-sensitive fields")
        asserts.assert_equal(acl.path_str, "EP0 Access Control.Acl", "Unexpected path string")

    def test_discovery_marks_deny_listed_attributes_write_denied(self):
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(ACL_ID, EXTENSION_ID)}})
        infos = dut.discover_fabric_scoped_attributes()

        # Both are writable per the spec, but the ACL is deny-listed because writing it
        # can strip the harness of its own Administer privilege. The report distinguishes
        # the two, so the deny list must not show up as read-only.
        acl = self.find(infos, Clusters.AccessControl.id, ACL_ID)
        asserts.assert_true(acl.writable, "ACL is writable per the spec")
        asserts.assert_true(acl.write_denied, "ACL must be excluded from the write sweep")

        extension = self.find(infos, Clusters.AccessControl.id, EXTENSION_ID)
        asserts.assert_true(extension.writable, "Extension is writable per the spec")
        asserts.assert_false(extension.write_denied, "Extension is not deny-listed")

    def test_attribute_is_writable_requires_the_dut_to_expose_it(self):
        # Extension is gated on the EXTS feature. A DUT that omits it must not be written
        # to, so the event trigger can report the event as untriggerable instead of
        # blaming the DUT for not reporting it.
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(ACL_ID)}})
        asserts.assert_true(dut.attribute_is_writable(0, Clusters.AccessControl.id, ACL_ID),
                            "Acl is present on the DUT and writable per the spec")
        asserts.assert_false(dut.attribute_is_writable(0, Clusters.AccessControl.id, EXTENSION_ID),
                             "Extension is absent from the DUT's AttributeList")
        asserts.assert_false(dut.attribute_is_writable(1, Clusters.AccessControl.id, ACL_ID),
                             "The cluster is not on endpoint 1")

    def test_attribute_is_writable_rejects_read_only_attributes(self):
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(ACL_ID, SUBJECTS_PER_ENTRY_ID)}})
        asserts.assert_false(dut.attribute_is_writable(0, Clusters.AccessControl.id, SUBJECTS_PER_ENTRY_ID),
                             "SubjectsPerAccessControlEntry has no write privilege in the spec")

    def test_discovery_reports_codegen_only_attributes(self):
        # PowerTopology.ElectricalCircuitNodes is a list of fabric-scoped structs in the
        # generated code, but the 1.6.1 XML marks neither the attribute nor its struct
        # fabricScoped. Discovery keeps it and flags the disagreement.
        attribute_id = Clusters.PowerTopology.Attributes.ElectricalCircuitNodes.attribute_id
        dut = make_dut({1: {Clusters.PowerTopology.id: attribute_list(attribute_id)}})
        infos = dut.discover_fabric_scoped_attributes()
        info = self.find(infos, Clusters.PowerTopology.id, attribute_id)
        asserts.assert_true(info.from_codegen, "Codegen should identify the attribute as fabric scoped")
        asserts.assert_false(info.from_spec_xml, "The 1.6.1 XML does not mark this attribute fabric scoped")

    def test_discovery_reports_spec_only_attributes(self):
        # Messages.Messages is fabricScoped in the XML, but the generated MessageStruct
        # has no FabricIndex field, so codegen cannot see it. Discovery keeps it so the
        # gap is reported rather than silently dropped.
        attribute_id = Clusters.Messages.Attributes.Messages.attribute_id
        dut = make_dut({1: {Clusters.Messages.id: attribute_list(attribute_id)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.Messages.id, attribute_id)
        asserts.assert_true(info.from_spec_xml, "The XML marks Messages fabric scoped")
        asserts.assert_false(info.from_codegen, "MessageStruct carries no FabricIndex field")

        # The sensitive field labels still map (the spec field IDs match the generated
        # tags), but without a FabricIndex field no entry can be attributed to another
        # fabric, so the masking assertion reports that it checked nothing rather than
        # passing vacuously.
        asserts.assert_in("messageText", info.fabric_sensitive_labels,
                          "Fabric-sensitive labels should still map through the descriptor tags")
        entries = [Clusters.Messages.Structs.MessageStruct(messageText="leaked")]
        asserts.assert_equal(dut.assert_other_fabric_entries_masked(info, entries, own_fabric_index=1), 0,
                             "Without a FabricIndex field no entry can be checked for masking")

    def test_discovery_skips_manufacturer_specific_clusters(self):
        dut = make_dut({1: {0xFFF1FC01: attribute_list(0x0000)}})
        asserts.assert_equal(dut.discover_fabric_scoped_attributes(), [],
                             "Manufacturer-specific clusters have no spec or codegen fabric data")

    def test_event_discovery_uses_the_spec_signal(self):
        dut = make_dut({0: {Clusters.AccessControl.id: {
            idm_support.EVENT_LIST_ID: [
                Clusters.AccessControl.Events.AccessControlEntryChanged.event_id,
                Clusters.AccessControl.Events.AccessControlExtensionChanged.event_id,
            ]}}})
        infos = dut.discover_fabric_sensitive_events()
        asserts.assert_equal(
            {i.event_id for i in infos},
            {Clusters.AccessControl.Events.AccessControlEntryChanged.event_id,
             Clusters.AccessControl.Events.AccessControlExtensionChanged.event_id},
            "Both AccessControl events are fabric sensitive")
        asserts.assert_true(all(i.from_codegen and i.from_spec_xml for i in infos),
                            "Both signals should agree on the AccessControl events")

    def test_event_discovery_excludes_non_sensitive_events(self):
        # BasicInformation.StartUp is not fabric sensitive and must not be swept.
        dut = make_dut({0: {Clusters.BasicInformation.id: {
            idm_support.EVENT_LIST_ID: [Clusters.BasicInformation.Events.StartUp.event_id]}}})
        asserts.assert_equal(dut.discover_fabric_sensitive_events(), [],
                             "A non-fabric-sensitive event must not be discovered")

    def test_event_discovery_falls_back_to_codegen_event_list(self):
        # EventList is not mandatory on every cluster; a cluster that omits it still
        # reports its events, so discovery must not skip it.
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(ACL_ID)}})
        infos = dut.discover_fabric_sensitive_events()
        asserts.assert_in(Clusters.AccessControl.Events.AccessControlEntryChanged.event_id,
                          {i.event_id for i in infos},
                          "Events should be discovered from the codegen list when EventList is absent")

    def test_masking_accepts_null_and_default_values(self):
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(EXTENSION_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.AccessControl.id, EXTENSION_ID)

        entries = [
            Clusters.AccessControl.Structs.AccessControlExtensionStruct(data=b'\x17\x18', fabricIndex=1),
            # Fabric 2's entry as fabric 1 should see it: sensitive data replaced by the default.
            Clusters.AccessControl.Structs.AccessControlExtensionStruct(data=b'', fabricIndex=2),
        ]
        asserts.assert_equal(dut.assert_other_fabric_entries_masked(info, entries, own_fabric_index=1), 1,
                             "Exactly one cross-fabric entry should have been checked")

        # Null is equally acceptable per the spec wording ("null or default values").
        nulled = [Clusters.AccessControl.Structs.AccessControlExtensionStruct(data=NullValue, fabricIndex=2)]
        asserts.assert_equal(dut.assert_other_fabric_entries_masked(info, nulled, own_fabric_index=1), 1,
                             "A nulled fabric-sensitive field should be accepted")

    def test_masking_fails_on_leaked_fabric_sensitive_data(self):
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(EXTENSION_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.AccessControl.id, EXTENSION_ID)
        leaked = [Clusters.AccessControl.Structs.AccessControlExtensionStruct(data=b'\x17\x18', fabricIndex=2)]
        try:
            dut.assert_other_fabric_entries_masked(info, leaked, own_fabric_index=1)
        except signals.TestFailure:
            return
        asserts.fail("Cross-fabric fabric-sensitive data must fail the masking assertion")

    def test_write_skip_reason_distinguishes_why_a_sweep_must_not_write(self):
        dut = make_dut({
            0: {Clusters.AccessControl.id: attribute_list(ACL_ID, EXTENSION_ID),
                Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID)},
            1: {Clusters.Binding.id: attribute_list(BINDING_ID)},
        })
        infos = dut.discover_fabric_scoped_attributes()

        acl = dut.fabric_write_skip_reason(self.find(infos, Clusters.AccessControl.id, ACL_ID))
        asserts.assert_in("break the test session", acl, "The ACL is deny-listed, not read-only")

        read_only = dut.fabric_write_skip_reason(
            self.find(infos, Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID))
        asserts.assert_equal(read_only, "the spec makes it read-only", "ProvisionedEndpoints is read-only")

        binding = dut.fabric_write_skip_reason(self.find(infos, Clusters.Binding.id, BINDING_ID))
        asserts.assert_in("does not report the change", binding,
                          "Binding is writable but reports no change, so the subscription half cannot run")

        asserts.assert_is_none(dut.fabric_write_skip_reason(self.find(infos, Clusters.AccessControl.id, EXTENSION_ID)),
                               "Extension is the attribute the sweep is expected to write")

    def test_attribute_level_fabric_sensitivity_masks_the_whole_entry(self):
        # ProvisionedEndpoints carries the S quality on the attribute itself rather than
        # on any field of TLSEndpointStruct, so every field but FabricIndex is masked.
        dut = make_dut({0: {Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(),
                         Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID)

        asserts.assert_true(info.whole_entry_sensitive,
                            "ProvisionedEndpoints is marked fabric sensitive on the attribute")
        asserts.assert_equal(info.fabric_sensitive_labels, frozenset(),
                             "TLSEndpointStruct marks no individual field fabric sensitive")
        asserts.assert_equal(
            info.masked_field_labels,
            frozenset({"endpointID", "hostname", "port", "caid", "ccdid", "referenceCount"}),
            "Every field but FabricIndex must be masked when the attribute itself is sensitive")

    def test_field_level_sensitivity_masks_only_the_marked_fields(self):
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(ACL_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.AccessControl.id, ACL_ID)
        asserts.assert_false(info.whole_entry_sensitive,
                             "The ACL attribute itself is fabric scoped, not fabric sensitive")
        asserts.assert_equal(info.masked_field_labels, info.fabric_sensitive_labels,
                             "Only the fields the struct marks should be masked")

    def test_masking_fails_on_a_leaked_whole_sensitive_entry(self):
        dut = make_dut({0: {Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(),
                         Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID)
        # Hostname is not fabric sensitive on its own, so this leak is only caught once
        # the attribute's own S quality is honoured.
        leaked = [Clusters.TlsClientManagement.Structs.TLSEndpointStruct(hostname=b'example.com', fabricIndex=2)]
        try:
            dut.assert_other_fabric_entries_masked(info, leaked, own_fabric_index=1)
        except signals.TestFailure:
            return
        asserts.fail("A cross-fabric entry of a fabric-sensitive attribute must fail the masking assertion")

    def test_masking_reports_nothing_checked_for_own_fabric_only(self):
        # A fabric-filtered read returns only own-fabric entries, so there is nothing to
        # mask. The caller uses the count to tell that apart from a verified check.
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(EXTENSION_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.AccessControl.id, EXTENSION_ID)
        own_only = [Clusters.AccessControl.Structs.AccessControlExtensionStruct(data=b'\x17\x18', fabricIndex=1)]
        asserts.assert_equal(dut.assert_other_fabric_entries_masked(info, own_only, own_fabric_index=1), 0,
                             "No cross-fabric entries means nothing was checked")

    def test_populator_is_registered_for_command_owned_attributes(self):
        # The attributes that most need one: read-only, so the sweep can never write
        # them, and fabric sensitive as a whole, so a cross-fabric entry must be
        # withheld entirely. Without a populator neither fabric has an entry and the
        # masking assertions have nothing to check.
        dut = make_dut({
            0: {Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID),
                Clusters.TlsCertificateManagement.id: attribute_list(PROVISIONED_ROOT_CERTIFICATES_ID),
                Clusters.GroupKeyManagement.id: attribute_list(GROUP_TABLE_ID)},
        })
        infos = dut.discover_fabric_scoped_attributes()
        for cluster_id, attribute_id in ((Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID),
                                         (Clusters.TlsCertificateManagement.id, PROVISIONED_ROOT_CERTIFICATES_ID),
                                         (Clusters.GroupKeyManagement.id, GROUP_TABLE_ID)):
            info = self.find(infos, cluster_id, attribute_id)
            asserts.assert_is_not_none(dut.fabric_entry_populator(info),
                                       f"{info.path_str} is command owned and needs a populator")
            asserts.assert_is_not_none(dut.fabric_write_skip_reason(info),
                                       f"{info.path_str} must not be written by the sweep")

    def test_no_populator_for_attributes_the_sweep_can_write(self):
        # Extension is writable, so the sweep creates its own entry and a populator
        # would be redundant work with a second way to leave state behind.
        dut = make_dut({0: {Clusters.AccessControl.id: attribute_list(EXTENSION_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(), Clusters.AccessControl.id, EXTENSION_ID)
        asserts.assert_is_none(dut.fabric_entry_populator(info),
                               "A writable attribute is populated by the sweep's own write")

    def test_withholding_assertion_fails_on_a_returned_cross_fabric_entry(self):
        # The check that a fabric-sensitive attribute withholds another fabric's
        # entries rather than masking their fields. Exercised here against a
        # synthetic response because the clusters carrying that quality on a real
        # DUT need a live session or a provisioned certificate before they hold
        # anything, so a passing device run does not prove the assertion works.
        dut = make_dut({0: {Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(),
                         Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID)
        returned = [Clusters.TlsClientManagement.Structs.TLSEndpointStruct(hostname=b'example.com', fabricIndex=2)]
        try:
            dut.assert_other_fabrics_withheld(info, returned, own_fabric_index=1, reader_name="TH1")
        except signals.TestFailure:
            return
        asserts.fail("An entry belonging to another fabric must fail the withholding assertion")

    def test_withholding_assertion_passes_when_the_entry_is_absent(self):
        dut = make_dut({0: {Clusters.TlsClientManagement.id: attribute_list(PROVISIONED_ENDPOINTS_ID)}})
        info = self.find(dut.discover_fabric_scoped_attributes(),
                         Clusters.TlsClientManagement.id, PROVISIONED_ENDPOINTS_ID)
        own_only = [Clusters.TlsClientManagement.Structs.TLSEndpointStruct(hostname=b'example.com', fabricIndex=1)]
        dut.assert_other_fabrics_withheld(info, own_only, own_fabric_index=1, reader_name="TH1")

    def test_cleanups_run_in_reverse_order(self):
        # A TLS endpoint names the root certificate it trusts, so it has to be removed
        # before that certificate is. Reverse order is what makes the dependent entry
        # go first.
        dut = make_dut({})
        removed = []

        async def undo(name):
            removed.append(name)

        dut.record_fabric_entry_cleanup("root certificate", lambda: undo("root certificate"))
        dut.record_fabric_entry_cleanup("endpoint", lambda: undo("endpoint"))
        asyncio.run(dut.run_fabric_entry_cleanups())

        asserts.assert_equal(removed, ["endpoint", "root certificate"],
                             "Cleanups must run most recent first")
        asserts.assert_equal(dut.fabric_entry_cleanups, [], "The cleanup list is emptied once drained")

    def test_a_failing_cleanup_does_not_stop_the_others(self):
        # One cluster refusing to remove an entry must not leave the rest behind, and
        # must not replace the test's own result with a teardown error.
        dut = make_dut({})
        removed = []

        async def fails():
            raise RuntimeError("DUT refused")

        async def succeeds():
            removed.append("second")

        dut.record_fabric_entry_cleanup("second", succeeds)
        dut.record_fabric_entry_cleanup("first", fails)
        asyncio.run(dut.run_fabric_entry_cleanups())

        asserts.assert_equal(removed, ["second"], "A failing cleanup must not skip the remaining ones")


if __name__ == "__main__":
    default_matter_test_main()
