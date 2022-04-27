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
DEVELOPMENT_PAA_LIST = './credentials/development/paa-root-certs'


class App:

    def __init__(self, runner, command):
        self.process = None
        self.outpipe = None
        self.runner = runner
        self.command = command
        self.cv_stopped = threading.Condition()
        self.stopped = False
        self.lastLogIndex = 0
        self.kvs = '/tmp/chip_kvs'

    def start(self, options=None):
        if not self.process:
            # Make sure to assign self.process before we do any operations that
            # might fail, so attempts to kill us on failure actually work.
            self.process, self.outpipe, errpipe = self.__startServer(
                self.runner, self.command, options)
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
        if os.path.exists(self.kvs):
            os.unlink(self.kvs)
        return True

    def waitForAnyAdvertisement(self):
        self.__waitFor("mDNS service published:", self.process, self.outpipe)

    def waitForCommissionableAdvertisement(self):
        self.__waitFor("mDNS service published: _matterc._udp",
                       self.process, self.outpipe)
        return True

    def waitForOperationalAdvertisement(self):
        self.__waitFor("mDNS service published: _matter._tcp",
                       self.process, self.outpipe)
        return True

    def kill(self):
        if self.process:
            self.process.kill()

    def wait(self, timeout=None):
        while True:
            code = self.process.wait(timeout)
            with self.cv_stopped:
                if not self.stopped:
                    return code
                # When the server is manually stopped, process waiting is
                # overridden so the other processes that depends on the
                # accessory beeing alive does not stop.
                while self.stopped:
                    self.cv_stopped.wait()

    def __startServer(self, runner, command, options):
        app_cmd = command + ['--interface-id', str(-1)]

        if not options:
            logging.debug('Executing application under test with default args')
        else:
            logging.debug('Executing application under test with the following args:')
            for key, value in options.items():
                logging.debug('   %s: %s' % (key, value))
                app_cmd = app_cmd + [key, value]
                if key == '--KVS':
                    self.kvs = value
        return runner.RunSubprocess(app_cmd, name='APP ', wait=False)

    def __waitFor(self, waitForString, server_process, outpipe):
        logging.debug('Waiting for %s' % waitForString)

        start_time = time.time()
        ready, self.lastLogIndex = outpipe.CapturedLogContains(
            waitForString, self.lastLogIndex)
        while not ready:
            if server_process.poll() is not None:
                died_str = ('Server died while waiting for %s, returncode %d' %
                            (waitForString, server_process.returncode))
                logging.error(died_str)
                raise Exception(died_str)
            if time.time() - start_time > 10:
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


@dataclass
class ApplicationPaths:
    chip_tool: typing.List[str]
    all_clusters_app: typing.List[str]
    lock_app: typing.List[str]
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

    def Run(self, runner, apps_register, paths: ApplicationPaths, pics_file: str):
        """
        Executes the given test case using the provided runner for execution.
        """
        runner.capture_delegate = ExecutionCapture()

        try:
            if self.target == TestTarget.ALL_CLUSTERS:
                app_cmd = paths.all_clusters_app
            elif self.target == TestTarget.TV:
                app_cmd = paths.tv_app
            elif self.target == TestTarget.LOCK:
                app_cmd = paths.lock_app
            else:
                raise Exception("Unknown test target - "
                                "don't know which application to run")

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
            # Add the App to the register immediately, so if it fails during
            # start() we will be able to clean things up properly.
            apps_register.add("default", app)
            # Remove server application storage (factory reset),
            # so it will be commissionable again.
            app.factoryReset()
            app.start()
            pairing_cmd = tool_cmd + ['pairing', 'qrcode', TEST_NODE_ID, app.setupCode]
            if sys.platform != 'darwin':
                pairing_cmd.append('--paa-trust-store-path')
                pairing_cmd.append(DEVELOPMENT_PAA_LIST)

            runner.RunSubprocess(pairing_cmd,
                                 name='PAIR', dependencies=[apps_register])

            test_cmd = tool_cmd + ['tests', self.run_name] + ['--PICS', pics_file]
            if sys.platform != 'darwin':
                test_cmd.append('--paa-trust-store-path')
                test_cmd.append(DEVELOPMENT_PAA_LIST)
            runner.RunSubprocess(
                test_cmd,
                name='TEST', dependencies=[apps_register])

        except Exception:
            logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            raise
        finally:
            apps_register.killAll()
            apps_register.factoryResetAll()
            apps_register.removeAll()
