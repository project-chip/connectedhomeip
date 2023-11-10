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

import click
import coloredlogs
import json
import logging
import os
import sys
import xml.etree.ElementTree as ET

DATA_MODEL_PATH = 'data_model/clusters'

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


@click.command()
@click.option(
    '--zap',
    required=True,
    type=str,
    help='ZAP file')
@click.option(
    '--matter-root',
    required=True,
    default=os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')),
    type=str,
    help='Path to the matter root')
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def main(zap, matter_root, log_level):

    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )

    all_checks_successful = True

    CLUSTER_REVISIONS_BY_CLUSTER_ID = {}

    logging.info('Getting latest cluster revisions from XML data model')
    for file in os.listdir(os.path.join(matter_root, DATA_MODEL_PATH)):
        if file.endswith(".xml"):
            with open(os.path.join(matter_root, DATA_MODEL_PATH, file), "r") as cluster_xml_file:
                logging.info(f'Processing {file}')
                xml_et = ET.parse(cluster_xml_file)
                root = xml_et.getroot()

                try:
                    CLUSTER_REVISIONS_BY_CLUSTER_ID[int(root.attrib['id'], 16)] = root.attrib['revision']
                except ValueError:
                    logging.warning(f' ⚠️ Failed to parse Cluster ID in {file}')

    logging.debug(CLUSTER_REVISIONS_BY_CLUSTER_ID)

    with open(zap, "r") as zap_file:
        zap_data = json.load(zap_file)

        for endpoint in zap_data["endpointTypes"]:
            logging.info(f'Checking endpoint {endpoint["name"]}')

            for cluster in endpoint["clusters"]:
                logging.info(f' Checking {cluster["side"]} cluster {cluster["name"]}')

                if "attributes" in cluster:
                    for attribute in cluster["attributes"]:

                        if attribute["name"] == "ClusterRevision":
                            logging.info('  Checking value of attribute ClusterRevision')

                            try:
                                value_in_zap = attribute["defaultValue"]
                                value_in_xml = CLUSTER_REVISIONS_BY_CLUSTER_ID[cluster["code"]]

                                if value_in_xml == value_in_zap:
                                    logging.info(f'   ✅ ClusterRevision: XML:{value_in_xml} == ZAP:{value_in_zap}')
                                else:
                                    logging.fatal(f'   ❌ ClusterRevision: XML:{value_in_xml} != ZAP:{value_in_zap}')
                                    all_checks_successful = False
                            except KeyError:
                                logging.warning(f'   ⚠️ No ClusterRevision found for cluster >{cluster["name"]}< {cluster["code"]}')
                else:
                    logging.info('  No attributes in this cluster')

    if all_checks_successful:
        sys.exit(0)
    else:
        logging.error('Found zap lint errors')
        sys.exit(-1)


if __name__ == '__main__':
    main()
