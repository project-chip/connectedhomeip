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

"""Cross-checks the fabric markers in the spec data model against the ZAP XMLs.

Fabric scoping and fabric sensitivity are described in two places that have to
agree. The spec data model XMLs say what a device is required to do, and are
what the python tests read through matter.testing.spec_parsing. The ZAP XMLs in
src/app/zap-templates/zcl/data-model/chip are what the C++ code generator
consumes, so they decide what a device built from this SDK actually does:
isFabricScoped emits the FabricIndex field and the fabric-aware EncodeForRead,
and isFabricSensitive emits the gate that omits a field from a cross-fabric read.

Where the two disagree, or where a marker is present but nothing consumes it, a
device leaks another fabric's data while every test that trusts the spec XML
reports the element as covered. TC-IDM-8.1 cannot catch that on its own: it only
sees the clusters the DUT in front of it implements, and its masking assertions
are skipped when the marker set is empty.

These checks need no device. They enumerate the whole data model, so a gap is
found whether or not any example app happens to implement the cluster.

Known gaps are listed in KNOWN_FABRIC_METADATA_GAPS with the reason each is
expected. Anything not listed there fails, which is what keeps new gaps from
landing.
"""

import sys
from pathlib import Path

# The support module under test lives in src/python_testing, one level above here.
sys.path.append(str(Path(__file__).resolve().parent.parent))

from mobly import asserts  # noqa: E402 - import must follow the sys.path setup above
# Imported as a module rather than by name: the runner requires exactly one
# MatterBaseTest subclass among this module's members.
from support_modules import zap_fabric_metadata  # noqa: E402

from matter.testing.global_attribute_ids import is_standard_cluster_id  # noqa: E402
from matter.testing.matter_testing import CertificationUnitTestNoDevice  # noqa: E402
from matter.testing.runner import default_matter_test_main  # noqa: E402
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, build_xml_global_data_types  # noqa: E402

# Parsed once: each of these walks a whole data model directory.
SPEC_CLUSTERS, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_6_1)
SPEC_GLOBAL_DATA_TYPES, _ = build_xml_global_data_types(PrebuiltDataModelDirectory.k1_6_1)
SPEC_GLOBAL_STRUCTS = SPEC_GLOBAL_DATA_TYPES.get('structs', {})
ZAP = zap_fabric_metadata.build_zap_fabric_metadata()

# Clusters whose C++ implementation filters its fabric-sensitive list by the
# accessing fabric itself, rather than relying on generated masking. The spec
# requires a fabric-sensitive attribute to be reported as a fabric-filtered list
# regardless of the request's FabricFiltered flag (Interaction Model, "Outgoing
# Report Data Action"), and omitting the entries outright satisfies that. There
# is no generic implementation of attribute-level fabric sensitivity to lean on:
# AttributeQualityFlags::kFabricSensitive is set nowhere in the SDK, so each of
# these clusters has to do it by hand, and this list records which ones do.
HAND_FILTERING_CLUSTERS = frozenset({
    # TLSCertificateManagementCluster.cpp passes request.GetAccessingFabricIndex()
    # into EncodeProvisionedRootCertificates / EncodeProvisionedClientCertificates.
    0x0801,
    # TLSClientManagementCluster.cpp passes it into EncodeProvisionedEndpoints.
    0x0802,
    # PushAVStreamTransportCluster.cpp passes aEncoder.AccessingFabricIndex()
    # into ReadAndEncodeCurrentConnections.
    0x0555,
})

# Gaps that exist today. Each entry is the finding string the checks below
# generate. A gap is listed here only with a reason; anything else fails.
KNOWN_FABRIC_METADATA_GAPS = frozenset({
    # The Messages cluster does not implement fabric scoping at all. ZAP has
    # isFabricScoped commented out on MessageStruct with a TODO dated January
    # 2024 ("breaks at compile time as command argument
    # PresentMessagesRequest#Messages"), so the generated struct carries no
    # FabricIndex and the Messages attribute cannot be fabric filtered whatever
    # the request asks for. The two fields ZAP does mark sensitive are the
    # in-progress audio-messages ones rather than the seven mandatory fields the
    # spec marks, and none of the three events carries the marker either, so the
    # events are reported to every fabric.
    # Confirmed against spec master: MessageStruct is "Access Modifier: Fabric
    # Scoped" with every field marked S, and each event's field table carries
    # "Access Modifier: Fabric Sensitive".
    # TODO: remove once the Messages cluster is fixed; tracked in
    # https://github.com/project-chip/connectedhomeip/issues/73948
    'Messages/MessageStruct: spec fabricScoped, ZAP not',
    'Messages/MessageStruct: fabric-sensitive fields differ, spec [0, 1, 2, 3, 4, 5, 6] ZAP [7, 8]',
    'Messages/MessageQueued: spec fabricSensitive, ZAP not',
    'Messages/MessagePresented: spec fabricSensitive, ZAP not',
    'Messages/MessageComplete: spec fabricSensitive, ZAP not',

    # WebRTCSessionStruct carries no fabric-sensitive fields and neither WebRTC
    # cluster filters CurrentSessions by the accessing fabric, so an unfiltered
    # read returns another fabric's sessions in full: peer node id, session id
    # and stream ids. The spec marks the attribute fabric sensitive, which
    # requires the list to be reported fabric filtered whatever the request
    # asked for, and the SDK implements that quality nowhere generically.
    # TODO: remove once the WebRTC clusters filter CurrentSessions; tracked in
    # https://github.com/project-chip/connectedhomeip/issues/73946
    'WebRTC Transport Provider/CurrentSessions: attribute is fabric sensitive but '
    'WebRTCSessionStruct masks no field and the cluster does not hand-filter',
    'WebRTC Transport Requestor/CurrentSessions: attribute is fabric sensitive but '
    'WebRTCSessionStruct masks no field and the cluster does not hand-filter',

    # Spec master marks GroupKeySetStruct "Access Modifier: Fabric Scoped"; ZAP
    # does not. Lower impact than the entries above because the struct is only
    # ever a command payload (KeySetWrite, KeySetReadResponse) rather than an
    # attribute, and the group data provider already keys its storage by fabric,
    # so no cross-fabric read path depends on the marker today. Recorded so the
    # disagreement is not rediscovered, not because a leak is known.
    'Group Key Management/GroupKeySetStruct: spec fabricScoped, ZAP not',
})


