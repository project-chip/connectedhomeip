#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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
import importlib
import os
from pathlib import Path

import click
from matter_testing_support import MatterTestConfig, generate_mobly_test_config


def indent_multiline(multiline: str, num_spaces: int) -> str:
    ''' Indents subsequent lines of a multiline string by num_spaces spaces'''
    s = multiline.split('\n')
    s = [(num_spaces * ' ' + line.lstrip()).rstrip() for line in s]
    return '\n'.join(s).lstrip()


@click.command()
@click.argument('filename', type=click.Path(exists=True))
@click.argument('classname', type=str)
@click.argument('test', type=str)
def main(filename, classname, test):
    module = importlib.import_module(Path(os.path.basename(filename)).stem)
    test_class = getattr(module, classname)
    config = generate_mobly_test_config(MatterTestConfig())
    test_instance = test_class(config)
    steps = test_instance.get_test_steps(test)
    indent = 6
    header_num = f'{"**#**":<{indent}}'
    header_num_step = f'|{header_num} |*TestStep*   '
    s = ('[cols="5%,45%,45%"]\n'
         '|===\n'
         f'{header_num_step}|*Expected Outcome*\n')
    for step in steps:
        step_num = f'|{step.test_plan_number:<{indent}}a|'
        s += f'{step_num}{indent_multiline(step.description, len(step_num))}\n'

        padding = (len(header_num_step) - 1) * ' '
        # add 2 to indent for a| at start
        s += f'{padding}a|{indent_multiline(step.expectation, len(padding)+2)}\n\n'
    s += '|===\n'

    print(s)


if __name__ == "__main__":
    main()
