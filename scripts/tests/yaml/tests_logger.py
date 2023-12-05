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

import json
import sys
import traceback
from dataclasses import dataclass

import click
from matter_yamltests.errors import TestStepError, TestStepKeyError
from matter_yamltests.hooks import TestParserHooks, TestRunnerHooks, WebSocketRunnerHooks
from matter_yamltests.parser import TestStep


def _strikethrough(str):
    return '\u0336'.join(str[i:i+1] for i in range(0, len(str), 1))


_SUCCESS = click.style(u'\N{check mark}', fg='green')
_FAILURE = click.style(u'\N{ballot x}', fg='red')
_WARNING = click.style(u'\N{warning sign}', fg='yellow')


class TestColoredLogPrinter():
    def __init__(self, log_format='[{module}] {message}'):
        self.__log_format = log_format
        self.__log_styles = {
            'Info': 'green',
            'Error': 'bright_red',
            'Debug': 'blue',
            'Others': 'white'
        }

    def print(self, logs):
        for log in logs:
            fg = self.__log_styles[log.level]
            click.secho(self.__log_format.format(module=log.module, message=log.message), fg=fg)


@dataclass
class ParserStrings:
    start = 'Parsing {count} files.'
    stop = '{state} Parsing finished in {duration}ms with {successes} success and {errors} errors.'
    test_start = click.style('\t\tParsing: ', fg='white') + '{name}'
    test_result = '\r{state} ' + click.style('{duration}ms', fg='white')
    error_header = click.style('\t\tError at step {index}:', fg='white', bold=True)
    error_line = click.style('\t\t{error_line}', fg='white')


class TestParserLogger(TestParserHooks):
    def __init__(self):
        self.__success = 0
        self.__errors = 0
        self.__strings = ParserStrings()

    def parsing_start(self, count: int):
        print(self.__strings.start.format(count=count))

    def parsing_stop(self, duration: int):
        state = _FAILURE if self.__errors else _SUCCESS
        success = click.style(self.__success, bold=True)
        errors = click.style(self.__errors, bold=True)
        print(self.__strings.stop.format(state=state, successes=success, errors=errors, duration=duration))

    def test_parsing_start(self, name: str):
        print(self.__strings.test_start.format(name=name), end='')

    def test_parsing_failure(self, exception: Exception, duration: int):
        print(self.__strings.test_result.format(state=_FAILURE, duration=duration))

        try:
            raise exception
        except TestStepError:
            self.__print_step_exception(exception)
        else:
            traceback.print_exc()

        self.__errors += 1

    def test_parsing_success(self, duration: int):
        print(self.__strings.test_result.format(state=_SUCCESS, duration=duration))
        self.__success += 1

    def __print_step_exception(self, exception: TestStepError):
        if exception.context is None:
            return

        print('')
        print(self.__strings.error_header.format(index=exception.step_index))
        for line in exception.context.split('\n'):
            if '__error_start__' in line and '__error_end__' in line:
                before_end, after_end = line.split('__error_end__')
                before_start, after_start = before_end.split('__error_start__')
                line = click.style(before_start, fg='white')
                line += click.style(after_start, fg='red', bold=True)
                line += click.style(after_end, fg='white')
                line += click.style(f' <-- {exception}', bold=True)

            print(self.__strings.error_line.format(error_line=line))