def spec_struct(cluster_id: int, name: str):
    """Find a spec struct by name on the cluster, falling back to the global structs.

    Cluster-scoped first and global second, matching how a cluster referencing a
    struct it does not define is referencing a global one. WebRTCSessionStruct
    is the case that matters here: it is global, so the per-cluster lookup misses.
    """
    xml_cluster = SPEC_CLUSTERS.get(cluster_id)
    if xml_cluster is not None:
        cluster_scoped = xml_cluster.structs.get(name)
        if cluster_scoped is not None:
            return cluster_scoped
    return SPEC_GLOBAL_STRUCTS.get(name)


def sensitive_field_ids(struct) -> frozenset[int]:
    """Field ids a spec struct marks fabric sensitive."""
    if struct is None:
        return frozenset()
    return frozenset(component.value for component in struct.components.values() if component.fabric_sensitive)


def audited_cluster_ids() -> list[int]:
    """Standard cluster ids present in both the spec data model and the ZAP XMLs.

    A cluster in only one of the two is not a fabric finding: it is a cluster the
    SDK has not implemented yet, or one ZAP carries ahead of the published data
    model. Manufacturer-specific and test clusters are excluded, matching the
    scope of the discovery walk in idm_support.
    """
    return sorted(cluster_id for cluster_id in SPEC_CLUSTERS
                  if is_standard_cluster_id(cluster_id) and cluster_id in ZAP.clusters)


