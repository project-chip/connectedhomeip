#!/usr/bin/env python3

# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import json
import xml.etree.ElementTree as ET

import click

DATA_MODEL_PATH = 'data_model/clusters'


@click.command()
@click.option(
    '--zap',
    required=True,
    type=str,
    help='ZAP file')
@click.option(
    '--matter-root',
    required=True,
    type=str,
    help='Path to the matter root')
def main(zap, matter_root):

    all_checks_successful = True

    CLUSTER_REVISIONS_BY_CLUSTER_ID = {}

    print('Getting latest cluster revisions from XML data model')
    for file in os.listdir(os.path.join(matter_root, DATA_MODEL_PATH)):
        if file.endswith(".xml"):
            with open(os.path.join(matter_root, DATA_MODEL_PATH, file), "r") as cluster_xml_file:
                print(f'Processing {file}')
                xml_et = ET.parse(cluster_xml_file)
                root = xml_et.getroot()

                try:
                    CLUSTER_REVISIONS_BY_CLUSTER_ID[int(root.attrib['id'], 16)] = root.attrib['revision']
                except ValueError:
                    print(' ⚠️ Failed to parse Cluster ID')

    # print(CLUSTER_REVISIONS_BY_CLUSTER_ID)
    print()

    with open(zap, "r") as zap_file:
        zap_data = json.load(zap_file)

        for endpoint in zap_data["endpointTypes"]:
            print(f'Checking endpoint {endpoint["name"]}')

            for cluster in endpoint["clusters"]:
                print(f' Checking {cluster["side"]} cluster {cluster["name"]}')

                if "attributes" in cluster:
                    for attribute in cluster["attributes"]:

                        if attribute["name"] == "ClusterRevision":
                            print('  Checking value of attribute ClusterRevision')

                            try:
                                value_in_zap = attribute["defaultValue"]
                                value_in_xml = CLUSTER_REVISIONS_BY_CLUSTER_ID[cluster["code"]]

                                if value_in_xml == value_in_zap:
                                    print(f'   ✅ ClusterRevision: XML:{value_in_xml} == ZAP:{value_in_zap}')
                                else:
                                    print(f'   ❌ ClusterRevision: XML:{value_in_xml} != ZAP:{value_in_zap}')
                                    all_checks_successful = False
                            except KeyError:
                                print(f'   ⚠️ No ClusterRevision found for cluster >{cluster["name"]}< {cluster["code"]}')
                else:
                    print('  No attributes in this cluster')

    if all_checks_successful:
        sys.exit(0)
    else:
        sys.exit(-1)


if __name__ == '__main__':
    main()
