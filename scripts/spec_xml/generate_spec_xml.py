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

import glob
import logging
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Optional

import click

DEFAULT_CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT_DIR = os.path.abspath(os.path.join(DEFAULT_CHIP_ROOT, 'data_model'))


# Known TOC differences
# - NOC has an additional command section
# - DiagnosticsGeneral has an additional DoNotUse attribute that does not appear in the TOC
# - GeneralCommissioning has an additional section in the TOC for common error handling
# - ballast configuration is missing two attributes from the TOC (D attributes)
# - LevelControl has an additional attribute in the TOC for scene table extensions
# - ModeSelect has an attitional attribute in the TOC for scene table extensions
# - Scenes has an extra command in the TOC for usage notes
# - onoff has an additional attribute in the TOC for scene table extensions
# - color control is all over the place - ignore this for attributes.
# - microwave oven has an additional TOC section in commands for operational state description
# - EnergyEVSE - attributes SessionID, SessionDuration, SessionEnergyCharged and SessionEnergyDischarged are all collapsed into one section, meaning the TOC has three fewer sections than the XML


# Known ok warnings
# - ballast config has two D attributes (power on level and power on fade time) that don't have access

@dataclass
class ElementCounts:
    attributes: int
    commands: int
    events: int


def get_xml_path(filename, output_dir):
    xml = os.path.basename(filename).replace('.adoc', '.xml')
    return os.path.abspath(os.path.join(output_dir, xml))


def get_toc(filename: str, dry_run: bool) -> Optional[str]:
    # -o - sends output to stdout so we can scrape it without needing an intermediate file
    cmd = ['asciidoctor', '-a', 'toc', '-a', 'toclevel=2', '-d', 'book', '-o', '-', filename]
    if dry_run:
        print(cmd)
        return
    try:
        process = subprocess.run(cmd, capture_output=True)
        html = process.stdout.decode('utf-8')
    except subprocess.CalledProcessError as e:
        logging.error("Unable to compile adoc file")
        return None

    try:
        table_start = '<div id="toctitle">Table of Contents</div>'
        toc_start = html.index(table_start) + len(table_start)
        toc_end = html.index('</div>', toc_start)
        toc = html[toc_start: toc_end]
    except ValueError as e:
        logging.error("Unable to find TOC")
        return None

    return toc


def get_element_counts_from_toc(filename: str, dry_run: bool) -> None:
    toc = get_toc(filename, dry_run)
    if not toc:
        return ElementCounts(0, 0, 0)

    def _get_single_element_count_from_toc(element_tag: str) -> int:
        try:
            start = toc.index(element_tag)
            end = toc.index('</ul>', start)
            # We don't care about the final </ul>, just the number of <li> items in between
            element_list = toc[start: end]
            return element_list.count('<li>')
        except ValueError:
            return 0

    attributes = _get_single_element_count_from_toc('Attributes')
    commands = _get_single_element_count_from_toc("Commands")
    events = _get_single_element_count_from_toc("Events")
    return ElementCounts(attributes, commands, events)


def scrape_cluster(filename: str, scraper: str, output_dir: str, dry_run: bool, verify: bool) -> None:
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))
    if not os.path.exists(clusters_output_dir):
        os.makedirs(clusters_output_dir)

    xml_path = get_xml_path(filename, clusters_output_dir)
    cmd = [scraper, 'cluster', '-i', filename, '-o', xml_path, '-nd', '--define', 'in-progress']
    if dry_run:
        print(cmd)
    else:
        subprocess.run(cmd)

    if verify:
        if not dry_run:
            verify_against_toc(filename, xml_path)


def verify_against_toc(adoc_filename: str, xml_filename: str):
    DEFAULT_SPEC_PARSING_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'src', 'python_testing')
    sys.path.append(DEFAULT_SPEC_PARSING_PATH)

    from matter_testing_support import ProblemNotice
    from spec_parsing_support import XmlCluster, add_cluster_data_from_xml
    from global_attribute_ids import GlobalAttributeIds
    import xml.etree.ElementTree as ElementTree

    xml_clusters: dict[int, XmlCluster] = {}
    pure_base_clusters: dict[str, XmlCluster] = {}
    ids_by_name: dict[str, int] = {}
    problems: list[ProblemNotice] = []
    try:
        tree = ElementTree.parse(xml_filename)
        root = tree.getroot()
        add_cluster_data_from_xml(root, xml_clusters, pure_base_clusters, ids_by_name, problems)
    except ElementTree.ParseError:
        logging.error(f"Error parsing the generated XML from {adoc_filename}")
        return
    except FileNotFoundError:
        logging.error(f'failed to generate an XML file for {adoc_filename} - consider whether this needs to be in the exclude list')
        return

    counts = get_element_counts_from_toc(adoc_filename, dry_run=False)
    cluster_to_check = xml_clusters
    if len(xml_clusters.keys()) != 1:
        if len(pure_base_clusters.keys()) == 1:
            cluster_to_check = pure_base_clusters
        else:
            logging.error(f"Unexpected number of clusters in the file {adoc_filename} - unable to test against TOC")
            for problem in problems:
                print(str(problem))
        return

    ok = True
    for c in cluster_to_check.values():
        non_global_attrs = [a for a in c.attributes if a not in [g.value for g in GlobalAttributeIds]]
        if len(non_global_attrs) != counts.attributes:
            logging.error(
                f"Unexpected number of attributes from {adoc_filename} - TOC lists {counts.attributes}, xml has {len(non_global_attrs)}")
            ok = False
        if (len(c.accepted_commands) + len(c.generated_commands)) != counts.commands:
            logging.error(
                f"Unexpected number of commands from {adoc_filename} - TOC lists {counts.commands}, xml has {len(c.accepted_commands)} accepted, {len(c.generated_commands)} generated")
            ok = False
        if len(c.events) != counts.events:
            logging.error(f"Unexpected number of events from {adoc_filename} - TOC lists {counts.events}, xml has {len(c.events)}")
            ok = False

    if not ok:
        print("TOC:")
        print(get_toc(adoc_filename, False))


