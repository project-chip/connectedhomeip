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

import os
import pprint
import sys

import chip.clusters as Clusters
import graphviz
from rich.console import Console

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main  # noqa: E402

console = None
maxClusterNameLength = 30


# Given there is currently no tranlation from DeviceTypeID to the device type name,
# this dict is created for now. When some more general is available, it should be updated to use this.
deviceTypeDict = {
    22: "Root Node",
    17: "Power Source",
    18: "OTA Requestor",
    20: "OTA Provider",
    14: "Aggregator",
    19: "Bridged Node",
    256: "On/Off Light",
    257: "Dimmable Light",
    268: "Color Temperature Light",
    269: "Extended Color Light",
    266: "On/Off Plug-in Unit",
    267: "Dimmable Plug-In Unit",
    771: "Pump",
    259: "On/Off Light Switch",
    260: "Dimmer Switch",
    261: "Color Dimmer Switch",
    2112: "Control Bridge",
    772: "Pump Controller",
    15: "Generic Switch",
    21: "Contact Sensor",
    262: "Light Sensor",
    263: "Occupancy Sensor",
    770: "Temperature Sensor",
    773: "Pressure Sensor",
    774: "Flow Sensor",
    775: "Humidity Sensor",
    2128: "On/Off Sensor",
    10: "Door Lock",
    11: "Door Lock Controller",
    514: "Window Covering",
    515: "Window Covering Controller",
    768: "Heating/Cooling Unit",
    769: "Thermostat",
    43: "Fan",
    35: "Casting Video Player",
    34: "Speaker",
    36: "Content App",
    40: "Basic Video Player",
    41: "Casting Video Client",
    42: "Video Remote Control",
    39: "Mode Select",
    45: "Air Purifier",
    44: "Air Quality Sensor",
    112: "Refrigerator",
    113: "Temperature Controlled Cabinet",
    114: "Room Air Conditioner",
    115: "Laundry Washer",
    116: "Robotic Vacuum Cleaner",
    117: "Dishwasher",
    118: "Smoke CO Alarm"
}


def AddServerOrClientNode(graphSection, endpoint, clusterName, color, nodeRef):

    if (len(clusterName) > maxClusterNameLength):
        clusterNameAdjustedLength = clusterName[:maxClusterNameLength] + '...'
    else:
        clusterNameAdjustedLength = clusterName

    graphSection.node(f"ep{endpoint}_{clusterName}", label=f"{clusterNameAdjustedLength}", style="filled,rounded",
                      color=color, shape="box", fixedsize="true", width="3", height="0.5")
    graphSection.edge(nodeRef, f"ep{endpoint}_{clusterName}", style="invis")


def CreateEndpointGraph(graph, graphSection, endpoint, wildcardResponse):

    numberOfRowsInEndpoint = 2

    partsListFromWildcardRead = wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.PartsList]

    listOfDeviceTypes = []
    for deviceTypeStruct in wildcardResponse[endpoint][Clusters.Objects.Descriptor][Clusters.Objects.Descriptor.Attributes.DeviceTypeList]:
        try:
            listOfDeviceTypes.append(deviceTypeDict[deviceTypeStruct.deviceType])
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
            graph.edge(f"ep{endpoint}", f"ep{part}", ltail=f"cluster_{endpoint}", minlen=f"{numberOfRowsInEndpoint}")


class TC_MatterDeviceGraph(MatterBaseTest):
    @async_test_body
    async def test_matter_device_graph(self):

        # Create console to print
        global console
        console = Console()

        # Run descriptor validation test
        dev_ctrl = self.default_controller

        # Perform wildcard read to get all attributes from device
        console.print("[blue]Capturing data from device")
        wildcardResponse = await dev_ctrl.ReadAttribute(self.dut_node_id, [('*')])
        # console.print(wildcardResponse)

        # Creating graph object
        deviceGraph = graphviz.Digraph()
        deviceGraph.attr(style="rounded", splines="line", compound="true")

        console.print("[blue]Generating graph")
        # Loop through each endpoint in the response from the wildcard read
        for endpoint in wildcardResponse:

            if endpoint == 0:
                with deviceGraph.subgraph(name='cluster_rootnode') as rootNodeSection:
                    CreateEndpointGraph(deviceGraph, rootNodeSection, endpoint, wildcardResponse)
            else:
                with deviceGraph.subgraph(name='cluster_endpoints') as endpointsSection:
                    with endpointsSection.subgraph(name=f'cluster_{endpoint}') as endpointSection:
                        CreateEndpointGraph(deviceGraph, endpointSection, endpoint, wildcardResponse)

        deviceGraph.save(f'{sys.path[0]}/matter-device-graph.dot')

        deviceDataFile = open(f'{sys.path[0]}/matter-device-data.txt', 'w')
        deviceDataFile.write(pprint.pformat((wildcardResponse)))
        deviceDataFile.close()


if __name__ == "__main__":
    default_matter_test_main()
