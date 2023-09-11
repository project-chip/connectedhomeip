#!/usr/bin/env python
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

"""
Writes a JSON file containing build configuration information.

See build/chip/java/tests/expected_output/* for example build configuration
files.
"""

import json
import optparse
import os
import sys


def LoadBuildConfigs(paths):
    build_configs = []
    for path in paths:
        with open(path, 'r') as file:
            build_configs.append(json.load(file))
    return build_configs


def ParseGnList(value):
    if not value:
        return []
    if value.startswith('[') and value.endswith(']'):
        gn_list = value.strip("[]").replace(
            "\"", "").replace(" ", "").split(",")
        if not gn_list[0]:
            return []
        else:
            return gn_list


def GetAllDependentJars(deps_configs_data):
    configs_to_process = deps_configs_data
    deps_jars = set()

    while configs_to_process:
        deps_config = configs_to_process.pop()
        child_configs = LoadBuildConfigs(
            deps_config['deps_info']['deps_configs'])
        deps_jars.add(deps_config['deps_info']['jar_path'])

        configs_to_process += child_configs

    return deps_jars


def main(argv):
    parser = optparse.OptionParser()
    parser.add_option('--build-config', help='Path to build_config output')
    parser.add_option('--deps-configs',
                      help='GN-list of dependent build_config files')
    parser.add_option('--jar-path', help='Path to the .jar')
    options, args = parser.parse_args(argv)

    deps_configs_list = ParseGnList(options.deps_configs)
    deps_configs_data = LoadBuildConfigs(deps_configs_list)

    deps_jars_set = GetAllDependentJars(deps_configs_data)

    config = {
        "deps_info": {
            "name": os.path.basename(options.build_config),
            "jar_path": options.jar_path,
            # List of configs depended on by this config. Not recursive.
            "deps_configs": deps_configs_list,
            # List of all jars needed by all dependencies of this config (recursive).
            "deps_jars": list(deps_jars_set)
        }
    }

    with open(options.build_config, 'w') as file:
        json.dump(config, file)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
