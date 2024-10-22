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
import importlib
import os
import sys
import traceback
from dataclasses import dataclass

import click
from matter_yamltests.definitions import SpecDefinitionsFromPaths
from matter_yamltests.parser import TestParserConfig
from matter_yamltests.parser_builder import TestParserBuilderConfig
from matter_yamltests.parser_config import TestConfigParser
from matter_yamltests.pseudo_clusters.pseudo_clusters import PseudoClusters, get_default_pseudo_clusters
from matter_yamltests.runner import TestRunner, TestRunnerConfig, TestRunnerOptions
from matter_yamltests.websocket_runner import WebSocketRunner, WebSocketRunnerConfig
from paths_finder import PathsFinder
from tests_finder import TestsFinder
from tests_logger import TestParserLogger, TestRunnerLogger, WebSocketRunnerLogger

#
# Options
#

_DEFAULT_CONFIG_NAME = TestsFinder.get_default_configuration_name()
_DEFAULT_CONFIG_DIR = TestsFinder.get_default_configuration_directory()
_DEFAULT_SPECIFICATIONS_DIR = 'src/app/zap-templates/zcl/data-model/chip/*.xml'
_DEFAULT_PICS_FILE = 'src/app/tests/suites/certification/ci-pics-values'


def get_custom_pseudo_clusters(additional_pseudo_clusters_directory: str):
    clusters = get_default_pseudo_clusters()

    if additional_pseudo_clusters_directory:
        sys.path.insert(0, additional_pseudo_clusters_directory)
        for filepath in os.listdir(additional_pseudo_clusters_directory):
            if filepath != '__init__.py' and filepath[-3:] == '.py':
                module = importlib.import_module(f'{filepath[:-3]}')
                constructor = getattr(module, module.__name__)
                if constructor:
                    clusters.add(constructor())

    return clusters


def test_parser_options(f):
    f = click.option('--configuration_name', type=str, show_default=True, default=_DEFAULT_CONFIG_NAME,
                     help='Name of the collection configuration json file to use.')(f)
    f = click.option('--configuration_directory', type=click.Path(exists=True), show_default=True, default=_DEFAULT_CONFIG_DIR,
                     help='Path to the directory containing the tests configuration.')(f)
    f = click.option('--specifications_paths', type=click.Path(), show_default=True, default=_DEFAULT_SPECIFICATIONS_DIR,
                     help='Path to a set of files containing clusters definitions.')(f)
    f = click.option('--PICS', type=click.Path(exists=True), show_default=True, default=_DEFAULT_PICS_FILE,
                     help='Path to the PICS file to use.')(f)
    f = click.option('--stop_on_error', type=bool, show_default=True, default=True,
                     help='Stop parsing on first error.')(f)
    f = click.option('--use_default_pseudo_clusters', type=bool, show_default=True, default=True,
                     help='If enable this option use the set of default clusters provided by the matter_yamltests package.')(f)
    f = click.option('--additional_pseudo_clusters_directory', type=click.Path(), show_default=True, default=None,
                     help='Path to a directory containing additional pseudo clusters.')(f)
    return f


def test_runner_options(f):
    f = click.option('--adapter', type=str, default=None, required=True, show_default=True,
                     help='The adapter to run the test with.')(f)
    f = click.option('--stop_on_error', type=bool, default=True, show_default=True,
                     help='Stop the test suite on first error.')(f)
    f = click.option('--stop_on_warning', type=bool, default=False, show_default=True,
                     help='Stop the test suite on first warning.')(f)
    f = click.option('--stop_at_number', type=int, default=-1, show_default=True,
                     help='Stop the the test suite at the specified test number.')(f)
    f = click.option('--show_adapter_logs', type=bool, default=False, show_default=True,
                     help='Show additional logs provided by the adapter.')(f)
    f = click.option('--show_adapter_logs_on_error', type=bool, default=True, show_default=True,
                     help='Show additional logs provided by the adapter on error.')(f)
    f = click.option('--use_test_harness_log_format', type=bool, default=False, show_default=True,
                     help='Use the test harness log format.')(f)
    f = click.option('--delay-in-ms', type=int, default=0, show_default=True,
                     help='Add a delay between test suite steps.')(f)
    return f


