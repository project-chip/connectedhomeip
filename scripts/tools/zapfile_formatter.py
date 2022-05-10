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
    "storageOption": "RAM",
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
    "storageOption": "RAM",
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


class ValidateManditoryServerClusterAttributes(Mutator):
    def __init__(self, attribute_entry, add_if_missing, forces_include, force_storage_default):
        self._attribute_entry = attribute_entry
        self._add_if_missing = add_if_missing
        self._forces_include = forces_include
        self._force_storage_default = force_storage_default
        super().__init__()

    def _addMissingManditoryAttribute(self, candidate: object):
        attributes = candidate.get("attributes", [])
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

        for attribute in candidate.get("attributes", []):
            if attribute["code"] != self._attribute_entry["code"]:
                continue
            if attribute["name"] != self._attribute_entry["name"]:
                print(
                    "WARNING: attribute 0x%X has mismatching name %s (should be %s)" %
                    (self._attribute_entry["code"], attribute["name"],
                     self._attribute_entry["name"]))
                # TODO what do we want to do here?
                continue
            else:
                if not attribute["included"]:
                    print("WARNING: attribute 0x%X(%s) in cluster %s found, but included is false" %
                          (self._attribute_entry["code"], self._attribute_entry["name"], candidate["name"]))
                    if self._forces_include:
                        attribute["included"] = self._attribute_entry["included"]
                if attribute["storageOption"] != self._attribute_entry["storageOption"]:
                    print("WARNING: attribute 0x%X(%s) in cluster %s found, but storageOption isn't expected value of %s" %
                          (self._attribute_entry["code"], self._attribute_entry["name"], candidate["name"], self._attribute_entry["storageOption"]))
                    if self._force_storage_default:
                        attribute["storageOption"] = self._attribute_entry["storageOption"]
                break
        else:
            print(
                "WARNING: Did not find mandatory attribute %s in cluster %s (0x%X)" %
                (self._attribute_entry["name"], candidate["name"], candidate["code"]))
            if self._add_if_missing:
                self._addMissingManditoryAttribute(candidate)


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
    parser.add_argument('--manditory-attributes-add-missing', default=False, action='store_true',
                        help="Add missing manditory attributes to server clusters (default: False)")
    parser.add_argument('--manditory-attributes-force-included', default=False, action='store_true',
                        help="If manditory attribute is not included, include it (default: False)")
    parser.add_argument('--manditory-attributes-force-storage-default', default=False, action='store_true',
                        help="Enforce manditory attribute use default storage type (default: False)")
    return parser.parse_args()


def main():
    args = setupArgumentsParser()

    mutators = []
    add_missing_cluster_revision = ValidateManditoryServerClusterAttributes(
        _DEFAULT_CLUSTER_REVISION_ATTRIBUTE, args.manditory_attributes_add_missing, args.manditory_attributes_force_included, args.manditory_attributes_force_storage_default)
    add_missing_feature_map = ValidateManditoryServerClusterAttributes(
        _DEFAULT_FEATURE_MAP_ATTRIBUTE, args.manditory_attributes_add_missing, args.manditory_attributes_force_included, args.manditory_attributes_force_storage_default)

    mutators.extend([add_missing_cluster_revision, add_missing_feature_map])

    for zap_filename in args.zap_filenames:
        body = loadZapfile(zap_filename)

        print("==== Processing %s ====" % zap_filename)
        mutateZapbody(
            body, mutators=mutators)
        saveZapfile(body, zap_filename)


if __name__ == "__main__":
    main()
