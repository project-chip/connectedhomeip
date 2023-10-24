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
import os
import subprocess

import click

DEFAULT_CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFAULT_OUTPUT_DIR = os.path.abspath(os.path.join(DEFAULT_CHIP_ROOT, 'data_model'))


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
    # TODO: Add scrapers for device types

    src_dir = os.path.abspath(os.path.join(spec_root, 'src'))
    sdm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'service_device_management'))
    app_clusters_dir = os.path.abspath(os.path.join(src_dir, 'app_clusters'))
    dm_clusters_dir = os.path.abspath(os.path.join(src_dir, 'data_model'))
    media_clusters_dir = os.path.abspath(os.path.join(app_clusters_dir, 'media'))
    clusters_output_dir = os.path.abspath(os.path.join(output_dir, 'clusters'))
    dm_clusters_list = ['ACL-Cluster.adoc', 'Binding-Cluster.adoc', 'bridge-clusters.adoc',
                        'Descriptor-Cluster.adoc', 'Group-Key-Management-Cluster.adoc', 'Label-Cluster.adoc']
    sdm_exclude_list = ['AdminAssistedCommissioningFlows.adoc', 'BulkDataExchange.adoc', 'CommissioningFlows.adoc',
                        'DeviceCommissioningFlows.adoc', 'DistributedComplianceLedger.adoc', 'OTAFileFormat.adoc']
    app_exclude_list = ['appliances.adoc', 'closures.adoc', 'general.adoc',
                        'hvac.adoc', 'lighting.adoc', 'meas_and_sense.adoc', 'robots.adoc']
    media_exclude_list = ['media.adoc', 'VideoPlayerArchitecture.adoc']

    if not os.path.exists(clusters_output_dir):
        os.makedirs(clusters_output_dir)

    def scrape_cluster(filename: str) -> None:
        xml = os.path.basename(filename).replace('.adoc', '.xml')
        xml_path = os.path.abspath(os.path.join(clusters_output_dir, xml))
        cmd = [scraper, 'cluster', filename, xml_path, '-nd']
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

    # Put the current spec sha into the cluster dir
    sha_file = os.path.abspath(os.path.join(output_dir, 'spec_sha'))
    out = subprocess.run(['git', 'rev-parse', 'HEAD'], capture_output=True, encoding="utf8", cwd=spec_root)
    sha = out.stdout
    with open(sha_file, 'wt', encoding='utf8') as output:
        output.write(sha)


if __name__ == '__main__':
    main()
