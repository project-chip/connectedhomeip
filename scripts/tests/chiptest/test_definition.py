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
import threading
from pathlib import Path
import platform

from enum import Enum, auto
from dataclasses import dataclass
from random import randrange

TEST_NODE_ID = '0x12344321'


class App:
    def __init__(self, runner, command):
        self.process = None
        self.runner = runner
        self.command = command
        self.stopped = False

    def start(self, discriminator):
        if not self.process:
            self.process = None
            process, outpipe, errpipe = self.__startServer(
                self.runner, self.command, discriminator)
            self.__waitForServerReady(process, outpipe)
            self.__updateSetUpCode(outpipe)
            self.process = process
            self.stopped = False
            return True
        return False

    def stop(self):
        if self.process:
            self.stopped = True
            self.process.kill()
            self.process.wait(10)
            self.process = None
            return True
        return False

    def reboot(self, discriminator):
        if self.process:
            self.stop()
            self.start(discriminator)
            return True
        return False

    def factoryReset(self):
        storage = '/tmp/chip_kvs'
        if os.path.exists(storage):
            os.unlink(storage)

        return True

    def poll(self):
        # When the server is manually stopped, process polling is overriden so the other
        # processes that depends on the accessory beeing alive does not stop.
        if self.stopped:
            return None
        return self.process.poll()

    def kill(self):
        if self.process:
            self.process.kill()

    def wait(self, duration):
        if self.process:
            self.process.wait(duration)

    def __startServer(self, runner, command, discriminator):
        logging.debug(
            'Executing application under test with discriminator %s.' % discriminator)
        app_cmd = command + ['--discriminator', str(discriminator)]
        return runner.RunSubprocess(app_cmd, name='APP ', wait=False)

    def __waitForServerReady(self, server_process, outpipe):
        logging.debug('Waiting for server to listen.')
        start_time = time.time()
        server_is_listening = outpipe.CapturedLogContains("Server Listening")
        while not server_is_listening:
            if server_process.poll() is not None:
                died_str = 'Server died during startup, returncode %d' % server_process.returncode
                logging.error(died_str)
                raise Exception(died_str)
            if time.time() - start_time > 10:
                raise Exception('Timeout for server listening')
            time.sleep(0.1)
            server_is_listening = outpipe.CapturedLogContains(
                "Server Listening")
        logging.debug('Server is listening. Can proceed.')

    def __updateSetUpCode(self, outpipe):
        qrLine = outpipe.FindLastMatchingLine('.*SetupQRCode: *\\[(.*)]')
        if not qrLine:
            raise Exception("Unable to find QR code")
        self.setupCode = qrLine.group(1)


class TestTarget(Enum):
    ALL_CLUSTERS = auto()
    TV = auto()
    DOOR_LOCK = auto()


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
        self.lock = threading.Lock()
        self.captures = []

    def Log(self, source, line):
        with self.lock:
            self.captures.append(CaptureLine(
                when=datetime.now(),
                source=source,
                line=line.strip('\n')
            ))

    def LogContents(self):
        logging.error('================ CAPTURED LOG START ==================')
        with self.lock:
            for entry in self.captures:
                logging.error('%02d:%02d:%02d.%03d - %-10s: %s',
                              entry.when.hour,
                              entry.when.minute,
                              entry.when.second,
                              entry.when.microsecond/1000,
                              entry.source,
                              entry.line
                              )
        logging.error('================ CAPTURED LOG END ====================')


@dataclass
class TestDefinition:
    name: str
    run_name: str
    target: TestTarget

    def Run(self, runner, apps_register, paths: ApplicationPaths):
        """Executes the given test case using the provided runner for execution."""
        runner.capture_delegate = ExecutionCapture()

        try:
            if self.target == TestTarget.ALL_CLUSTERS:
                app_cmd = paths.all_clusters_app
            elif self.target == TestTarget.TV:
                app_cmd = paths.tv_app
            elif self.target == TestTarget.DOOR_LOCK:
                logging.info(
                    "Ignore test - test is made for door lock which is not supported yet")
                return
            else:
                raise Exception(
                    "Unknown test target - don't know which application to run")

            tool_cmd = paths.chip_tool

            files_to_unlink = [
                '/tmp/chip_tool_config.ini',
                '/tmp/chip_tool_config.alpha.ini',
                '/tmp/chip_tool_config.beta.ini',
                '/tmp/chip_tool_config.gamma.ini',
            ]

            for f in files_to_unlink:
                if os.path.exists(f):
                    os.unlink(f)

            app = App(runner, app_cmd)
            app.factoryReset()  # Remove server application storage, so it will be commissionable again
            app.start(str(randrange(1, 4096)))
            apps_register.add("default", app)

            runner.RunSubprocess(tool_cmd + ['pairing', 'qrcode', TEST_NODE_ID, app.setupCode],
                                 name='PAIR', dependencies=[apps_register])

            runner.RunSubprocess(tool_cmd + ['tests', self.run_name, TEST_NODE_ID],
                                 name='TEST', dependencies=[apps_register])
        except:
            logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            raise
        finally:
            apps_register.killAll()
            apps_register.factoryResetAll()
            apps_register.removeAll()
