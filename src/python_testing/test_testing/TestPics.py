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

import os
import tempfile
import zipfile

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.matter_testing import CertificationUnitTestNoDevice
from matter.testing.pics import (BASE_PICS_CODES_DERIVED, BasePicsFacts, base_pics_facts_to_pics_codes,
                                 derive_base_pics_facts_from_device_wildcard, generate_device_element_pics_from_device_wildcard,
                                 read_pics_from_file)
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters


# Reusable single-item PICS XML fixture. Kept as a helper so the zip
# tests below can build tiny archives without duplicating boilerplate.
def _pics_xml(name: str, support: bool = True) -> str:
    return (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<clusterPICS>"
        f"<picsItem><itemNumber>{name}</itemNumber>"
        f"<support>{'true' if support else 'false'}</support></picsItem>"
        "</clusterPICS>"
    )


class TestPicsHelpers(CertificationUnitTestNoDevice):

    def test_pics_generation(self):
        xml_cluster, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        # 0: opcreds with an attribute, an accepted command, a generated command
        # 0: cadmin with a feature, an attribute, an accepted command
        # 1: Door lock cluster with two features, two attributes, two accepted commands, two generated commands
        wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})
        desc = Clusters.Descriptor
        opcreds = Clusters.OperationalCredentials
        cadmin = Clusters.AdministratorCommissioning
        lock = Clusters.DoorLock

        wildcard.tlvAttributes[0] = {desc.id: {}, opcreds.id: {}, cadmin.id: {}}
        wildcard.tlvAttributes[1] = {lock.id: {}}

        wildcard.tlvAttributes[0][desc.id][desc.Attributes.AttributeList.attribute_id] = [
            desc.Attributes.DeviceTypeList.attribute_id]
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.AcceptedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.GeneratedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][desc.id][desc.Attributes.FeatureMap.attribute_id] = 0
        desc_expected = ['DESC.S', 'DESC.S.A0000']
        # The rest of the processing is done through tlvAttributes, but this one needs to be parsed, so it comes from here.
        wildcard.attributes = {0: {desc: {desc.Attributes.DeviceTypeList: [
            desc.Structs.DeviceTypeStruct(deviceType=0x16, revision=1)]}}}

        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.AttributeList.attribute_id] = [
            opcreds.Attributes.NOCs.attribute_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.AcceptedCommandList.attribute_id] = [
            opcreds.Commands.AttestationRequest.command_id, opcreds.Commands.AddTrustedRootCertificate.command_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.GeneratedCommandList.attribute_id] = [
            opcreds.Commands.AttestationResponse.command_id]
        wildcard.tlvAttributes[0][opcreds.id][opcreds.Attributes.FeatureMap.attribute_id] = 0
        opcreds_expected_suffix = ['A0000', 'C00.Rsp', 'C0b.Rsp', 'C01.Tx']
        opcreds_expected = [f'OPCREDS.S.{s}' for s in opcreds_expected_suffix]
        opcreds_expected.append('OPCREDS.S')

        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.AttributeList.attribute_id] = [
            cadmin.Attributes.AdminFabricIndex.attribute_id]
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.AcceptedCommandList.attribute_id] = [
            cadmin.Commands.OpenCommissioningWindow.command_id]
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.GeneratedCommandList.attribute_id] = []
        wildcard.tlvAttributes[0][cadmin.id][cadmin.Attributes.FeatureMap.attribute_id] = 1
        cadmin_expected_suffix = ['A0001', 'C00.Rsp', 'F00']
        cadmin_expected = [f'CADMIN.S.{s}' for s in cadmin_expected_suffix]
        cadmin_expected.append('CADMIN.S')

        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id] = [
            lock.Attributes.DoorState.attribute_id, lock.Attributes.LockType.attribute_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AcceptedCommandList.attribute_id] = [
            lock.Commands.GetUser.command_id, lock.Commands.GetCredentialStatus.command_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.GeneratedCommandList.attribute_id] = [
            lock.Commands.GetUserResponse.command_id, lock.Commands.GetCredentialStatusResponse.command_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.FeatureMap.attribute_id] = 3
        lock_expected_suffix = ['A0003', 'A0001', 'C1b.Rsp', 'C24.Rsp', 'C1c.Tx', 'C25.Tx', 'F00', 'F01']
        lock_expected = [f'DRLK.S.{s}' for s in lock_expected_suffix]
        lock_expected.append('DRLK.S')

        def check_expected_pics(pics_list: dict[int, list[str]]):
            asserts.assert_equal(set(pics_list.keys()), {0, 1}, "Unexpected endpoints in PICS list")
            asserts.assert_equal(set(pics_list[0]), set(desc_expected + opcreds_expected + cadmin_expected +
                                 ['IDM.S', 'MCORE.ROLE.COMMISSIONEE']), "Incorrect PICS list on EP0")
            asserts.assert_equal(set(pics_list[1]), set(lock_expected + ['IDM.S']), "Incorrect PICS list on EP1")

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add globals, should be no errors, should not appear
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id].extend(
            [lock.Attributes.AttributeList.attribute_id, lock.Attributes.AcceptedCommandList.attribute_id, lock.Attributes.GeneratedCommandList.attribute_id, lock.Attributes.FeatureMap.attribute_id, lock.Attributes.ClusterRevision.attribute_id])

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add MEI cluster, MEI attribute, MEI accepted command, MEI generated command - should be no errors, should not appear
        unit_testing = Clusters.UnitTesting
        wildcard.tlvAttributes[1][unit_testing.id] = {}
        # MEI cluster
        wildcard.tlvAttributes[1][unit_testing.id][unit_testing.Attributes.AttributeList.attribute_id] = [
            unit_testing.Attributes.Bitmap16.attribute_id]
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AttributeList.attribute_id].append(0x60060000)
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.AcceptedCommandList.attribute_id].append(0x60060000)
        wildcard.tlvAttributes[1][lock.id][lock.Attributes.GeneratedCommandList.attribute_id].append(0x60060001)

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

        # Add a standard cluster that's not part of the standard cluster set - this should cause an error because we don't know the PICS
        unknown_standard_cluster = max(xml_cluster.keys()) + 1
        wildcard.tlvAttributes[1][unknown_standard_cluster] = {}
        wildcard.tlvAttributes[1][unknown_standard_cluster][unit_testing.Attributes.AttributeList.attribute_id] = [0]

        pics_list, problems = generate_device_element_pics_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(len(problems), 1, "Unexpected problems found generating PICS list")
        check_expected_pics(pics_list)

    def test_base_pics_facts_to_pics_codes(self):
        # Empty facts must produce an empty code set so a non-commissioned,
        # non-server fact bundle does not accidentally mark anything.
        asserts.assert_equal(base_pics_facts_to_pics_codes(BasePicsFacts()), set(),
                             "Empty BasePicsFacts must produce no PICS codes")

        # Every individual flag, mapped one at a time, must appear in the
        # tracked-codes set so callers can rely on BASE_PICS_CODES_DERIVED.
        cases = [
            (BasePicsFacts(is_commissionee=True), "MCORE.ROLE.COMMISSIONEE"),
            (BasePicsFacts(is_server=True), "MCORE.IDM.S"),
            (BasePicsFacts(is_bridge=True), "MCORE.BRIDGE"),
            (BasePicsFacts(is_ota_requestor=True), "MCORE.OTA.Requestor"),
            (BasePicsFacts(is_ota_provider=True), "MCORE.OTA.Provider"),
            (BasePicsFacts(has_groups_on_multiple_endpoints=True), "MCORE.G.MULTIENDPOINT"),
        ]
        for facts, expected_code in cases:
            codes = base_pics_facts_to_pics_codes(facts)
            asserts.assert_equal(codes, {expected_code}, f"Expected only {expected_code}, got {codes}")
            asserts.assert_in(expected_code, BASE_PICS_CODES_DERIVED,
                              f"{expected_code} must be in BASE_PICS_CODES_DERIVED so the test step iterates it")

        # All-on must produce all codes.
        all_on = BasePicsFacts(
            is_commissionee=True, is_server=True, is_bridge=True,
            is_ota_requestor=True, is_ota_provider=True,
            has_groups_on_multiple_endpoints=True)
        asserts.assert_equal(base_pics_facts_to_pics_codes(all_on), set(BASE_PICS_CODES_DERIVED),
                             "All-on facts must produce the full tracked-codes set")

    def test_derive_base_pics_facts_from_device_wildcard(self):
        xml_cluster, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        desc = Clusters.Descriptor
        opcreds = Clusters.OperationalCredentials
        groups = Clusters.Groups
        ota_req = Clusters.OtaSoftwareUpdateRequestor

        # Minimal "root node plus bridged child with Groups on EP1 and EP2" wildcard.
        # EP0: root node device type, OpCreds (so endpoint has a server),
        #      OTA Requestor (server-side).
        # EP1: Aggregator device type, Groups server.
        # EP2: Groups server.
        wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})

        wildcard.attributes[0] = {
            desc: {desc.Attributes.DeviceTypeList: [
                desc.Structs.DeviceTypeStruct(deviceType=0x16, revision=1)]},
        }
        wildcard.attributes[1] = {
            desc: {desc.Attributes.DeviceTypeList: [
                desc.Structs.DeviceTypeStruct(deviceType=0x000E, revision=1)]},
        }
        wildcard.attributes[2] = {
            desc: {desc.Attributes.DeviceTypeList: []},
        }

        # Build minimal global-attribute fills for each cluster the helper
        # will visit. Empty attribute/command lists are fine for the
        # rules covered here; event conformance is covered in its own test.
        def _empty_globals(feature_map=0):
            return {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: feature_map,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            }

        wildcard.tlvAttributes[0] = {
            desc.id: _empty_globals(),
            opcreds.id: _empty_globals(),
            ota_req.id: _empty_globals(),
        }
        wildcard.tlvAttributes[1] = {
            desc.id: _empty_globals(),
            groups.id: _empty_globals(),
        }
        wildcard.tlvAttributes[2] = {
            desc.id: _empty_globals(),
            groups.id: _empty_globals(),
        }

        facts, problems = derive_base_pics_facts_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(problems, [], "Unexpected derivation problems")
        asserts.assert_true(facts.is_commissionee, "Root node device type on EP0 must derive is_commissionee")
        asserts.assert_true(facts.is_server, "Standard server clusters on EP0 must derive is_server")
        asserts.assert_true(facts.is_bridge, "Aggregator device type on EP1 must derive is_bridge")
        asserts.assert_true(facts.is_ota_requestor, "OTA Requestor cluster on EP0 must derive is_ota_requestor")
        asserts.assert_false(facts.is_ota_provider, "OTA Provider not present, must not derive is_ota_provider")
        asserts.assert_true(facts.has_groups_on_multiple_endpoints,
                            "Groups on EP1 + EP2 must derive has_groups_on_multiple_endpoints")

        # Translator should map this exactly to the expected MCORE set,
        # excluding the one it doesn't cover (OTA Provider).
        expected_codes = {
            "MCORE.ROLE.COMMISSIONEE", "MCORE.IDM.S", "MCORE.BRIDGE",
            "MCORE.OTA.Requestor", "MCORE.G.MULTIENDPOINT",
        }
        asserts.assert_equal(base_pics_facts_to_pics_codes(facts), expected_codes,
                             "Translator output drifted from the per-rule facts")

    def test_derive_base_pics_facts_groups_on_single_endpoint(self):
        # Groups cluster on a single endpoint must NOT derive
        # has_groups_on_multiple_endpoints. The >= 2 threshold is the
        # whole point of the MCORE.G.MULTIENDPOINT PICS.
        xml_cluster, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        desc = Clusters.Descriptor
        groups = Clusters.Groups

        wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})
        wildcard.attributes[0] = {desc: {desc.Attributes.DeviceTypeList: []}}
        wildcard.attributes[1] = {desc: {desc.Attributes.DeviceTypeList: []}}
        wildcard.tlvAttributes[0] = {
            desc.id: {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: 0,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            },
        }
        wildcard.tlvAttributes[1] = {
            desc.id: {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: 0,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            },
            groups.id: {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: 0,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            },
        }

        facts, problems = derive_base_pics_facts_from_device_wildcard(wildcard, xml_cluster)
        asserts.assert_equal(problems, [], "Unexpected derivation problems")
        asserts.assert_false(facts.has_groups_on_multiple_endpoints,
                             "Groups on a single endpoint must not derive MCORE.G.MULTIENDPOINT")

    def test_derive_base_pics_facts_mandatory_events(self):
        # AccessControl on EP0 has spec-MANDATORY events (AccessControlEntryChanged,
        # AccessControlExtensionChanged). Their conformance evaluates to
        # mandatory with no feature-map dependency, so they should land in
        # facts.mandatory_events_by_cluster regardless of feature bits.
        xml_cluster, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        acl = Clusters.AccessControl
        desc = Clusters.Descriptor

        # Only run this rule if the chosen DM XML actually has events for
        # AccessControl; otherwise the test would be tautological.
        asserts.assert_true(
            len(xml_cluster[acl.id].events) > 0,
            "Test fixture assumption broken: AccessControl 1.4.1 must have events in DM XML")

        wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})
        wildcard.attributes[0] = {desc: {desc.Attributes.DeviceTypeList: []}}
        wildcard.tlvAttributes[0] = {
            desc.id: {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: 0,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            },
            acl.id: {
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: [],
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: [],
                GlobalAttributeIds.FEATURE_MAP_ID: 0,
                GlobalAttributeIds.CLUSTER_REVISION_ID: 1,
            },
        }

        facts, _ = derive_base_pics_facts_from_device_wildcard(wildcard, xml_cluster)
        ep_events = facts.mandatory_events_by_cluster.get(0, {})
        acl_mandatory = ep_events.get(acl.id, set())
        asserts.assert_true(
            len(acl_mandatory) > 0,
            "AccessControl spec-mandatory events must be derived for any DUT with AccessControl on EP0")

    def test_read_pics_from_file_endpoint_naming_variants(self):
        """Endpoint subdir matching tolerates common naming conventions."""
        test_xml = _pics_xml('TEST.S')
        for subdir_name in ['endpoint0', 'EP0', 'ep_0', 'Endpoint 0', '0']:
            with tempfile.TemporaryDirectory() as d:
                ep_dir = os.path.join(d, subdir_name)
                os.makedirs(ep_dir)
                with open(os.path.join(ep_dir, 'cluster.xml'), 'w') as f:
                    f.write(test_xml)
                pics = read_pics_from_file(d, endpoint=0)
                asserts.assert_true(pics.get('TEST.S'), f'Failed for subdir name: {subdir_name}')

    def test_read_pics_from_zip_file(self):
        """
        A zip archive of PICS XML files is treated the same as a directory:
        top-level XML always loads, per-endpoint XML only loads when the
        matching endpoint is requested, and non-matching endpoint subdirs
        are skipped so foreign-cluster PICS don't leak in.
        """
        with tempfile.TemporaryDirectory() as d:
            zip_path = os.path.join(d, 'pics.zip')
            with zipfile.ZipFile(zip_path, 'w') as zf:
                zf.writestr('base.xml', _pics_xml('MCORE.BASE'))
                zf.writestr('endpoint0/desc.xml', _pics_xml('DESC.S'))
                zf.writestr('endpoint1/drlk.xml', _pics_xml('DRLK.S'))

            # No endpoint: only device-wide PICS load.
            pics_none = read_pics_from_file(zip_path)
            asserts.assert_true(pics_none.get('MCORE.BASE'),
                                'Top-level base PICS must always load from zip')
            asserts.assert_not_in('DESC.S', pics_none,
                                  'EP0 PICS must not leak when endpoint is unspecified')
            asserts.assert_not_in('DRLK.S', pics_none,
                                  'EP1 PICS must not leak when endpoint is unspecified')

            # endpoint=0: base + EP0 only, EP1 stays out.
            pics_ep0 = read_pics_from_file(zip_path, endpoint=0)
            asserts.assert_true(pics_ep0.get('MCORE.BASE'), 'Base still loads with endpoint arg')
            asserts.assert_true(pics_ep0.get('DESC.S'), 'endpoint=0 must load EP0 PICS from zip')
            asserts.assert_not_in('DRLK.S', pics_ep0,
                                  'endpoint=0 must not load foreign EP1 PICS from zip')

            # endpoint=1: base + EP1 only, EP0 stays out.
            pics_ep1 = read_pics_from_file(zip_path, endpoint=1)
            asserts.assert_true(pics_ep1.get('MCORE.BASE'), 'Base still loads with endpoint arg')
            asserts.assert_true(pics_ep1.get('DRLK.S'), 'endpoint=1 must load EP1 PICS from zip')
            asserts.assert_not_in('DESC.S', pics_ep1,
                                  'endpoint=1 must not load foreign EP0 PICS from zip')

    def test_read_pics_from_zip_with_wrapping_directory(self):
        """
        Archives produced by OS "compress folder" utilities (Finder,
        Explorer, `zip -r some_folder`) nest everything under a single
        top-level directory. The reader must descend into it automatically
        so users don't have to re-zip.
        """
        with tempfile.TemporaryDirectory() as d:
            zip_path = os.path.join(d, 'wrapped.zip')
            with zipfile.ZipFile(zip_path, 'w') as zf:
                zf.writestr('pics_root/base.xml', _pics_xml('WRAPPED.S'))
                zf.writestr('pics_root/endpoint0/ep.xml', _pics_xml('WRAPPED.EP.S'))
            pics = read_pics_from_file(zip_path, endpoint=0)
            asserts.assert_true(pics.get('WRAPPED.S'),
                                'Base PICS inside a wrapping directory must be discovered')
            asserts.assert_true(pics.get('WRAPPED.EP.S'),
                                'Per-endpoint PICS inside a wrapping directory must be discovered')

    def test_read_pics_from_zip_endpoint_naming_variants(self):
        """
        Zip endpoint subdir matching accepts the same naming variants as
        loose directories, so a user zipping up an existing PICS folder
        doesn't hit new naming constraints just by archiving it.
        """
        for subdir_name in ['endpoint0', 'EP0', 'ep_0', 'Endpoint 0', '0']:
            with tempfile.TemporaryDirectory() as d:
                zip_path = os.path.join(d, 'variant.zip')
                with zipfile.ZipFile(zip_path, 'w') as zf:
                    zf.writestr(f'{subdir_name}/cluster.xml', _pics_xml('ZIP.NAMING.S'))
                pics = read_pics_from_file(zip_path, endpoint=0)
                asserts.assert_true(pics.get('ZIP.NAMING.S'),
                                    f'Zip endpoint subdir name not matched: {subdir_name!r}')

    def test_read_pics_from_zip_ignores_non_matching_endpoint(self):
        """
        When endpoint=N is requested and the archive contains an endpoint
        subdir for M != N, the M PICS must stay out. Same rule as loose
        directories; belt-and-braces alongside the main zip test since a
        past regression here would silently pollute per-endpoint checks.

        Also covers a subtle interaction with the wrapping-directory
        heuristic: an archive containing only `endpoint5/` looks like a
        "single top-level directory" archive, but that lone directory IS
        the per-endpoint subdir, not a wrapper. If the heuristic drilled
        into it, its per-endpoint PICS would be demoted to device-wide
        PICS and would leak into every endpoint (or into endpoint=None).
        Asserting endpoint=5 still resolves correctly pins the shape.
        """
        with tempfile.TemporaryDirectory() as d:
            zip_path = os.path.join(d, 'mismatch.zip')
            with zipfile.ZipFile(zip_path, 'w') as zf:
                zf.writestr('endpoint5/only_ep5.xml', _pics_xml('EP5.ONLY.S'))

            # Wrong endpoint: EP5 PICS must not leak in.
            pics_ep0 = read_pics_from_file(zip_path, endpoint=0)
            asserts.assert_not_in('EP5.ONLY.S', pics_ep0,
                                  'endpoint=0 must not pick up PICS from unrelated endpoint5 subdir')

            # No endpoint: same — no top-level XML, no leak.
            pics_none = read_pics_from_file(zip_path)
            asserts.assert_equal(pics_none, {},
                                 'Archive with only per-endpoint dirs and no top-level XML '
                                 'must produce no PICS when no endpoint is requested')

            # Correct endpoint: EP5 PICS must load. This is the positive
            # side of the wrapping-directory heuristic pin.
            pics_ep5 = read_pics_from_file(zip_path, endpoint=5)
            asserts.assert_true(pics_ep5.get('EP5.ONLY.S'),
                                'endpoint=5 must resolve a lone endpoint5/ archive as its per-endpoint subdir, '
                                'not as a wrapping directory')

    def test_read_pics_from_zip_rejects_path_traversal(self):
        """
        Zip entries whose paths resolve outside the extraction destination
        must be refused before extraction (zip slip). Portable pre-check
        rather than relying on `extractall(filter=...)` which is only
        available on newer Python.
        """
        with tempfile.TemporaryDirectory() as d:
            zip_path = os.path.join(d, 'evil.zip')
            with zipfile.ZipFile(zip_path, 'w') as zf:
                zf.writestr('../escape.xml', '<clusterPICS/>')
            with asserts.assert_raises(ValueError):
                read_pics_from_file(zip_path)


if __name__ == "__main__":
    default_matter_test_main()
