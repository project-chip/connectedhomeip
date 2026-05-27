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

import logging
from xml.etree import ElementTree

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.device_conformance_tests import DeviceConformanceTests
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import (ClusterSide, ConformanceParseParameters, XmlComposedDeviceTypeRequirement, XmlDeviceType,
                                         XmlDeviceTypeClusterRequirements, parse_callable_from_xml)

log = logging.getLogger(__name__)


def get_mandatory_conformance():
    return parse_callable_from_xml(ElementTree.Element('mandatoryConform'), ConformanceParseParameters({}, {}, {}))


def get_optional_conformance():
    return parse_callable_from_xml(ElementTree.Element('optionalConform'), ConformanceParseParameters({}, {}, {}))


class TestComposedDeviceTypeMatching(DeviceConformanceTests):
    def setup_class(self):
        # We don't load real XMLs here to isolate the tests
        self.xml_device_types = {}
        self.xml_clusters = {}
        self.problems = []

    def _create_mock_composed_req(self, dt_id, name, conformance, min_instances=None, max_instances=None, cluster_requirements=None):
        return XmlComposedDeviceTypeRequirement(
            device_type_id=dt_id,
            device_type_name=name,
            conformance=conformance,
            min_instances=min_instances,
            max_instances=max_instances,
            cluster_requirements=cluster_requirements or {}
        )

    # ==========================================================================
    # Scenario 1: Simple 1-to-1 Match
    # ==========================================================================
    # Requirement: Parent device type requires exactly 1 instance of Child DT.
    # Topology: Endpoint 1 has Parent DT. Endpoint 2 has Child DT listed in PartsList.
    # Expected: PASS.
    # ==========================================================================
    def test_scenario_simple_match(self):
        dt_parent_id = 0x0001
        dt_child_id = 0x0002

        # Mock spec data
        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={},
                composed_device_types=[
                    self._create_mock_composed_req(dt_child_id, "Child Device",
                                                   get_mandatory_conformance(), min_instances=1, max_instances=1)
                ]
            ),
            dt_child_id: XmlDeviceType(
                name="Child Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={}
            )
        }

        # Mock device endpoints
        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: [2]
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [
                        Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)]
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure on simple 1-to-1 match scenario")

    # ==========================================================================
    # Scenario 2: Missing Mandatory Instance
    # ==========================================================================
    # Requirement: Parent device type requires exactly 1 instance of Child DT.
    # Topology: Endpoint 1 has Parent DT. PartsList is empty.
    # Expected: FAIL (Missing mandatory composed device type).
    # ==========================================================================
    def test_scenario_missing_mandatory(self):
        dt_parent_id = 0x0001
        dt_child_id = 0x0002

        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={},
                composed_device_types=[
                    self._create_mock_composed_req(dt_child_id, "Child Device",
                                                   get_mandatory_conformance(), min_instances=1, max_instances=1)
                ]
            ),
            dt_child_id: XmlDeviceType(
                name="Child Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={}
            )
        }

        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        asserts.assert_false(success, "Unexpected success when mandatory composed device type is missing")

    # ==========================================================================
    # Scenario 3: Bipartite Matching (Distinct Overrides)
    # ==========================================================================
    # Requirement: Parent requires TWO instances of Child DT.
    #              Instance #1 requires Cluster X to be present.
    #              Instance #2 requires Cluster Y to be present.
    # Topology: Endpoint 1 has Parent DT. PartsList = [2, 3].
    #           Endpoint 2 has Child DT and Cluster X.
    #           Endpoint 3 has Child DT and Cluster Y.
    # Expected: PASS (The test should find a valid 1-to-1 assignment).
    # ==========================================================================
    def test_scenario_bipartite_matching(self):
        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        cluster_x_id = 0x0090
        cluster_y_id = 0x0091

        # Mock spec data
        req_base = self._create_mock_composed_req(dt_child_id, "Child Device Base", get_mandatory_conformance(), min_instances=2)

        req1 = self._create_mock_composed_req(dt_child_id, "Child Device #1", get_mandatory_conformance())
        req1.cluster_requirements = {
            cluster_x_id: XmlDeviceTypeClusterRequirements(
                name="Cluster X", side=ClusterSide.SERVER, conformance=get_mandatory_conformance())
        }

        req2 = self._create_mock_composed_req(dt_child_id, "Child Device #2", get_mandatory_conformance())
        req2.cluster_requirements = {
            cluster_y_id: XmlDeviceTypeClusterRequirements(
                name="Cluster Y", side=ClusterSide.SERVER, conformance=get_mandatory_conformance())
        }

        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={},
                composed_device_types=[req_base, req1, req2]
            ),
            dt_child_id: XmlDeviceType(
                name="Child Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={}
            )
        }

        # Mock device endpoints
        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: [2, 3]
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                }
            },
            3: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_y_id]
                },
                Clusters.ElectricalEnergyMeasurement: {
                    Clusters.ElectricalEnergyMeasurement.Attributes.FeatureMap: 0x02,
                    Clusters.ElectricalEnergyMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.ClusterRevision: 1
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure on bipartite matching scenario")

    # ==========================================================================
    # Scenario 4: Bipartite Matching Conflict
    # ==========================================================================
    # Requirement: Parent requires TWO instances of Child DT.
    #              Instance #1 requires Cluster X.
    #              Instance #2 requires Cluster Y.
    # Topology: Endpoint 1 has Parent DT. PartsList = [2, 3].
    #           Endpoint 2 has Child DT and Cluster X.
    #           Endpoint 3 has Child DT and Cluster X (Conflict!).
    # Expected: FAIL (Could not find distinct child endpoints satisfying all labeled instances).
    # ==========================================================================
    def test_scenario_bipartite_conflict(self):
        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        cluster_x_id = 0x0090
        cluster_y_id = 0x0091

        # Mock spec data
        req1 = self._create_mock_composed_req(dt_child_id, "Child Device #1", get_mandatory_conformance())
        req1.cluster_requirements = {
            cluster_x_id: XmlDeviceTypeClusterRequirements(
                name="Cluster X", side=ClusterSide.SERVER, conformance=get_mandatory_conformance())
        }

        req2 = self._create_mock_composed_req(dt_child_id, "Child Device #2", get_mandatory_conformance())
        req2.cluster_requirements = {
            cluster_y_id: XmlDeviceTypeClusterRequirements(
                name="Cluster Y", side=ClusterSide.SERVER, conformance=get_mandatory_conformance())
        }

        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={},
                composed_device_types=[req1, req2]
            ),
            dt_child_id: XmlDeviceType(
                name="Child Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="simple",
                classification_scope="endpoint",
                revision_desc={}
            )
        }

        # Mock device endpoints
        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: [2, 3]
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                }
            },
            3: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]  # Conflict!
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        asserts.assert_false(success, "Unexpected success in bipartite conflict scenario")


if __name__ == "__main__":
    default_matter_test_main()
