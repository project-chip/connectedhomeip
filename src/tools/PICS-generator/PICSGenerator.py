#
#    Copyright (c) 2023-2024 Project CHIP Authors
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
import os
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from pics_generator_support import map_cluster_name_to_pics_xml, pics_xml_file_list_loader
from rich.console import Console

import matter.clusters as Clusters
from matter.clusters.Attribute import AsyncReadTransaction
from matter.testing.decorators import async_test_body
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.runner import default_matter_test_main
from matter.tlv import uint

# Add the path to python_testing folder, in order to be able to import from matter.testing.matter_testing
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from matter.testing.conformance import ConformanceAssessmentData, ConformanceException  # noqa: E402
from matter.testing.matter_testing import MatterBaseTest  # noqa: E402
from matter.testing.pics import BasePicsFacts, derive_base_pics_facts_from_device_wildcard  # noqa: E402
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, XmlEvent, build_xml_clusters  # noqa: E402

console = None
xml_clusters = None

# Matches the trailing ".E<hex>" event-id suffix in a PICS itemNumber like "ACL.S.E01".
_EVENT_ID_RE = re.compile(r'\.E([0-9A-Fa-f]+)$')

# Filename of the MCORE/base template in the CSA PICS XML bundle.
_BASE_PICS_TEMPLATE_FILENAME = "Base.xml"

# Network Commissioning cluster ID. Used to detect the cluster on any endpoint
# without depending on it living on EP0.
_NETWORK_COMMISSIONING_CLUSTER_ID = 0x0031

# Network Commissioning feature bits (see NetworkCommissioningCluster.xml).
_NETCOMM_FEATURE_BIT_WIFI = 0
_NETCOMM_FEATURE_BIT_THREAD = 1
_NETCOMM_FEATURE_BIT_ETHERNET = 2

# Aggregator device type ID (see data_model/.../device_types/Aggregator.xml).
# Endpoints with this device type identify the device as a Bridge.
_AGGREGATOR_DEVICE_TYPE_ID = 0x000E


# Local-only fact extension. Inherits everything BasePicsFacts derives from
# the wildcard (commissionee, server, bridge, OTA, multi-endpoint groups,
# Wi-Fi bands, mandatory events). Adds the MCORE.COM.WIFI/THR/ETH/WIRELESS
# transport bits that the shared helper deliberately does not derive while
# GRL stress-test feedback is outstanding (Cecille, May 2026). Once the
# test-plans cleanup PRs land, this extension can be deleted and the
# generator can use BasePicsFacts directly.
@dataclass
class _BasePicsFacts(BasePicsFacts):
    supports_wifi: bool = False
    supports_thread: bool = False
    supports_ethernet: bool = False


def _extract_event_id(item_number: Optional[str]) -> Optional[int]:
    """
    Parse the event id from a PICS itemNumber like 'ACL.S.E01'.

    Returns None if item_number is missing or doesn't end in '.E<hex>'.
    """
    if not item_number:
        return None
    match = _EVENT_ID_RE.search(item_number)
    if match is None:
        return None
    return int(match.group(1), 16)


