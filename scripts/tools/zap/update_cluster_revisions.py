#!/usr/bin/env python3
#
#    Copyright (c) 2020 Project CHIP Authors
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
import json
import logging
import multiprocessing
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

BASIC_INFORMATION_CLUSTER_ID = int("0x0039", 16)
CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../../..'))
EXCLUDE_FROM_UPDATE_DICTIONARY = {
    BASIC_INFORMATION_CLUSTER_ID: ["lighting-app-data-mode-no-unique-id"]
}


@dataclass
class ClusterInfo():
    endpoint_id: int
    cluster_code: int
    cluster_spec_revision: int
    cluster_name: str
    json_attribute: object

    def cluster_revision(self):
        return int(self.json_attribute["defaultValue"])

    def __str__(self):
        return ('Endpoint: %d cluster_code: %s cluster_revision: %d cluster_spec_revision: %d name: %s' % (self.endpoint_id, hex(self.cluster_code), self.cluster_revision(), self.cluster_spec_revision, self.cluster_name))

    def update_cluster_revision(self):
        self.json_attribute["defaultValue"] = self.cluster_spec_revision


def getTargets(cluster_id: int):
    ROOTS_TO_SEARCH = [
        './examples',
        './src/controller/data_model',
        './scripts/tools/zap/tests/inputs',
    ]

    targets = []
    for root in ROOTS_TO_SEARCH:
        for filepath in Path(root).rglob('*.zap'):
            targets.append(filepath)

    if cluster_id in EXCLUDE_FROM_UPDATE_DICTIONARY:
        for target_to_exclude in EXCLUDE_FROM_UPDATE_DICTIONARY[cluster_id]:
            targets = [target for target in targets if target_to_exclude not in target.parts]

    return targets


def get_outdated_clusters(data: object, xml_clusters: dict, args) -> list[ClusterInfo]:
    result = []
    for endpoint in data.get("endpointTypes", []):
        endpoint_id = endpoint.get("id") - 1
        for cluster in endpoint.get("clusters", []):
            # Skip cluster if user passed a cluster id and it doesn't match
            if args.cluster_id is not None and cluster['code'] != args.cluster_id:
                continue
            for attribute in cluster.get("attributes", []):
                if attribute.get("name") != "ClusterRevision" or attribute.get("storageOption") != "RAM":
                    continue
                try:
                    cluster_revision = int(attribute.get("defaultValue"))
                    spec_revision = xml_clusters[cluster.get("code")].revision
                except (KeyError, ValueError):
                    continue
                # Filter in outdated clusters only
                if (cluster_revision == spec_revision):
                    break
                # If old_revision is present, filter in matching only
                if (args.old_revision is not None and cluster_revision != args.old_revision):
                    break
                cluster_info = ClusterInfo(endpoint_id=endpoint_id, cluster_code=cluster.get("code"),
                                           cluster_spec_revision=spec_revision, cluster_name=cluster.get("name"), json_attribute=attribute)
                result.append(cluster_info)
    return result


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def runArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Update the ClusterRevision for a chosen cluster in all .zap files')
    parser.add_argument('--cluster-id', required='--new-revision' in sys.argv, default=None, action='store',
                        help='The id of the cluster, as hex, for which the cluster revision should be updated. If omitted, all outdated clusters are updated.')
    parser.add_argument('--new-revision', required='--old-revision' in sys.argv, default=None, action='store',
                        help='The new cluster revision as a decimal integer. If omitted, the cluster revision will be updated to the latest according to the specification')
    parser.add_argument('--old-revision', default=None, action='store',
                        help='If set, only clusters with this old revision will be updated. This is a decimal integer.')
    parser.add_argument('--dry-run', default=False, action='store_true',
                        help="Don't do any generation, just log what .zap files would be updated (default: False)")
    parser.add_argument('--parallel', action='store_true')
    parser.add_argument('--no-parallel', action='store_false', dest='parallel')
    parser.set_defaults(parallel=True)

    args = parser.parse_args()

    if args.cluster_id:
        args.cluster_id = int(args.cluster_id, 16)

    return args


def isClusterRevisionAttribute(attribute):
    if attribute['mfgCode'] is not None:
        return False

    if attribute['code'] != 0xFFFD:
        return False

    if attribute['name'] != "ClusterRevision":
        logging.error("Attribute has ClusterRevision id but wrong name")
        return False

    return True


def updateOne(item):
    """
    Helper method that may be run in parallel to update a single target.
    """
    (args, target, spec_xml_clusters) = item

    print(f"Will try to update: {target}")
    if args.dry_run:
        return

    with open(target, "r") as file:
        data = json.load(file)

    for endpointType in data['endpointTypes']:
        for cluster in endpointType['clusters']:
            if cluster['mfgCode'] is None and cluster['code'] == args.cluster_id:
                for attribute in cluster['attributes']:
                    if isClusterRevisionAttribute(attribute):
                        if args.old_revision is None or attribute['defaultValue'] == args.old_revision:
                            attribute['defaultValue'] = args.new_revision

    with open(target, "w") as file:
        json.dump(data, file)

    # Now run convert.py on the file to have ZAP reformat it however it likes.
    subprocess.check_call(['./scripts/tools/zap/convert.py', target])


def updateOneToLatest(item):
    """
    Helper method that may be run in parallel to update all clusters in a single .zap file to the latest revision according to the spec.
    """
    (args, target, spec_xml_clusters) = item

    with open(target, "r") as file:
        data = json.load(file)

    print("Checking for outdated cluster revisions on: %s" % target)

    outdated_clusters = get_outdated_clusters(data, spec_xml_clusters, args)
    print("%d found!" % len(outdated_clusters))
    print(*outdated_clusters, sep='\n')

    if args.dry_run:
        return

    # Update outdated cluster revisions according to the spec
    for cluster in outdated_clusters:
        cluster.update_cluster_revision()

    # Check there's no longer any outdated cluster
    assert (not get_outdated_clusters(data, spec_xml_clusters, args))

    # If found outdated clusters, then save and reformat zap
    if (outdated_clusters):
        print('Cluster revisions updated successfully!\n')
        with open(target, "w") as file:
            json.dump(data, file)

        # Now run convert.py on the file to have ZAP reformat it however it likes.
        subprocess.check_call(['./scripts/tools/zap/convert.py', target])


def main():
    checkPythonVersion()

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s'
    )

    args = runArgumentsParser()

    print("**WARNING**: This tool only updates the revision number. Please ensure any new attributes, events or commands are implemented accordingly.")

    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets(args.cluster_id if args.cluster_id else 0)

    if args.new_revision:
        update_func = updateOne
        items = [(args, target, None) for target in targets]
    else:
        update_func = updateOneToLatest
        try:
            from chip.testing.spec_parsing import build_xml_clusters
        except ImportError:
            print("Couldn't import 'chip.testing.spec_parsing'. Try building/activating your python environment: ./scripts/build_python.sh -i out/python_env && source out/python_env/bin/activate)")
            return 1
        spec_xml_clusters, problems = build_xml_clusters()
        items = [(args, target, spec_xml_clusters) for target in targets]

    if args.parallel:
        # Ensure each zap run is independent
        os.environ['ZAP_TEMPSTATE'] = '1'
        with multiprocessing.Pool() as pool:
            for _ in pool.imap_unordered(update_func, items):
                pass
    else:
        for item in items:
            update_func(item)


if __name__ == '__main__':
    main()