@dataclass
class RunnerStrings:
    start = ''
    stop = 'Run finished in {duration}ms with {runned} steps runned and {skipped} steps skipped.'
    test_start = 'Running: "{name}" with {count} steps.'
    test_stop = '{state} Test finished in {duration}ms with {successes} success, {errors} errors and {warnings} warnings'
    step_skipped = click.style('\t\t{index}. ' + _strikethrough('Running ') + '{name}', fg='white')
    step_start = click.style('\t\t{index}. Running ', fg='white') + '{name}'
    step_unknown = ''
    step_result = '\r{state} ' + click.style('{duration}ms', fg='white')
    result_entry = click.style('\t\t  {state} [{category} check] {message}', fg='white')
    result_log = '\t\t    [{module}] {message}'
    result_failure = '\t\t    {key}: {value}'
    error_header = click.style('\t\t    Error at step {index}:', fg='white', bold=True)
    error_line = click.style('\t\t    {error_line}', fg='white')

    test_harness_test_start = '\t\t***** Test Start : {filename}'
    test_harness_test_stop_success = '\t\t***** Test Complete: {filename}'
    test_harness_step_skipped = '\t\t**** Skipping: {expression} == false'
    test_harness_step_start = '\t\t***** Test Step {index} : {name}'
    test_harness_step_failure = '\t\t***** Test Failure : {message}'
    test_harness_setup_device_connection_success = '\t\t**** Test Setup: Device Connected'
    test_harness_setup_device_connection_failure = '\t\t**** Test Setup: Device Connection Failure [deviceId={deviceId}. Error {message}]'
    log = '\t\t{message}'
    user_prompt = '\t\tUSER_PROMPT: {message}'