def GenerateBasePicsXmlFile(facts: _BasePicsFacts, outputPathStr: str) -> None:
    """
    Auto-mark the base/MCORE PICS we can derive from the DUT.

    Only flips items with a deterministic protocol readback. Everything else
    is left at the template default (false) so the reviewer knows to look at it.
    """
    template_path = Path(xmlTemplatePathStr) / _BASE_PICS_TEMPLATE_FILENAME
    if not template_path.exists():
        console.print(f"[red]Base PICS template ({_BASE_PICS_TEMPLATE_FILENAME}) not found in {xmlTemplatePathStr}; skipping ❌")
        return

    # itemNumber -> True if we want to flip support to "true". Items not in
    # this map keep the template value (false).
    auto_marked: dict[str, bool] = {}
    if facts.is_commissionee:
        auto_marked["MCORE.ROLE.COMMISSIONEE"] = True

    # MCORE.COM.WIFI_2P4GHZ / MCORE.COM.WIFI_5GHZ are about Public Action
    # Frame support on the corresponding band, not radio band capability,
    # and there's no protocol-observable signal for that. Leave both at the
    # template default (false) and let the reviewer flip them if needed.
    if facts.supports_wifi:
        auto_marked["MCORE.COM.WIFI"] = True

    if facts.supports_thread:
        auto_marked["MCORE.COM.THR"] = True

    if facts.supports_ethernet:
        auto_marked["MCORE.COM.ETH"] = True

    # MCORE.COM.WIRELESS conformance requires at least one of WIFI / THR to
    # be set; mirror what we marked above so we don't leave the file
    # internally inconsistent.
    if facts.supports_wifi or facts.supports_thread:
        auto_marked["MCORE.COM.WIRELESS"] = True

    if facts.is_server:
        auto_marked["MCORE.IDM.S"] = True

    if facts.is_bridge:
        auto_marked["MCORE.BRIDGE"] = True

    if facts.is_ota_requestor:
        auto_marked["MCORE.OTA.Requestor"] = True

    if facts.is_ota_provider:
        auto_marked["MCORE.OTA.Provider"] = True

    if facts.has_groups_on_multiple_endpoints:
        auto_marked["MCORE.G.MULTIENDPOINT"] = True

    parser = ET.XMLParser(target=ET.TreeBuilder(insert_comments=True))
    tree = ET.parse(template_path, parser)
    root = tree.getroot()

    marked_count = 0
    for picsItem in root.iter('picsItem'):
        itemNumberElement = picsItem.find('itemNumber')
        if itemNumberElement is None or itemNumberElement.text is None:
            continue
        item_id = itemNumberElement.text.strip()
        if auto_marked.get(item_id):
            supportElement = picsItem.find('support')
            if supportElement is not None:
                supportElement.text = "true"
                console.print(f"Auto-marked {item_id} in Base.xml ✅")
                marked_count += 1

    if marked_count == 0:
        console.print("[yellow]Base.xml: no MCORE items auto-marked (Network Commissioning not detected?)")

    # Preserve the template's leading xml declaration + autogenerated comment
    # block by streaming raw lines until we hit the root <generalPICS>, then
    # writing the parsed tree. Matches the style used by GenerateDevicePicsXmlFiles.
    output_file_path = Path(outputPathStr) / _BASE_PICS_TEMPLATE_FILENAME
    with (open(template_path) as inputFile,
          open(output_file_path, "wb") as outputFile):
        header = ""
        line = inputFile.readline()
        while line and 'generalPICS' not in line:
            header += line
            line = inputFile.readline()
        outputFile.write(header.encode())
        tree.write(outputFile, encoding='utf-8', xml_declaration=False)

    console.print(f"[blue]Wrote Base.xml: {output_file_path}")


