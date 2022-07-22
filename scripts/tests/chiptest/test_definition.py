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
import sys
import threading
import time
import typing
from dataclasses import dataclass
from datetime import datetime
from enum import Enum, auto
from random import randrange

TEST_NODE_ID = '0x12344321'


class App:

    def __init__(self, runner, command):
        self.process = None
        self.outpipe = None
        self.runner = runner
        self.command = command
        self.cv_stopped = threading.Condition()
        self.stopped = True
        self.lastLogIndex = 0
        self.kvsPathSet = {'/tmp/chip_kvs'}
        self.options = None

    def start(self, options=None):
        if not self.process:
            # Cache command line options to be used for reboots
            if options:
                self.options = options
            # Make sure to assign self.process before we do any operations that
            # might fail, so attempts to kill us on failure actually work.
            self.process, self.outpipe, errpipe = self.__startServer(
                self.runner, self.command)
            self.waitForAnyAdvertisement()
            self.__updateSetUpCode()
            with self.cv_stopped:
                self.stopped = False
                self.cv_stopped.notify()
            return True
        return False

    def stop(self):
        if self.process:
            with self.cv_stopped:
                self.stopped = True
                self.cv_stopped.notify()
            self.process.kill()
            self.process.wait(10)
            self.process = None
            self.outpipe = None
            return True
        return False

    def factoryReset(self):
        for kvs in self.kvsPathSet:
            if os.path.exists(kvs):
                os.unlink(kvs)
        return True

    def waitForAnyAdvertisement(self):
        self.__waitFor("mDNS service published:", self.process, self.outpipe)

    def waitForMessage(self, message):
        self.__waitFor(message, self.process, self.outpipe)
        return True

    def kill(self):
        if self.process:
            self.process.kill()

    def wait(self, timeout=None):
        while True:
            # If the App was never started, wait cannot be called on the process
            if self.process == None:
                time.sleep(0.1)
                continue
            code = self.process.wait(timeout)
            with self.cv_stopped:
                if not self.stopped:
                    return code
                # When the server is manually stopped, process waiting is
                # overridden so the other processes that depends on the
                # accessory beeing alive does not stop.
                while self.stopped:
                    self.cv_stopped.wait()

    def __startServer(self, runner, command):
        app_cmd = command + ['--interface-id', str(-1)]

        if not self.options:
            logging.debug('Executing application under test with default args')
        else:
            logging.debug('Executing application under test with the following args:')
            for key, value in self.options.items():
                logging.debug('   %s: %s' % (key, value))
                app_cmd = app_cmd + [key, value]
                if key == '--KVS':
                    self.kvsPathSet.add(value)
        return runner.RunSubprocess(app_cmd, name='APP ', wait=False)

    def __waitFor(self, waitForString, server_process, outpipe):
        logging.debug('Waiting for %s' % waitForString)

        start_time = time.monotonic()
        ready, self.lastLogIndex = outpipe.CapturedLogContains(
            waitForString, self.lastLogIndex)
        while not ready:
            if server_process.poll() is not None:
                died_str = ('Server died while waiting for %s, returncode %d' %
                            (waitForString, server_process.returncode))
                logging.error(died_str)
                raise Exception(died_str)
            if time.monotonic() - start_time > 10:
                raise Exception('Timeout while waiting for %s' % waitForString)
            time.sleep(0.1)
            ready, self.lastLogIndex = outpipe.CapturedLogContains(
                waitForString, self.lastLogIndex)

        logging.debug('Success waiting for: %s' % waitForString)

    def __updateSetUpCode(self):
        qrLine = self.outpipe.FindLastMatchingLine('.*SetupQRCode: *\\[(.*)]')
        if not qrLine:
            raise Exception("Unable to find QR code")
        self.setupCode = qrLine.group(1)


class TestTarget(Enum):
    ALL_CLUSTERS = auto()
    TV = auto()
    LOCK = auto()
    OTA = auto()


@dataclass
class ApplicationPaths:
    chip_tool: typing.List[str]
    all_clusters_app: typing.List[str]
    lock_app: typing.List[str]
    ota_provider_app: typing.List[str]
    ota_requestor_app: typing.List[str]
    tv_app: typing.List[str]

    def items(self):
        return [self.chip_tool, self.all_clusters_app, self.lock_app, self.ota_provider_app, self.ota_requestor_app, self.tv_app]


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
    is_manual: bool

    def Run(self, runner, apps_register, paths: ApplicationPaths, pics_file: str, timeout_seconds: typing.Optional[int], dry_run=False):
        """
        Executes the given test case using the provided runner for execution.
        """
        runner.capture_delegate = ExecutionCapture()

        try:
            if self.target == TestTarget.ALL_CLUSTERS:
                target_app = paths.all_clusters_app
            elif self.target == TestTarget.TV:
                target_app = paths.tv_app
            elif self.target == TestTarget.LOCK:
                target_app = paths.lock_app
            elif self.target == TestTarget.OTA:
                target_app = paths.ota_requestor_app
            else:
                raise Exception("Unknown test target - "
                                "don't know which application to run")

            for path in paths.items():
                # Do not add chip-tool to the register
                if path == paths.chip_tool:
                    continue

                # For the app indicated by self.target, give it the 'default' key to add to the register
                if path == target_app:
                    key = 'default'
                else:
                    key = os.path.basename(path[-1])

                app = App(runner, path)
                # Add the App to the register immediately, so if it fails during
                # start() we will be able to clean things up properly.
                apps_register.add(key, app)
                # Remove server application storage (factory reset),
                # so it will be commissionable again.
                app.factoryReset()

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

            # Only start and pair the default app
            app = apps_register.get('default')
            app.start()
            pairing_cmd = tool_cmd + ['pairing', 'code', TEST_NODE_ID, app.setupCode]
            test_cmd = tool_cmd + ['tests', self.run_name] + ['--PICS', pics_file]

            if dry_run:
                logging.info(" ".join(pairing_cmd))
                logging.info(" ".join(test_cmd))

            else:
                runner.RunSubprocess(pairing_cmd,
                                     name='PAIR', dependencies=[apps_register])

                runner.RunSubprocess(
                    test_cmd,
                    name='TEST', dependencies=[apps_register],
                    timeout_seconds=timeout_seconds)

        except Exception:
            logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            raise
        finally:
            apps_register.killAll()
            apps_register.factoryResetAll()
            apps_register.removeAll()
