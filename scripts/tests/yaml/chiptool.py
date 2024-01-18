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

import relative_importer  # isort: split # noqa: F401

import sys
from typing import List

import chiptest
import click
from paths_finder import PathsFinder
from runner import CONTEXT_SETTINGS, chiptool
from tests_finder import TestsFinder
from tests_tool import send_raw_command, send_yaml_command

_DEFAULT_EXTENSIONS_DIR = 'scripts/tests/yaml/extensions'
_DEFAULT_PICS_FILE = 'src/app/tests/suites/certification/ci-pics-values'
_DEFAULT_SPECIFICATIONS_DIR = 'src/app/zap-templates/zcl/data-model/chip/*.xml'


def chiptool_runner_options(f):
    f = click.option('--server_path', type=click.Path(exists=True), default=None,
                     help='Path to a websocket server that will be executed to forward parsed command. Most likely you want to use chiptool.')(f)
    f = click.option('--server_name', type=str, default='chip-tool',
                     help='If server_path is not provided, we use this argument to seach various directories within the SDK for a binary that matches this name.')(f)
    f = click.option('--server_arguments', type=str, default='interactive server',
                     help='Arguments to pass to the websocket server at launch.')(f)
    f = click.option('--show_adapter_logs', type=bool, default=False, show_default=True,
                     help='Show additional logs provided by the adapter.')(f)
    f = click.option('--trace_file', type=click.Path(), default=None,
                     help='Optional file path to save the tracing output to.')(f)
    f = click.option('--trace_decode', type=bool, default=True,
                     help='Decode the tracing ouput to a human readable format.')(f)
    f = click.option('--delay-in-ms', '--delayInMs', type=int, default=0, show_default=True,
                     help='Add a delay between each test suite steps.')(f)
    f = click.option('--continueOnFailure', type=bool, default=False, show_default=True,
                     help='Do not stop running the test suite on first error.')(f)
    f = click.option('--specifications_paths', type=click.Path(), show_default=True, default=_DEFAULT_SPECIFICATIONS_DIR,
                     help='Path to a set of files containing clusters definitions.')(f)
    f = click.option('--PICS', type=click.Path(exists=True), show_default=True, default=_DEFAULT_PICS_FILE,
                     help='Path to the PICS file to use.')(f)
    f = click.option('--additional_pseudo_clusters_directory', type=click.Path(), show_default=True, default=_DEFAULT_EXTENSIONS_DIR,
                     help='Path to a directory containing additional pseudo clusters.')(f)
    return f


CONTEXT_SETTINGS['ignore_unknown_options'] = True
CONTEXT_SETTINGS['default_map']['chiptool']['use_test_harness_log_format'] = True


def maybe_update_server_arguments(ctx):
    if ctx.params['trace_file']:
        ctx.params['server_arguments'] += ' --trace_file {}'.format(ctx.params['trace_file'])

    if ctx.params['trace_decode']:
        ctx.params['server_arguments'] += ' --trace_decode 1'

    del ctx.params['trace_file']
    del ctx.params['trace_decode']

    return ctx.params['server_arguments']


def maybe_update_stop_on_error(ctx):
    if ctx.params['continueonfailure']:
        ctx.params['stop_on_error'] = False

    del ctx.params['continueonfailure']


@click.command(context_settings=CONTEXT_SETTINGS)
@click.argument('commands', nargs=-1)
@chiptool_runner_options
@click.pass_context
def chiptool_py(ctx, commands: List[str], server_path: str, server_name: str, server_arguments: str, show_adapter_logs: bool, trace_file: str, trace_decode: bool, delay_in_ms: int, continueonfailure: bool, specifications_paths: str, pics: str, additional_pseudo_clusters_directory: str):
    success = False

    server_arguments = maybe_update_server_arguments(ctx)
    maybe_update_stop_on_error(ctx)

    if len(commands) == 1 and commands[0] == 'list':
        print('List of all individual tests that can be run:')
        for test in chiptest.AllChipToolYamlTests():
            print(f'   {test.name}')

        tests_finder = TestsFinder()
        tests_collections = tests_finder.get_collections()
        if tests_collections:
            print('\nList of all collections tests that can be run:')
            for test in tests_collections:
                print(f'   {test}')
    elif len(commands) > 1 and commands[0] == 'tests':
        success = send_yaml_command(chiptool, commands[1], server_path, server_arguments, show_adapter_logs, specifications_paths, pics,
                                    additional_pseudo_clusters_directory, commands[2:])
    else:
        if server_path is None and server_name:
            paths_finder = PathsFinder()
            server_path = paths_finder.get(server_name)
        success = send_raw_command(' '.join(commands), server_path, server_arguments)

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    chiptool_py()
