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

import click
from chipyaml.adapters.chiptool.decoder import MatterLog
from runner import runner_base
from tests_logger import TestColoredLogPrinter, WebSocketRunnerLogger

from matter.yamltests.websocket_runner import WebSocketRunner, WebSocketRunnerConfig


@click.pass_context
def send_yaml_command(ctx, test_tool, test_name: str, server_path: str, server_arguments: str, show_adapter_logs: bool, specifications_paths: str, pics: str, additional_pseudo_clusters_directory: str, commands: list[str]):
    kwargs = {'test_name': test_name, 'show_adapter_logs': show_adapter_logs, 'specifications_paths': specifications_paths, 'pics': pics,
              'additional_pseudo_clusters_directory': additional_pseudo_clusters_directory}

    # Translate command-line arguments to Click/Python parameter names:
    #   - option to variable name translation (e.g. `--stop-on-error` translates to `stop_on_error`)
    #   - custom mapping for option destination variables (e.g. `--PICS` maps to `pics`,
    #     `--value-wait-extra-duration-ms` maps to `valueWaitExtraDurationMs`)
    #   - manual type conversion (since options are parsed manually as strings, we must cast
    #     to expected types like int or bool to match the function signature)
    #
    # See runner_base definition in scripts/tests/chipyaml/runner.py for the full parameter list and target types.
    #
    # These are known mappings between one argument and the corresponding known name. Everything else
    # would just replace '-' with '_':
    option_mapping = {
        'PICS': 'pics',
        'value-wait-extra-duration-ms': 'valueWaitExtraDurationMs',
    }

    def str_to_bool(val): return val.lower() in ('true', '1', 'yes')

    type_conversions = {
        'valueWaitExtraDurationMs': int,
        'stop_on_error': str_to_bool,
        'use_default_pseudo_clusters': str_to_bool,
    }

    index = 0
    while len(commands) - index > 1:
        key = commands[index].replace('--', '')
        # Map things like `--stop-on-error` to `stop_on_error` since we ctx.invoke directly (need python names)
        mapped_key = option_mapping.get(key, key.replace('-', '_'))
        val = commands[index+1]

        # convert known typed values
        if mapped_key in type_conversions:
            val = type_conversions[mapped_key](val)

        kwargs[mapped_key] = val
        index += 2
    ctx.invoke(runner_base, **kwargs)

    del ctx.params['commands']
    del ctx.params['specifications_paths']
    del ctx.params['pics']
    del ctx.params['additional_pseudo_clusters_directory']

    return ctx.forward(test_tool)


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

    return not bool(len([lambda x: x.get('error') for x in json_payload.get('results')]))
