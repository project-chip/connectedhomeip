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


import re

import graphviz
from rich.console import Console

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.decorators import async_test_body
from matter.testing.runner import default_matter_test_main
from matter.testing.spec_parsing import XmlNamespace, build_xml_namespaces
from matter.testing.taglist_and_topology_test import build_tree_for_graph

console = None


def format_cluster_label(clusterName: str, single_line_max: int = 22) -> str:
    """
    Split a camelCase cluster name into words, wrapping to two lines when long.

    The wrap point is picked near the character-count midpoint so the two lines
    are roughly balanced (e.g. ThermostatUserInterfaceConfiguration becomes
    'Thermostat User Interface' / 'Configuration').
    """
    spaced = re.sub(r"(?<!^)(?=[A-Z])", " ", clusterName)
    if len(spaced) <= single_line_max:
        return spaced

    words = spaced.split(" ")
    if len(words) < 2:
        return spaced

    target = len(spaced) // 2
    acc = 0
    split = 1
    for i, word in enumerate(words):
        acc += len(word) + 1
        if acc >= target:
            split = i + 1
            break
    return " ".join(words[:split]) + r"\n" + " ".join(words[split:])


def AddServerOrClientNode(graphSection, endpoint, clusterName, color, nodeRef, kind):
    # A cluster can appear in both ServerList and ClientList on the same endpoint
    # (e.g. an On/Off Light that also binds to OnOff on another endpoint). Include
    # the kind in the node id so the two declarations don't collide and overwrite
    # each other in graphviz.
    nodeId = f"ep{endpoint}_{kind}_{clusterName}"
    label = format_cluster_label(clusterName)
    graphSection.node(nodeId, label=label, style="filled,rounded",
                      color=color, shape="box", fixedsize="true", width="3.5", height="0.7")
    graphSection.edge(nodeRef, nodeId, style="invis")
    return nodeId


def tag_str(tag: Clusters.Objects.Globals.Structs.SemanticTagStruct,
            xml_namespaces: dict[int, XmlNamespace] | None = None) -> str:
    # Manufacturer-defined namespaces aren't in the spec XMLs, so fall back to hex.
    if tag.mfgCode is not None and tag.mfgCode is not NullValue:
        body = f"Mfg 0x{tag.mfgCode:04X}: 0x{tag.namespaceID:04X}: 0x{tag.tag:04X}"
    else:
        ns = xml_namespaces.get(tag.namespaceID) if xml_namespaces else None
        if ns is None:
            body = f"Unknown Namespace 0x{tag.namespaceID:04X}: 0x{tag.tag:04X}"
        else:
            xml_tag = ns.tags.get(tag.tag)
            if xml_tag is None:
                body = f"{ns.name}: Unknown Tag 0x{tag.tag:04X}"
            else:
                body = f"{ns.name}: {xml_tag.name}"

    if tag.label is not None and tag.label is not NullValue and tag.label != "":
        body = f'{body} "{tag.label}"'
    return f"({body})"


def AddNodeLabel(graphSection, endpoint, wildcardResponse, device_type_xml, aggregator, xml_namespaces=None):

    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    listOfDeviceTypes = []
    for deviceTypeStruct in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.DeviceTypeList]:
        try:
            listOfDeviceTypes.append(device_type_xml[deviceTypeStruct.deviceType].name)
        except KeyError:
            listOfDeviceTypes.append(deviceTypeStruct.deviceType)

    try:
        tags = [tag_str(t, xml_namespaces)
                for t in wildcardResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList]]
    except KeyError:
        tags = []

    if tags:
        tags_section = r"Tags:\l  " + r"\l  ".join(tags)
    else:
        tags_section = "Tags: []"
    endpointLabel = fr"Endpoint: {endpoint}\lDeviceTypeList: {listOfDeviceTypes}\lPartsList: {partsListFromWildcardRead}\l{tags_section}\l"

    if aggregator:
        color = 'grey'
    else:
        color = 'dodgerblue'
    graphSection.node(f"ep{endpoint}", label=endpointLabel, style="filled,rounded",
                      color=color, shape="box", fixedsize="false", width="5", height="1")


def CreateEndpointGraph(graph, graphSection, endpoint, wildcardResponse, device_type_xml, xml_namespaces=None):
    AddNodeLabel(graphSection, endpoint, wildcardResponse, device_type_xml, False, xml_namespaces)
    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    numberOfRowsInEndpoint = 2

    nextNodeRef = ""
    nodeRef = f"ep{endpoint}"
    clusterColumnCount = 0
    lastNodeId = nodeRef

    serverList = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.ServerList]
    clientList = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.ClientList]

    for clusterId in serverList:
        clusterColumnCount += 1

        try:
            clusterName = Clusters.ClusterObjects.ALL_CLUSTERS[clusterId].__name__
        except KeyError:
            clusterName = f"MEI 0x{clusterId:08X}"

        lastNodeId = AddServerOrClientNode(graphSection, endpoint, clusterName, "olivedrab", nodeRef, "srv")

        if clusterColumnCount == 2:
            nextNodeRef = lastNodeId
        elif clusterColumnCount == 3:
            nodeRef = nextNodeRef
            clusterColumnCount = 0
            numberOfRowsInEndpoint += 1

    # Force a new row before client clusters so the green server block and the orange
    # client block don't share a row.
    if serverList and clientList and clusterColumnCount != 0:
        nodeRef = lastNodeId
        clusterColumnCount = 0
        numberOfRowsInEndpoint += 1

    for clusterId in clientList:
        clusterColumnCount += 1

        try:
            clusterName = Clusters.ClusterObjects.ALL_CLUSTERS[clusterId].__name__
        except KeyError:
            clusterName = f"MEI 0x{clusterId:08X}"

        lastNodeId = AddServerOrClientNode(graphSection, endpoint, clusterName, "orange", nodeRef, "cli")

        if clusterColumnCount == 2:
            nextNodeRef = lastNodeId
        elif clusterColumnCount == 3:
            nodeRef = nextNodeRef
            clusterColumnCount = 0
            numberOfRowsInEndpoint += 1

    if endpoint != 0:
        # Create link to endpoints in the parts list
        for part in partsListFromWildcardRead:
            graph.edge(f"ep{endpoint}", f"ep{part}", ltail=f"cluster_EP{endpoint}",
                       lhead=f'cluster_EP{part}', minlen=f"{numberOfRowsInEndpoint}")


