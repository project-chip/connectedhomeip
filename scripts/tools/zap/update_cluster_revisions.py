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
from pathlib import Path

BASIC_INFORMATION_CLUSTER_ID = int("0x0039", 16)
CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../../..'))
EXCLUDE_FROM_UPDATE_DICTIONARY = {
    BASIC_INFORMATION_CLUSTER_ID: ["lighting-app-data-mode-no-unique-id"]
}


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


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def runArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Update the ClusterRevision for a chosen cluster in all .zap files')
    parser.add_argument('--cluster-id', default=None, action='store',
                        help='The id of the cluster, as hex, for which the cluster revision should be updated.')
    parser.add_argument('--new-revision', default=None, action='store',
                        help='The new cluster revision as a decimal integer')
    parser.add_argument('--old-revision', default=None, action='store',
                        help='If set, only clusters with this old revision will be updated.  This is a decimal integer.')
    parser.add_argument('--dry-run', default=False, action='store_true',
                        help="Don't do any generation, just log what .zap files would be updated (default: False)")
    parser.add_argument('--parallel', action='store_true')
    parser.add_argument('--no-parallel', action='store_false', dest='parallel')
    parser.set_defaults(parallel=True)

    args = parser.parse_args()

    if args.cluster_id is None:
        logging.error("Must have a cluster id")
        sys.exit(1)

    if args.new_revision is None:
        logging.error("Must have a new cluster revision")
        sys.exit(1)

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
    (args, target) = item

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


def main():
    checkPythonVersion()

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s'
    )

    args = runArgumentsParser()

    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets(args.cluster_id)

    if args.dry_run:
        for target in targets:
            print(f"Will try to update: {target}")
        sys.exit(0)

    items = [(args, target) for target in targets]

    if args.parallel:
        # Ensure each zap run is independent
        os.environ['ZAP_TEMPSTATE'] = '1'
        with multiprocessing.Pool() as pool:
            for _ in pool.imap_unordered(updateOne, items):
                pass
    else:
        for item in items:
            updateOne(item)


if __name__ == '__main__':
    main()
