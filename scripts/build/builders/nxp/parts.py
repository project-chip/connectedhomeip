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
import jsonschema
import os
import sys

from dataclasses import dataclass


def validate_json(data: str):
    with open(os.path.join(os.path.dirname(__file__), 'platforms/platform.schema'), 'r') as fd:
        schema = json.load(fd)

    try:
        jsonschema.validate(data, schema)
    except jsonschema.exceptions.ValidationError as err:
        print(f'JSON data is invalid: {err}')
        sys.exit(1)


@dataclass
class NxpBoard:
    """Class for describing NXP board examples support.

    Args:
        name (str): Name of the board, e.g. k32w0.
        base_path (str): Parent folder of reference apps, e.g. k32w/k32w0
                         in examples/lighting-app/nxp/k32w/k32w0
    """
    name: str
    base_path: str


@dataclass
class NxpApp:
    """Class for describing NXP reference apps and supported boards.

    Args:
        name (str): Name of the application as metadata.
        artifacts (dict): Mapping between board and artifact name.
        supported_boards (list(str)): List of boards as strings that support this application.
    """
    name: str
    artifacts: dict
    supported_boards: list

    def add_board(self, board, artifact_name):
        if board not in self.supported_boards:
            self.supported_boards.append(board)
            self.artifacts[board] = artifact_name

    def get_regex(self):
        return '-(' + '|'.join(self.supported_boards) + ')'


class NxpParts:
    """Class for generating NXP parts used by the NXP builder.

    There are three main components: the board, the app and the modifiers.
    The configuration should be defined in a JSON file. All JSON files under
    platforms will be parsed when generate_parts static method is called.
    """

    def __init__(self):
        self.boards = list()
        self.apps = dict()
        self.modifiers = dict()
        # Used by applications that have support for gn args with variable value
        self.var_modifiers = dict()

    def add_board(self, board):
        if board['name'] != 'common':
            self.boards.append(NxpBoard(board['name'], board['base_path']))

        for app in board['apps']:
            name = app["name"]
            if name not in self.apps:
                self.apps[name] = NxpApp(app['name'], dict(), list())
            self.apps[name].add_board(board['name'], app['artifact'])

        if 'modifiers' in board:
            self.modifiers[board['name']] = board['modifiers']

        if 'variable_modifiers' in board:
            self.var_modifiers[board['name']] = board['variable_modifiers']

    def append_modifiers(self, target):
        """Appends the corresponding modifiers to the given target."""
        for board, modifiers in self.modifiers.items():
            for modifier in modifiers:
                part = target.AppendModifier(name=modifier['id'], **modifier['value'])

                rule = f'{board}.*' if board != 'common' else ''
                if 'only_if' in modifier:
                    rule += f'{modifier["only_if"]}'
                part.OnlyIfRe(rule)

                if 'except_if' in modifier:
                    part.ExceptIfRe(modifier['except_if'])

    @staticmethod
    def generate_parts():
        """This function creates the NxpParts instance based on JSON files.

        Supported platforms should be described in a JSON file found under platforms folder.
        Please see the currently supported platforms for a reference JSON format. Each JSON
        platform file will be checked against the platform.schema.
        """
        parts = NxpParts()
        platforms = os.path.join(os.path.dirname(__file__), 'platforms')
        json_files = [_file for _file in os.listdir(platforms) if _file.endswith('.json')]

        for _file in json_files:
            with open(os.path.join(platforms, _file)) as _fd:
                board = json.load(_fd)
                validate_json(board)
                print(f'JSON data is valid for {_file}')
                parts.add_board(board)

        return parts


if __name__ == '__main__':
    parts = NxpParts.generate_parts()
    parts.append_modifiers(None)
