#!/usr/bin/env -S python3 -B

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

import coloredlogs
import click
import logging
import os
import sys
import typing

from pathlib import Path
from enum import Enum, auto
from dataclasses import dataclass

sys.path.append(os.path.abspath(os.path.dirname(__file__)))

DEFAULT_CHIP_ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

class TestTarget(Enum):
    ALL_CLUSTERS = auto()
    TV = auto()

@dataclass
class TestDefinition:
    yaml_file: str
    name: str
    target: TestTarget

@dataclass
class RunContext:
    tests: typing.List[TestDefinition]

def AllTests(root: str):
    """Gets all the tests that can be found in the ROOT directory based on
       yaml file names.
    """
    for path in Path(os.path.join(root, 'src', 'app',  'tests', 'suites')).rglob("*.yaml"):
        logging.debug('Found YAML: %s' % path)

        # grab the name without the extension
        name = path.stem.lower()

        if name.startswith('tv_'):
            target = TestTarget.ALL_CLUSTERS
            name = 'tv-'  + name[3:]
        elif name.startswith('test_'):
            target = TestTarget.TV
            name = 'app-'  + name[5:]
        else:
            continue
        
        yield TestDefinition(yaml_file=path, name=name,target=target)


@click.group(chain=True)
@click.option(
    '--log-level',
    default='WARN',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--target',
    default=['all'],
    multiple=True,
    help='Test to run (use "all" to run all tests)'
)
@click.option(
    '--no-log-timestamps',
    default=False,
    is_flag=True,
    help='Skip timestaps in log output')
@click.option(
    '--root',
    default=DEFAULT_CHIP_ROOT,
    help='Default directory path for CHIP. Used to determine what tests exist')
@click.pass_context
def main(context, log_level, target, no_log_timestamps, root):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    # Figures out selected test that match the given name(s)
    tests = [test for test in AllTests(root)]
    if 'all' not in target:
        tests = [test for test in tests if test.name.upper() in target]
    tests.sort(key=lambda x: x.name)
    
    context.obj = RunContext(tests=tests)

@main.command(
    'list', help='List available test suites')
@click.pass_context
def cmd_generate(context):
    for test in context.obj.tests:
        print(test.name)


if __name__ == '__main__':
    main()
