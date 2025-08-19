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
maxClusterNameLength = 30


def AddServerOrClientNode(graphSection, endpoint, clusterName, color, nodeRef):

    if (len(clusterName) > maxClusterNameLength):
        clusterNameAdjustedLength = clusterName[:maxClusterNameLength] + '...'
    else:
        clusterNameAdjustedLength = clusterName

    graphSection.node(f"ep{endpoint}_{clusterName}", label=f"{clusterNameAdjustedLength}", style="filled,rounded",
                      color=color, shape="box", fixedsize="true", width="3", height="0.5")
    graphSection.edge(nodeRef, f"ep{endpoint}_{clusterName}", style="invis")


def CreateEndpointGraph(graph, graphSection, endpoint, wildcardResponse, device_type_xml):

    numberOfRowsInEndpoint = 2

    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    listOfDeviceTypes = []
    for deviceTypeStruct in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.DeviceTypeList]:
        try:
            listOfDeviceTypes.append(device_type_xml[deviceTypeStruct.deviceType].name)
        except KeyError:
            listOfDeviceTypes.append(deviceTypeStruct.deviceType)

    # console.print(f"Endpoint: {endpoint}")
    # console.print(f"DeviceTypeList: {listOfDeviceTypes}")
    # console.print(f"PartsList: {partsListFromWildcardRead}")

    endpointLabel = f"Endpoint: {endpoint}\lDeviceTypeList: {listOfDeviceTypes}\lPartsList: {partsListFromWildcardRead}\l"  # noqa: W605

    nextNodeRef = ""
    nodeRef = f"ep{endpoint}"
    clusterColumnCount = 0

    graphSection.node(f"ep{endpoint}", label=endpointLabel, style="filled,rounded",
                      color="dodgerblue", shape="box", fixedsize="true", width="4", height="1")

    for clusterId in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.ServerList]:
        clusterColumnCount += 1

        try:
            clusterName = Clusters.ClusterObjects.ALL_CLUSTERS[clusterId].__name__
        except KeyError:
            clusterName = f"Custom server\l0x{clusterId:08X}"  # noqa: W605

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
            clusterName = f"Custom client\l0x{clusterId:08X}"  # noqa: W605

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


def create_graph(wildcardResponse, xml_device_types):

    # Create console to print
    global console
    console = Console()

    # Perform wildcard read to get all attributes from device
    console.print("[blue]Capturing data from device")

    # Creating graph object
    deviceGraph = graphviz.Digraph()
    deviceGraph.attr(style="rounded", splines="line", compound="true")

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
                currentAggregator = subgraph[endpoint]
                for c in tree[endpoint].children:
                    create_subgraph(c)
        else:
            print(f"adding tree on EP {endpoint}")
            # normal tree, this will appear in its nearest container, but the children are created external
            with currentAggregator.subgraph(name=f'cluster_EP{endpoint}') as subgraph[endpoint]:
                CreateEndpointGraph(deviceGraph, subgraph[endpoint],
                                    tree[endpoint].endpoint, wildcardResponse, xml_device_types)
            for c in tree[endpoint].children:
                create_subgraph(c)

    create_subgraph(0)

    vid = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.VendorID]
    pid = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.ProductID]
    software_version = wildcardResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SoftwareVersion]
    deviceGraph.save(f'matter_device_graph_0x{vid:04X}_0x{pid:04X}_{software_version}.dot')


class TC_MatterDeviceGraph(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper(False)
        self.build_spec_xmls()

    @async_test_body
    async def test_matter_device_graph(self):
        create_graph(self.endpoints, self.xml_device_types)


if __name__ == "__main__":
    default_matter_test_main()
