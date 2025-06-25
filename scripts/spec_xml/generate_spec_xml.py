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

import contextlib
import glob
import json
import os
import re
import subprocess
import textwrap
import xml.etree.ElementTree as ElementTree
from pathlib import Path

import click
from chip.testing.spec_parsing import ConformanceDecision, PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types
from paths import get_chip_root, get_cluster_documentation_file_path, get_data_model_path, get_device_types_documentation_file_path

CURRENT_IN_PROGRESS_DEFINES = [
    "cameras",
    "closures",
    "device-location",
    "endpointuniqueid",
    "energy-drlc",
    "energy-mtrid",
    "energy-price",
    "energy-tariff",
    "hrap-2",
    "hrap-tbrd",
    "hvac-preset-suggestions",
    "hvac-thermostat-events",
    "irrigation-system",
    "metering network-recovery",
    "nfcCommissioning",
    "paftp",
    "rvc-direct-mode",
    "rvc-moreopstates",
    "rvc-vacthenmop",
    "soil-sensor",
    "thermostat-controller",
    "tls",
]


# Replace hardcoded paths with dynamic paths using paths.py functions
DEFAULT_CHIP_ROOT = get_chip_root()


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
    required=True,
    help='Path to output xml files')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Flag for dry run')
@click.option(
    '--include-in-progress',
    type=click.Choice(['All', 'None', 'Current']), default='All')
@click.option(
    '--legacy',
    default=False,
    is_flag=True,
    help='Use the DM editor spec scraper (legacy) rather than alchemy')
def main(scraper, spec_root, output_dir, dry_run, include_in_progress, legacy):
    dump_ids_from_prebuilt_dirs()
    return
    if legacy:
        scrape_clusters(scraper, spec_root, output_dir, dry_run, include_in_progress)
        scrape_device_types(scraper, spec_root, output_dir, dry_run, include_in_progress)
    else:
        scrape_all(scraper, spec_root, output_dir, dry_run, include_in_progress)
    if not dry_run:
        dump_versions(scraper, spec_root, output_dir, legacy)
        dump_json_ids(output_dir)
        dump_ids_from_prebuilt_dirs()


def scrape_all(scraper, spec_root, output_dir, dry_run, include_in_progress):
    print('Generating main spec to get file include list - this may take a few minutes')
    main_out = make_asciidoc('pdf', include_in_progress, spec_root, dry_run)
    print('Generating cluster spec to get file include list - this may take a few minutes')
    cluster_out = make_asciidoc('pdf-appclusters-book', include_in_progress, spec_root, dry_run)
    print('Generating device type library to get file include list - this may take a few minutes')
    device_type_files = make_asciidoc('pdf-devicelibrary-book', include_in_progress, spec_root, dry_run)
    namespace_files = make_asciidoc('pdf-standardnamespaces-book', include_in_progress, spec_root, dry_run)

    cluster_files = main_out + cluster_out
    cmd = [scraper, 'dm', '--dmRoot', output_dir, '--specRoot', spec_root]
    if include_in_progress == 'All':
        cmd.extend(['-a', 'in-progress'])
    elif include_in_progress == 'Current':
        for d in CURRENT_IN_PROGRESS_DEFINES:
            cmd.extend(['-a'])
            cmd.extend([d])

    if (dry_run):
        print(cmd)
        return
    subprocess.run(cmd)
    # Remove all the files that weren't compiled into the spec
    clusters_output_dir = os.path.join(output_dir, 'clusters')
    device_types_output_dir = os.path.abspath(os.path.join(output_dir, 'device_types'))
    namespaces_output_dir = os.path.abspath(os.path.join(output_dir, 'namespaces'))
    for filename in os.listdir(clusters_output_dir):
        # There are a couple of clusters that appear in the same adoc file and they have prefixes.
        # Look for these specifically.
        # For 1.5 onward, we should separate these.
        if "Label-Cluster" in filename or "bridge-clusters" in filename:
            continue
        adoc = os.path.basename(filename).replace('.xml', '.adoc')
        if adoc not in cluster_files:
            print(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(clusters_output_dir, filename))

    for filename in os.listdir(device_types_output_dir):
        # Closures also has multiple device types in one file, should fix this, but for now a workaround:
        if filename.startswith('ClosureBaseDeviceTypes'):
            continue
        adoc = os.path.basename(filename).replace('.xml', '.adoc')
        if adoc not in device_type_files:
            print(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(device_types_output_dir, filename))

    for filename in os.listdir(namespaces_output_dir):
        adoc = os.path.basename(filename).replace('.xml', '.adoc')
        if adoc not in namespace_files:
            print(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(namespaces_output_dir, filename))


