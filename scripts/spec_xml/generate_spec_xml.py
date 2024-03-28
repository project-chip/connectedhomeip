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
import json
import os
import re
import subprocess
import sys

import click

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT_DIR = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'data_model'))
DEFAULT_DOCUMENTATION_FILE = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'docs', 'spec_clusters.md'))


def get_xml_path(filename, output_dir):
    xml = os.path.basename(filename).replace('.adoc', '.xml')
    return os.path.abspath(os.path.join(output_dir, xml))


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
def main(scraper, spec_root, output_dir, dry_run):
    # Clusters need to be scraped first because the cluster directory is passed to the device type directory
    scrape_clusters(scraper, spec_root, output_dir, dry_run)
    scrape_device_types(scraper, spec_root, output_dir, dry_run)
    if not dry_run:
        dump_versions(scraper, spec_root, output_dir)
        dump_cluster_ids(output_dir)


def scrape_clusters(scraper, spec_root, output_dir, dry_run):
    src_dir = os.path.abspath(os.path.join(spec_root, 'src'))
    sdm_clusters_dir = os.path.abspath(
        os.path.join(src_dir, 'service_device_management'))
    app_clusters_dir = os.path.abspath(os.path.join(src_dir, 'app_clusters'))
    dm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'data_model'))
    media_clusters_dir = os.path.abspath(
        os.path.join(app_clusters_dir, 'media'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))
    dm_clusters_list = ['ACL-Cluster.adoc', 'Binding-Cluster.adoc', 'bridge-clusters.adoc',
                        'Descriptor-Cluster.adoc', 'Group-Key-Management-Cluster.adoc', 'ICDManagement.adoc',
                        'Label-Cluster.adoc']
    sdm_exclude_list = ['AdminAssistedCommissioningFlows.adoc', 'BulkDataExchange.adoc', 'CommissioningFlows.adoc',
                        'DeviceCommissioningFlows.adoc', 'DistributedComplianceLedger.adoc', 'OTAFileFormat.adoc']
    app_exclude_list = ['appliances.adoc', 'closures.adoc', 'general.adoc',
                        'hvac.adoc', 'lighting.adoc', 'meas_and_sense.adoc', 'robots.adoc']
    media_exclude_list = ['media.adoc', 'VideoPlayerArchitecture.adoc']

    if not os.path.exists(clusters_output_dir):
        os.makedirs(clusters_output_dir)

    def scrape_cluster(filename: str) -> None:
        xml_path = get_xml_path(filename, clusters_output_dir)
        cmd = [scraper, 'cluster', '-i', filename, '-o',
               xml_path, '-nd', '--define', 'in-progress']
        if dry_run:
            print(cmd)
        else:
            subprocess.run(cmd)

    def scrape_all_clusters(dir: str, exclude_list: list[str] = []) -> None:
        for filename in glob.glob(f'{dir}/*.adoc'):
            if os.path.basename(filename) in exclude_list:
                continue
            scrape_cluster(filename)

    scrape_all_clusters(sdm_clusters_dir, sdm_exclude_list)
    scrape_all_clusters(app_clusters_dir, app_exclude_list)
    scrape_all_clusters(media_clusters_dir, media_exclude_list)
    for f in dm_clusters_list:
        filename = f'{dm_clusters_dir}/{f}'
        scrape_cluster(filename)


def scrape_device_types(scraper, spec_root, output_dir, dry_run):
    device_type_dir = os.path.abspath(
        os.path.join(spec_root, 'src', 'device_types'))
    device_types_output_dir = os.path.abspath(
        os.path.join(output_dir, 'device_types'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))

    if not os.path.exists(device_types_output_dir):
        os.makedirs(device_types_output_dir)

    def scrape_device_type(filename: str) -> None:
        xml_path = get_xml_path(filename, device_types_output_dir)
        cmd = [scraper, 'devicetype', '-c', clusters_output_dir,
               '-nd', '-i', filename, '-o', xml_path]
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
    out = subprocess.run(['git', 'rev-parse', 'HEAD'],
                         capture_output=True, encoding="utf8", cwd=spec_root)
    sha = out.stdout
    with open(sha_file, 'wt', encoding='utf8') as output:
        output.write(sha)

    scraper_file = os.path.abspath(os.path.join(output_dir, 'scraper_version'))
    out = subprocess.run([scraper, '--version'],
                         capture_output=True, encoding="utf8")
    version = out.stdout
    with open(scraper_file, "wt", encoding='utf8') as output:
        output.write(version)


def dump_cluster_ids(output_dir):
    python_testing_path = os.path.abspath(
        os.path.join(DEFAULT_CHIP_ROOT, 'src', 'python_testing'))
    sys.path.insert(0, python_testing_path)
    clusters_output_dir = os.path.abspath(
        os.path.join(output_dir, 'clusters'))

    from spec_parsing_support import build_xml_clusters

    header = '# List of currently defined spec clusters\n'
    header += 'This file was **AUTOMATICALLY** generated by `python scripts/generate_spec_xml.py`. DO NOT EDIT BY HAND!\n\n'

    clusters, problems = build_xml_clusters()
    all_name_lens = [len(c.name) for c in clusters.values()]
    name_len = max(all_name_lens)
    title_id_decimal = ' ID (Decimal) '
    title_id_hex = ' ID (hex) '
    title_name_raw = ' Name '
    title_name = f'{title_name_raw:<{name_len}}'
    dec_len = len(title_id_decimal)
    hex_len = len(title_id_hex)
    title = f'|{title_id_decimal}|{title_id_hex}|{title_name}|\n'
    hashes = f'|{"-" * dec_len}|{"-" * hex_len}|{"-" * name_len}|\n'
    s = title + hashes
    json_dict = {id: c.name for id, c in sorted(clusters.items())}
    for id, cluster in sorted(clusters.items()):
        hex_id = f'0x{id:04X}'
        s += f'|{id:<{dec_len}}|{hex_id:<{hex_len}}|{cluster.name:<{name_len}}|\n'

    with open(DEFAULT_DOCUMENTATION_FILE, 'w') as fout:
        fout.write(header)
        fout.write(s)

    json_file = os.path.join(clusters_output_dir, 'cluster_ids.json')
    with open(json_file, "w") as outfile:
        json.dump(json_dict, outfile, indent=2)


if __name__ == '__main__':
    main()
