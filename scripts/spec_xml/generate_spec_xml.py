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
import json
import logging
import os
import subprocess
import sys
import textwrap
from pathlib import Path

import click
import paths
from lxml import etree

from matter.testing.conformance import ConformanceDecision
from matter.testing.spec_parsing import build_xml_clusters, build_xml_device_types

# Replace hardcoded paths with dynamic paths using paths.py functions
DEFAULT_CHIP_ROOT = paths.get_chip_root()
try:
    import generate_data_model_xmls_gni
except ModuleNotFoundError:
    sys.path.append(os.path.join(paths.get_python_testing_path(), 'matter_testing_infrastructure'))
    import generate_data_model_xmls_gni


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
        logging.info(cmd)
        return ''
    else:
        ret = subprocess.check_output(cmd, cwd=spec_dir).decode('UTF-8').rstrip()
        logging.info(ret)
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
    '--skip-scrape',
    help='Runs only the updates that happen after the scrape from the spec repo',
    is_flag=True,
    default=False,
)
def main(scraper, spec_root, output_dir, dry_run, include_in_progress, skip_scrape):
    '''
    This script is used to generate the data model XML files found in the SDK
    data_model directory.

    This script runs against a checked out version of the specification
    repo (https://github.com/CHIP-Specifications/connectedhomeip-spec).
    Check out the desired spec revision, then pass the root of
    the checkout to the script in the --spec-root parameter.

    This script can set the desired include flags for the data model
    files. For in-progress (ballot) specification revisions, the
    include level will be "Current". This sets the include flags to
    the set of flags at the top of this file. When generating files for
    an in-progress spec, ensure the include flags match the set of
    includes noted in the ballot. For specifications that are being
    finalized, where the includes have been removed from the spec in
    preparation for release, use "None". "All" can be used to get a
    full tip-of-tree spec data model. None of the files checked into
    the repo currently use "All".

    The output directory will be data_model/<desired_revision>, passed
    in through the --output-dir flag.

    This script uses alchemy (https://github.com/project-chip/alchemy) for
    creating the data model XML files. Download the desired release package
    and pass the location of the alchemy executable in the --scraper parameter.

    Information on structuring PRs for data model files and adding new files
    to the supporting test script can be found in the README.md file in
    the data_model folder.
    '''
    if not skip_scrape:
        scrape_all(scraper, spec_root, output_dir, dry_run, include_in_progress)
        if not dry_run:
            dump_versions(scraper, spec_root, output_dir)
    if not dry_run:
        cleanup_old_spec_dms(output_dir)
        dump_json_ids(output_dir)
        dump_ids_from_data_model_dirs()
        # Update all the files in the python wheels
        generate_data_model_xmls_gni.generate_gni_file()


def scrape_all(scraper, spec_root, output_dir, dry_run, include_in_progress):
    logging.info('Generating main spec to get file include list - this may take a few minutes')
    main_out = make_asciidoc('pdf', include_in_progress, spec_root, dry_run)
    logging.info('Generating cluster spec to get file include list - this may take a few minutes')
    cluster_out = make_asciidoc('pdf-appclusters-book', include_in_progress, spec_root, dry_run)
    logging.info('Generating device type library to get file include list - this may take a few minutes')
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
        logging.info(cmd)
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
            logging.info(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(clusters_output_dir, filename))

    for filename in os.listdir(device_types_output_dir):
        # Closures also has multiple device types in one file, should fix this, but for now a workaround:
        if filename.startswith('ClosureBaseDeviceTypes'):
            continue
        adoc = os.path.basename(filename).replace('.xml', '.adoc')
        if adoc not in device_type_files:
            logging.info(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(device_types_output_dir, filename))

    for filename in os.listdir(namespaces_output_dir):
        adoc = os.path.basename(filename).replace('.xml', '.adoc')
        if adoc not in namespace_files:
            logging.info(f'Removing {adoc} as it was not in the generated spec document')
            os.remove(os.path.join(namespaces_output_dir, filename))


