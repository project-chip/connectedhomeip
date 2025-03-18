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
import chip.clusters as Clusters
from chip.testing.conformance import ConformanceDecision, ConformanceException
from chip.testing.global_attribute_ids import is_standard_attribute_id
from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from chip.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, dm_from_spec_version
from chip.tlv import uint
from mobly import asserts, signals
from TC_DeviceConformance import DeviceConformanceTests


class TestSpecParsingSelection(MatterBaseTest, DeviceConformanceTests):
    def setup_class(self):
        # Overriding the DeviceConformanceTest setup_class so we don't go out to a real device
        pass

    def test_dm_from_spec_version(self):
        asserts.assert_equal(dm_from_spec_version(0x01030000), PrebuiltDataModelDirectory.k1_3,
                             "Incorrect directory selected for 1.3 with patch 0")
        asserts.assert_equal(dm_from_spec_version(0x01030100), PrebuiltDataModelDirectory.k1_3,
                             "Incorrect directory selected for 1.3 with patch 1")
        asserts.assert_equal(dm_from_spec_version(0x01040100), PrebuiltDataModelDirectory.k1_4_1,
                             "Incorrect directory selected for 1.4.1")
        asserts.assert_equal(dm_from_spec_version(0x01040100), PrebuiltDataModelDirectory.k1_4_1,
                             "Incorrect directory selected for 1.4.1")
        asserts.assert_equal(dm_from_spec_version(0x01050000), PrebuiltDataModelDirectory.kMaster,
                             "Incorrect directory selected for 1.5")

        # We don't have data model files for 1.2, so these should error
        with asserts.assert_raises(ConformanceException, "Expected assertion was not raised for spec version 1.2"):
            dm_from_spec_version(0x01020000)

        # Any dot release besides 0 and 1 for 1.4 should error
        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.4.2"):
            dm_from_spec_version(0x01040200)

        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.4.FF"):
            dm_from_spec_version(0x0104FF00)

        # Any dot release besides 0 for 1.5 should error
        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.5.1"):
            dm_from_spec_version(0x01050100)
        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.5.FF"):
            dm_from_spec_version(0x0105FF00)

        # Any value with stuff in reserved should error
        with asserts.assert_raises(ConformanceException, "Error not returned for specification revision with non-zero reserved values"):
            dm_from_spec_version(0x01030001)
        with asserts.assert_raises(ConformanceException, "Error not returned for specification revision with non-zero reserved values"):
            dm_from_spec_version(0x01040001)
        with asserts.assert_raises(ConformanceException, "Error not returned for specification revision with non-zero reserved values"):
            dm_from_spec_version(0x01040101)
        with asserts.assert_raises(ConformanceException, "Error not returned for specification revision with non-zero reserved values"):
            dm_from_spec_version(0x01050001)

    def _create_device(self, spec_version: uint, tc_enabled: bool):
        # Build at 1.4.1 so we can have TC info
        xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)

        gc_feature_map = Clusters.GeneralCommissioning.Bitmaps.Feature.kTermsAndConditions if tc_enabled else 0

        def create_cluster_globals(cluster, feature_map):
            spec_attributes = xml_clusters[cluster.id].attributes
            spec_accepted_commands = xml_clusters[cluster.id].accepted_commands
            spec_generated_commands = xml_clusters[cluster.id].generated_commands
            # Build just the lists - basic composition checks the wildcard against the lists, conformance just uses lists
            attributes = [id for id, a in spec_attributes.items() if a.conformance(
                feature_map, [], []).decision == ConformanceDecision.MANDATORY]
            accepted_commands = [id for id, c in spec_accepted_commands.items() if c.conformance(
                feature_map, [], []).decision == ConformanceDecision.MANDATORY]
            generated_commands = [id for id, c in spec_generated_commands.items() if c.conformance(
                feature_map, [], []).decision == ConformanceDecision.MANDATORY]
            attr = cluster.Attributes

            resp = {}
            non_global_attrs = [a for a in attributes if is_standard_attribute_id(a)]
            for attribute_id in non_global_attrs:
                # We don't use the values in these tests, set them all to 0. The types are wrong, but it shouldn't matter
                resp[Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster.id][attribute_id]] = 0

            resp[attr.AttributeList] = attributes
            resp[attr.AcceptedCommandList] = accepted_commands
            resp[attr.GeneratedCommandList] = generated_commands
            resp[attr.FeatureMap] = feature_map
            resp[attr.ClusterRevision] = xml_clusters[cluster.id].revision

            return resp

        def get_tlv(resp):
            # This only works because there are no structs in here.
            # structs need special handling. Beware.
            return {k.attribute_id: v for k, v in resp.items()}

        gc_resp = create_cluster_globals(Clusters.GeneralCommissioning, gc_feature_map)
        bi_resp = create_cluster_globals(Clusters.BasicInformation, 0)
        bi_resp[Clusters.BasicInformation.Attributes.SpecificationVersion] = spec_version

        self.endpoints = {0: {Clusters.GeneralCommissioning: gc_resp, Clusters.BasicInformation: bi_resp}}
        self.endpoints_tlv = {0: {Clusters.GeneralCommissioning.id: get_tlv(
            gc_resp), Clusters.BasicInformation.id: get_tlv(bi_resp)}}

    def _run_conformance_against_device(self, spec_version: uint, tc_enabled: bool, expect_success_conformance: bool, expect_success_revisions: bool):
        self._create_device(spec_version, tc_enabled)
        # build the spec XMLs for the stated version
        self.build_spec_xmls()
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
        problem_strs = [str(p) for p in problems]
        problem_str = "\n".join(problem_strs)
        asserts.assert_equal(success, expect_success_conformance,
                             f"Improper conformance result for spec version {spec_version:08X}, TC: {tc_enabled} problems: {problem_str}")

        success, problems = self.check_revisions(ignore_in_progress=False)
        asserts.assert_equal(success, expect_success_revisions,
                             f"Improper revision result for spec version {spec_version:08X}, TC: {tc_enabled} problems: {problems}")

    def test_conformance(self):

        # 1.4 is OK if TC is off
        self._run_conformance_against_device(0x01040000, False, expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.1 is OK if TC is off
        self._run_conformance_against_device(0x01040100, False, expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.1 is OK if TC is on
        self._run_conformance_against_device(0x01040100, True, expect_success_conformance=True, expect_success_revisions=True)
        # 1.4 is NOT OK if TC is on
        self._run_conformance_against_device(0x01040000, True, expect_success_conformance=False, expect_success_revisions=True)

        # Check that we get a test failure on a bad spec revision
        self._create_device(0xFFFFFFFF, False)
        with asserts.assert_raises(signals.TestFailure, "Exception not properly raised for bad spec type"):
            self.build_spec_xmls()


if __name__ == "__main__":
    default_matter_test_main()
