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

import argparse
import json
import os
import pathlib
import sys
import xml.etree.ElementTree as ET

import chip.clusters as Clusters
from rich.console import Console

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main  # noqa: E402

console = None


def GenerateDevicePicsXmlFiles(clusterName, clusterPicsCode, featurePicsList, attributePicsList, acceptedCommandPicsList, generatedCommandPicsList, outputPathStr):

    xmlPath = xmlTemplatePathStr
    fileName = ""

    print(f"Handling PICS for {clusterName}")

    # Map clusters to common XML template if needed
    otaProviderCluster = "OTA Software Update Provider Cluster"
    otaRequestorCluster = "OTA Software Update Requestor Cluster"
    onOffCluster = "On/Off Cluster"
    groupKeyManagementCluster = "Group Key Management Cluster"
    nodeOperationalCredentialsCluster = "Node Operational Credentials Cluster"
    basicInformationCluster = "Basic Information Cluster"
    networkCommissioningCluster = "Network Commissioning Cluster"

    if otaProviderCluster in clusterName or otaRequestorCluster in clusterName:
        clusterName = "OTA Software Update"

    elif onOffCluster == clusterName:
        clusterName = clusterName.replace("/", "-")

    elif groupKeyManagementCluster == clusterName:
        clusterName = "Group Communication"

    elif nodeOperationalCredentialsCluster == clusterName or basicInformationCluster == clusterName or networkCommissioningCluster == clusterName:
        clusterName = clusterName.replace("Cluster", "").strip()

    # Determine if file has already been handled and use this file
    for outputFolderFileName in os.listdir(outputPathStr):
        if clusterName in outputFolderFileName:
            xmlPath = outputPathStr
            fileName = outputFolderFileName
            break

    # If no file is found in output folder, determine if there is a match for the cluster name in input folder
    if fileName == "":
        for file in xmlFileList:
            if file.lower().startswith(clusterName.lower()):
                fileName = file
                break
        else:
            console.print(f"[red]Could not find matching file for \"{clusterName}\" ❌")
            return

    try:
        # Open the XML PICS template file
        console.print(f"Open \"{xmlPath}{fileName}\"")
        parser = ET.XMLParser(target=ET.TreeBuilder(insert_comments=True))
        tree = ET.parse(f"{xmlPath}{fileName}", parser)
        root = tree.getroot()
    except ET.ParseError:
        console.print(f"[red]Could not find \"{fileName}\" ❌")
        return

    # Usage PICS
    # console.print(clusterPicsCode)
    usageNode = root.find('usage')
    for picsItem in usageNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if itemNumberElement.text == f"{clusterPicsCode}":
            console.print("Found usage PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            # console.print(f"Support: {supportElement.text}")
            supportElement.text = "true"

            # Since usage PICS (server or client) is not a list, we can break out when a match is found,
            # no reason to keep iterating through the elements.
            break

    # Feature PICS
    # console.print(featurePicsList)
    featureNode = root.find("./clusterSide[@type='Server']/features")
    for picsItem in featureNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if f"{itemNumberElement.text}" in featurePicsList:
            console.print("Found feature PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            supportElement.text = "true"

    # Attributes PICS
    # TODO: Only check if list is not empty
    # console.print(attributePicsList)
    serverAttributesNode = root.find("./clusterSide[@type='Server']/attributes")
    for picsItem in serverAttributesNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if f"{itemNumberElement.text}" in attributePicsList:
            console.print("Found attribute PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            supportElement.text = "true"

    # AcceptedCommandList PICS
    # TODO: Only check if list is not empty
    # console.print(acceptedCommandPicsList)
    serverCommandsReceivedNode = root.find("./clusterSide[@type='Server']/commandsReceived")
    for picsItem in serverCommandsReceivedNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if f"{itemNumberElement.text}" in acceptedCommandPicsList:
            console.print("Found acceptedCommand PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            supportElement.text = "true"

    # GeneratedCommandList PICS
    # console.print(generatedCommandPicsList)
    # TODO: Only check if list is not empty
    serverCommandsGeneratedNode = root.find("./clusterSide[@type='Server']/commandsGenerated")
    for picsItem in serverCommandsGeneratedNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if f"{itemNumberElement.text}" in generatedCommandPicsList:
            console.print("Found generatedCommand PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            supportElement.text = "true"

    # Event PICS (Work in progress)
    # The ability to set event PICS is fairly limited, due to EventList not being supported,
    # as well as no way to check for event support in the current Matter SDK.

    # This implementation marks an event as supported if:
    # 1) Event is mandatody
    # 2) The event is mandatory based on a feature that is supported (Cross check against feature list) (Not supported yet)
    serverEventsNode = root.find("./clusterSide[@type='Server']/events")
    for picsItem in serverEventsNode:
        itemNumberElement = picsItem.find('itemNumber')
        statusElement = picsItem.find('status')

        try:
            condition = statusElement.attrib['cond']
            console.print(f"Checking {itemNumberElement.text} with conformance {statusElement.text} and condition {condition}")
        except ET.ParseError:
            condition = ""
            console.print(f"Checking {itemNumberElement.text} with conformance {statusElement.text}")

        if statusElement.text == "M":

            # Is event mandated by the server
            if condition == clusterPicsCode:
                console.print("Found event mandated by server ✅")
                supportElement = picsItem.find('support')
                supportElement.text = "true"
                continue

            if condition in featurePicsList:
                console.print("Found event mandated by feature ✅")
                supportElement = picsItem.find('support')
                supportElement.text = "true"
                continue

            if condition == "":
                console.print("Event is mandated without a condition ✅")
                continue

    # Grabbing the header from the XML templates
    inputFile = open(f"{xmlPath}{fileName}", "r")
    outputFile = open(f"{outputPathStr}/{fileName}", "ab")
    xmlHeader = ""
    inputLine = inputFile.readline().lstrip()

    while 'clusterPICS' not in inputLine:
        xmlHeader += inputLine
        inputLine = inputFile.readline().lstrip()

    # Write the PICS XML header
    outputFile.write(xmlHeader.encode())

    # Write the PICS XML data
    tree.write(outputFile)


async def DeviceMapping(devCtrl, nodeID, outputPathStr):

    # --- Device mapping --- #
    console.print("[blue]Perform device mapping")
    # Determine how many endpoints to map
    # Test step 1 - Read parts list

    partsListResponse = await devCtrl.ReadAttribute(nodeID, [(rootNodeEndpointID, Clusters.Descriptor.Attributes.PartsList)])
    partsList = partsListResponse[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]

    # Add endpoint 0 to the parts list, since this is not returned by the device
    partsList.insert(0, 0)
    # console.print(partsList)

    for endpoint in partsList:
        # Test step 2 - Map each available endpoint
        console.print(f"Mapping endpoint: {endpoint}")

        # Create endpoint specific output folder and register this as output folder
        endpointOutputPathStr = f"{outputPathStr}endpoint{endpoint}/"
        endpointOutputPath = pathlib.Path(endpointOutputPathStr)
        if not endpointOutputPath.exists():
            endpointOutputPath.mkdir()

        # Read device list (Not required)
        deviceListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, Clusters.Descriptor.Attributes.DeviceTypeList)])

        for deviceTypeData in deviceListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]:
            console.print(f"Device Type: {deviceTypeData.deviceType}")

        # Read server list
        serverListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, Clusters.Descriptor.Attributes.ServerList)])
        serverList = serverListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]

        for server in serverList:
            featurePicsList = []
            attributePicsList = []
            acceptedCommandListPicsList = []
            generatedCommandListPicsList = []

            clusterClass = getattr(Clusters, devCtrl.GetClusterHandler().GetClusterInfoById(server)['clusterName'])
            clusterID = f"0x{server:04x}"

            # Does the clusterInfoDict contain the found cluster ID?
            if clusterID not in clusterInfoDict:
                console.print(f"[red]Cluster ID ({clusterID}) not in list! ❌")
                continue

            clusterName = clusterInfoDict[clusterID]['Name']
            clusterPICS = f"{clusterInfoDict[clusterID]['PICS_Code']}{serverTag}"

            console.print(f"{clusterName} - {clusterPICS}")

            # Print PICS for specific server from dict
            # console.print(clusterInfoDict[f"0x{server:04x}"])

            # Read feature map
            featureMapResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, clusterClass.Attributes.FeatureMap)])
            # console.print(f"FeatureMap: {featureMapResponse[endpoint][clusterClass][clusterClass.Attributes.FeatureMap]}")

            featureMapValue = featureMapResponse[endpoint][clusterClass][clusterClass.Attributes.FeatureMap]
            featureMapBitString = "{:08b}".format(featureMapValue).lstrip("0")
            for bitLocation in range(len(featureMapBitString)):
                if featureMapValue >> bitLocation & 1 == 1:
                    # console.print(f"{clusterPICS}{featureTag}{bitLocation:02x}")
                    featurePicsList.append(f"{clusterPICS}{featureTag}{bitLocation:02x}")

            console.print("Collected feature PICS:")
            console.print(featurePicsList)

            # Read attribute list
            attributeListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, clusterClass.Attributes.AttributeList)])
            attributeList = attributeListResponse[endpoint][clusterClass][clusterClass.Attributes.AttributeList]
            # console.print(f"AttributeList: {attributeList}")

            # Convert attribute to PICS code
            for attribute in attributeList:
                if (attribute != 0xfff8 and attribute != 0xfff9 and attribute != 0xfffa and attribute != 0xfffb and attribute != 0xfffc and attribute != 0xfffd):
                    # console.print(f"{clusterPICS}{attributeTag}{attribute:04x}")
                    attributePicsList.append(f"{clusterPICS}{attributeTag}{attribute:04x}")
                '''
                else:
                    console.print(f"[yellow]Ignore global attribute 0x{attribute:04x}")
                '''

            console.print("Collected attribute PICS:")
            console.print(attributePicsList)

            # Read AcceptedCommandList
            acceptedCommandListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, clusterClass.Attributes.AcceptedCommandList)])
            acceptedCommandList = acceptedCommandListResponse[endpoint][clusterClass][clusterClass.Attributes.AcceptedCommandList]
            # console.print(f"AcceptedCommandList: {acceptedCommandList}")

            # Convert accepted command to PICS code
            for acceptedCommand in acceptedCommandList:
                # console.print(f"{clusterPICS}{commandTag}{acceptedCommand:02x}{acceptedCommandTag}")
                acceptedCommandListPicsList.append(f"{clusterPICS}{commandTag}{acceptedCommand:02x}{acceptedCommandTag}")

            console.print("Collected accepted command PICS:")
            console.print(acceptedCommandListPicsList)

            # Read GeneratedCommandList
            generatedCommandListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, clusterClass.Attributes.GeneratedCommandList)])
            generatedCommandList = generatedCommandListResponse[endpoint][clusterClass][clusterClass.Attributes.GeneratedCommandList]
            # console.print(f"GeneratedCommandList: {generatedCommandList}")

            # Convert accepted command to PICS code
            for generatedCommand in generatedCommandList:
                # console.print(f"{clusterPICS}{commandTag}{generatedCommand:02x}{generatedCommandTag}")
                generatedCommandListPicsList.append(f"{clusterPICS}{commandTag}{generatedCommand:02x}{generatedCommandTag}")

            console.print("Collected generated command PICS:")
            console.print(generatedCommandListPicsList)

            # Write the collected PICS to a PICS XML file
            GenerateDevicePicsXmlFiles(clusterName, clusterPICS, featurePicsList, attributePicsList,
                                       acceptedCommandListPicsList, generatedCommandListPicsList, endpointOutputPathStr)

        # Read client list
        clientListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, Clusters.Descriptor.Attributes.ClientList)])
        clientList = clientListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ClientList]

        for client in clientList:
            clusterID = f"0x{client:04x}"
            clusterName = clusterInfoDict[clusterID]['Name']
            clusterPICS = f"{clusterInfoDict[clusterID]['PICS_Code']}{clientTag}"

            console.print(f"{clusterName} - {clusterPICS}")

            GenerateDevicePicsXmlFiles(clusterName, clusterPICS, [], [], [], [], endpointOutputPathStr)


