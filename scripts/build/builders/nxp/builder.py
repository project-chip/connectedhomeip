# Copyright (c) 2021 Project CHIP Authors
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

import os
import sys

from ..gn import GnBuilder


def check_modifiers(modifiers, supported_modifiers):
    """Checks that every modifier is a key in the supported modifiers dictionary.

    If a modifier is not present in the dictionary, it most likely means a gn generation error.
    """
    if not supported_modifiers:
        print((
            'A list with supported variable modifiers is not specified.\n'
            'Either provide one or do not use --variable-modifiers option in build_examples command.'
        ))
        raise ValueError

    supported_ids = [modifier['id'] for modifier in supported_modifiers]
    ids = [modifier.split('=')[0] for modifier in modifiers.split(' ')]
    for id_ in ids:
        if id_ not in supported_ids:
            print(f'"{id_}" arg is not supported. Please check the supported list: {supported_ids}')
            raise ValueError

    return True


def generate_option_from_modifiers(modifiers, supported_modifiers):
    """Generates an aggregated gn option by replacing the modifier target name with the actual gn option.

    For example, if the modifiers is a string with the following value "var-mod=foo another-var-mod=bar" and
    the platform JSON file contains a "variable_modifiers" list entry:
        "variable_modifiers": [
            {
                "name": "Variable modifier",
                "description": "",
                "id": "var-mod",
                "value": {
                    "gn_variable_modifier": ""
                }
            },
            {
                "name": "Another variable modifier",
                "description": "",
                "id": "another-var-mod",
                "value": {
                    "gn_another_variable_modifier": ""
                }
            }
        ]
    then the generated gn option would be "gn_variable_modifier=foo gn_another_variable_modifier=bar".
    """
    gn_options = str()

    for modifier in modifiers.split(' '):
        id_, val = modifier.split('=')
        for supported_modifier in supported_modifiers:
            if id_ == supported_modifier['id']:
                if len(supported_modifier['value']) != 1:
                    print(f'Currently, variable modifier format supports only one entry in the "value" dict. Please check "{id_}" modifier.')
                    raise ValueError
                gn_options += f'{next(iter(supported_modifier["value"]))}={val} '

    return gn_options


class NxpBuilder(GnBuilder):

    def __init__(self, root, runner, **kwargs):
        self.app = kwargs.pop('app')
        self.board = kwargs.pop('board')
        self.var_modifiers = kwargs.pop('var_modifiers')
        self.args = kwargs
        
        build_root = os.path.join(root, 'examples', self.app.name, 'nxp', self.board.base_path)

        super(NxpBuilder, self).__init__(root=build_root, runner=runner)

    def GnBuildArgs(self):
        args = list()

        try:
            args.append(
                '%s_sdk_root="%s"' % (self.board.name, os.environ['NXP_%s_SDK_ROOT' % (self.board.name.upper())])
            )
        except Exception:
            raise Exception('NXP_%s_SDK_ROOT not defined' % (self.board.name.upper()))

        if 'release' in self.args and self.args.pop('release') is True:
            args.append('is_debug=false optimize_debug=true')

        if 'rpc_server' in self.args and self.args.pop('rpc_server') is True:
            args.append('import("//with_pw_rpc.gni") treat_warnings_as_errors=false')

        for key, val in self.args.items():
            args.append(f"{key}={val}")

        if self.options.variable_modifiers is not None:
            try:
                check_modifiers(self.options.variable_modifiers, self.var_modifiers)
                gn_option = generate_option_from_modifiers(self.options.variable_modifiers, self.var_modifiers)
                args.append(gn_option)
            except ValueError:
                sys.exit(1)

        return args

    def build_outputs(self):
        name = self.app.artifacts[self.board.name]
        return {
            f'{name}.elf': os.path.join(self.output_dir, name),
            f'{name}.map': os.path.join(self.output_dir, f'{name}.map')
        }
