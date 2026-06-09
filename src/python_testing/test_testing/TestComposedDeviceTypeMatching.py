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

    def _create_mock_composed_req(self, dt_id, name, conformance, min_instances=None, max_instances=None, cluster_requirements=None, device_type_location='childEndpoint'):
        return XmlComposedDeviceTypeRequirement(
            device_type_id=dt_id,
            device_type_name=name,
            conformance=conformance,
            min_instances=min_instances,
            max_instances=max_instances,
            device_type_location=device_type_location,
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
    # Scenario 3b: Bipartite Matching (Swapped Requirements)
    # ==========================================================================
    # Requirement: Parent requires TWO instances of Child DT.
    #              Instance #1 requires Cluster Y to be present.
    #              Instance #2 requires Cluster X to be present.
    #              (Swapped order in composed_device_types list compared to Scenario 3)
    # Topology: Endpoint 1 has Parent DT. PartsList = [2, 3].
    #           Endpoint 2 has Child DT and Cluster X.
    #           Endpoint 3 has Child DT and Cluster Y.
    # Expected: PASS (The test should find a valid 1-to-1 assignment regardless of order).
    # ==========================================================================
    def test_scenario_bipartite_matching_swapped(self):
        log.info("Running Scenario 3b: Bipartite Matching (Swapped Requirements)")
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
                composed_device_types=[req_base, req2, req1]  # Swapped!
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
        asserts.assert_true(success, "Failure on bipartite matching swapped scenario")

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

    # ==========================================================================
    # Scenario 5: Anywhere Location Match
    # ==========================================================================

    def test_scenario_anywhere_location(self):
        log.info("Running Scenario 5: Anywhere Location Match")

        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        dt_power_source_id = 0x0011

        # Mock spec
        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="Simple",
                classification_scope="Endpoint",
                revision_desc={},
                composed_device_types=[
                    self._create_mock_composed_req(dt_child_id, "Child Device", get_mandatory_conformance()),
                    self._create_mock_composed_req(dt_power_source_id, "Power Source",
                                                   get_mandatory_conformance(), device_type_location='anyEndpoint')
                ]
            ),
            dt_child_id: XmlDeviceType(name="Child Device", revision=1, server_clusters={}, client_clusters={}, classification_class="Simple", classification_scope="Endpoint", revision_desc={}),
            dt_power_source_id: XmlDeviceType(name="Power Source", revision=1, server_clusters={}, client_clusters={
            }, classification_class="Utility", classification_scope="Endpoint", revision_desc={})
        }

        # Mock endpoints
        # EP1: Parent Device
        # EP2: Child Device (direct child of EP1)
        # EP3: Power Source (NOT in EP1 parts list!)

        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: [2]  # Only EP2 is a child!
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            },
            3: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_power_source_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure in anywhere location scenario")

    # ==========================================================================
    # Scenario 6: Self Location Match
    # ==========================================================================

    def test_scenario_self_location(self):
        log.info("Running Scenario 6: Self Location Match")

        dt_parent_id = 0x0001
        dt_power_source_id = 0x0011

        # Mock spec
        self.xml_device_types = {
            dt_parent_id: XmlDeviceType(
                name="Parent Device",
                revision=1,
                server_clusters={},
                client_clusters={},
                classification_class="Simple",
                classification_scope="Endpoint",
                revision_desc={},
                composed_device_types=[
                    self._create_mock_composed_req(dt_power_source_id, "Power Source",
                                                   get_mandatory_conformance(), device_type_location='deviceEndpoint')
                ]
            ),
            dt_power_source_id: XmlDeviceType(name="Power Source", revision=1, server_clusters={}, client_clusters={
            }, classification_class="Utility", classification_scope="Endpoint", revision_desc={})
        }

        # Mock endpoints
        # EP1: Parent Device AND Power Source!

        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [
                        Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1),
                        Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_power_source_id, revision=1)
                    ],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure in self location scenario (positive case)")

        # Now make it fail by removing Power Source from EP1 and adding to EP2 (child)
        self.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList] = [
            Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)
        ]
        self.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList] = [2]

        self.endpoints[2] = {
            Clusters.Descriptor: {
                Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_power_source_id, revision=1)],
                Clusters.Descriptor.Attributes.PartsList: []
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        asserts.assert_false(success, "Unexpected success in self location scenario (negative case)")

    # ==========================================================================
    # Scenario 7: Scattered Matching (Cyclic)
    # ==========================================================================
    # Requirement: Parent requires THREE instances of Child DT.
    #              Instance #1 requires Cluster X.
    #              Instance #2 requires Cluster Y.
    #              Instance #3 requires Cluster Z.
    # Topology: Endpoint 1 has Parent DT. PartsList = [2, 3, 4, 5, 6].
    #           Endpoint 2 has Child DT and Clusters X, Y.
    #           Endpoint 3 has Child DT and Clusters Y, Z.
    #           Endpoint 4 has Child DT and Clusters Z, X.
    #           Endpoint 5 has Child DT (Base).
    #           Endpoint 6 has Child DT (Base).
    # Expected: PASS (The test should find a valid assignment resolving overlapping capabilities).
    # ==========================================================================

    def test_scenario_scattered_matching(self):
        log.info("Running Scenario 7: Scattered Matching (Cyclic)")

        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        cluster_x_id = 0x0090
        cluster_y_id = 0x0091
        cluster_z_id = 0x0201  # Thermostat

        # Mock spec data
        req_base = self._create_mock_composed_req(dt_child_id, "Child Device Base", get_mandatory_conformance(), min_instances=3)

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

        req3 = self._create_mock_composed_req(dt_child_id, "Child Device #3", get_mandatory_conformance())
        req3.cluster_requirements = {
            cluster_z_id: XmlDeviceTypeClusterRequirements(
                name="Cluster Z", side=ClusterSide.SERVER, conformance=get_mandatory_conformance())
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
                composed_device_types=[req_base, req1, req2, req3]
            ),
            dt_child_id: XmlDeviceType(name="Child Device", revision=1, server_clusters={}, client_clusters={},
                                       classification_class="simple", classification_scope="endpoint", revision_desc={})
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
                    Clusters.Descriptor.Attributes.PartsList: [2, 3, 4, 5, 6]
                }
            },
            2: {  # Has X and Y
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id, cluster_y_id]
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                },
                Clusters.ElectricalEnergyMeasurement: {
                    Clusters.ElectricalEnergyMeasurement.Attributes.FeatureMap: 0x02,
                    Clusters.ElectricalEnergyMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.ClusterRevision: 1
                }
            },
            3: {  # Has Y and Z
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_y_id, cluster_z_id]
                },
                Clusters.ElectricalEnergyMeasurement: {
                    Clusters.ElectricalEnergyMeasurement.Attributes.FeatureMap: 0x02,
                    Clusters.ElectricalEnergyMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalEnergyMeasurement.Attributes.ClusterRevision: 1
                },
                Clusters.Thermostat: {
                    Clusters.Thermostat.Attributes.FeatureMap: 0x03,
                    Clusters.Thermostat.Attributes.AttributeList: [],
                    Clusters.Thermostat.Attributes.AcceptedCommandList: [],
                    Clusters.Thermostat.Attributes.ClusterRevision: 1
                }
            },
            4: {  # Has Z and X
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_z_id, cluster_x_id]
                },
                Clusters.Thermostat: {
                    Clusters.Thermostat.Attributes.FeatureMap: 0x03,
                    Clusters.Thermostat.Attributes.AttributeList: [],
                    Clusters.Thermostat.Attributes.AcceptedCommandList: [],
                    Clusters.Thermostat.Attributes.ClusterRevision: 1
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                }
            },
            5: {  # Base
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: []
                }
            },
            6: {  # Base
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: []
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure on scattered matching scenario")

    # ==========================================================================
    # Scenario 8: Complex Location Matching
    # ==========================================================================
    # Requirement: Parent requires:
    #              - 1 instance of Child DT on childEndpoint (direct child).
    #              - 1 instance of Power Source on anywhere (any endpoint).
    #              - 1 instance of Thermostat on deviceEndpoint (same endpoint).
    # Topology (Positive):
    #           Endpoint 1 has Parent DT and Thermostat. PartsList = [2].
    #           Endpoint 2 has Child DT.
    #           Endpoint 3 has Power Source (Standalone).
    # Expected: PASS
    # Topology (Negative):
    #           Move Thermostat to Endpoint 2 (Child).
    # Expected: FAIL (Thermostat is not on Self).
    # ==========================================================================

    def test_scenario_complex_location_matching(self):
        log.info("Running Scenario 8: Complex Location Matching")

        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        dt_power_source_id = 0x0011
        dt_thermostat_id = 0x0301

        # Mock spec
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
                    self._create_mock_composed_req(dt_child_id, "Child Device", get_mandatory_conformance()),
                    self._create_mock_composed_req(dt_power_source_id, "Power Source",
                                                   get_mandatory_conformance(), device_type_location='anyEndpoint'),
                    self._create_mock_composed_req(dt_thermostat_id, "Thermostat",
                                                   get_mandatory_conformance(), device_type_location='deviceEndpoint')
                ]
            ),
            dt_child_id: XmlDeviceType(name="Child Device", revision=1, server_clusters={}, client_clusters={}, classification_class="simple", classification_scope="endpoint", revision_desc={}),
            dt_power_source_id: XmlDeviceType(name="Power Source", revision=1, server_clusters={}, client_clusters={}, classification_class="Utility", classification_scope="endpoint", revision_desc={}),
            dt_thermostat_id: XmlDeviceType(name="Thermostat", revision=1, server_clusters={}, client_clusters={
            }, classification_class="Simple", classification_scope="endpoint", revision_desc={})
        }

        # Mock endpoints
        # EP1: Parent Device AND Thermostat!
        # EP2: Child Device (direct child of EP1)
        # EP3: Power Source (Standalone)

        self.endpoints = {
            0: {
                Clusters.BasicInformation: {
                    Clusters.BasicInformation.Attributes.SpecificationVersion: 0x01060000
                }
            },
            1: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [
                        Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1),
                        Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_thermostat_id, revision=1)
                    ],
                    Clusters.Descriptor.Attributes.PartsList: [2]
                }
            },
            2: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            },
            3: {
                Clusters.Descriptor: {
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_power_source_id, revision=1)],
                    Clusters.Descriptor.Attributes.PartsList: []
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure in complex location scenario (positive case)")

        # Break it by moving Thermostat to EP2
        self.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList] = [
            Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_parent_id, revision=1)
        ]
        self.endpoints[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList] = [
            Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1),
            Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_thermostat_id, revision=1)
        ]

        success, problems = self.check_composed_device_type_requirements()
        asserts.assert_false(success, "Unexpected success in complex location scenario (negative case)")

    # ==========================================================================
    # Scenario 9: Element Overrides (Feature)
    # ==========================================================================
    # Requirement: Parent requires 1 instance of Child DT.
    #              That instance requires Feature bit 0 to be enabled on Cluster X.
    # Topology (Negative):
    #           Endpoint 1 has Parent DT. PartsList = [2].
    #           Endpoint 2 has Child DT and Cluster X (FeatureMap = 0).
    # Expected: FAIL (Feature requirement not met).
    # Topology (Positive):
    #           Endpoint 1 has Parent DT. PartsList = [3].
    #           Endpoint 3 has Child DT and Cluster X (FeatureMap = 1).
    # Expected: PASS
    # ==========================================================================

    def test_scenario_element_overrides(self):
        log.info("Running Scenario 9: Element Overrides (Feature)")

        dt_parent_id = 0x0001
        dt_child_id = 0x0002
        cluster_x_id = 0x0090  # ElectricalPowerMeasurement

        # Mock spec data
        req = self._create_mock_composed_req(dt_child_id, "Child Device", get_mandatory_conformance())
        req.cluster_requirements = {
            cluster_x_id: XmlDeviceTypeClusterRequirements(
                name="Cluster X",
                side=ClusterSide.SERVER,
                conformance=get_mandatory_conformance(),
                feature_overrides={0x01: get_mandatory_conformance()}  # Require bit 0!
            )
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
                composed_device_types=[req]
            ),
            dt_child_id: XmlDeviceType(name="Child Device", revision=1, server_clusters={}, client_clusters={},
                                       classification_class="simple", classification_scope="endpoint", revision_desc={})
        }

        # Mock device endpoints
        # EP1: Parent Device
        # EP2: Child Device (has cluster X, but FeatureMap = 0!) -> Should fail!

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
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                    Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]
                },
                Clusters.ElectricalPowerMeasurement: {
                    Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x00,  # No features!
                    Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                    Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
                }
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        asserts.assert_false(success, "Unexpected success when feature override is not met")

        # Now add EP3 which meets the requirement
        self.endpoints[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList] = [3]
        self.endpoints[3] = {
            Clusters.Descriptor: {
                Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=dt_child_id, revision=1)],
                Clusters.Descriptor.Attributes.ServerList: [cluster_x_id]
            },
            Clusters.ElectricalPowerMeasurement: {
                Clusters.ElectricalPowerMeasurement.Attributes.FeatureMap: 0x01,  # Has feature bit 0!
                Clusters.ElectricalPowerMeasurement.Attributes.AttributeList: [],
                Clusters.ElectricalPowerMeasurement.Attributes.AcceptedCommandList: [],
                Clusters.ElectricalPowerMeasurement.Attributes.ClusterRevision: 1
            }
        }

        success, problems = self.check_composed_device_type_requirements()
        for p in problems:
            log.info(p)
        asserts.assert_true(success, "Failure when feature override is met")


if __name__ == "__main__":
    default_matter_test_main()