def GenerateDevicePicsXmlFiles(clusterName, clusterPicsCode, featurePicsList, attributePicsList,
                               acceptedCommandPicsList, generatedCommandPicsList, outputPathStr,
                               xml_events: Optional[dict[uint, XmlEvent]] = None,
                               assessment_data: Optional[ConformanceAssessmentData] = None):

    xmlPath = xmlTemplatePathStr
    fileName = ""

    console.print(f"Handling PICS for {clusterName}")

    picsFileName = map_cluster_name_to_pics_xml(clusterName, xmlFileList)

    # If we've already written an output for this cluster's template
    # (e.g. OTA Software Update Provider and Requestor both resolve to
    # the same template, or a cluster appears as both server and client
    # on this endpoint), reuse the existing file as input so the new
    # markings get merged in instead of writing a fresh copy.
    if picsFileName:
        existing_output = Path(outputPathStr) / picsFileName
        if existing_output.is_file():
            xmlPath = outputPathStr
            fileName = picsFileName

    # If no file is found in output folder, determine if there is a match for the cluster name in input folder
    if fileName == "":
        for file in xmlFileList:
            if file.lower().startswith(picsFileName.lower()):
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
    usageNode = root.find('usage')
    for picsItem in usageNode:
        itemNumberElement = picsItem.find('itemNumber')

        console.print(f"Searching for {itemNumberElement.text}")

        if itemNumberElement.text == f"{clusterPicsCode}":
            console.print("Found usage PICS value in XML template ✅")
            supportElement = picsItem.find('support')
            supportElement.text = "true"

            # Since usage PICS (server or client) is not a list, we can break out when a match is found,
            # no reason to keep iterating through the elements.
            break

    # Feature PICS
    # console.print(featurePicsList)
    featureNode = root.find("./clusterSide[@type='Server']/features")
    if featureNode is not None:
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
    if serverAttributesNode is not None:
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
    if serverCommandsReceivedNode is not None:
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
    if serverCommandsGeneratedNode is not None:
        for picsItem in serverCommandsGeneratedNode:
            itemNumberElement = picsItem.find('itemNumber')

            console.print(f"Searching for {itemNumberElement.text}")

            if f"{itemNumberElement.text}" in generatedCommandPicsList:
                console.print("Found generatedCommand PICS value in XML template ✅")
                supportElement = picsItem.find('support')
                supportElement.text = "true"

    # Event PICS.
    # EventList (0xFFFA) is deprecated and the SDK no longer implements it, so
    # we can't read supported events back from the device. Use the parsed
    # cluster conformance instead and auto-mark events that come back
    # MANDATORY for this DUT's feature set. Handles AND/OR/NOT and parens
    # that the previous string equality check missed (e.g. "ACL.S AND ACL.S.F00").
    serverEventsNode = root.find("./clusterSide[@type='Server']/events")
    if serverEventsNode is not None:
        for picsItem in serverEventsNode:
            itemNumberElement = picsItem.find('itemNumber')
            statusElement = picsItem.find('status')

            condition = statusElement.attrib.get('cond', '')
            if condition:
                console.print(f"Checking {itemNumberElement.text} with conformance {statusElement.text} and condition {condition}")
            else:
                console.print(f"Checking {itemNumberElement.text} with conformance {statusElement.text}")

            event_id = _extract_event_id(itemNumberElement.text)
            if event_id is None or xml_events is None or assessment_data is None or event_id not in xml_events:
                # No parsed conformance for this item (template references an event
                # the DM XML scrape doesn't have, or itemNumber isn't .E<hex>).
                # Leave support=false for the reviewer.
                continue

            try:
                decision = xml_events[event_id].conformance(assessment_data)
            except ConformanceException as e:
                console.print(f"[yellow]  → conformance evaluation failed for {itemNumberElement.text}: {e}; leaving support=false")
                continue

            if decision.is_mandatory():
                console.print(f"Event {itemNumberElement.text} is mandatory by spec conformance ✅")
                supportElement = picsItem.find('support')
                supportElement.text = "true"
            else:
                console.print(f"  → not mandatory for this device (decision={decision.decision.name})")

    # Read the template/existing header before opening the output for
    # write. "wb" truncates on open, so doing it under a single `with`
    # would wipe the file we're reading from when xmlPath ==
    # outputPathStr (server+client merge case, where the second pass
    # reuses the file the first pass wrote).
    xmlHeader = ""
    with open(f"{xmlPath}{fileName}") as inputFile:
        inputLine = inputFile.readline().lstrip()
        while 'clusterPICS' not in inputLine:
            xmlHeader += inputLine
            inputLine = inputFile.readline().lstrip()

    with open(f"{outputPathStr}/{fileName}", "wb") as outputFile:
        outputFile.write(xmlHeader.encode())
        tree.write(outputFile, encoding='utf-8', xml_declaration=False)


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

    # Track the transport bits the shared helper deliberately does not
    # derive yet. Everything else on _BasePicsFacts comes from
    # derive_base_pics_facts_from_device_wildcard at the bottom of this
    # function.
    transport_supports_wifi = False
    transport_supports_thread = False
    transport_supports_ethernet = False

    # Accumulator that mirrors the shape of AsyncReadTransaction.ReadResponse
    # so the shared helper can derive the in-scope facts from a single pass
    # at the end. We populate the slices the helper actually reads:
    #   - attributes[ep][Clusters.Descriptor][...DeviceTypeList]  (parsed objects)
    #   - tlvAttributes[ep][cluster_id][global_attr_id]           (raw values)
    wildcard = AsyncReadTransaction.ReadResponse(attributes={}, events=[], tlvAttributes={})

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
        device_type_list = deviceListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]

        # Mirror device types into the wildcard accumulator so the shared
        # helper can see Aggregator (-> MCORE.BRIDGE) and Root Node
        # (-> MCORE.ROLE.COMMISSIONEE).
        wildcard.attributes.setdefault(endpoint, {}).setdefault(
            Clusters.Descriptor, {})[Clusters.Descriptor.Attributes.DeviceTypeList] = device_type_list

        for deviceTypeData in device_type_list:
            console.print(f"Device Type: {deviceTypeData.deviceType}")

        # Read server list
        serverListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, Clusters.Descriptor.Attributes.ServerList)])
        serverList = serverListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]

        for server in serverList:
            featurePicsList = []
            attributePicsList = []
            acceptedCommandListPicsList = []
            generatedCommandListPicsList = []

            clusterID = f"0x{server:04x}"

            if server > 0x7FFF:
                console.print(f"[red]Cluster outside standard range ({clusterID}) not handled! ❌")
                continue

            try:
                clusterClass = getattr(Clusters, devCtrl.GetClusterHandler().GetClusterInfoById(server)['clusterName'])
            except AttributeError:
                console.print(f"[red]Cluster class not found for ({clusterID}) not found! ❌")
                continue

            # Does the the DM XML contain the found cluster ID?
            try:
                clusterName = xml_clusters[server].name
                clusterPICS = f"{xml_clusters[server].pics}{serverTag}"

            except KeyError:
                console.print(f"[red]Cluster ({clusterID}) not found in DM XML! ❌")
                continue

            console.print(f"{clusterName} - {clusterPICS}")

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

            # Transport bits the shared helper does not derive yet. Keep
            # MCORE.COM.WIFI / THR / ETH driven from the featuremap here
            # until Cecille's test-plans cleanup PRs land and these PICS
            # items are removed from Base.xml.
            if server == _NETWORK_COMMISSIONING_CLUSTER_ID:
                if featureMapValue & (1 << _NETCOMM_FEATURE_BIT_WIFI):
                    transport_supports_wifi = True
                if featureMapValue & (1 << _NETCOMM_FEATURE_BIT_THREAD):
                    transport_supports_thread = True
                if featureMapValue & (1 << _NETCOMM_FEATURE_BIT_ETHERNET):
                    transport_supports_ethernet = True

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

            # Read ClusterRevision (needed by ConformanceAssessmentData for
            # revision-gated conformance, which is rare but possible).
            clusterRevisionResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, clusterClass.Attributes.ClusterRevision)])
            clusterRevision = clusterRevisionResponse[endpoint][clusterClass][clusterClass.Attributes.ClusterRevision]

            # Mirror the per-cluster global attributes into the wildcard
            # accumulator so the shared helper sees the same inputs that
            # the inline assessment_data below uses.
            wildcard.tlvAttributes.setdefault(endpoint, {})[server] = {
                GlobalAttributeIds.FEATURE_MAP_ID: featureMapValue,
                GlobalAttributeIds.ATTRIBUTE_LIST_ID: list(attributeList),
                GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID: list(acceptedCommandList),
                GlobalAttributeIds.GENERATED_COMMAND_LIST_ID: list(generatedCommandList),
                GlobalAttributeIds.CLUSTER_REVISION_ID: clusterRevision,
            }

            # Inputs the conformance evaluator needs to decide which events
            # are mandatory on this DUT.
            assessment_data = ConformanceAssessmentData(
                feature_map=uint(featureMapValue),
                attribute_list=list(attributeList),
                all_command_list=list(acceptedCommandList) + list(generatedCommandList),
                cluster_revision=uint(clusterRevision),
            )

            # Write the collected PICS to a PICS XML file
            GenerateDevicePicsXmlFiles(clusterName, clusterPICS, featurePicsList, attributePicsList,
                                       acceptedCommandListPicsList, generatedCommandListPicsList, endpointOutputPathStr,
                                       xml_events=xml_clusters[server].events,
                                       assessment_data=assessment_data)

        # Read client list
        clientListResponse = await devCtrl.ReadAttribute(nodeID, [(endpoint, Clusters.Descriptor.Attributes.ClientList)])
        clientList = clientListResponse[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ClientList]

        for client in clientList:
            clusterID = f"0x{client:04x}"

            try:
                clusterName = xml_clusters[client].name
                clusterPICS = f"{xml_clusters[client].pics}{clientTag}"

            except KeyError:
                console.print(f"[red]Cluster ({clusterID}) not found in DM XML! ❌")
                continue

            console.print(f"{clusterName} - {clusterPICS}")

            GenerateDevicePicsXmlFiles(clusterName, clusterPICS, [], [], [], [], endpointOutputPathStr)

    # Derive the in-scope Base/MCORE facts from the wildcard we accumulated
    # during the parts-list walk. Keeping this in a single shared helper
    # means TC_IDM_10_4 and the generator can never disagree on these bits.
    shared_facts, derivation_problems = derive_base_pics_facts_from_device_wildcard(wildcard, xml_clusters)
    for problem in derivation_problems:
        console.print(f"[yellow]{problem.problem}")

    base_pics_facts = _BasePicsFacts(
        is_commissionee=shared_facts.is_commissionee,
        is_server=shared_facts.is_server,
        is_bridge=shared_facts.is_bridge,
        is_ota_requestor=shared_facts.is_ota_requestor,
        is_ota_provider=shared_facts.is_ota_provider,
        has_groups_on_multiple_endpoints=shared_facts.has_groups_on_multiple_endpoints,
        mandatory_events_by_cluster=shared_facts.mandatory_events_by_cluster,
        supports_wifi=transport_supports_wifi,
        supports_thread=transport_supports_thread,
        supports_ethernet=transport_supports_ethernet,
    )

    # Base/MCORE PICS are per-device, not per-endpoint, so this runs once after
    # the parts list walk completes and writes Base.xml at the device root.
    GenerateBasePicsXmlFile(base_pics_facts, outputPathStr)