def scrape_clusters(scraper, spec_root, output_dir, dry_run, include_in_progress):
    src_dir = os.path.abspath(os.path.join(spec_root, 'src'))
    sdm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'service_device_management'))
    app_clusters_dir = os.path.abspath(os.path.join(src_dir, 'app_clusters'))
    dm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'data_model'))
    media_clusters_dir = os.path.abspath(os.path.join(app_clusters_dir, 'media'))

    clusters_output_dir = os.path.join(output_dir, 'clusters')

    if not os.path.exists(clusters_output_dir):
        os.makedirs(clusters_output_dir)

    print('Generating main spec to get file include list - this may take a few minutes')
    main_out = make_asciidoc('pdf', include_in_progress, spec_root, dry_run)
    print('Generating cluster spec to get file include list - this may take a few minutes')
    cluster_out = make_asciidoc('pdf-appclusters-book', include_in_progress, spec_root, dry_run)

    def scrape_cluster(filename: str) -> None:
        base = Path(filename).stem
        if base not in main_out and base not in cluster_out:
            print(f'Skipping file: {base} as it is not compiled into the asciidoc')
            return
        xml_path = get_xml_path(filename, clusters_output_dir)
        cmd = [scraper, 'cluster', '-i', filename, '-o', xml_path, '-nd']
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
        try:
            next(cluster.iter('clusterIds'))
        except StopIteration:
            print(f'Removing file {xml_path} as it does not include any cluster definitions')
            os.remove(xml_path)
            continue


def scrape_device_types(scraper, spec_root, output_dir, dry_run, include_in_progress):
    device_type_dir = os.path.abspath(os.path.join(spec_root, 'src', 'device_types'))
    device_types_output_dir = os.path.abspath(os.path.join(output_dir, 'device_types'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))

    if not os.path.exists(device_types_output_dir):
        os.makedirs(device_types_output_dir)

    print('Generating device type library to get file include list - this may take a few minutes')
    device_type_output = make_asciidoc('pdf-devicelibrary-book', include_in_progress, spec_root, dry_run)

    def scrape_device_type(filename: str) -> None:
        base = Path(filename).stem
        if base not in device_type_output:
            print(f'Skipping file: {filename} as it is not compiled into the asciidoc')
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


def dump_versions(scraper, spec_root, output_dir, legacy):
    sha_file = os.path.abspath(os.path.join(output_dir, 'spec_sha'))
    tag_file = os.path.abspath(os.path.join(output_dir, 'spec_tag'))
    out = subprocess.run(['git', 'rev-parse', 'HEAD'],
                         capture_output=True, encoding="utf8", cwd=spec_root)
    sha = out.stdout
    with open(sha_file, 'wt', encoding='utf8') as output:
        output.write(sha)
    cmd = ['git', 'show-ref',  '--tags']
    out = subprocess.run(cmd, capture_output=True, encoding="utf-8", cwd=spec_root)
    tags = out.stdout.splitlines()
    tag = [t for t in tags if sha.strip() in t]
    if tag:
        with open(tag_file, 'wt', encoding='utf8') as output:
            output.write(f'{tag[0].split("/")[-1]}\n')
    else:
        print(f"WARNING: no tag found for sha {sha}")
        with contextlib.suppress(FileNotFoundError):
            os.remove(tag_file)

    scraper_file = os.path.abspath(os.path.join(output_dir, 'scraper_version'))
    version_cmd = 'version'
    if legacy:
        version_cmd = '--version'
    out = subprocess.run([scraper, version_cmd],
                         capture_output=True, encoding="utf8")
    version = out.stdout
    with open(scraper_file, "wt", encoding='utf8') as output:
        if legacy:
            output.write(version)
        else:
            output.write(f'alchemy {version}')


def dump_json_ids(output_dir):
    clusters_output_dir = os.path.abspath(
        os.path.join(output_dir, 'clusters'))
    device_types_output_dir = os.path.abspath(
        os.path.join(output_dir, 'device_types'))
    clusters_json_file = os.path.join(clusters_output_dir, 'cluster_ids.json')
    device_type_json_file = os.path.join(device_types_output_dir, 'device_type_ids.json')

    clusters, _ = build_xml_clusters(Path(clusters_output_dir))
    device_types, _ = build_xml_device_types(Path(device_types_output_dir))

    def write_json(elements: dict, path: str) -> None:
        json_dict = {id: c.name for id, c in sorted(elements.items())}

        with open(path, "wt+") as outfile:
            json.dump(json_dict, outfile, indent=4)
    write_json(clusters, clusters_json_file)
    write_json(device_types, device_type_json_file)