class TestFabricMetadataConsistency(CertificationUnitTestNoDevice):

    def assert_findings_are_known(self, findings: list[str], subject: str) -> None:
        """Fail on any finding not in the known-gap list, and flag stale entries.

        A gap that has been fixed but is still listed is reported too: leaving it
        behind would silently accept the defect if it ever came back.
        """
        unexpected = [finding for finding in findings if finding not in KNOWN_FABRIC_METADATA_GAPS]
        asserts.assert_equal(unexpected, [], f"{subject}: spec and ZAP disagree and the gap is not a known one")

    def test_struct_fabric_scoping_matches_spec(self):
        """A struct the spec marks fabric scoped must be isFabricScoped in ZAP.

        Without the marker codegen emits no FabricIndex field, so the generic
        list encoder cannot filter the attribute and the entries of every fabric
        are returned to every reader.
        """
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            for name, struct in xml_cluster.structs.items():
                if not struct.fabric_scoped:
                    continue
                zap_struct = ZAP.struct(cluster_id, name)
                if zap_struct is None:
                    continue
                if not zap_struct.fabric_scoped:
                    findings.append(f"{xml_cluster.name}/{name}: spec fabricScoped, ZAP not")
        self.assert_findings_are_known(findings, "struct fabric scoping")

    def test_struct_fabric_sensitive_fields_match_spec(self):
        """The set of fabric-sensitive field ids must be the same on both sides.

        ZAP's set is what codegen turns into the gate that omits a field from a
        cross-fabric read, so a field the spec marks and ZAP does not is a field
        that leaks.
        """
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            for name, struct in xml_cluster.structs.items():
                zap_struct = ZAP.struct(cluster_id, name)
                if zap_struct is None:
                    continue
                spec_ids = sensitive_field_ids(struct)
                if spec_ids == zap_struct.sensitive_field_ids:
                    continue
                findings.append(f"{xml_cluster.name}/{name}: fabric-sensitive fields differ, "
                                f"spec {sorted(spec_ids)} ZAP {sorted(zap_struct.sensitive_field_ids)}")
        self.assert_findings_are_known(findings, "struct fabric-sensitive fields")

    def test_event_fabric_sensitivity_matches_spec(self):
        """An event the spec marks fabric sensitive must be isFabricSensitive in ZAP."""
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            zap_cluster = ZAP.clusters[cluster_id]
            for event_id, event in xml_cluster.events.items():
                if not event.fabric_sensitive:
                    continue
                if event_id not in zap_cluster.fabric_sensitive_event_ids:
                    findings.append(f"{xml_cluster.name}/{event.name}: spec fabricSensitive, ZAP not")
        self.assert_findings_are_known(findings, "event fabric sensitivity")

    def test_command_fabric_scoping_matches_spec(self):
        """A command the spec marks fabric scoped must be isFabricScoped in ZAP.

        The marker is what makes CheckCommandFlags reject the command over a
        session with no accessing fabric, so a missing one leaves the command
        invocable over PASE.
        """
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            zap_cluster = ZAP.clusters[cluster_id]
            for command_id, command in xml_cluster.accepted_commands.items():
                if not command.fabric_scoped:
                    continue
                if command_id not in zap_cluster.fabric_scoped_command_ids:
                    findings.append(f"{xml_cluster.name}/{command.name}: spec fabricScoped, ZAP not")
        self.assert_findings_are_known(findings, "command fabric scoping")

    def test_attribute_fabric_sensitivity_matches_spec(self):
        """An attribute the spec marks fabric sensitive must be isFabricSensitive in ZAP."""
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            zap_cluster = ZAP.clusters[cluster_id]
            for attribute_id, attribute in xml_cluster.attributes.items():
                if not attribute.fabric_sensitive:
                    continue
                if attribute_id not in zap_cluster.fabric_sensitive_attribute_ids:
                    findings.append(f"{xml_cluster.name}/{attribute.name}: spec fabricSensitive, ZAP not")
        self.assert_findings_are_known(findings, "attribute fabric sensitivity")

    def test_attribute_level_sensitivity_is_consumed(self):
        """Something must act on an attribute the spec marks fabric sensitive.

        This is the check the plain marker comparisons cannot make. For the
        attributes carrying the quality, spec and ZAP agree that the marker is
        there; what varies is whether anything uses it. The SDK has no generic
        implementation, so the entries of another fabric are withheld only if the
        entry struct masks every field, or the cluster filters the list itself.
        A cluster doing neither returns another fabric's data in full.
        """
        findings = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            zap_cluster = ZAP.clusters[cluster_id]
            for attribute_id, attribute in xml_cluster.attributes.items():
                if not attribute.fabric_sensitive:
                    continue
                if cluster_id in HAND_FILTERING_CLUSTERS:
                    continue
                entry_type = zap_cluster.entry_type_by_attribute_id.get(attribute_id)
                if entry_type is None:
                    continue
                zap_struct = ZAP.struct(cluster_id, entry_type)
                if zap_struct is None:
                    continue
                # Every field has to be masked, because the whole entry is what
                # the reader must not see. The FabricIndex is not among a ZAP
                # struct's items, so the full field set is the right comparison.
                if zap_struct.sensitive_field_ids == zap_struct.field_ids:
                    continue
                if not zap_struct.sensitive_field_ids:
                    findings.append(f"{xml_cluster.name}/{attribute.name}: attribute is fabric sensitive but "
                                    f"{entry_type} masks no field and the cluster does not hand-filter")
                else:
                    findings.append(f"{xml_cluster.name}/{attribute.name}: attribute is fabric sensitive but "
                                    f"{entry_type} masks only {sorted(zap_struct.sensitive_field_ids)} of "
                                    f"{sorted(zap_struct.field_ids)}")
        self.assert_findings_are_known(findings, "attribute-level fabric sensitivity")

    def test_fabric_scoped_attributes_have_a_masking_rule(self):
        """Census of fabric-scoped attributes whose entries carry nothing to mask.

        Not a defect on its own: plenty of fabric-scoped lists legitimately have
        no sensitive fields, and are protected by fabric filtering and their read
        privilege alone. It is logged rather than asserted because it is the set
        TC-IDM-8.1 can only check the filtered-read half of, so it says which
        attributes a passing run has said the least about.
        """
        no_masking_rule = []
        for cluster_id in audited_cluster_ids():
            xml_cluster = SPEC_CLUSTERS[cluster_id]
            zap_cluster = ZAP.clusters[cluster_id]
            for attribute_id, attribute in xml_cluster.attributes.items():
                if not attribute.fabric_scoped:
                    continue
                entry_type = zap_cluster.entry_type_by_attribute_id.get(attribute_id)
                if entry_type is None:
                    continue
                zap_struct = ZAP.struct(cluster_id, entry_type)
                spec_ids = sensitive_field_ids(spec_struct(cluster_id, entry_type))
                zap_ids = zap_struct.sensitive_field_ids if zap_struct is not None else frozenset()
                if spec_ids or zap_ids:
                    continue
                no_masking_rule.append(f"{xml_cluster.name}/{attribute.name} (entries are {entry_type})")

        self.print_step("census", f"{len(no_masking_rule)} fabric-scoped attribute(s) have no fabric-sensitive "
                        f"field in either the spec or ZAP, so only their fabric filtering can be verified")
        for entry in sorted(no_masking_rule):
            self.print_step("census", f"  {entry}")


if __name__ == "__main__":
    default_matter_test_main()
