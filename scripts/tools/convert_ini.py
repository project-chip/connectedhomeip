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
import logging
import re
import typing
from configparser import ConfigParser
from os.path import exists

import click


def convert_ini_to_json(ini_dir: str, json_path: str):
    """ Converts chip-tool INI files found in 'ini_dir' into the JSON format expected by the matter python REPL
        and associated test infrastructure.

        Specifically, this will take the three INI files that correspond to the three example CAs (alpha, beta, gamma)
        and munge them together into a single REPL-compatible JSON file.
    """
    python_json_store = {}

    ini_file_paths = ['/chip_tool_config.alpha.ini', '/chip_tool_config.beta.ini', '/chip_tool_config.gamma.ini']
    counter = 1

    for path in ini_file_paths:
        full_path = ini_dir + path
        if (exists(full_path)):
            logging.critical(f"Found chip tool INI file at: {full_path} - Converting...")
            create_repl_config_from_init(ini_file=full_path,
                                         json_dict=python_json_store, replace_suffix=str(counter))
        counter = counter + 1

    json_file = open(json_path, 'w')
    json.dump(python_json_store, json_file, ensure_ascii=True, indent=4)


def create_repl_config_from_init(ini_file: str, json_dict: typing.Dict, replace_suffix: str):
    ''' This updates a provided JSON dictionary to create a REPL compliant configuration store that
        contains the correct 'repl-config' and 'sdk-config' keys built from the provided chip-tool
        INI file that contains the root public keys. The INI file will typically be named
        with the word 'alpha', 'beta' or 'gamma' in the name.

        ini_file:           Path to source INI file
        json_dict:          JSON dictionary to be updated. Multiple passes through this function using
                            the same dictionary is possible.
        replace_suffix:     The credentials in the INI file typically have keys that end with 0. This suffix
                            can be replaced with a different number.
    '''
    if ('repl-config' not in json_dict):
        json_dict['repl-config'] = {}

    if ('caList' not in json_dict['repl-config']):
        json_dict['repl-config']['caList'] = {}

    json_dict['repl-config']['caList'][replace_suffix] = [
        {'fabricId': int(replace_suffix), 'vendorId': 0XFFF1}
    ]

    if ('sdk-config' not in json_dict):
        json_dict['sdk-config'] = {}

    load_ini_into_dict(ini_file=ini_file, json_dict=json_dict['sdk-config'], replace_suffix=replace_suffix)


def load_ini_into_dict(ini_file: str, json_dict: typing.Dict, replace_suffix: str):
    """ Loads the specific INI file containing CA credential information into the provided dictionary. A 'replace_suffix' string
        has to be provided to convert the existing numerical suffix to a different value.

        NOTE: This does not do any conversion of the keys into a format acceptable by the Python REPL environment. Please see
              create_repl_config_from_init above if that is desired.
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
    main(auto_envvar_prefix='CHIP')