def websocket_runner_options(f):
    f = click.option('--server_address', type=str, default='localhost', show_default=True,
                     help='The websocket server address to connect to.')(f)
    f = click.option('--server_port', type=int, default=9002, show_default=True,
                     help='The websocket server port to connect to.')(f)
    f = click.option('--server_name', type=str, default=None,
                     help='Name of a websocket server to run at launch.')(f)
    f = click.option('--server_path', type=click.Path(exists=True), default=None,
                     help='Path to a websocket server to run at launch.')(f)
    f = click.option('--server_arguments', type=str, default=None,
                     help='Optional arguments to pass to the websocket server at launch.')(f)
    return f


def chip_repl_runner_options(f):
    f = click.option('--repl_storage_path', type=str, default='/tmp/repl-storage.json',
                     help='Path to persistent storage configuration file.')(f)
    f = click.option('--commission_on_network_dut', type=bool, default=False,
                     help='Prior to running test should we try to commission DUT on network.')(f)
    f = click.option('--runner', type=str, default=None, show_default=True,
                     help='The runner to run the test with.')(f)
    return f


@dataclass
class ParserGroup:
    builder_config: TestParserBuilderConfig
    pseudo_clusters: PseudoClusters


pass_parser_group = click.make_pass_decorator(ParserGroup)


# YAML test file contains configurable options defined in their config section.
#
# Those options are test specific and as such can not be listed directly here but
# instead are retrieved from the target test file (if there is a single file) and
# are exposed to click dynamically.
#
# The following code extracts those and, to make it easy for the user to see
# which options are available, list them in a custom section when --help
# is invoked.

class YamlTestParserGroup(click.Group):
    def format_options(self, ctx, formatter):
        """Writes all the options into the formatter if they exist."""
        if getattr(ctx, 'custom_options', None):
            params_copy = self.params
            non_custom_params = list(filter(lambda x: x.name not in ctx.custom_options, self.params))
            custom_params = list(filter(lambda x: x.name in ctx.custom_options, self.params))

            self.params = non_custom_params
            super().format_options(ctx, formatter)
            self.params = params_copy

            opts = []
            for param in custom_params:
                rv = param.get_help_record(ctx)
                if rv is not None:
                    opts.append(rv)

            if opts:
                custom_section_title = ctx.params.get('test_name') + ' Options'
                with formatter.section(custom_section_title):
                    formatter.write_dl(opts)
        else:
            super().format_options(ctx, formatter)

    def parse_args(self, ctx, args):
        # Run the parser on the supported arguments first in order to get a
        # the necessary informations to get read the test file and add
        # the potential additional arguments.
        supported_args = self.__remove_custom_args(ctx, args)
        super().parse_args(ctx, supported_args)

        # Add the potential new arguments to the list of supported params and
        # run the parser a new time to read those.
        self.__add_custom_params(ctx)
        return super().parse_args(ctx, args)

    def __remove_custom_args(self, ctx, args):
        # Remove all the unsupported options from the command line string.
        params_name = [param.name for param in self.params]

        supported_args = []
        skipArgument = False
        for arg in args:
            if arg.startswith('--') and arg not in params_name:
                skipArgument = True
                continue
            if skipArgument:
                skipArgument = False
                continue
            supported_args.append(arg)

        return supported_args

    def __add_custom_params(self, ctx):
        tests_finder = TestsFinder(ctx.params.get('configuration_directory'), ctx.params.get('configuration_name'))
        tests = tests_finder.get(ctx.params.get('test_name'))

        custom_options = {}

        # There is a single test, extract the custom config
        if len(tests) == 1:
            try:
                custom_options = TestConfigParser.get_config(tests[0])
            except Exception:
                pass
            for key, value in custom_options.items():
                param = click.Option(['--' + key], default=value, show_default=True)
                # click converts parameter name to lowercase internally, so we need to override
                # this behavior in order to override the correct key.
                param.name = key
                self.params.append(param)

        ctx.custom_options = custom_options


