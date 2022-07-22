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
    "entry": {
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
    },
    "clusterCode": None,
    "clusterParamKey": "attributes",
    "side": "server"
}

_DEFAULT_FEATURE_MAP_ATTRIBUTE = {
    "entry": {
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
    },
    "clusterCode": None,
    "clusterParamKey": "attributes",
    "side": "server"
}

_TEST_EVENT_TRIGGERS_ENABLED_ATTRIBUTE = {
    "entry": {
        "name": "TestEventTriggersEnabled",
        "code": 8,
        "mfgCode": None,
        "side": "server",
        "type": "boolean",
        "included": 1,
        "storageOption": "External",
        "singleton": 0,
        "bounded": 0,
        "defaultValue": "false",
        "reportable": 1,
        "minInterval": 1,
        "maxInterval": 65534,
        "reportableChange": 0
    },
    "clusterCode": 51,
    "clusterParamKey": "attributes",
    "side": "server"
}

_TEST_EVENT_TRIGGERS_CLIENT_COMMAND = {
    "entry": {
        "name": "TestEventTrigger",
        "code": 0,
        "mfgCode": None,
        "source": "client",
        "incoming": 1,
        "outgoing": 0
    },
    "clusterCode": 51,
    "clusterParamKey": "commands",
    "side": "server"
}


class Mutator:
    def __init__(self):
        pass

    def handle(self, candidate: object):
        pass


class ValidateMandatoryClusterParam(Mutator):
    def __init__(self, param_details, args):
        self._param_entry = param_details["entry"]
        self._cluster_specific_code = param_details["clusterCode"]
        self._param_key = param_details["clusterParamKey"]
        self._cluster_side = param_details["side"]
        self._add_if_missing = args.add_missing_cluster_param
        self._forces_enable = args.force_enable_cluster_param
        self._replace_if_storage_nvm = args.mandatory_attributes_replace_if_storage_nvm
        super().__init__()

    def _addEntry(self, candidate: object):
        param_list = candidate.get(self._param_key, [])
        insert_index = 0
        for entry in param_list:
            code = entry.get("code")
            if code is not None and code > self._param_entry["code"]:
                break

            insert_index += 1

        # Insert the new entry in the right place, with no renumbering
        new_param_list = param_list[0:insert_index]
        new_param_list.append(self._param_entry)
        new_param_list.extend(param_list[insert_index:])

        # Replace the param list with the augmented item
        candidate[self._param_key] = new_param_list

    def _isCandidateValidCluster(self, candidate: object):
        if not isinstance(candidate, dict):
            return False

        # We only care about adding mandatory entires to .
        if self._param_key not in candidate:
            return False

        # Valid clusters must have enabled and side.
        if (("enabled" not in candidate) or ("side" not in candidate)):
            return False

        if self._cluster_side not in candidate.get("side"):
            return False

        # Command clusters do not need to be enabled for it to have an effect on auto generated
        # files based off of the zap file.
        if (self._param_key != "commands") and (not candidate.get("enabled")):
            return False

        if self._cluster_specific_code is not None and self._cluster_specific_code != candidate["code"]:
            return False

        return True

    def _attributeSpecificChecks(self, param: object, cluster_name):
        if not param["included"]:
            print("WARNING: param 0x%X(%s) in cluster %s found, but included is false" %
                  (self._param_entry["code"], self._param_entry["name"], cluster_name))
            if self._forces_enable:
                param["included"] = self._param_entry["included"]

        if param["storageOption"] == "NVM":
            print("WARNING: param 0x%X(%s) in cluster %s found, but storageOption was NVM" %
                  (self._param_entry["code"], self._param_entry["name"], cluster_name))
            if self._replace_if_storage_nvm:
                param["storageOption"] = self._param_entry["storageOption"]

    def _commandSpecificChecks(self, param: object, cluster_name):
        if param["incoming"] != self._param_entry["incoming"]:
            print("WARNING: param 0x%X(%s) in cluster %s found, but incoming field isn't correct" %
                  (self._param_entry["code"], self._param_entry["name"], cluster_name))
            if self._forces_enable:
                param["incoming"] = self._param_entry["incoming"]

        if param["outgoing"] != self._param_entry["outgoing"]:
            print("WARNING: param 0x%X(%s) in cluster %s found, but outgoing field isn't correct" %
                  (self._param_entry["code"], self._param_entry["name"], cluster_name))
            if self._forces_enable:
                param["outgoing"] = self._param_entry["outgoing"]

    def handle(self, candidate: object):
        if not self._isCandidateValidCluster(candidate):
            return

        for param in candidate.get(self._param_key, []):
            if param["code"] != self._param_entry["code"]:
                continue

            if param["name"] != self._param_entry["name"]:
                print(
                    "WARNING: param 0x%X has mismatching name %s (should be %s)" %
                    (self._param_entry["code"], param["name"],
                     self._param_entry["name"]))

            if self._param_key == "attributes":
                self._attributeSpecificChecks(param, candidate["name"])
            elif self._param_key == "commands":
                self._commandSpecificChecks(param, candidate["name"])
            break
        else:
            print(
                "WARNING: Did not find mandatory param %s in cluster %s (0x%X)" %
                (self._param_entry["name"], candidate["name"], candidate["code"]))
            if self._add_if_missing:
                self._addEntry(candidate)


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
    parser.add_argument('--add-missing-cluster-param', default=False, action='store_true',
                        help="Add missing mandatory cluster parameters (default: False)")
    parser.add_argument('--force-enable-cluster-param', default=False, action='store_true',
                        help="If mandatory cluster paramater is not missing and not enabled, enable it (default: False)")
    parser.add_argument('--mandatory-attributes-replace-if-storage-nvm', default=False, action='store_true',
                        help="Enforce mandatory attribute use default storage type (default: False)")
    return parser.parse_args()


def main():
    args = setupArgumentsParser()

    mutators = []
    add_missing_cluster_revision = ValidateMandatoryClusterParam(
        _DEFAULT_CLUSTER_REVISION_ATTRIBUTE, args)
    add_missing_feature_map = ValidateMandatoryClusterParam(
        _DEFAULT_FEATURE_MAP_ATTRIBUTE, args)
    add_general_diagnostic_test_event_trigger_enabled = ValidateMandatoryClusterParam(
        _TEST_EVENT_TRIGGERS_ENABLED_ATTRIBUTE, args)
    add_general_diagnostic_test_event_trigger_command = ValidateMandatoryClusterParam(
        _TEST_EVENT_TRIGGERS_CLIENT_COMMAND, args)

    mutators.extend([add_missing_cluster_revision, add_missing_feature_map,
                    add_general_diagnostic_test_event_trigger_enabled, add_general_diagnostic_test_event_trigger_command])

    for zap_filename in args.zap_filenames:
        body = loadZapfile(zap_filename)

        print("==== Processing %s ====" % zap_filename)
        mutateZapbody(
            body, mutators=mutators)
        saveZapfile(body, zap_filename)


if __name__ == "__main__":
    main()
