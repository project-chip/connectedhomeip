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


import matter.clusters as Clusters
import graphviz
from rich.console import Console

from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main  # noqa: E402
from matter.testing.taglist_and_topology_test import build_tree_for_graph

console = None
maxClusterNameLength = 20


def AddServerOrClientNode(graphSection, endpoint, clusterName, color, nodeRef):

    if (len(clusterName) > maxClusterNameLength):
        clusterNameAdjustedLength = clusterName[:maxClusterNameLength] + '...'
    else:
        clusterNameAdjustedLength = clusterName

    graphSection.node(f"ep{endpoint}_{clusterName}", label=f"{clusterNameAdjustedLength}", style="filled,rounded",
                      color=color, shape="box", fixedsize="true", width="2", height="0.5")
    graphSection.edge(nodeRef, f"ep{endpoint}_{clusterName}", style="invis")


def tag_str(tag: Clusters.Descriptor.Structs.SemanticTagStruct):
    # TODO: resolve to names once the namespace stuff lands
    return f"({tag.namespaceID}: {tag.tag}{f' {tag.label}' if tag.label is not None else ''})"


def AddNodeLabel(graphSection, endpoint, wildcardResponse, device_type_xml, aggregator):

    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    listOfDeviceTypes = []
    for deviceTypeStruct in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.DeviceTypeList]:
        try:
            listOfDeviceTypes.append(device_type_xml[deviceTypeStruct.deviceType].name)
        except KeyError:
            listOfDeviceTypes.append(deviceTypeStruct.deviceType)

    try:
        tags = [tag_str(t) for t in wildcardResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList]]
    except KeyError:
        tags = []

    # console.print(f"Endpoint: {endpoint}")
    # console.print(f"DeviceTypeList: {listOfDeviceTypes}")
    # console.print(f"PartsList: {partsListFromWildcardRead}")

    endpointLabel = f"Endpoint: {endpoint}\lDeviceTypeList: {listOfDeviceTypes}\lPartsList: {partsListFromWildcardRead}\lTags: {tags}\l"  # noqa: W605

    if aggregator:
        color = 'grey'
    else:
        color = 'dodgerblue'
    graphSection.node(f"ep{endpoint}", label=endpointLabel, style="filled,rounded",
                      color=color, shape="box", fixedsize="true", width="5", height="1")


def CreateEndpointGraph(graph, graphSection, endpoint, wildcardResponse, device_type_xml):
    AddNodeLabel(graphSection, endpoint, wildcardResponse, device_type_xml, False)
    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    numberOfRowsInEndpoint = 2

    nextNodeRef = ""
    nodeRef = f"ep{endpoint}"
    clusterColumnCount = 0

    for clusterId in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.ServerList]:
        clusterColumnCount += 1

        try:
            clusterName = Clusters.ClusterObjects.ALL_CLUSTERS[clusterId].__name__
        except KeyError:
            clusterName = f"MEI 0x{clusterId:08X}"

        AddServerOrClientNode(graphSection, endpoint, clusterName, "olivedrab", nodeRef)

        if clusterColumnCount == 2:
            nextNodeRef = f"ep{endpoint}_{clusterName}"
        elif clusterColumnCount == 3:
            nodeRef = nextNodeRef
            clusterColumnCount = 0
            numberOfRowsInEndpoint += 1

    for clusterId in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.ClientList]:
        clusterColumnCount += 1

        try:
            clusterName = Clusters.ClusterObjects.ALL_CLUSTERS[clusterId].__name__
        except KeyError:
            clusterName = f"MEI 0x{clusterId:08X}"

        AddServerOrClientNode(graphSection, endpoint, clusterName, "orange", nodeRef)

        if clusterColumnCount == 2:
            nextNodeRef = f"ep{endpoint}_{clusterName}"
        elif clusterColumnCount == 3:
            nodeRef = nextNodeRef
            clusterColumnCount = 0
            numberOfRowsInEndpoint += 1

    if endpoint != 0:
        # Create link to endpoints in the parts list
        for part in partsListFromWildcardRead:
            graph.edge(f"ep{endpoint}", f"ep{part}", ltail=f"cluster_EP{endpoint}",
                       lhead=f'cluster_EP{part}', minlen=f"{numberOfRowsInEndpoint}")


def create_graph(wildcardResponse, xml_device_types, outfile_dir: str = '.'):

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

    def create_subgraph(endpoint: int):
        nonlocal currentAggregator
        if tree[endpoint].is_aggregator_or_root:
            # For aggregators, just open a container
            print(f"adding aggregator on EP {endpoint}")
            with currentAggregator.subgraph(name=f'cluster_EP{endpoint}') as subgraph[endpoint]:
                AddNodeLabel(subgraph[endpoint], endpoint, wildcardResponse, xml_device_types, True)
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
                                    tree[endpoint].endpoint, wildcardResponse, xml_device_types)
                subgraph[endpoint].attr(rank="same", label='')
            for c in tree[endpoint].children:
                create_subgraph(c)

    create_subgraph(0)

    filename = f'{outfile_dir}/matter_device_graph_0x{vid:04X}_0x{pid:04X}_{software_version}.dot'
    deviceGraph.render(filename, format='png')


class TC_MatterDeviceGraph(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper(False)
        self.build_spec_xmls()

    @async_test_body
    async def test_matter_device_graph(self):
        outfile_dir = self.user_params.get("outfile_path", '.')
        create_graph(self.endpoints, self.xml_device_types, outfile_dir)


if __name__ == "__main__":
    default_matter_test_main()