CONTEXT_SETTINGS = dict(
    default_map={
        'chiptool': {
            'adapter': 'matter_chip_tool_adapter.adapter',
            'server_name': 'chip-tool',
            'server_arguments': 'interactive server',
        },
        'darwinframeworktool': {
            'adapter': 'matter_chip_tool_adapter.adapter',
            'server_name': 'darwin-framework-tool',
            'server_arguments': 'interactive server',
        },
        'app1': {
            'configuration_directory': 'examples/placeholder/linux/apps/app1',
            'adapter': 'matter_placeholder_adapter.adapter',
            'server_name': 'chip-app1',
            'server_arguments': '--interactive',
        },
        'app2': {
            'configuration_directory': 'examples/placeholder/linux/apps/app2',
            'adapter': 'matter_placeholder_adapter.adapter',
            'server_name': 'chip-app2',
            'server_arguments': '--interactive',
        },
        'chip-repl': {
            'adapter': 'matter_yamltest_repl_adapter.adapter',
            'runner': 'matter_yamltest_repl_adapter.runner',
        },
    },
    max_content_width=120,
)


@click.group(cls=YamlTestParserGroup, context_settings=CONTEXT_SETTINGS)
@click.argument('test_name')
@test_parser_options
@click.pass_context
def runner_base(ctx, configuration_directory: str, test_name: str, configuration_name: str, pics: str, specifications_paths: str, stop_on_error: bool, use_default_pseudo_clusters: bool, additional_pseudo_clusters_directory: str, **kwargs):
    pseudo_clusters = get_custom_pseudo_clusters(
        additional_pseudo_clusters_directory) if use_default_pseudo_clusters else PseudoClusters([])
    specifications = SpecDefinitionsFromPaths(specifications_paths.split(','), pseudo_clusters)
    tests_finder = TestsFinder(configuration_directory, configuration_name)

    test_list = tests_finder.get(test_name)
    if len(test_list) == 0:
        raise Exception(f"No tests found for test name '{test_name}'")

    parser_config = TestParserConfig(pics, specifications, kwargs)
    parser_builder_config = TestParserBuilderConfig(test_list, parser_config, hooks=TestParserLogger())
    parser_builder_config.options.stop_on_error = stop_on_error
    while ctx:
        ctx.obj = ParserGroup(parser_builder_config, pseudo_clusters)
        ctx = ctx.parent


@runner_base.command()
@pass_parser_group
def parse(parser_group: ParserGroup):
    """Parse the test suite."""
    runner_config = None

    runner = TestRunner()
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(runner.run(parser_group.builder_config, runner_config))


@runner_base.command()
@pass_parser_group
def dry_run(parser_group: ParserGroup):
    """Simulate a run of the test suite."""
    runner_config = TestRunnerConfig(hooks=TestRunnerLogger())

    runner = TestRunner()
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(runner.run(parser_group.builder_config, runner_config))


@runner_base.command()
@test_runner_options
@pass_parser_group
def run(parser_group: ParserGroup, adapter: str, stop_on_error: bool, stop_on_warning: bool, stop_at_number: int, show_adapter_logs: bool, show_adapter_logs_on_error: bool, use_test_harness_log_format: bool, delay_in_ms: int):
    """Run the test suite."""
    adapter = __import__(adapter, fromlist=[None]).Adapter(parser_group.builder_config.parser_config.definitions)
    runner_options = TestRunnerOptions(stop_on_error, stop_on_warning, stop_at_number, delay_in_ms)
    runner_hooks = TestRunnerLogger(show_adapter_logs, show_adapter_logs_on_error, use_test_harness_log_format)
    runner_config = TestRunnerConfig(adapter, parser_group.pseudo_clusters, runner_options, runner_hooks)

    runner = TestRunner()
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(runner.run(parser_group.builder_config, runner_config))


