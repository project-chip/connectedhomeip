#
#    Copyright (c) 2024 Project CHIP Authors
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
import sys
from pathlib import Path
import xml.etree.ElementTree as ET

from pics_generator_support import map_cluster_name_to_pics_xml, pics_xml_file_list_loader

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from chip.testing.spec_parsing import build_xml_clusters  # noqa: E402

parser = argparse.ArgumentParser()
parser.add_argument('--pics-template', required=True)
parser.add_argument('--dm-xml', required=True)
parser.add_argument('--verbose', action="store_true")
args, unknown = parser.parse_known_args()

xml_template_path_str = args.pics_template
if not xml_template_path_str.endswith('/'):
    xml_template_path_str += '/'

print("Build list of PICS XML")
pics_xml_file_list = pics_xml_file_list_loader(xml_template_path_str, True)

print("Build list of spec XML")
xml_clusters, problems = build_xml_clusters(Path(f"{args.dm_xml}/clusters"))


for cluster in xml_clusters:
    pics_xml_file_name = map_cluster_name_to_pics_xml(xml_clusters[cluster].name, pics_xml_file_list)

    if pics_xml_file_name:
        if args.verbose:
            print(
                f"Found PICS XML file for {xml_clusters[cluster].name} - {pics_xml_file_name} (Provisional: {xml_clusters[cluster].is_provisional}) ✅")

        try:
            # Open the XML PICS template file
            print(f"Open \"{xml_template_path_str}{pics_xml_file_name}\"")
            parser = ET.XMLParser(target=ET.TreeBuilder(insert_comments=True))
            tree = ET.parse(f"{xml_template_path_str}{pics_xml_file_name}", parser)
            root = tree.getroot()
        except ET.ParseError:
            print(f"[red]Could not find \"{pics_xml_file_name}\" ❌")
            continue

        print(f"PICS Code: {xml_clusters[cluster].pics}")
        pics_code = xml_clusters[cluster].pics
        pics_code_server = f"{pics_code}.S"
        pics_code_client = f"{pics_code}.C"

        pics_code_dm_list = [pics_code_server, pics_code_client]

        pics_code_xml_list = []
        usage_node = root.find('usage')
        for pics_item in usage_node:
            item_number_element = pics_item.find('itemNumber')
            # print(f"PICS XML - {item_number_element.text}")

            # Media PICS contains multiple PICS codes so only append if the PICS code is the selected one.
            if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                if pics_code in item_number_element.text:
                    pics_code_xml_list.append(item_number_element.text)
            else:
                pics_code_xml_list.append(item_number_element.text)

        matched_pics_code = list(set(pics_code_dm_list) & set(pics_code_xml_list))
        pics_xml_only_pics_code = list(set(pics_code_xml_list) - set(pics_code_dm_list))
        dm_scrape_only_pics_code = list(set(pics_code_dm_list) - set(pics_code_xml_list))

        if args.verbose and matched_pics_code:
            print(f"Matched PICS Code: {matched_pics_code} ✅")

        if pics_xml_only_pics_code:
            print(f"PICS XML Only PICS Code: {pics_xml_only_pics_code} ❌")
        if dm_scrape_only_pics_code:
            print(f"DM Scrape Only PICS Code: {dm_scrape_only_pics_code} ❌")

        # print(f"FeatureMap: {xml_clusters[cluster].feature_map}")
        if xml_clusters[cluster].feature_map:

            picsXmlFeatureList = []
            featureNode = root.find("./clusterSide[@type='Server']/features")
            for pics_item in featureNode:
                item_number_element = pics_item.find('itemNumber')
                # print(f"PICS XML - {item_number_element.text}")
                if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                    if pics_code in item_number_element.text:
                        picsXmlFeatureList.append(item_number_element.text)
                else:
                    picsXmlFeatureList.append(item_number_element.text)

            dmScrapeFeatureList = []
            for feature in xml_clusters[cluster].feature_map:
                # print(f"Feature: {xml_clusters[cluster].feature_map[feature]} - {feature}")
                for bit_index in range(0, 32):
                    if xml_clusters[cluster].feature_map[feature] >> bit_index == 1:
                        # print(f"DM Scrape - {pics_code_server}.F{bit_index:02x}")
                        dmScrapeFeatureList.append(f"{pics_code_server}.F{bit_index:02x}")

            matchedFeatures = list(set(picsXmlFeatureList) & set(dmScrapeFeatureList))
            picsXmlOnlyFeatures = list(set(picsXmlFeatureList) - set(dmScrapeFeatureList))
            dmScrapeOnlyFeatures = list(set(dmScrapeFeatureList) - set(picsXmlFeatureList))

            if args.verbose and matchedFeatures:
                print(f"Matched Feature PICS: {matchedFeatures} ✅")
            if picsXmlOnlyFeatures:
                print(f"PICS XML Only Feature PICS: {picsXmlOnlyFeatures} ❌")
            if dmScrapeOnlyFeatures:
                print(f"DM Scrape Only Feature PICS: {dmScrapeOnlyFeatures} ❌")

        # print(f"Attributes: {xml_clusters[cluster].attribute_map}")
        if xml_clusters[cluster].attribute_map:
            picsXmlAttributeList = []
            serverAttributesNode = root.find("./clusterSide[@type='Server']/attributes")
            for pics_item in serverAttributesNode:
                item_number_element = pics_item.find('itemNumber')
                # print(f"PICS XML - {item_number_element.text}")
                if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                    if pics_code in item_number_element.text:
                        picsXmlAttributeList.append(item_number_element.text)
                else:
                    picsXmlAttributeList.append(item_number_element.text)

            dmScrapeAttributeList = []
            for attribute in xml_clusters[cluster].attribute_map:
                # print(f"DM Scrape - {pics_code_server}.A{xml_clusters[cluster].attribute_map[attribute]:04x}")
                dmScrapeAttributeList.append(f"{pics_code_server}.A{xml_clusters[cluster].attribute_map[attribute]:04x}")

            matchedAttributes = list(set(picsXmlAttributeList) & set(dmScrapeAttributeList))
            picsXmlOnlyAttributes = list(set(picsXmlAttributeList) - set(dmScrapeAttributeList))
            dmScrapeOnlyAttributes = list(set(dmScrapeAttributeList) - set(picsXmlAttributeList))

            if args.verbose and matchedAttributes:
                print(f"Matched Attribute PICS: {matchedAttributes} ✅")
            if picsXmlOnlyAttributes:
                print(f"PICS XML Only Attribute PICS: {picsXmlOnlyAttributes} ❌")
            if dmScrapeOnlyAttributes:
                print(f"DM Scrape Only Attribute PICS: {dmScrapeOnlyAttributes} ❌")

        # print(f"Accepted Commands: {xml_clusters[cluster].accepted_commands}")
        if xml_clusters[cluster].accepted_commands:
            picsXmlCommandReceivedList = []
            serverCommandsNode = root.find("./clusterSide[@type='Server']/commandsReceived")
            for pics_item in serverCommandsNode:
                item_number_element = pics_item.find('itemNumber')
                # print(f"PICS XML - {item_number_element.text}")
                if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                    if pics_code in item_number_element.text:
                        picsXmlCommandReceivedList.append(item_number_element.text)
                else:
                    picsXmlCommandReceivedList.append(item_number_element.text)

            dmScrapeCommandReceivedList = []
            for command in xml_clusters[cluster].accepted_commands:
                # print(f"DM Scrape - {pics_code_server}.C{command:02x}.Rsp")
                dmScrapeCommandReceivedList.append(f"{pics_code_server}.C{command:02x}.Rsp")

            matchedRspCommands = list(set(picsXmlCommandReceivedList) & set(dmScrapeCommandReceivedList))
            picsXmlOnlyRspCommands = list(set(picsXmlCommandReceivedList) - set(dmScrapeCommandReceivedList))
            dmScrapeOnlyRspCommands = list(set(dmScrapeCommandReceivedList) - set(picsXmlCommandReceivedList))

            if args.verbose and matchedRspCommands:
                print(f"Matched Command PICS: {matchedRspCommands} ✅")
            if picsXmlOnlyRspCommands:
                print(f"PICS XML Only Command PICS: {picsXmlOnlyRspCommands} ❌")
            if dmScrapeOnlyRspCommands:
                print(f"DM Scrape Only Command PICS: {dmScrapeOnlyRspCommands} ❌")

        if xml_clusters[cluster].generated_commands:
            picsXmlCommandGeneratedList = []
            serverCommandsNode = root.find("./clusterSide[@type='Server']/commandsGenerated")
            for pics_item in serverCommandsNode:
                item_number_element = pics_item.find('itemNumber')
                # print(f"PICS XML - {item_number_element.text}")
                if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                    if pics_code in item_number_element.text:
                        picsXmlCommandGeneratedList.append(item_number_element.text)
                else:
                    picsXmlCommandGeneratedList.append(item_number_element.text)

            dmScrapeCommandGeneratedList = []
            for command in xml_clusters[cluster].generated_commands:
                # print(f"DM Scrape - {pics_code_server}.C{command:02x}.Tx")
                dmScrapeCommandGeneratedList.append(f"{pics_code_server}.C{command:02x}.Tx")

            matchedTxCommands = list(set(picsXmlCommandGeneratedList) & set(dmScrapeCommandGeneratedList))
            picsXmlOnlyTxCommands = list(set(picsXmlCommandGeneratedList) - set(dmScrapeCommandGeneratedList))
            dmScrapeOnlyTxCommands = list(set(dmScrapeCommandGeneratedList) - set(picsXmlCommandGeneratedList))

            if args.verbose and matchedTxCommands:
                print(f"Matched Command PICS: {matchedTxCommands} ✅")
            if picsXmlOnlyTxCommands:
                print(f"PICS XML Only Command PICS: {picsXmlOnlyTxCommands} ❌")
            if dmScrapeOnlyTxCommands:
                print(f"DM Scrape Only Command PICS: {dmScrapeOnlyTxCommands} ❌")

        if xml_clusters[cluster].events:
            picsXmlEventList = []
            serverEventsNode = root.find("./clusterSide[@type='Server']/events")
            for pics_item in serverEventsNode:
                item_number_element = pics_item.find('itemNumber')
                # print(f"PICS XML - {item_number_element.text}")
                if "media" in pics_xml_file_name.lower() or "group communication" in pics_xml_file_name.lower() or "ota software update" in pics_xml_file_name.lower():
                    if pics_code in item_number_element.text:
                        picsXmlEventList.append(item_number_element.text)
                else:
                    picsXmlEventList.append(item_number_element.text)

            dmScrapeEventList = []
            for event in xml_clusters[cluster].events:
                # print(f"DM Scrape - {pics_code_server}.E{event:02x}")
                dmScrapeEventList.append(f"{pics_code_server}.E{event:02x}")

            matchedEvents = list(set(picsXmlEventList) & set(dmScrapeEventList))
            picsXmlOnlyEvents = list(set(picsXmlEventList) - set(dmScrapeEventList))
            dmScrapeOnlyEvents = list(set(dmScrapeEventList) - set(picsXmlEventList))

            if args.verbose and matchedEvents:
                print(f"Matched Event PICS: {matchedEvents} ✅")
            if picsXmlOnlyEvents:
                print(f"PICS XML Only Event PICS: {picsXmlOnlyEvents} ❌")
            if dmScrapeOnlyEvents:
                print(f"DM Scrape Only Event PICS: {dmScrapeOnlyEvents} ❌")

    else:
        print(
            f"Could not find matching PICS XML file for {xml_clusters[cluster].name} - {xml_clusters[cluster].pics} (Provisional: {xml_clusters[cluster].is_provisional}) ❌")