class TestRunnerLogger(TestRunnerHooks):
    def __init__(self, show_adapter_logs: bool = False, show_adapter_logs_on_error: bool = True, use_test_harness_log_format: bool = False):
        self.__show_adapter_logs = show_adapter_logs
        self.__show_adapter_logs_on_error = show_adapter_logs_on_error
        self.__use_test_harness_log_format = use_test_harness_log_format
        self.__filename = None
        self.__index = 1
        self.__successes = 0
        self.__warnings = 0
        self.__errors = 0
        self.__runned = 0
        self.__skipped = 0
        self.__strings = RunnerStrings()
        self.__log_printer = TestColoredLogPrinter(self.__strings.result_log)

    def start(self, count: int):
        print(self.__strings.start)

    def stop(self, duration: int):
        print(self.__strings.stop.format(runned=self.__runned, skipped=self.__skipped, duration=duration))

    def test_start(self, filename: str, name: str, count: int):
        print(self.__strings.test_start.format(name=click.style(name, bold=True), count=click.style(count, bold=True)))

        if self.__use_test_harness_log_format:
            self.__filename = filename
            print(self.__strings.test_harness_test_start.format(filename=filename))

    def test_stop(self, duration: int):
        if self.__errors:
            state = _FAILURE
        elif self.__warnings:
            state = _WARNING
        else:
            state = _SUCCESS

        if self.__use_test_harness_log_format and (state == _SUCCESS or state == _WARNING):
            print(self.__strings.test_harness_test_stop_success.format(filename=self.__filename))

        successes = click.style(self.__successes, bold=True)
        errors = click.style(self.__errors, bold=True)
        warnings = click.style(self.__warnings, bold=True)
        print(self.__strings.test_stop.format(state=state, successes=successes, errors=errors, warnings=warnings, duration=duration))

    def step_skipped(self, name: str, expression: str):
        print(self.__strings.step_skipped.format(index=self.__index, name=_strikethrough(name)))

        if self.__use_test_harness_log_format:
            print(self.__strings.test_harness_step_start.format(index=self.__index, name=name))
            print(self.__strings.test_harness_step_skipped.format(expression=expression))

        self.__index += 1
        self.__skipped += 1

    def step_start(self, request: TestStep):
        if self.__use_test_harness_log_format:
            print(self.__strings.test_harness_step_start.format(index=self.__index, name=request.label))

        print(self.__strings.step_start.format(index=self.__index, name=click.style(request.label, bold=True)), end='')
        # This is to keep previous behavior of UserPrompt. Where it logs USER_PROMPT prior to user input. See link below:
        # https://github.com/project-chip/connectedhomeip/blob/6644a4b0b0d1272ae325c651b27bd0e7068f3a8a/src/app/tests/suites/commands/log/LogCommands.cpp#L31
        if request.command == 'UserPrompt':
            message = request.arguments['values'][0]['value']
            print("\n" + self.__strings.user_prompt.format(message=f'{message}'))
        # flushing stdout such that the previous print statement is visible on the screen for long running tasks.
        sys.stdout.flush()

        self.__index += 1

    def step_unknown(self):
        print(self.__strings.step_unknown)

        self.__runned += 1

    def step_success(self, logger, logs, duration: int, request: TestStep):
        print(self.__strings.step_result.format(state=_SUCCESS, duration=duration))

        self.__print_results(logger)
        if self.__use_test_harness_log_format:
            if request.command == 'WaitForCommissionee':
                print(self.__strings.test_harness_setup_device_connection_success)
            elif request.command == 'Log':
                message = request.arguments['values'][0]['value']
                print(self.__strings.log.format(message=f'{message}'))

        if self.__show_adapter_logs:
            self.__log_printer.print(logs)

        self.__successes += logger.successes
        self.__warnings += logger.warnings
        self.__errors += logger.errors
        self.__runned += 1

    def step_failure(self, logger, logs, duration: int, request: TestStep, received):
        print(self.__strings.step_result.format(state=_FAILURE, duration=duration))

        self.__print_results(logger)

        if self.__show_adapter_logs or self.__show_adapter_logs_on_error:
            self.__log_printer.print(logs)

        has_failures_without_exception = False
        for entry in logger.entries:
            if entry.is_error() and entry.exception:
                try:
                    raise entry.exception
                except TestStepError as e:
                    self.__print_step_exception(e)
                else:
                    traceback.print_exc()
            elif entry.is_error():
                has_failures_without_exception = True

        if has_failures_without_exception:
            self.__print_failure(request.responses, received)

        self.__successes += logger.successes
        self.__warnings += logger.warnings
        self.__errors += logger.errors
        self.__runned += 1

        if self.__use_test_harness_log_format:
            message = ''
            for entry in logger.entries:
                if entry.is_error():
                    message = entry.message
                    print(self.__strings.test_harness_step_failure.format(message=message))
                    break

            if request.command == 'WaitForCommissionee':
                print(self.__strings.test_harness_setup_device_connection_failure.format(deviceId=request.node_id, message=message))

    def __print_step_exception(self, exception: TestStepError):
        if exception.context is None:
            return

        print('')
        print(self.__strings.error_header.format(index=exception.step_index))
        for line in exception.context.split('\n'):
            if '__error_start__' in line and '__error_end__' in line:
                before_end, after_end = line.split('__error_end__')
                before_start, after_start = before_end.split('__error_start__')
                line = click.style(before_start, fg='white')
                line += click.style(after_start, fg='red', bold=True)
                line += click.style(after_end, fg='white')
                line += click.style(f' <-- {exception}', bold=True)

            print(self.__strings.error_line.format(error_line=line))

    def __print_results(self, logger):
        for entry in logger.entries:
            if entry.is_warning():
                state = _WARNING
            elif entry.is_error():
                state = _FAILURE
            else:
                state = ' '  # Do not mark success to not make the output hard to read

            print(self.__strings.result_entry.format(state=state, category=entry.category, message=entry.message))

    def __print_failure(self, expected_response, received_response):
        expected_response = self.__prepare_data_for_printing(expected_response)
        expected_value = json.dumps(
            expected_response, sort_keys=True, indent=2, separators=(',', ': '))
        expected_value = expected_value.replace('\n', '\n\t\t              ')

        received_response = self.__prepare_data_for_printing(
            received_response)
        received_value = json.dumps(
            received_response, sort_keys=True, indent=2, separators=(',', ': '))
        received_value = received_value.replace('\n', '\n\t\t               ')
        print(self.__strings.result_failure.format(key=click.style("Expected", bold=True), value=expected_value))
        print(self.__strings.result_failure.format(key=click.style("Received", bold=True), value=received_value))

    def __prepare_data_for_printing(self, data):
        if isinstance(data, bytes):
            return data.decode('unicode_escape')
        elif isinstance(data, list):
            return [self.__prepare_data_for_printing(entry) for entry in data]
        elif isinstance(data, dict):
            result = {}
            for key, value in data.items():
                result[key] = self.__prepare_data_for_printing(value)
            return result

        return data


@dataclass
class WebSocketRunnerStrings:
    connecting = click.style('\t\tConnecting: ' + click.style('{url}', bold=True), fg='white')
    abort = 'Connecting to {url} failed.'
    success = click.style(f'\r{_SUCCESS} {{duration}}ms', fg='white')
    failure = click.style(f'\r{_WARNING} {{duration}}ms', fg='white')
    retry = click.style('\t\t  Retrying in {interval} seconds.', fg='white')