@click.command()
@click.option(
    '--scraper',
    required=True,
    type=str,
    help='Path to the location of the scraper tool')
@click.option(
    '--spec-root',
    required=True,
    type=str,
    help='Path to the spec root')
@click.option(
    '--output-dir',
    default=DEFAULT_OUTPUT_DIR,
    help='Path to output xml files')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Flag for dry run')
@click.option(
    '--verify',
    default=False,
    is_flag=True,
    help="Verify cluster scrapes against TOC after scraping"
)
@click.option(
    '--single-cluster',
    type=str,
    help="scrape just this cluster file"
)
def main(scraper, spec_root, output_dir, dry_run, verify, single_cluster):
    # Clusters need to be scraped first because the cluster directory is passed to the device type directory
    if single_cluster:
        scrape_cluster(single_cluster, scraper, output_dir, dry_run, verify)
        return
    scrape_clusters(scraper, spec_root, output_dir, dry_run, verify)
    scrape_device_types(scraper, spec_root, output_dir, dry_run)
    if not dry_run:
        dump_versions(scraper, spec_root, output_dir)


def scrape_clusters(scraper, spec_root, output_dir, dry_run, verify):
    src_dir = os.path.abspath(os.path.join(spec_root, 'src'))
    sdm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'service_device_management'))
    app_clusters_dir = os.path.abspath(os.path.join(src_dir, 'app_clusters'))
    dm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'data_model'))
    media_clusters_dir = os.path.abspath(os.path.join(app_clusters_dir, 'media'))
    dm_clusters_list = ['ACL-Cluster.adoc', 'Binding-Cluster.adoc', 'bridge-clusters.adoc',
                        'Descriptor-Cluster.adoc', 'Group-Key-Management-Cluster.adoc', 'ICDManagement.adoc',
                        'Label-Cluster.adoc']
    sdm_exclude_list = ['AdminAssistedCommissioningFlows.adoc', 'BulkDataExchange.adoc', 'CommissioningFlows.adoc',
                        'DeviceCommissioningFlows.adoc', 'DistributedComplianceLedger.adoc', 'OTAFileFormat.adoc', 'OTASoftwareUpdate.adoc',
                        'Resource_Minima.adoc', 'WiFiPerDeviceCredentials.adoc', 'ClientSideLocalization.adoc']
    app_exclude_list = ['appliances.adoc', 'closures.adoc', 'general.adoc',
                        'hvac.adoc', 'lighting.adoc', 'meas_and_sense.adoc', 'robots.adoc', 'energy_management.adoc', 'network_infrastructure.adoc']
    media_exclude_list = ['media.adoc', 'VideoPlayerArchitecture.adoc']

    def scrape_all_clusters(dir: str, exclude_list: list[str] = []) -> None:
        for filename in glob.glob(f'{dir}/*.adoc'):
            if os.path.basename(filename) in exclude_list:
                continue
            scrape_cluster(filename, scraper, output_dir, dry_run, verify)

    scrape_all_clusters(sdm_clusters_dir, sdm_exclude_list)
    scrape_all_clusters(app_clusters_dir, app_exclude_list)
    scrape_all_clusters(media_clusters_dir, media_exclude_list)
    for f in dm_clusters_list:
        filename = f'{dm_clusters_dir}/{f}'
        scrape_cluster(filename, scraper, output_dir, dry_run, verify)


def scrape_device_types(scraper, spec_root, output_dir, dry_run):
    device_type_dir = os.path.abspath(os.path.join(spec_root, 'src', 'device_types'))
    device_types_output_dir = os.path.abspath(os.path.join(output_dir, 'device_types'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))

    if not os.path.exists(device_types_output_dir):
        os.makedirs(device_types_output_dir)

    def scrape_device_type(filename: str) -> None:
        xml_path = get_xml_path(filename, device_types_output_dir)
        cmd = [scraper, 'devicetype', '-c', clusters_output_dir, '-nd', '-i', filename, '-o', xml_path]
        if dry_run:
            print(cmd)
        else:
            print(' '.join(cmd))
            subprocess.run(cmd)

    exclude_list = [r"section_*"]
    for filename in glob.glob(f'{device_type_dir}/*.adoc'):
        for exclude in exclude_list:
            if not re.match(exclude, os.path.basename(filename)):
                scrape_device_type(filename)


def dump_versions(scraper, spec_root, output_dir):
    sha_file = os.path.abspath(os.path.join(output_dir, 'spec_sha'))
    out = subprocess.run(['git', 'rev-parse', 'HEAD'], capture_output=True, encoding="utf8", cwd=spec_root)
    sha = out.stdout
    with open(sha_file, 'wt', encoding='utf8') as output:
        output.write(sha)

    scraper_file = os.path.abspath(os.path.join(output_dir, 'scraper_version'))
    out = subprocess.run([scraper, '--version'], capture_output=True, encoding="utf8")
    version = out.stdout
    with open(scraper_file, "wt", encoding='utf8') as output:
        output.write(version)


if __name__ == '__main__':
    main()