def create_graph(wildcardResponse, xml_device_types, xml_namespaces=None, outfile_dir: str = '.'):

    # Create console to print
    global console
    console = Console()

    # Perform wildcard read to get all attributes from device
    console.print("[blue]Capturing data from device")

    vid = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.VendorID]
    pid = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.ProductID]
    software_version = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SoftwareVersion]
    vendor_name = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.VendorName]
    product_name = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.ProductName]
    try:
        spec_version = f'0x{wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SpecificationVersion]:04X}'
    except KeyError:
        # TODO: we can probably guess from the data model version, but I don't care that much right now
        spec_version = "pre-1.3"

    cluster_info_string = f'''
    Vendor: {vendor_name} (0x{vid:04X})
    Product: {product_name} (0x{pid:04X})
    Software version: {software_version}
    Specification version: {spec_version}
    '''

    # Creating graph object
    deviceGraph = graphviz.Digraph()
    deviceGraph.attr(style="rounded", splines="line", compound="true")
    deviceGraph.attr(label=cluster_info_string, labelloc='t')

    console.print("[blue]Generating graph")
    # Loop through each endpoint in the response from the wildcard read
    tree, problems = build_tree_for_graph(wildcardResponse)
    if problems:
        console.print("Found problems parsing topology:")
        for p in problems:
            console.print(p)
            return

    subgraph = {}
    currentAggregator = deviceGraph

    ROOT_NODE_DEVICE_TYPE_ID = 0x0016

    def is_root_node(endpoint: int) -> bool:
        device_types = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.DeviceTypeList]
        return any(dt.deviceType == ROOT_NODE_DEVICE_TYPE_ID for dt in device_types)

    def create_subgraph(endpoint: int):
        nonlocal currentAggregator
        # The Root Node carries the is_aggregator_or_root flag but has no application children
        # of its own, it collects every other endpoint underneath it. Nesting all the
        # children inside a cluster_EP0 subgraph makes the root box visually swallow the rest
        # of the device, so render the root label and recurse children at the top level.
        if is_root_node(endpoint):
            print(f"adding root on EP {endpoint}")
            AddNodeLabel(deviceGraph, endpoint, wildcardResponse, xml_device_types, True, xml_namespaces)
            for c in tree[endpoint].children:
                create_subgraph(c)
            for c in tree[endpoint].children:
                deviceGraph.edge(f"ep{endpoint}", f'ep{c}', style='invis')
        elif tree[endpoint].is_aggregator_or_root:
            print(f"adding aggregator on EP {endpoint}")
            with currentAggregator.subgraph(name=f'cluster_EP{endpoint}') as subgraph[endpoint]:
                AddNodeLabel(subgraph[endpoint], endpoint, wildcardResponse, xml_device_types, True, xml_namespaces)
                subgraph[endpoint].attr(rank="same", label='')

                currentAggregator = subgraph[endpoint]
                for c in tree[endpoint].children:
                    create_subgraph(c)

                # required for ordering these correctly - for some reason graphiz puts these backwards if we don't have
                # explicit edge ordering
                for c in tree[endpoint].children:
                    subgraph[endpoint].edge(f"ep{endpoint}", f'ep{c}', style='invis')
        else:
            print(f"adding tree on EP {endpoint}")
            # normal tree, this will appear in its nearest container, but the children are created external
            with currentAggregator.subgraph(name=f'cluster_EP{endpoint}') as subgraph[endpoint]:
                CreateEndpointGraph(deviceGraph, subgraph[endpoint],
                                    tree[endpoint].endpoint, wildcardResponse, xml_device_types, xml_namespaces)
                subgraph[endpoint].attr(rank="same", label='')
            for c in tree[endpoint].children:
                create_subgraph(c)

    create_subgraph(0)

    filename = f'{outfile_dir}/matter_device_graph_0x{vid:04X}_0x{pid:04X}_{software_version}.dot'
    deviceGraph.render(filename, format='png')


class TC_MatterDeviceGraph(BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper(False)
        self.build_spec_xmls()
        self.xml_namespaces, namespace_problems = build_xml_namespaces(self._get_dm())
        self.problems.extend(namespace_problems)

    @async_test_body
    async def test_matter_device_graph(self):
        outfile_dir = self.user_params.get("outfile_path", '.')
        create_graph(self.endpoints, self.xml_device_types, self.xml_namespaces, outfile_dir)


if __name__ == "__main__":
    default_matter_test_main()