class WebSocketRunnerLogger(WebSocketRunnerHooks):
    def __init__(self):
        self.__strings = WebSocketRunnerStrings()

    def connecting(self, url: str):
        print(self.__strings.connecting.format(url=url), end='')
        sys.stdout.flush()

    def abort(self, url: str):
        print(self.__strings.abort.format(url=url))

    def success(self, duration: int):
        print(self.__strings.success.format(duration=duration))

    def failure(self, duration: int):
        print(self.__strings.failure.format(duration=duration))

    def retry(self, interval_between_retries_in_seconds: int):
        print(self.__strings.retry.format(interval=interval_between_retries_in_seconds))


#
# Everything below this comment is for testing purposes only.
# It is here to quickly check the look and feel of the output
# that is produced via the different hooks.
#
@click.group()
def simulate():
    pass


@simulate.command()
def parser():
    """Simulate parsing tests."""
    parser_logger = TestParserLogger()

    test_step = {
        'label': 'This is a fake test step',
        'nodeId': 1,
        'cluster': 'TestStepCluster',
        'commandd': 'WrongCommandKey',
        'attribute': 'TestStepAttribute',
    }

    test_step

    parser_logger.start(99)
    parser_logger.test_start('test.yaml')
    parser_logger.test_success(10)
    parser_logger.test_start('test2.yaml')
    exception = TestStepKeyError(test_step, 'commandd')
    exception.update_context(test_step, 1)
    parser_logger.test_failure(exception, 200)
    parser_logger.stop(10 + 200)


@simulate.command()
def runner():
    """Simulate running tests."""
    runner_logger = TestRunnerLogger(use_test_harness_log_format=True)

    class TestLogger:
        def __init__(self, entries=[], successes=0, warnings=0, errors=0):
            self.entries = entries
            self.successes = successes
            self.warnings = warnings
            self.errors = errors

    class LogEntry:
        def __init__(self, message, module='CTL', level='Others'):
            self.message = message
            self.module = module
            self.level = level

    success_logger = TestLogger(successes=3)
    error_logger = TestLogger(errors=2)

    expected_response = {}
    received_response = {'error': 'UNSUPPORTED_COMMAND'}

    empty_logs = []
    other_logs = [
        LogEntry('This is a message without a category'),
        LogEntry('This is an info message', level='Info'),
        LogEntry('This is an error message', level='Error'),
        LogEntry('This is a debug message', level='Debug'),
    ]

    runner_logger.start(99)
    runner_logger.test_start('Test_File', 'A test with multiple steps', 23)
    runner_logger.step_start('First Step')
    runner_logger.step_success(success_logger, empty_logs, 1234)
    runner_logger.step_start('Second Step')
    runner_logger.step_failure(error_logger, other_logs, 4321, expected_response, received_response)
    runner_logger.step_skipped('Third Step', 'SHOULD_RUN')
    runner_logger.step_start('Fourth Step')
    runner_logger.step_unknown()
    runner_logger.test_stop(1234 + 4321)
    runner_logger.stop(123456)


@simulate.command()
def websocket_abort():
    """Simulate a websocket connection aborting."""
    websocket_runner_logger = WebSocketRunnerLogger()
    url = 'ws://localhost:9002'

    websocket_runner_logger.connecting(url)
    websocket_runner_logger.failure(30)
    websocket_runner_logger.retry(1)
    websocket_runner_logger.connecting(url)
    websocket_runner_logger.failure(30)
    websocket_runner_logger.abort(url)


@simulate.command()
def websocket_connected():
    """Simulate a websocket connection that successfuly connects."""
    websocket_runner_logger = WebSocketRunnerLogger()
    url = 'ws://localhost:9002'

    websocket_runner_logger.connecting(url)
    websocket_runner_logger.failure(30)
    websocket_runner_logger.retry(1)
    websocket_runner_logger.connecting(url)
    websocket_runner_logger.success(30)


if __name__ == '__main__':
    simulate()
