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
import xml.etree.ElementTree as ET
from pathlib import Path

from chip.testing.pics import (accepted_cmd_pics_str, attribute_pics_str, client_pics_str, event_pics_str, feature_pics_str,
                               generated_cmd_pics_str, server_pics_str)
from pics_generator_support import map_cluster_name_to_pics_xml, pics_xml_file_list_loader

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from chip.testing.spec_parsing import build_xml_clusters  # noqa: E402


def pics_validation(dm_pics, xml_pics):
    """
    Validate the PICS codes from DM and XML.
    """
    matched_pics_code = list(set(dm_pics) & set(xml_pics))
    pics_xml_only_pics_code = list(set(xml_pics) - set(dm_pics))
    dm_scrape_only_pics_code = list(set(dm_pics) - set(xml_pics))

    if args.verbose and matched_pics_code:
        print(f"Matched PICS: {matched_pics_code} ✅")
    if pics_xml_only_pics_code:
        print(f"PICS XML Only PICS: {pics_xml_only_pics_code} ❌")
    if dm_scrape_only_pics_code:
        print(f"DM Scrape Only PICS: {dm_scrape_only_pics_code} ❌")


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
            print(f"Could not parse \"{pics_xml_file_name}\" ❌")
            continue

        print(f"PICS Code: {xml_clusters[cluster].pics}")
        pics_code = xml_clusters[cluster].pics
        if not pics_code:
            print(f"No PICS code found for {xml_clusters[cluster].name} ❌")
            continue

        pics_code_dm_list = [server_pics_str(pics_code), client_pics_str(pics_code)]

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

        pics_validation(pics_code_dm_list, pics_code_xml_list)

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
                        # print(f"DM Scrape - {feature_pics_str(pics_code, bit_index)}")
                        dmScrapeFeatureList.append(feature_pics_str(pics_code, bit_index))

            pics_validation(dmScrapeFeatureList, picsXmlFeatureList)

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
                # print(f"DM Scrape - {attribute_pics_str(pics_code, xml_clusters[cluster].attribute_map[attribute])}")
                dmScrapeAttributeList.append(attribute_pics_str(pics_code, xml_clusters[cluster].attribute_map[attribute]))

            pics_validation(dmScrapeAttributeList, picsXmlAttributeList)

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
                # print(f"DM Scrape - {accepted_cmd_pics_str(pics_code, command)}")
                dmScrapeCommandReceivedList.append(accepted_cmd_pics_str(pics_code, command))

            pics_validation(dmScrapeCommandReceivedList, picsXmlCommandReceivedList)

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
                # print(f"DM Scrape - {generated_cmd_pics_str(pics_code, command)}")
                dmScrapeCommandGeneratedList.append(generated_cmd_pics_str(pics_code, command))

            pics_validation(dmScrapeCommandGeneratedList, picsXmlCommandGeneratedList)

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
                # print(f"DM Scrape - {event_pics_str(pics_code, event)}")
                dmScrapeEventList.append(event_pics_str(pics_code, event))

            pics_validation(dmScrapeEventList, picsXmlEventList)

    else:
        print(
            f"Could not find matching PICS XML file for {xml_clusters[cluster].name} - {xml_clusters[cluster].pics} (Provisional: {xml_clusters[cluster].is_provisional}) ❌")
