#!/usr/bin/env -S python3 -B

#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
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
import json
from configparser import ConfigParser
import click
import typing
import re


def convert_ini_to_json(ini_dir: str, json_path: str):
    """ Converts chip-tool INI files found in 'ini_dir' into the JSON format expected by the matter python REPL
        and associated test infrastructure.

        Specifically, this will take the three INI files that correspond to the three example CAs (alpha, beta, gamma)
        and munge them together into a single REPL-compatible JSON file.
    """
    python_json_store = {}

    python_json_store['repl-config'] = {
        'fabricAdmins': {
            '1': {
                'fabricId': 1,
                'vendorId': 65521
            },
            '2': {
                'fabricId': 2,
                'vendorId': 65521
            },
            '3': {
                'fabricId': 3,
                'vendorId': 65521
            }
        }
    }

    python_json_store['sdk-config'] = {}

    load_ini_into_dict(ini_file=ini_dir + '/chip_tool_config.alpha.ini',
                       json_dict=python_json_store['sdk-config'], replace_suffix='1')
    load_ini_into_dict(ini_file=ini_dir + '/chip_tool_config.beta.ini',
                       json_dict=python_json_store['sdk-config'], replace_suffix='2')
    load_ini_into_dict(ini_file=ini_dir + '/chip_tool_config.gamma.ini',
                       json_dict=python_json_store['sdk-config'], replace_suffix='3')

    json_file = open(json_path, 'w')
    json.dump(python_json_store, json_file, ensure_ascii=True, indent=4)


def load_ini_into_dict(ini_file: str, json_dict: typing.Dict, replace_suffix: str):
    """ Loads the specific INI file into the provided dictionary. A 'replace_suffix' string
        has to be provided to convert the existing numerical suffix to a different value.
    """
    config = ConfigParser()

    # Enable case-sensitive keys.
    config.optionxform = str

    config.read(ini_file)

    for key in config['Default']:
        value = config['Default'][key]
        key = re.sub(r'(.*?)[0-9]+', r'\1', key) + replace_suffix
        json_dict[key] = value


@click.command()
@click.option('--ini-dir', type=click.Path(exists=True), show_default=True, default='/tmp', help='Path to directory containing INI files that chip-tool uses for its tests')
@click.option('--json-path', type=click.Path(exists=False), show_default=True, default='/tmp/repl-storage.json', help='Path to JSON file used by Python infrastructure')
def main(ini_dir: str, json_path: str):
    convert_ini_to_json(ini_dir, json_path)
    return 0


if __name__ == '__main__':
    main()
