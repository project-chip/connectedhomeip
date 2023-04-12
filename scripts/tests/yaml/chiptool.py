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

import asyncio
import json
import sys

import click
from matter_chip_tool_adapter.decoder import MatterLog
from matter_yamltests.websocket_runner import WebSocketRunner, WebSocketRunnerConfig
from paths_finder import PathsFinder
from runner import CONTEXT_SETTINGS, chiptool, runner_base
from tests_logger import TestColoredLogPrinter, WebSocketRunnerLogger


@click.pass_context
def send_yaml_command(ctx, test_name: str, server_path: str, server_arguments: str, pics: str, commands: list[str]):
    kwargs = {'test_name': test_name, 'pics': pics}

    index = 0
    while len(commands) - index > 1:
        kwargs[commands[index].replace('--', '')] = commands[index+1]
        index += 2
    ctx.invoke(runner_base, **kwargs)

    del ctx.params['commands']
    del ctx.params['pics']

    return ctx.forward(chiptool)


def send_raw_command(command: str, server_path: str, server_arguments: str):
    websocket_runner_hooks = WebSocketRunnerLogger()
    websocket_runner_config = WebSocketRunnerConfig(
        server_path=server_path, server_arguments=server_arguments, hooks=websocket_runner_hooks)
    runner = WebSocketRunner(websocket_runner_config)

    async def send_over_websocket():
        payload = None
        try:
            await runner.start()
            payload = await runner.execute(command)
        finally:
            await runner.stop()
        return payload

    payload = asyncio.run(send_over_websocket())
    json_payload = json.loads(payload)

    log_printer = TestColoredLogPrinter()
    log_printer.print(MatterLog.decode_logs(json_payload.get('logs')))

    success = not bool(len([lambda x: x.get('error') for x in json_payload.get('results')]))
    return success


_DEFAULT_PICS_FILE = 'src/app/tests/suites/certification/ci-pics-values'


def chiptool_runner_options(f):
    f = click.option('--server_path', type=click.Path(exists=True), default=None,
                     help='Path to an websocket server to run at launch.')(f)
    f = click.option('--server_name', type=str, default='chip-tool',
                     help='Name of a websocket server to run at launch.')(f)
    f = click.option('--server_arguments', type=str, default='interactive server',
                     help='Optional arguments to pass to the websocket server at launch.')(f)
    f = click.option('--PICS', type=click.Path(exists=True), show_default=True, default=_DEFAULT_PICS_FILE,
                     help='Path to the PICS file to use.')(f)
    return f


CONTEXT_SETTINGS['ignore_unknown_options'] = True


@click.command(context_settings=CONTEXT_SETTINGS)
@click.argument('commands', nargs=-1)
@chiptool_runner_options
@click.pass_context
def chiptool_py(ctx, commands: list[str], server_path: str, server_name: str, server_arguments: str, pics: str):
    success = False

    if len(commands) > 1 and commands[0] == 'tests':
        success = send_yaml_command(commands[1], server_path, server_arguments, pics, commands[2:])
    else:
        if server_path is None and server_name:
            paths_finder = PathsFinder()
            server_path = paths_finder.get(server_name)
        success = send_raw_command(' '.join(commands), server_path, server_arguments)

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    chiptool_py()