def cleanDirectory(pathToClean):
    for entry in pathToClean.iterdir():
        if entry.is_file():
            pathlib.Path(entry).unlink()
        elif entry.is_dir():
            cleanDirectory(entry)
            pathlib.Path(entry).rmdir()


parser = argparse.ArgumentParser()
parser.add_argument('--cluster-data', required=True)
parser.add_argument('--pics-template', required=True)
parser.add_argument('--pics-output', required=True)
args, unknown = parser.parse_known_args()

basePath = os.path.dirname(__file__)
clusterInfoInputPathStr = args.cluster_data

xmlTemplatePathStr = args.pics_template
if not xmlTemplatePathStr.endswith('/'):
    xmlTemplatePathStr += '/'

baseOutputPathStr = args.pics_output
if not baseOutputPathStr.endswith('/'):
    baseOutputPathStr += '/'

serverTag = ".S"
clientTag = ".C"
featureTag = ".F"
attributeTag = ".A"
commandTag = ".C"
acceptedCommandTag = ".Rsp"
generatedCommandTag = ".Tx"

# List of globale attributes (server)
# Does not read ClusterRevision [0xFFFD] (not relevant), EventList [0xFFFA] (Provisional)
featureMapAttributeId = "0xFFFC"
attributeListAttributeId = "0xFFFB"
acceptedCommandListAttributeId = "0xFFF9"
generatedCommandListAttributeId = "0xFFF8"

