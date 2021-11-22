#
#    Copyright (c) 2021 Project CHIP Authors
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

import logging
import os
import time
from datetime import datetime
import typing

from enum import Enum, auto
from dataclasses import dataclass

TEST_NODE_ID = '0x12344321'


class TestTarget(Enum):
    ALL_CLUSTERS = auto()
    TV = auto()


@dataclass
class ApplicationPaths:
    chip_tool: typing.List[str]
    all_clusters_app: typing.List[str]
    tv_app: typing.List[str]


@dataclass
class CaptureLine:
    when: datetime
    source: str
    line: str


class ExecutionCapture:
    """
    Keeps track of output lines in a process, to help debug failures.
    """

    def __init__(self):
        self.captures = []

    def Log(self, source, line):
        self.captures.append(CaptureLine(
            when=datetime.now(),
            source=source,
            line=line.strip('\n')
        ))

    def LogContents(self):
        for entry in self.captures:
            logging.error('%02d:%02d:%02d.%03d - %-10s: %s',
                          entry.when.hour,
                          entry.when.minute,
                          entry.when.second,
                          entry.when.microsecond/1000,
                          entry.source,
                          entry.line
                          )
        pass


@dataclass
class TestDefinition:
    yaml_file: str
    name: str
    run_name: str
    target: TestTarget

    def Run(self, runner, paths: ApplicationPaths):
        """Executes the given test case using the provided runner for execution."""
        app_process = None
        runner.capture_delegate = ExecutionCapture()

        try:
            if self.target == TestTarget.ALL_CLUSTERS:
                app_cmd = paths.all_clusters_app
            elif self.target == TestTarget.TV:
                app_cmd = paths.tv_app
            else:
                raise Exception(
                    "Unknown test target - don't know which application to run")

            tool_cmd = paths.chip_tool
            if os.path.exists('/tmp/chip_tool_config.ini'):
                os.unlink('/tmp/chip_tool_config.ini')

            logging.debug('Executing application under test.')
            app_process, outpipe, errpipe = runner.RunSubprocess(
                app_cmd, name='APP ', wait=False)

            logging.debug('Waiting for server to listen.')
            start_time = time.time()
            server_is_listening = outpipe.CapturedLogContains(
                "Server Listening")
            while not server_is_listening:
                if time.time() - start_time > 10:
                    raise Exception('Timeout for server listening')
                time.sleep(0.1)
                server_is_listening = outpipe.CapturedLogContains(
                    "Server Listening")
            logging.debug('Server is listening. Can proceed.')

            runner.RunSubprocess(tool_cmd + ['pairing', 'qrcode', TEST_NODE_ID, 'MT:D8XA0CQM00KA0648G00'],
                                 name='PAIR')

            runner.RunSubprocess(tool_cmd + ['tests', self.run_name, TEST_NODE_ID],
                                 name='TEST')
        except:
            logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            raise
        finally:
            if app_process:
                app_process.kill()
