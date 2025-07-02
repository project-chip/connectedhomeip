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
from typing import Any, Optional

import chip.clusters as Clusters
from chip.testing.conformance import ConformanceDecision
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.spec_parsing import XmlCluster, XmlDeviceType
from chip.tlv import uint


def _is_mandatory(conformance, feature_map=0):
    return conformance(feature_map, [], []).decision == ConformanceDecision.MANDATORY


def _get_field_by_label(cl_object: Clusters.ClusterObjects.ClusterObject, label: str) -> Optional[Clusters.ClusterObjects.ClusterObjectFieldDescriptor]:
    for field in cl_object.descriptor.Fields:
        if field.Label == label:
            return field
    return None


def create_minimal_cluster(xml_clusters: dict[uint, XmlCluster], cluster_id: int) -> dict[int, Any]:
    attrs = {}
    mandatory_features = [mask for mask, f in xml_clusters[cluster_id].features.items() if _is_mandatory(f.conformance)]
    feature_map = 0
    for mask in mandatory_features:
        feature_map |= mask

    mandatory_attributes = [id for id, a in xml_clusters[cluster_id].attributes.items()
                            if _is_mandatory(a.conformance, feature_map)]
    for m in mandatory_attributes:
        # dummy versions - we're not using the values in this test
        attrs[m] = 0
    attrs[GlobalAttributeIds.FEATURE_MAP_ID] = feature_map
    attrs[GlobalAttributeIds.ATTRIBUTE_LIST_ID] = mandatory_attributes
    mandatory_accepted_commands = [id for id, a in xml_clusters[cluster_id].accepted_commands.items()
                                   if _is_mandatory(a.conformance, feature_map)]
    attrs[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] = mandatory_accepted_commands
    mandatory_generated_commands = [id for id, a in xml_clusters[cluster_id].generated_commands.items()
                                    if _is_mandatory(a.conformance, feature_map)]
    attrs[GlobalAttributeIds.GENERATED_COMMAND_LIST_ID] = mandatory_generated_commands
    attrs[GlobalAttributeIds.CLUSTER_REVISION_ID] = xml_clusters[cluster_id].revision
    return attrs


def create_minimal_dt(xml_clusters: dict[uint, XmlCluster], xml_device_types: dict[uint, XmlDeviceType], device_type_id: int, is_tlv_endpoint: bool = True) -> dict[int, dict[int, Any]]:
    ''' Creates the internals of an endpoint with the minimal set of clusters, with the minimal set of attributes and commands. Global attributes only.
        Does NOT take into account overrides yet.
    '''
    endpoint = {}
    required_servers = [id for id, c in xml_device_types[device_type_id].server_clusters.items()
                        if _is_mandatory(c.conformance)]
    required_clients = [id for id, c in xml_device_types[device_type_id].client_clusters.items()
                        if _is_mandatory(c.conformance)]
    device_type_revision = xml_device_types[device_type_id].revision

    for s in required_servers:
        endpoint[s] = create_minimal_cluster(xml_clusters, s)

    # Descriptor
    attr = Clusters.Descriptor.Attributes
    structs = Clusters.Descriptor.Structs
    attrs = {}

    attributes = [
        attr.FeatureMap,
        attr.AcceptedCommandList,
        attr.GeneratedCommandList,
        attr.ClusterRevision,
        attr.DeviceTypeList,
        attr.ServerList,
        attr.ClientList,
        attr.PartsList,
    ]

    attribute_values = [
        (0, 0),  # FeatureMap
        ([], []),  # AcceptedCommandList
        ([], []),  # GeneratedCommandList
        (xml_clusters[Clusters.Descriptor.id].revision,
            xml_clusters[Clusters.Descriptor.id].revision),  # ClusterRevision
        ([{_get_field_by_label(structs.DeviceTypeStruct, "deviceType").Tag: device_type_id,
            _get_field_by_label(structs.DeviceTypeStruct, "revision").Tag: device_type_revision}],
            [Clusters.Descriptor.Structs.DeviceTypeStruct(
                deviceType=device_type_id, revision=device_type_revision)]),  # DeviceTypeList
        (required_servers, required_servers),  # ServerList
        (required_clients, required_clients),  # ClientList
        ([], []),  # PartsList
    ]

    for attribute_name, attribute_value in zip(attributes, attribute_values):
        key = attribute_name.attribute_id if is_tlv_endpoint else attribute_name
        attrs[key] = attribute_value[0] if is_tlv_endpoint else attribute_value[1]

    # Append the attribute list now that is populated.
    attrs[attr.AttributeList.attribute_id if is_tlv_endpoint else attr.AttributeList] = list(attrs.keys())

    endpoint[Clusters.Descriptor.id if is_tlv_endpoint else Clusters.Descriptor] = attrs

    return endpoint