@runner_base.command()
@test_runner_options
@websocket_runner_options
@pass_parser_group
def websocket(parser_group: ParserGroup, adapter: str, stop_on_error: bool, stop_on_warning: bool, stop_at_number: int, show_adapter_logs: bool, show_adapter_logs_on_error: bool, use_test_harness_log_format: bool, delay_in_ms: int, server_address: str, server_port: int, server_path: str, server_name: str, server_arguments: str):
    """Run the test suite using websockets."""
    adapter = __import__(adapter, fromlist=[None]).Adapter(parser_group.builder_config.parser_config.definitions)
    runner_options = TestRunnerOptions(stop_on_error, stop_on_warning, stop_at_number, delay_in_ms)
    runner_hooks = TestRunnerLogger(show_adapter_logs, show_adapter_logs_on_error, use_test_harness_log_format)
    runner_config = TestRunnerConfig(adapter, parser_group.pseudo_clusters, runner_options, runner_hooks)

    if server_path is None and server_name:
        paths_finder = PathsFinder()
        server_path = paths_finder.get(server_name)

    websocket_runner_hooks = WebSocketRunnerLogger()
    websocket_runner_config = WebSocketRunnerConfig(
        server_address, server_port, server_path, server_arguments, websocket_runner_hooks)

    runner = WebSocketRunner(websocket_runner_config)
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(runner.run(parser_group.builder_config, runner_config))


@runner_base.command()
@test_runner_options
@chip_repl_runner_options
@pass_parser_group
def chip_repl(parser_group: ParserGroup, adapter: str, stop_on_error: bool, stop_on_warning: bool, stop_at_number: int, show_adapter_logs: bool, show_adapter_logs_on_error: bool, use_test_harness_log_format: bool, delay_in_ms: int, runner: str, repl_storage_path: str, commission_on_network_dut: bool):
    """Run the test suite using chip-repl."""
    adapter = __import__(adapter, fromlist=[None]).Adapter(parser_group.builder_config.parser_config.definitions)
    runner_options = TestRunnerOptions(stop_on_error, stop_on_warning, stop_at_number, delay_in_ms)
    runner_hooks = TestRunnerLogger(show_adapter_logs, show_adapter_logs_on_error, use_test_harness_log_format)
    runner_config = TestRunnerConfig(adapter, parser_group.pseudo_clusters, runner_options, runner_hooks)

    node_id_to_commission = None
    if commission_on_network_dut:
        node_id_to_commission = parser_group.builder_config.parser_config.config_override['nodeId']
    runner = __import__(runner, fromlist=[None]).Runner(repl_storage_path, node_id_to_commission=node_id_to_commission)
    loop = asyncio.get_event_loop()
    return loop.run_until_complete(runner.run(parser_group.builder_config, runner_config))


@runner_base.command()
@test_runner_options
@websocket_runner_options
@click.pass_context
def chiptool(ctx, *args, **kwargs):
    """Run the test suite using chip-tool."""
    return ctx.forward(websocket)


@runner_base.command()
@test_runner_options
@websocket_runner_options
@click.pass_context
def darwinframeworktool(ctx, *args, **kwargs):
    """Run the test suite using darwin-framework-tool."""
    return ctx.forward(websocket)


@runner_base.command()
@test_runner_options
@websocket_runner_options
@click.pass_context
def app1(ctx, *args, **kwargs):
    """Run the test suite using app1."""
    return ctx.forward(websocket)


@runner_base.command()
@test_runner_options
@websocket_runner_options
@click.pass_context
def app2(ctx, *args, **kwargs):
    """Run the test suite using app2."""
    return ctx.forward(websocket)


if __name__ == '__main__':
    success = False
    try:
        # By default click runs in standalone mode and it will handle exceptions and the
        # different commands return values for us. For example it will set sys.exit to
        # 0 if the test runs fails unless an exception is raised. Simple test failure
        # does not raise exception but we want to set the exit code to 1.
        # So standalone_mode is set to False to let us manage this exit behavior.
        success = runner_base(standalone_mode=False)
    except Exception:
        print('')
        traceback.print_exc()

    sys.exit(0 if success else 1)
