#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
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
from typing import List

_DEFAULT_CLUSTER_REVISION_ATTRIBUTE = {
    "name": "ClusterRevision",
    "code": 0xFFFD,
    "mfgCode": None,
    "side": "server",
    "type": "int16u",
    "included": 1,
    "storageOption": "NVM",
    "singleton": 0,
    "bounded": 0,
    "defaultValue": "1",
    "reportable": 1,
    "minInterval": 1,
    "maxInterval": 65534,
    "reportableChange": 0
}

_DEFAULT_FEATURE_MAP_ATTRIBUTE = {
    "name": "FeatureMap",
    "code": 0xFFFC,
    "mfgCode": None,
    "side": "server",
    "type": "bitmap32",
    "included": 1,
    "storageOption": "NVM",
    "singleton": 0,
    "bounded": 0,
    "defaultValue": "0",
    "reportable": 1,
    "minInterval": 1,
    "maxInterval": 65534,
    "reportableChange": 0
}


class Mutator:
    def __init__(self):
        pass

    def handle(self, candidate: object):
        pass


class AddMissingManditoryServerClusterAttributes(Mutator):
    def __init__(self, attribute_entry):
        self._attribute_entry = attribute_entry
        super().__init__()

    def handle(self, candidate: object):
        if not isinstance(candidate, dict):
            return

        # We only care about adding manditory attributes.
        if "attributes" not in candidate:
            return

        # If the cluster is not a server or is not enabled we do not enforce adding manidory attribute.
        if (("enabled" not in candidate) or ("side" not in candidate)):
            return
        if (not candidate.get("enabled")) or ("server"
                                              not in candidate.get("side")):
            return

        attributes = candidate.get("attributes", [])

        for attribute in attributes:
            if attribute["code"] == self._attribute_entry["code"]:
                if attribute["name"] != self._attribute_entry["name"]:
                    print(
                        "WARNING: attribute 0x%X has mismatching name %s (should be %s)" %
                        (self._attribute_entry["code"], attribute["name"],
                         self._attribute_entry["name"]))
                    # TODO what do we want to do here?
                    continue
                else:
                    break
        else:
            print(
                "WARNING: Did not find mandatory attribute %s in cluster %s (0x%X)" %
                (self._attribute_entry["name"], candidate["name"], candidate["code"]))
            insert_index = 0
            for attribute in attributes:
                attribute_code = attribute.get("code")

                if attribute_code is not None and attribute_code > self._attribute_entry[
                        "code"]:
                    break

                insert_index += 1

            # Insert the new attribute in the right place, WITH NO RENUMBERING
            new_attrib_list = attributes[0:insert_index]
            new_attrib_list.append(self._attribute_entry)
            new_attrib_list.extend(attributes[insert_index:])

            # Replace the attribute list with the augmented item
            candidate["attributes"] = new_attrib_list


def loadZapfile(filename: str):
    with open(filename, "rt") as infile:
        return json.load(infile)


def saveZapfile(body: object, filename: str):
    with open(filename, "wt+") as outfile:
        return json.dump(body, outfile, indent=2)


def mutateZapbody(body: object, mutators: List[Mutator]):
    work_list = [body]
    while len(work_list):
        current_item = work_list.pop()

        for mutator in mutators:
            mutator.handle(current_item)

        if isinstance(current_item, list):
            for item in current_item:
                work_list.append(item)
        elif isinstance(current_item, dict):
            for item in current_item.values():
                work_list.append(item)


def setupArgumentsParser():
    parser = argparse.ArgumentParser(description='Mutate ZAP files')
    parser.add_argument('zap_filenames', metavar='zap-filename', type=str, nargs='+',
                        help='zapfiles that need mutating')
    parser.add_argument('--add-manditory-attributes', default=False, action='store_true',
                        help="Add missing manditory attributes to server clusters (default: False)")
    return parser.parse_args()


def main():
    args = setupArgumentsParser()

    mutators = []
    if args.add_manditory_attributes:
        add_missing_cluster_revision = AddMissingManditoryServerClusterAttributes(
            _DEFAULT_CLUSTER_REVISION_ATTRIBUTE)
        add_missing_feature_map = AddMissingManditoryServerClusterAttributes(
            _DEFAULT_FEATURE_MAP_ATTRIBUTE)
        mutators.extend([add_missing_cluster_revision, add_missing_feature_map])

    for zap_filename in args.zap_filenames:
        body = loadZapfile(zap_filename)

        print("==== Processing %s ====" % zap_filename)
        mutateZapbody(
            body, mutators=[add_missing_cluster_revision, add_missing_feature_map])
        saveZapfile(body, zap_filename)


if __name__ == "__main__":
    main()