def cleanup_old_spec_dms(output_dir):
    ''' There are a few old spec versions that have the same longstanding problems.
        Unfortunately, we don't have specific branches for all these spec versions
        since some were only tagged, and some were re-used for dot releases.
        Applying the fixes here. Alchemy will continue to provide a faithful
        representation of the spec, and going forward, we should correct these
        types of problems in the spec where possible.
    '''
    def _fix_door_lock_device_type_features():
        '''Door lock device type has a typo in the required features
           for the user feature in the door lock cluster element requirements.
           - FPG should be FGP (fingerprint)
        '''
        changed = False
        filename = os.path.join(output_dir, 'device_types', 'DoorLock.xml')
        if not os.path.exists(filename):
            return
        with open(filename, 'rt+') as file:
            tree = etree.parse(file)
            root = tree.getroot()
            for feature_tag in root.iter('feature'):
                if feature_tag.attrib.get('name', '') == 'FPG':
                    feature_tag.attrib['name'] = 'FGP'
                    changed = True
            for condition_tag in root.iter('condition'):
                if condition_tag.attrib.get('name', '') == 'FPG':
                    condition_tag.attrib['name'] = 'FGP'
                    condition_tag.tag = 'feature'
                    changed = True
        if changed:
            tree.write(filename, pretty_print=True, xml_declaration=True, encoding='utf-8')

    def _fix_pre_1_3_base_device_type():
        ''' The 1.2 spec tables in the base device type are all weird because they
            include separate tables for matter and zigbee still. Given that those
            specs are frozen, add in the information from the missing table.
            This is taken directly from the 1.3 base device type specification.
            There were no changes between 1.0 and 1.3 to this device type.
        '''
        missing_pre_1_3_base_device_type_clusters = '''
            <clusters>
            <cluster id="0x001D" name="Descriptor" side="server">
            <mandatoryConform/>
            <features>
                <feature code="" name="TagList">
                <mandatoryConform>
                    <condition name="Duplicate"/>
                </mandatoryConform>
                </feature>
            </features>
            </cluster>
            <cluster id="0x001E" name="Binding" side="server">
            <mandatoryConform>
                <andTerm>
                <condition name="Simple"/>
                <condition name="Client"/>
                </andTerm>
            </mandatoryConform>
            </cluster>
            <cluster id="0x0040" name="Fixed Label" side="server">
            <optionalConform/>
            </cluster>
            <cluster id="0x0041" name="User Label" side="server">
            <optionalConform/>
            </cluster>
        </clusters>
        '''
        changed = False
        filename = os.path.join(output_dir, 'device_types', 'BaseDeviceType.xml')
        if not os.path.exists(filename):
            return
        with open(filename, 'rt+') as file:
            tree = etree.parse(file)
            root = tree.getroot()
            if root.find('clusters') is None:
                # Cluster info is missing and we need to add it
                new_xml = etree.fromstring(missing_pre_1_3_base_device_type_clusters)
                device_type = root.find('deviceType')
                if device_type is None:
                    logging.error("Unable to locate device type tag in BaseDeviceType")
                    return
                device_type.append(new_xml)
                changed = True
        if changed:
            tree.write(filename, pretty_print=True, xml_declaration=True, encoding='utf-8')

    def _fix_sit_lit_type():
        # In some spec revisions, the SIT and LIT conditions are listed as features.
        # This is probably fixable in alchemy, but let's just fix here until that
        # gets pushed through.
        changed = False
        filename = os.path.join(output_dir, 'device_types', 'RootNodeDeviceType.xml')
        if not os.path.exists(filename):
            return
        with open(filename, 'rt+') as file:
            tree = etree.parse(file)
            root = tree.getroot()
            for tag in root.iter('feature'):
                if tag.attrib.get('name', '') in ['SIT', 'LIT']:
                    # These should be conditions
                    tag.tag = 'condition'
                    changed = True
        if changed:
            tree.write(filename, pretty_print=True, xml_declaration=True, encoding='utf-8')

    _fix_door_lock_device_type_features()
    _fix_pre_1_3_base_device_type()
    _fix_sit_lit_type()


