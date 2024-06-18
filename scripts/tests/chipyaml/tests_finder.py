#!/usr/bin/env -S python3 -B
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json
import os.path
from typing import List

import click

_JSON_FILE_EXTENSION = '.json'
_YAML_FILE_EXTENSION = '.yaml'
_KNOWN_PREFIX = 'Test_TC_'

_KEYWORD_ALL_TESTS = 'all'
_DEFAULT_DIRECTORY = 'src/app/tests/suites/'

_CI_CONFIGURATION_NAME = 'ciTests'
_MANUAL_CONFIGURATION_NAME = 'manualTests'


class TestsFinder:
    def __init__(self, configuration_directory: str = _DEFAULT_DIRECTORY, configuration_name: str = _CI_CONFIGURATION_NAME):
        self.__test_directory = _DEFAULT_DIRECTORY
        self.__test_collections = self.__get_collections(configuration_directory, configuration_name)

    def get_default_configuration_directory() -> str:
        return _DEFAULT_DIRECTORY

    def get_default_configuration_name() -> str:
        return _CI_CONFIGURATION_NAME

    def get(self, test_name: str) -> List[str]:
        test_names = []

        if self.__test_collections and test_name == _KEYWORD_ALL_TESTS:
            for collection_name in self.__test_collections.get('collection'):
                for name in self.__test_collections.get(collection_name):
                    test_names.append(name)
        elif self.__test_collections and self.__test_collections.get(test_name):
            test_names = self.__test_collections.get(test_name)
        else:
            test_names.append(test_name)

        return self.__get_paths(test_names)

    def get_collections(self) -> List[str]:
        return self.__test_collections

    def __get_collections(self, configuration_directory: str, configuration_name: str) -> List[str]:
        if os.path.isfile(configuration_name):
            configuration_filepath = configuration_name
        elif os.path.isfile(os.path.join(configuration_directory, configuration_name + _JSON_FILE_EXTENSION)):
            configuration_filepath = os.path.join(configuration_directory, configuration_name + _JSON_FILE_EXTENSION)
        else:
            configuration_filepath = None

        collections = None
        if configuration_filepath:
            with open(configuration_filepath) as file:
                data = json.load(file)
                if 'include' in data:
                    include_filepath = os.path.join(os.path.dirname(configuration_filepath), data.get('include'))
                    with open(include_filepath) as included_file:
                        collections = json.load(included_file)
                else:
                    collections = data

                if collections and 'disable' in data:
                    disabled_tests = data.get('disable')
                    for disabled_test in disabled_tests:
                        for collection in collections:
                            if disabled_test in collections.get(collection):
                                collections.get(collection).remove(disabled_test)

        return collections

    def __get_paths(self, test_names: List[str]) -> List[str]:
        paths = []

        for name in test_names:
            for root, dir, files in os.walk(self.__test_directory):
                if name in files:
                    paths.append(os.path.join(root, name))
                elif (name + _YAML_FILE_EXTENSION) in files:
                    paths.append(os.path.join(root, name + _YAML_FILE_EXTENSION))
                elif (_KNOWN_PREFIX + name + _YAML_FILE_EXTENSION) in files:
                    paths.append(os.path.join(root, _KNOWN_PREFIX + name + _YAML_FILE_EXTENSION))

        return paths


def test_finder_options(f):
    f = click.option("--configuration_directory", type=click.Path(exists=True), required=True, show_default=True,
                     default=_DEFAULT_DIRECTORY, help='Path to the directory containing the tests configuration.')(f)
    f = click.option("--configuration_name", type=str, required=True, show_default=True,
                     default=_CI_CONFIGURATION_NAME, help='Name of the collection configuration json file to use.')(f)
    return f


@click.command()
@click.argument('test_name')
@test_finder_options
def run(test_name: str, configuration_directory: str, configuration_name: str):
    """ Find a test or a set of tests."""
    tests_finder = TestsFinder(configuration_directory, configuration_name)
    tests = tests_finder.get(test_name)
    for test in tests:
        print(test)
    print(f'{len(tests)} tests found.')


if __name__ == '__main__':
    run()
