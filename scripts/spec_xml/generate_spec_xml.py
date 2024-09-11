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
import xml.etree.ElementTree as ElementTree
from pathlib import Path

import click

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT_DIR_1_3 = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'data_model', '1.3'))
DEFAULT_OUTPUT_DIR_IN_PROGRESS = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'data_model', 'in_progress'))
DEFAULT_OUTPUT_DIR_TOT = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'data_model', 'master'))
DEFAULT_DOCUMENTATION_FILE = os.path.abspath(
    os.path.join(DEFAULT_CHIP_ROOT, 'docs', 'spec_clusters.md'))

# questions
# is energy-calendar still in?
# is heat-pump out? wasn't in 0.7
# location-cluster - is this define gone now?
# queuedpreset - is this define gone now?
CURRENT_IN_PROGRESS_DEFINES = ['aliro', 'atomicwrites', 'battery-storage', 'device-location', 'e2e-jf', 'energy-calendar', 'energy-drlc',
                               'energy-management', 'heat-pump', 'hrap-1', 'hvac', 'matter-fabric-synchronization', 'metering', 'secondary-net',
                               'service-area-cluster', 'solar-power', 'tcp', 'water-heater', 'wifiSetup']


def get_xml_path(filename, output_dir):
    xml = os.path.basename(filename).replace('.adoc', '.xml')
    return os.path.abspath(os.path.join(output_dir, xml))


def make_asciidoc(target: str, include_in_progress: str, spec_dir: str, dry_run: bool) -> str:
    cmd = ['make', 'PRINT_FILENAMES=1']
    if include_in_progress == 'All':
        cmd.append('INCLUDE_IN_PROGRESS=1')
    elif include_in_progress == 'Current':
        cmd.append(f'INCLUDE_IN_PROGRESS={" ".join(CURRENT_IN_PROGRESS_DEFINES)}')
    cmd.append(target)
    if dry_run:
        print(cmd)
        return ''
    else:
        ret = subprocess.check_output(cmd, cwd=spec_dir).decode('UTF-8').rstrip()
        print(ret)
        return ret


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
    help='Path to output xml files')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Flag for dry run')
@click.option(
    '--include-in-progress',
    type=click.Choice(['All', 'None', 'Current']), default='All')
def main(scraper, spec_root, output_dir, dry_run, include_in_progress):
    # Clusters need to be scraped first because the cluster directory is passed to the device type directory
    if not output_dir:
        output_dir_map = {'All': DEFAULT_OUTPUT_DIR_TOT, 'None': DEFAULT_OUTPUT_DIR_1_3, 'Current': DEFAULT_OUTPUT_DIR_IN_PROGRESS}
        output_dir = output_dir_map[include_in_progress]
    scrape_clusters(scraper, spec_root, output_dir, dry_run, include_in_progress)
    scrape_device_types(scraper, spec_root, output_dir, dry_run, include_in_progress)
    if not dry_run:
        dump_versions(scraper, spec_root, output_dir)
        dump_cluster_ids(output_dir)


def scrape_clusters(scraper, spec_root, output_dir, dry_run, include_in_progress):
    src_dir = os.path.abspath(os.path.join(spec_root, 'src'))
    sdm_clusters_dir = os.path.abspath(
        os.path.join(src_dir, 'service_device_management'))
    app_clusters_dir = os.path.abspath(os.path.join(src_dir, 'app_clusters'))
    dm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'data_model'))
    media_clusters_dir = os.path.abspath(
        os.path.join(app_clusters_dir, 'media'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))

    if not os.path.exists(clusters_output_dir):
        os.makedirs(clusters_output_dir)

    print('Generating main spec to get file include list - this make take a few minutes')
    main_out = make_asciidoc('pdf', include_in_progress, spec_root, dry_run)
    print('Generating cluster spec to get file include list - this make take a few minutes')
    cluster_out = make_asciidoc('pdf-appclusters-book', include_in_progress, spec_root, dry_run)

    def scrape_cluster(filename: str) -> None:
        base = Path(filename).stem
        if base not in main_out and base not in cluster_out:
            print(f'skipping file: {base} as it is not compiled into the asciidoc')
            return
        xml_path = get_xml_path(filename, clusters_output_dir)
        cmd = [scraper, 'cluster', '-i', filename, '-o',
               xml_path, '-nd']
        if include_in_progress == 'All':
            cmd.extend(['--define', 'in-progress'])
        elif include_in_progress == 'Current':
            cmd.extend(['--define'])
            cmd.extend(CURRENT_IN_PROGRESS_DEFINES)
        if dry_run:
            print(cmd)
        else:
            subprocess.run(cmd)

    def scrape_all_clusters(dir: str, exclude_list: list[str] = []) -> None:
        for filename in glob.glob(f'{dir}/*.adoc'):
            scrape_cluster(filename)

    scrape_all_clusters(dm_clusters_dir)
    scrape_all_clusters(sdm_clusters_dir)
    scrape_all_clusters(app_clusters_dir)
    scrape_all_clusters(media_clusters_dir)

    for xml_path in glob.glob(f'{clusters_output_dir}/*.xml'):
        tree = ElementTree.parse(f'{xml_path}')
        root = tree.getroot()
        cluster = next(root.iter('cluster'))
        # If there's no cluster ID table, this isn't a cluster
        try:
            next(cluster.iter('clusterIds'))
        except StopIteration:
            # If there's no cluster ID table, this isn't a cluster just some kind of intro adoc
            print(f'Removing file {xml_path} as it does not include any cluster definitions')
            os.remove(xml_path)
            continue


def scrape_device_types(scraper, spec_root, output_dir, dry_run, include_in_progress):
    device_type_dir = os.path.abspath(
        os.path.join(spec_root, 'src', 'device_types'))
    device_types_output_dir = os.path.abspath(
        os.path.join(output_dir, 'device_types'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))

    if not os.path.exists(device_types_output_dir):
        os.makedirs(device_types_output_dir)

    print('Generating device type library to get file include list - this make take a few minutes')
    device_type_output = make_asciidoc('pdf-devicelibrary-book', include_in_progress, spec_root, dry_run)

    def scrape_device_type(filename: str) -> None:
        base = Path(filename).stem
        if base not in device_type_output:
            print(f'skipping file: {filename} as it is not compiled into the asciidoc')
            return
        xml_path = get_xml_path(filename, device_types_output_dir)
        cmd = [scraper, 'devicetype', '-c', '-cls', clusters_output_dir,
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

    clusters, problems = build_xml_clusters(clusters_output_dir)
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
        json.dump(json_dict, outfile, indent=4)
        outfile.write('\n')


if __name__ == '__main__':
    main()