def dump_versions(scraper, spec_root, output_dir):
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
        logging.warning(f"WARNING: no tag found for sha {sha}")
        with contextlib.suppress(FileNotFoundError):
            os.remove(tag_file)

    scraper_file = os.path.abspath(os.path.join(output_dir, 'scraper_version'))
    version_cmd = 'version'
    out = subprocess.run([scraper, version_cmd],
                         capture_output=True, encoding="utf8")
    version = out.stdout
    with open(scraper_file, "wt", encoding='utf8') as output:
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
            outfile.write('\n')
    write_json(clusters, clusters_json_file)
    write_json(device_types, device_type_json_file)


IdToNameDict = dict[int, str]
IdToSupportedDict = dict[int, str]


def dump_ids_from_data_model_dirs():
    all_clusters: IdToNameDict = {}
    all_device_types: IdToNameDict = {}

    version_clusters: dict[str, IdToSupportedDict] = {}
    version_device_types: dict[str, IdToSupportedDict] = {}
    data_model_path = paths.get_data_model_path()
    data_model_dirs = [d for d in os.listdir(data_model_path) if os.path.isdir(os.path.join(data_model_path, d))]
    # Don't include master right now - it's in progress, not official
    data_model_dirs = sorted([d for d in data_model_dirs if 'master' not in d])
    for dir in data_model_dirs:
        clusters, _ = build_xml_clusters(Path(os.path.join(data_model_path, dir, 'clusters')))
        device_types, _ = build_xml_device_types(Path(os.path.join(data_model_path, dir, 'device_types')))

        def cluster_support_str(c):
            return "P" if c.is_provisional else "C"

        version_clusters[dir] = {id: cluster_support_str(c) for id, c in clusters.items()}
        # Device types don't currently have provisional markings in the spec
        # But a device type can't be certified if it has mandatory clusters that are provisional
        # TODO: create provisional

        def device_type_support_str(d):
            logging.info(f"checking device type for {d.name} for {dir}")
            dt_mandatory = [id for id, requirement in d.server_clusters.items() if requirement.conformance(
                [], 0, 0).decision == ConformanceDecision.MANDATORY]
            provisional = [clusters[c].name for c in dt_mandatory if clusters[c].is_provisional]
            if provisional:
                logging.info(f"Found provisional mandatory clusters {provisional} in device type {d.name} for revision {dir}")
                return "P"
            return "C"

        version_device_types[dir] = {id: device_type_support_str(d) for id, d in device_types.items()}

        all_clusters.update({id: c.name for id, c in clusters.items()})
        all_device_types.update({id: d.name for id, d in device_types.items()})

    def print_out_ids(filename: Path, spec_section: str, id_to_name: IdToNameDict, supported: dict[str, IdToSupportedDict]):
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
        version_len: dict[str, int] = {}
        hashes = f'|{"-" * dec_len}|{"-" * hex_len}|{"-" * name_len}|'
        for dir in supported.keys():
            tag_path = os.path.join(paths.get_data_model_path(), dir, 'spec_tag')
            try:
                with open(tag_path, "r") as tag_file:
                    version = tag_file.read().strip()
            except FileNotFoundError:
                version = dir
            title += f'{version}|'
            version_len[dir] = len(version)
            hashes += f'{"-" * version_len[dir]}|'
        table_header = f'{title}\n{hashes}\n'

        lines = []
        for id, name in sorted(id_to_name.items()):
            hex_id = f'0x{id:04X}'
            line = f'|{id:<{dec_len}}|{hex_id:<{hex_len}}|{name:<{name_len}}|'
            for dir in supported.keys():
                try:
                    line += f'{supported[dir][id]:<{version_len[dir]}}|'
                except KeyError:
                    line += f'{" " * version_len[dir]}|'
            lines.append(line)

        with open(filename, 'w') as fout:
            fout.write(header)
            fout.write(table_header)
            fout.write("\n".join(lines))
            fout.write("\n")

    print_out_ids(paths.get_cluster_documentation_file_path(), "clusters",  all_clusters, version_clusters)
    print_out_ids(paths.get_device_types_documentation_file_path(), "device types", all_device_types, version_device_types)


if __name__ == '__main__':
    main()