IdToNameDict = dict[int, str]
IdToSupportedDict = dict[int, str]


def dump_ids_from_prebuilt_dirs():
    all_clusters: IdToNameDict = {}
    all_device_types: IdToNameDict = {}

    version_clusters: dict[PrebuiltDataModelDirectory, IdToSupportedDict] = {}
    version_device_types: dict[PrebuiltDataModelDirectory, IdToSupportedDict] = {}
    for dir in PrebuiltDataModelDirectory:
        clusters, _ = build_xml_clusters(dir)
        device_types, _ = build_xml_device_types(dir)

        def cluster_support_str(c):
            return "P" if c.is_provisional else "C"

        version_clusters[dir] = {id: cluster_support_str(c) for id, c in clusters.items()}
        # Device types don't currently have provisional markings in the spec
        # But a device type can't be certified if it has mandatory clusters that are provisional
        # TODO: create provisional

        def device_type_support_str(d):
            print(f"checking device type for {d.name} for {dir}")
            dt_mandatory = [id for id, requirement in d.server_clusters.items() if requirement.conformance(
                [], 0, 0).decision == ConformanceDecision.MANDATORY]
            provisional = [clusters[c].name for c in dt_mandatory if clusters[c].is_provisional]
            if provisional:
                print(f"Found provisional mandatory clusters {provisional} in device type {d.name} for revision {dir.dirname}")
                return "P"
            return "C"

        version_device_types[dir] = {id: device_type_support_str(d) for id, d in device_types.items()}

        all_clusters.update({id: c.name for id, c in clusters.items()})
        all_device_types.update({id: d.name for id, d in device_types.items()})

    def print_out_ids(filename: Path, spec_section: str, id_to_name: IdToNameDict, supported: dict[PrebuiltDataModelDirectory, IdToSupportedDict]):
        header = f'# List of currently defined spec {spec_section}\n'
        header += 'This file was **AUTOMATICALLY** generated by `python scripts/generate_spec_xml.py`. DO NOT EDIT BY HAND!\n\n'
        table = '''
                     The following markers are used in this document (matches the ID master list):
                     | State | Description                    |
                     |-------|--------------------------------|
                     | blank | Not supported in this revision |
                     | C     | Certifiable                    |
                     | P     | Provisional                    |

                  '''
        header = header + textwrap.dedent(table)

        all_name_lens = [len(n) for n in id_to_name.values()]
        name_len = max(all_name_lens)
        title_id_decimal = ' ID (Decimal) '
        title_id_hex = ' ID (hex) '
        title_name_raw = ' Name '
        title_name = f'{title_name_raw:<{name_len}}'
        dec_len = len(title_id_decimal)
        hex_len = len(title_id_hex)
        title = f'|{title_id_decimal}|{title_id_hex}|{title_name}|'
        version_len: dict[PrebuiltDataModelDirectory, int] = {}
        hashes = f'|{"-" * dec_len}|{"-" * hex_len}|{"-" * name_len}|'
        for dir in PrebuiltDataModelDirectory:
            tag_path = os.path.join(get_data_model_path(dir), 'spec_tag')
            try:
                with open(tag_path, "r") as tag_file:
                    version = tag_file.read().strip()
            except FileNotFoundError:
                version = dir.dirname
            title += f'{version}|'
            version_len[dir] = len(version)
            hashes += f'{"-" * version_len[dir]}|'
        table_header = f'{title}\n{hashes}\n'

        lines = []
        for id, name in sorted(id_to_name.items()):
            hex_id = f'0x{id:04X}'
            line = f'|{id:<{dec_len}}|{hex_id:<{hex_len}}|{name:<{name_len}}|'
            for dir in PrebuiltDataModelDirectory:
                try:
                    line += f'{supported[dir][id]:<{version_len[dir]}}|'
                except KeyError:
                    line += f'{" " * version_len[dir]}|'
            lines.append(line)

        with open(filename, 'w') as fout:
            fout.write(header)
            fout.write(table_header)
            fout.write("\n".join(lines))

    print_out_ids(get_cluster_documentation_file_path(), "clusters",  all_clusters, version_clusters)
    print_out_ids(get_device_types_documentation_file_path(), "device types", all_device_types, version_device_types)


if __name__ == '__main__':
    main()