def cleanDirectory(pathToClean):
    for entry in pathToClean.iterdir():
        if entry.is_file():
            pathlib.Path(entry).unlink()
        elif entry.is_dir():
            cleanDirectory(entry)
            pathlib.Path(entry).rmdir()


parser = argparse.ArgumentParser()
parser.add_argument('--pics-template', required=True)
parser.add_argument('--pics-output', required=True)
parser.add_argument('--dm-xml')
args, unknown = parser.parse_known_args()

# The matter_testing framework does not accept unknown args,
# so all the handled args are removed from argv
sys.argv = sys.argv[:1] + unknown

xmlTemplatePathStr = args.pics_template
if not xmlTemplatePathStr.endswith('/'):
    xmlTemplatePathStr += '/'

baseOutputPathStr = args.pics_output
if not baseOutputPathStr.endswith('/'):
    baseOutputPathStr += '/'
outputPathStr = baseOutputPathStr + "GeneratedPICS/"

serverTag = ".S"
clientTag = ".C"
featureTag = ".F"
attributeTag = ".A"
commandTag = ".C"
acceptedCommandTag = ".Rsp"
generatedCommandTag = ".Tx"

# List of global attributes (server).
# Does not read ClusterRevision [0xFFFD] (not relevant) or EventList [0xFFFA]
# (deprecated, not implemented by the SDK).
featureMapAttributeId = "0xFFFC"
attributeListAttributeId = "0xFFFB"
acceptedCommandListAttributeId = "0xFFF9"
generatedCommandListAttributeId = "0xFFF8"