# Endpoint define
rootNodeEndpointID = 0

# Load cluster info
inputJson = {}
clusterInfoDict = {}

print("Generating cluster data dict from JSON input")

with open(clusterInfoInputPathStr, 'rb') as clusterInfoInputFile:
    clusterInfoJson = json.load(clusterInfoInputFile)

    for cluster in clusterInfoJson:
        clusterData = clusterInfoJson[f"{cluster}"]["Data created by Script"]

        try:
            # Check if cluster id is a value hex value
            clusterIdValid = int(clusterData["Id"].lower(), 16)

            # Add cluster data to dict
            clusterInfoDict[clusterData["Id"].lower()] = {
                "Name": clusterData["Cluster Name"],
                "PICS_Code": clusterData["PICS Code"],
            }

        except ValueError:
            print(f"Ignore ClusterID: {clusterData['Id']} - {clusterData['Cluster Name']}")

# Load PICS XML templates
print("Capture list of PICS XML templates")
xmlFileList = os.listdir(xmlTemplatePathStr)

# Setup output path
baseOutputPath = pathlib.Path(baseOutputPathStr)
if not baseOutputPath.exists():
    print("Create output folder")
    baseOutputPath.mkdir()
else:
    print("Clean output folder")
    cleanDirectory(baseOutputPath)


class DeviceMappingTest(MatterBaseTest):
    @async_test_body
    async def test_device_mapping(self):

        # Create console to print
        global console
        console = Console()

        # Run device mapping function
        await DeviceMapping(self.default_controller, self.dut_node_id, baseOutputPathStr)


if __name__ == "__main__":
    default_matter_test_main()
