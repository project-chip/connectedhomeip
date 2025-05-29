#
#    Copyright (c) 2023 Project CHIP Authors
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
import math

import chip.clusters as Clusters
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.matter_testing import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, FeaturePathLocation,
                                         MatterBaseTest, TestStep, UnknownProblemLocation, async_test_body,
                                         default_matter_test_main)
from chip.testing.pics import accepted_cmd_pics_str, attribute_pics_str, feature_pics_str, generated_cmd_pics_str
from mobly import asserts


class TC_PICS_Checker(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper(False)
        self.build_spec_xmls()

    def _check_and_record_errors(self, location, required, pics):
        if required and not self.check_pics(pics):
            self.record_error("PICS check", location=location,
                              problem=f"An element found on the device, but the corresponding PICS {pics} was not found in pics list")
            self.success = False
        elif not required and self.check_pics(pics):
            self.record_error("PICS check", location=location, problem=f"PICS {pics} found in PICS list, but not on device")
            self.success = False

    def _add_pics_for_lists(self, cluster_id: int, attribute_id_of_element_list: GlobalAttributeIds) -> None:
        try:
            if attribute_id_of_element_list == GlobalAttributeIds.ATTRIBUTE_LIST_ID:
                all_spec_elements_to_check = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]
                pics_mapper = attribute_pics_str
            elif attribute_id_of_element_list == GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID:
                all_spec_elements_to_check = Clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster_id]
                pics_mapper = accepted_cmd_pics_str

            elif attribute_id_of_element_list == GlobalAttributeIds.GENERATED_COMMAND_LIST_ID:
                all_spec_elements_to_check = Clusters.ClusterObjects.ALL_GENERATED_COMMANDS[cluster_id]
                pics_mapper = generated_cmd_pics_str
            else:
                asserts.fail("add_pics_for_list function called for non-list attribute")
        except KeyError:
            # This cluster does not have any of this element type
            return

        for element_id in all_spec_elements_to_check:
            if element_id > 0xF000:
                # No pics for global elements
                continue
            pics = pics_mapper(self.xml_clusters[cluster_id].pics, element_id)

            if cluster_id not in self.endpoint.keys():
                # This cluster is not on this endpoint
                required = False
            elif element_id in self.endpoint[cluster_id][attribute_id_of_element_list]:
                # Cluster and element are on the endpoint
                required = True
            else:
                # Cluster is on the endpoint but the element is not in the list
                required = False

            if attribute_id_of_element_list == GlobalAttributeIds.ATTRIBUTE_LIST_ID:
                location = AttributePathLocation(endpoint_id=self.endpoint_id, cluster_id=cluster_id, attribute_id=element_id)
            else:
                location = CommandPathLocation(endpoint_id=self.endpoint_id, cluster_id=cluster_id, command_id=element_id)

            self._check_and_record_errors(location, required, pics)

    def steps_TC_IDM_10_4(self):
        return [TestStep(1, "TH performs a wildcard read of all attributes on the endpoint under test"),
                TestStep(2, "For every standard cluster: If the cluster is present on the endpoint, ensure the server-side PICS code for the cluster is present in the PICS file (e.g. OO.S for On/Off cluster).If the cluster is not present on the endpoint, ensure the cluster server PICS code is not present in the PICS file.", "PICS exactly match for server clusters."),
                TestStep(3, "For every standard cluster, for every attribute in the cluster:If the cluster is present on the endpoint and the attribute ID is present in the AttributeList global attribute within the cluster, ensure the server-side PICS code for the attribute is present in the PICS file (e.g. OO.S.A000 for On/Off cluster’s OnOff attribute).Otherwise, ensure the attribute PICS code is NOT present in the PICS file.", "PICS exactly match for all attributes in all clusters."),
                TestStep(4, "For every cluster present in the spec, for every client → server command in the cluster: If the cluster is present on the endpoint and the command id is present in the accepted commands list, ensure the PICS code for the accepted command is present in the PICS file. Otherwise, ensure the accepted command PICS code is not present in the PICS file.", "PICS exactly match for all accepted commands in all clusters."),
                TestStep(5, "For every cluster present in the spec, for every server → client command in the cluster: If the cluster is present on the endpoint and the command id is present in the generated commands list, ensure the PICS code for the generated command is present in the PICS file. Otherwise, ensure the generated command PICS code is not present in the PICS file.", "PICS exactly match for all generated commands in all clusters."),
                TestStep(6, "For every cluster present in the spec, for every feature in the cluster: If the cluster is present on the endpoint and the feature is marked in the feature map, ensure the PICS code for the feature is present in the PICS file. Otherwise, ensure the feature PICS code is not present in the PICS file.", "PICS exactly match for all features in all clusters."),
                TestStep(7, "Ensure that the PICS_SDK_CI_ONLY PICS does not appear in the PICS file", "CI PICS is not present")]

    def test_TC_IDM_10_4(self):
        # wildcard read is done in setup_class
        self.step(1)
        self.endpoint_id = self.matter_test_config.endpoint
        self.endpoint = self.endpoints_tlv[self.endpoint_id]
        self.success = True

        # Data model XML is used to get the PICS code for this cluster. If we don't know the PICS
        # code, we can't evaluate the PICS list. Clusters that are present on the device but are
        # not present in the spec are checked in the IDM tests.
        checkable_clusters = {cluster_id: cluster for cluster_id, cluster in Clusters.ClusterObjects.ALL_CLUSTERS.items(
        ) if cluster_id in self.xml_clusters and self.xml_clusters[cluster_id].pics is not None}

        # TODO: consider what we want to do with the OTA clusters. They do not currently have PICS codes.
        ota_ids = [Clusters.OtaSoftwareUpdateProvider.id, Clusters.OtaSoftwareUpdateRequestor.id]
        checkable_clusters = {cluster_id: cluster for cluster_id,
                              cluster in checkable_clusters.items() if cluster_id not in ota_ids}

        self.step(2)
        for cluster_id, cluster in checkable_clusters.items():
            # Ensure the PICS.S code is correctly marked
            pics_cluster = f'{self.xml_clusters[cluster_id].pics}.S'
            location = ClusterPathLocation(endpoint_id=self.endpoint_id, cluster_id=cluster_id)
            self._check_and_record_errors(location, cluster_id in self.endpoint, pics_cluster)

        self.step(3)
        for cluster_id, cluster in checkable_clusters.items():
            self._add_pics_for_lists(cluster_id, GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        self.step(4)
        for cluster_id, cluster in checkable_clusters.items():
            self._add_pics_for_lists(cluster_id, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)

        self.step(5)
        for cluster_id, cluster in checkable_clusters.items():
            self._add_pics_for_lists(cluster_id, GlobalAttributeIds.GENERATED_COMMAND_LIST_ID)

        self.step(6)
        for cluster_id, cluster in checkable_clusters.items():
            try:
                cluster_features = cluster.Bitmaps.Feature
            except AttributeError:
                # cluster has no features
                continue

            pics_base = self.xml_clusters[cluster_id].pics
            try:
                feature_map = self.endpoint[cluster_id][GlobalAttributeIds.FEATURE_MAP_ID]
            except KeyError:
                feature_map = 0

            for feature_mask in cluster_features:
                # Codegen in python uses feature masks (0x01, 0x02, 0x04 etc.)
                # PICS uses the mask bit number (1, 2, 3)
                # Convert the mask to a bit number so we can check the PICS.
                try:
                    feature_bit = int(math.log2(feature_mask))
                except ValueError:
                    location = FeaturePathLocation(endpoint_id=self.endpoint_id,
                                                   cluster_id=cluster_id, feature_code=str(feature_mask))
                    # The feature_mask is from the code generated feature masks, not the features as listed on the
                    # device. If we get an error here, this is a problem with the codegen or spec, not with the device
                    # under test. We still want the problem recorded, but this does not indicate a problem on the DUT.
                    # There are two clusters with known bad features here - RvcRunMode and RvcCleanMode both have a
                    # feature mask of kNoFeatures and an empty "mask" of 0x0.
                    self.record_warning("PICS check", location=location,
                                        problem=f"Unable to parse feature mask {feature_mask} from cluster {cluster}")
                    continue
                pics = feature_pics_str(pics_base, feature_bit)
                if feature_mask & feature_map:
                    required = True
                else:
                    required = False

                try:
                    location = FeaturePathLocation(endpoint_id=self.endpoint_id, cluster_id=cluster_id,
                                                   feature_code=self.xml_clusters[cluster_id].features[feature_mask].code)
                except KeyError:
                    location = ClusterPathLocation(endpoint_id=self.endpoint_id, cluster_id=cluster_id)
                self._check_and_record_errors(location, required, pics)

        self.step(7)
        if self.is_pics_sdk_ci_only:
            self.record_error("PICS check", location=UnknownProblemLocation(),
                              problem="PICS PICS_SDK_CI_ONLY found in PICS list. This PICS is disallowed for certification.")
            self.success = False

        if not self.success:
            self.fail_current_test("At least one PICS error was found for this endpoint")


if __name__ == "__main__":
    default_matter_test_main()