# Endpoint define
rootNodeEndpointID = 0

# Load PICS XML templates
print("Capture list of PICS XML templates")
xmlFileList = pics_xml_file_list_loader(xmlTemplatePathStr, True)

# Setup output path
print(f"Output path: {outputPathStr}")

outputPath = pathlib.Path(outputPathStr)
if not outputPath.exists():
    print("Create output folder")
    outputPath.mkdir()
else:
    print("Clean output folder")
    cleanDirectory(outputPath)


class DeviceMappingTest(MatterBaseTest):
    @async_test_body
    async def test_device_mapping(self):

        # Create console to print
        global console
        console = Console()

        global xml_clusters
        if args.dm_xml:
            xml_clusters, problems = build_xml_clusters(Path(f"{args.dm_xml}/clusters"))
        else:
            specVersionResponse = await self.default_controller.ReadAttribute(self.dut_node_id, [(rootNodeEndpointID, Clusters.BasicInformation.Attributes.SpecificationVersion)])
            specVersion = specVersionResponse[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SpecificationVersion]
            console.print(f"Specification version received from device: {specVersion:x}")

            if specVersion == 0x1030000:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
            elif specVersion == 0x1040000:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
            elif specVersion == 0x1040100:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
            elif specVersion == 0x1040200:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_2)
            elif specVersion == 0x1050000:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)
            elif specVersion == 0x1050100:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5_1)
            elif specVersion == 0x1060000:
                xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_6)
            else:
                console.print("FAILURE: Specification version reported by device not supported")
                return

        # Run device mapping function
        await DeviceMapping(self.default_controller, self.dut_node_id, outputPathStr)


if __name__ == "__main__":
    default_matter_test_main()
