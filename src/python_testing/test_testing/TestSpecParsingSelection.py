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


from DeviceConformanceTests import DeviceConformanceTests
from mobly import asserts, signals

import matter.clusters as Clusters
from matter.testing.conformance import ConformanceAssessmentData, ConformanceDecision, ConformanceException
from matter.testing.global_attribute_ids import is_standard_attribute_id
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, dm_from_spec_version
from matter.tlv import uint


class TestSpecParsingSelection(DeviceConformanceTests):
    def setup_class(self):
        # Overriding the DeviceConformanceTest setup_class so we don't go out to a real device
        pass

    def test_dm_from_spec_version(self):
        # dm_from_spec_version is not implemented for 1.2 because this attribute does not exist.
        # Instead, the basic composition tests that use the _get_dm function using the endpoint
        # information. This selection mechanism is tested in the test conformance test, where we
        # build a 1.2-style mock device that will fail conformance checks if the wrong pre-built is
        # selected.
        asserts.assert_equal(dm_from_spec_version(0x01030000), PrebuiltDataModelDirectory.k1_3,
                             "Incorrect directory selected for 1.3 with patch 0")
        asserts.assert_equal(dm_from_spec_version(0x01030100), PrebuiltDataModelDirectory.k1_3,
                             "Incorrect directory selected for 1.3 with patch 1")
        asserts.assert_equal(dm_from_spec_version(0x01040100), PrebuiltDataModelDirectory.k1_4_1,
                             "Incorrect directory selected for 1.4.1")
        asserts.assert_equal(dm_from_spec_version(0x01040100), PrebuiltDataModelDirectory.k1_4_1,
                             "Incorrect directory selected for 1.4.1")
        asserts.assert_equal(dm_from_spec_version(0x01040200), PrebuiltDataModelDirectory.k1_4_2,
                             "Incorrect directory selected for 1.4.2")
        asserts.assert_equal(dm_from_spec_version(0x01050000), PrebuiltDataModelDirectory.k1_5,
                             "Incorrect directory selected for 1.5")
        asserts.assert_equal(dm_from_spec_version(0x01050100), PrebuiltDataModelDirectory.k1_5_1,
                             "Incorrect directory selected for 1.5.1")

        # 1.2 doesn't include a specification revision field, so this should error
        with asserts.assert_raises(ConformanceException, "Expected assertion was not raised for spec version 1.2"):
            dm_from_spec_version(0x01020000)

        # Any dot release besides 0, 1 and 2 for 1.4 should error
        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.4.2"):
            dm_from_spec_version(0x01040300)

        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.4.FF"):
            dm_from_spec_version(0x0104FF00)

        # Any dot release above .1 should error for 1.5
        with asserts.assert_raises(ConformanceException, "Data model incorrectly identified for 1.5.2"):
            dm_from_spec_version(0x01050200)
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
        if spec_version is None:
            dm = PrebuiltDataModelDirectory.k1_2
        elif spec_version == 0xFFFFFFFF:
            # use 1.4.1, but change the spec version to something bad.
            dm = PrebuiltDataModelDirectory.k1_4_1
        else:
            dm = dm_from_spec_version(spec_version)
        xml_clusters, _ = build_xml_clusters(dm)

        gc_feature_map = Clusters.GeneralCommissioning.Bitmaps.Feature.kTermsAndConditions if tc_enabled else 0

        def create_cluster_globals(cluster, feature_map):
            spec_attributes = xml_clusters[cluster.id].attributes
            spec_accepted_commands = xml_clusters[cluster.id].accepted_commands
            spec_generated_commands = xml_clusters[cluster.id].generated_commands
            info = ConformanceAssessmentData(feature_map=feature_map, attribute_list=[], all_command_list=[], cluster_revision=1)
            # Build just the lists - basic composition checks the wildcard against the lists, conformance just uses lists
            attributes = [id for id, a in spec_attributes.items() if a.conformance(
                info).decision == ConformanceDecision.MANDATORY]
            accepted_commands = [id for id, c in spec_accepted_commands.items() if c.conformance(
                info).decision == ConformanceDecision.MANDATORY]
            generated_commands = [id for id, c in spec_generated_commands.items() if c.conformance(
                info).decision == ConformanceDecision.MANDATORY]
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

        if spec_version is not None:
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
        spec_version_str = f'{spec_version:08X}' if spec_version is not None else "None (1.2)"
        asserts.assert_equal(success, expect_success_conformance,
                             f"Improper conformance result for spec version {spec_version_str}, TC: {tc_enabled} problems: {problem_str}")

        success, problems = self.check_revisions(ignore_in_progress=False)
        asserts.assert_equal(success, expect_success_revisions,
                             f"Improper revision result for spec version {spec_version_str}, TC: {tc_enabled} problems: {problems}")

    def test_conformance(self):
        # 1.2 is OK if TC is off. Note that 1.2 doesn't have a spec revision field, which is why ths is none.
        # Below, we can use this to force a conformance error when this value is populated to prove the 1.2
        # DM files are being properly selected.
        self._run_conformance_against_device(spec_version=None, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.3 is OK if TC is off
        self._run_conformance_against_device(spec_version=0x01030000, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4 is OK if TC is off
        self._run_conformance_against_device(spec_version=0x01040000, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.1 is OK if TC is off
        self._run_conformance_against_device(spec_version=0x01040100, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.2 is OK if TC is off
        self._run_conformance_against_device(spec_version=0x01040200, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.5 is OK if TC is off
        self._run_conformance_against_device(spec_version=0x01050000, tc_enabled=False,
                                             expect_success_conformance=True, expect_success_revisions=True)

        # 1.5 is OK if TC is on
        self._run_conformance_against_device(spec_version=0x01050000, tc_enabled=True,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.2 is OK if TC is on
        self._run_conformance_against_device(spec_version=0x01040200, tc_enabled=True,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4.1 is OK if TC is on
        self._run_conformance_against_device(spec_version=0x01040100, tc_enabled=True,
                                             expect_success_conformance=True, expect_success_revisions=True)
        # 1.4 is NOT OK if TC is on
        self._run_conformance_against_device(spec_version=0x01040000, tc_enabled=True,
                                             expect_success_conformance=False, expect_success_revisions=True)
        # 1.3 is NOT OK if TC is on
        self._run_conformance_against_device(spec_version=0x01030000, tc_enabled=True,
                                             expect_success_conformance=False, expect_success_revisions=True)
        # 1.2 is NOT OK if TC is on
        self._run_conformance_against_device(spec_version=None, tc_enabled=True,
                                             expect_success_conformance=False, expect_success_revisions=True)

        # Check that we get a test failure on a bad spec revision
        self._create_device(0xFFFFFFFF, False)
        with asserts.assert_raises(signals.TestFailure, "Exception not properly raised for bad spec type"):
            self.build_spec_xmls()


if __name__ == "__main__":
    default_matter_test_main()
