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
import shlex
import shutil
import subprocess
import tempfile
import threading
import time
import typing
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum, auto

from .runner import SubprocessInfo

log = logging.getLogger(__name__)

TEST_NODE_ID = '0x12344321'
TEST_DISCRIMINATOR = '3840'
TEST_PASSCODE = '20202021'
TEST_SETUP_QR_CODE = 'MT:-24J042C00KA0648G00'


class App:

    def __init__(self, runner, subproc: SubprocessInfo):
        self.process = None
        self.outpipe = None
        self.runner = runner
        self.subproc = subproc
        self.cv_stopped = threading.Condition()
        self.stopped = True
        self.lastLogIndex = 0
        self.kvsPathSet = {'/tmp/chip_kvs'}
        self.options = None
        self.killed = False

    def __repr__(self) -> str:
        return repr(self.subproc)

    @property
    def returncode(self):
        """Exposes return code of the underlying process, so that
           common code can be used between subprocess.Popen and Apps.
        """
        if not self.process:
            return None
        return self.process.returncode

    def start(self, options=None):
        if not self.process:
            # Cache command line options to be used for reboots
            if options:
                self.options = options
            # Make sure to assign self.process before we do any operations that
            # might fail, so attempts to kill us on failure actually work.
            self.process, self.outpipe, _ = self.__startServer()
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
            ok = self.__terminateProcess()
            if not ok:
                # For now just raise an exception; no other way to get tests to fail in this situation.
                raise Exception('Stopped subprocess terminated abnormally')
            return True
        return False

    def factoryReset(self):
        wasRunning = (not self.killed) and self.stop()

        for kvs in self.kvsPathSet:
            if os.path.exists(kvs):
                os.unlink(kvs)

        if wasRunning:
            return self.start()

        return True

    def waitForAnyAdvertisement(self):
        self.__waitFor("mDNS service published:", self.process, self.outpipe)

    def waitForMessage(self, message, timeoutInSeconds=10):
        self.__waitFor(message, self.process, self.outpipe, timeoutInSeconds)
        return True

    def kill(self):
        ok = self.__terminateProcess()
        self.killed = True
        return ok

    def wait(self, timeout=None):
        while True:
            # If the App was never started, AND was killed, exit immediately
            if self.killed:
                return 0
            # If the App was never started, wait cannot be called on the process
            if self.process is None:
                time.sleep(0.1)
                continue
            code = self.process.wait(timeout)
            with self.cv_stopped:
                if not self.stopped:
                    return code
                # When the server is manually stopped, process waiting is
                # overridden so the other processes that depends on the
                # accessory being alive does not stop.
                while self.stopped:
                    self.cv_stopped.wait()

    def __startServer(self):
        subproc = self.subproc.with_args('--interface-id', '-1')

        if not self.options:
            log.debug("Executing application under test with default args")
        else:
            log.debug("Executing application under test with the following args:")
            for key, value in self.options.items():
                log.debug("   %s: %s", key, value)
                subproc = subproc.with_args(key, value)
                if key == '--KVS':
                    self.kvsPathSet.add(value)
        return self.runner.RunSubprocess(subproc, name='APP ', wait=False)

    def __waitFor(self, waitForString, server_process, outpipe, timeoutInSeconds=10):
        log.debug("Waiting for '%s'", waitForString)

        start_time = time.monotonic()
        ready, self.lastLogIndex = outpipe.CapturedLogContains(
            waitForString, self.lastLogIndex)
        if ready:
            self.lastLogIndex += 1

        while not ready:
            if server_process.poll() is not None:
                died_str = ("Server died while waiting for %s, returncode %d" %
                            (waitForString, server_process.returncode))
                log.error(died_str)
                raise Exception(died_str)
            if time.monotonic() - start_time > timeoutInSeconds:
                raise Exception('Timeout while waiting for %s' % waitForString)
            time.sleep(0.1)
            ready, self.lastLogIndex = outpipe.CapturedLogContains(
                waitForString, self.lastLogIndex)
            if ready:
                self.lastLogIndex += 1

        log.debug("Success waiting for: '%s'", waitForString)

    def __updateSetUpCode(self):
        qrLine = self.outpipe.FindLastMatchingLine('.*SetupQRCode: *\\[(.*)]')
        if not qrLine:
            raise Exception("Unable to find QR code")
        self.setupCode = qrLine.group(1)

    def __terminateProcess(self):
        """
        Returns False if the process existed and had a nonzero exit code.
        """
        if self.process:
            self.process.terminate()  # sends SIGTERM
            try:
                exit_code = self.process.wait(10)
                if exit_code:
                    log.error("Subprocess failed with exit code: %d", exit_code)
                    return False
            except subprocess.TimeoutExpired:
                log.debug("Subprocess did not terminate on SIGTERM, killing it now")
                self.process.kill()
                # The exit code when using Python subprocess will be the signal used to kill it.
                # Ideally, we would recover the original exit code, but the process was already
                # ignoring SIGTERM, indicating something was already wrong.
                self.process.wait(10)
            self.process = None
            self.outpipe = None
        return True


class TestTarget(Enum):
    ALL_CLUSTERS = auto()
    TV = auto()
    LOCK = auto()
    OTA = auto()
    BRIDGE = auto()
    LIT_ICD = auto()
    FABRIC_SYNC = auto()
    MWO = auto()
    RVC = auto()
    NETWORK_MANAGER = auto()
    ENERGY_GATEWAY = auto()
    ENERGY_MANAGEMENT = auto()
    CLOSURE = auto()


@dataclass
class ApplicationPaths:
    chip_tool: SubprocessInfo
    all_clusters_app: SubprocessInfo
    lock_app: SubprocessInfo
    fabric_bridge_app: SubprocessInfo
    ota_provider_app: SubprocessInfo
    ota_requestor_app: SubprocessInfo
    tv_app: SubprocessInfo
    bridge_app: SubprocessInfo
    lit_icd_app: SubprocessInfo
    microwave_oven_app: SubprocessInfo
    matter_repl_yaml_tester_cmd: SubprocessInfo
    chip_tool_with_python_cmd: SubprocessInfo
    rvc_app: SubprocessInfo
    network_manager_app: SubprocessInfo
    energy_gateway_app: SubprocessInfo
    energy_management_app: SubprocessInfo
    closure_app: SubprocessInfo

    def items(self):
        return [self.chip_tool, self.all_clusters_app, self.lock_app,
                self.fabric_bridge_app, self.ota_provider_app, self.ota_requestor_app,
                self.tv_app, self.bridge_app, self.lit_icd_app,
                self.microwave_oven_app, self.matter_repl_yaml_tester_cmd,
                self.chip_tool_with_python_cmd, self.rvc_app, self.network_manager_app,
                self.energy_gateway_app, self.energy_management_app, self.closure_app]

    def items_with_key(self):
        """
        Returns all path items and also the corresponding "Application Key" which
        is the typical application name.

        This is to provide scripts a consistent way to reference a path, even if
        the paths used for individual appplications contain different names
        (e.g. they could be wrapper scripts).
        """
        return [
            (self.chip_tool, "chip-tool"),
            (self.all_clusters_app, "chip-all-clusters-app"),
            (self.lock_app, "chip-lock-app"),
            (self.fabric_bridge_app, "fabric-bridge-app"),
            (self.ota_provider_app, "chip-ota-provider-app"),
            (self.ota_requestor_app, "chip-ota-requestor-app"),
            (self.tv_app, "chip-tv-app"),
            (self.bridge_app, "chip-bridge-app"),
            (self.lit_icd_app, "lit-icd-app"),
            (self.microwave_oven_app, "chip-microwave-oven-app"),
            (self.matter_repl_yaml_tester_cmd, "yamltest_with_matter_repl_tester.py"),
            (
                # This path varies, however it is a fixed python tool so it may be ok
                self.chip_tool_with_python_cmd,
                self.chip_tool_with_python_cmd.path.name,
            ),
            (self.rvc_app, "chip-rvc-app"),
            (self.network_manager_app, "matter-network-manager-app"),
            (self.energy_gateway_app, "chip-energy-gateway-app"),
            (self.energy_management_app, "chip-energy-management-app"),
            (self.closure_app, "closure-app"),
        ]


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
        log.error("================ CAPTURED LOG START ==================")
        with self.lock:
            for entry in self.captures:
                log.error("%02d:%02d:%02d.%03d - %-10s: %s",
                          entry.when.hour,
                          entry.when.minute,
                          entry.when.second,
                          entry.when.microsecond/1000,
                          entry.source,
                          entry.line
                          )
        log.error("================ CAPTURED LOG END ====================")


class TestTag(Enum):
    MANUAL = auto()          # requires manual input. Generally not run automatically
    SLOW = auto()            # test uses Sleep and is generally slow (>=10s is a typical threshold)
    FLAKY = auto()           # test is considered flaky (usually a bug/time dependent issue)
    IN_DEVELOPMENT = auto()  # test may not pass or undergoes changes
    CHIP_TOOL_PYTHON_ONLY = auto()  # test uses YAML features only supported by the CHIP_TOOL_PYTHON runner.
    EXTRA_SLOW = auto()      # test uses Sleep and is generally _very_ slow (>= 60s is a typical threshold)
    PURPOSEFUL_FAILURE = auto()  # test fails on purpose

    def to_s(self):
        for (k, v) in TestTag.__members__.items():
            if self == v:
                return k
        raise Exception("Unknown tag: %r" % self)


class TestRunTime(Enum):
    CHIP_TOOL_PYTHON = auto()  # use the python yaml test parser with chip-tool
    DARWIN_FRAMEWORK_TOOL_PYTHON = auto()  # use the python yaml test parser with chip-tool
    MATTER_REPL_PYTHON = auto()       # use the python yaml test runner


@dataclass
class TestDefinition:
    name: str
    run_name: str
    target: TestTarget
    tags: typing.Set[TestTag] = field(default_factory=set)

    @property
    def is_manual(self) -> bool:
        return TestTag.MANUAL in self.tags

    @property
    def is_slow(self) -> bool:
        return TestTag.SLOW in self.tags

    @property
    def is_flaky(self) -> bool:
        return TestTag.FLAKY in self.tags

    def tags_str(self) -> str:
        """Get a human readable list of tags applied to this test"""
        return ", ".join([t.to_s() for t in self.tags])

    def Run(self, runner, apps_register, apps: ApplicationPaths, pics_file: str,
            timeout_seconds: typing.Optional[int], dry_run=False,
            test_runtime: TestRunTime = TestRunTime.CHIP_TOOL_PYTHON,
            ble_controller_app: typing.Optional[int] = None,
            ble_controller_tool: typing.Optional[int] = None):
        """
        Executes the given test case using the provided runner for execution.
        """
        runner.capture_delegate = ExecutionCapture()

        tool_storage_dir = None

        loggedCapturedLogs = False

        try:
            if self.target == TestTarget.ALL_CLUSTERS:
                target_app = apps.all_clusters_app
            elif self.target == TestTarget.TV:
                target_app = apps.tv_app
            elif self.target == TestTarget.LOCK:
                target_app = apps.lock_app
            elif self.target == TestTarget.FABRIC_SYNC:
                target_app = apps.fabric_bridge_app
            elif self.target == TestTarget.OTA:
                target_app = apps.ota_requestor_app
            elif self.target == TestTarget.BRIDGE:
                target_app = apps.bridge_app
            elif self.target == TestTarget.LIT_ICD:
                target_app = apps.lit_icd_app
            elif self.target == TestTarget.MWO:
                target_app = apps.microwave_oven_app
            elif self.target == TestTarget.RVC:
                target_app = apps.rvc_app
            elif self.target == TestTarget.NETWORK_MANAGER:
                target_app = apps.network_manager_app
            elif self.target == TestTarget.ENERGY_GATEWAY:
                target_app = apps.energy_gateway_app
            elif self.target == TestTarget.ENERGY_MANAGEMENT:
                target_app = apps.energy_management_app
            elif self.target == TestTarget.CLOSURE:
                target_app = apps.closure_app
            else:
                raise Exception("Unknown test target - "
                                "don't know which application to run")

            if not dry_run:
                for command, key in apps.items_with_key():
                    # Do not add chip-tool or matter-repl-yaml-tester-cmd to the register
                    if (command == apps.chip_tool
                            or command == apps.matter_repl_yaml_tester_cmd
                            or command == apps.chip_tool_with_python_cmd):
                        continue

                    # Skip items where we don't actually have a path.  This can
                    # happen if the relevant application does not exist.  It's
                    # non-fatal as long as we are not trying to run any tests that
                    # need that application.
                    if command is None:
                        continue

                    # For the app indicated by self.target, give it the 'default' key to add to the register
                    if command == target_app:
                        key = 'default'
                    if ble_controller_app is not None:
                        command = command.with_args("--ble-controller", str(ble_controller_app), "--wifi")
                    app = App(runner, command)
                    # Add the App to the register immediately, so if it fails during
                    # start() we will be able to clean things up properly.
                    apps_register.add(key, app)
                    # Remove server application storage (factory reset),
                    # so it will be commissionable again.
                    app.factoryReset()

                    # It may sometimes be useful to run the same app multiple times depending
                    # on the implementation. So this code creates a duplicate entry but with a different
                    # key.
                    app = App(runner, command)
                    apps_register.add(f'{key}#2', app)
                    app.factoryReset()

            if dry_run:
                tool_storage_dir = None
                tool_storage_args = []
            else:
                tool_storage_dir = tempfile.mkdtemp()
                tool_storage_args = ['--storage-directory', tool_storage_dir]

            # Only start and pair the default app
            if dry_run:
                setupCode = '${SETUP_PAYLOAD}'
            else:
                app = apps_register.get('default')
                app.start()
                setupCode = app.setupCode

            if test_runtime == TestRunTime.MATTER_REPL_PYTHON:
                python_cmd = apps.matter_repl_yaml_tester_cmd.with_args(
                    '--setup-code', setupCode, '--yaml-path', self.run_name, "--pics-file", pics_file)

                if dry_run:
                    log.info(shlex.join(python_cmd))
                else:
                    runner.RunSubprocess(python_cmd, name='MATTER_REPL_YAML_TESTER',
                                         dependencies=[apps_register], timeout_seconds=timeout_seconds)
            else:
                pairing_server_args = []

                if ble_controller_tool is not None:
                    pairing_cmd = apps.chip_tool_with_python_cmd.with_args(
                        "pairing", "code-wifi", TEST_NODE_ID, "MatterAP", "MatterAPPassword", TEST_SETUP_QR_CODE)
                    pairing_server_args = ["--ble-controller", str(ble_controller_tool)]
                else:
                    pairing_cmd = apps.chip_tool_with_python_cmd.with_args('pairing', 'code', TEST_NODE_ID, setupCode)

                if self.target == TestTarget.LIT_ICD and test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    pairing_cmd = pairing_cmd.with_args('--icd-registration', 'true')

                test_cmd = apps.chip_tool_with_python_cmd.with_args('tests', self.run_name, '--PICS', pics_file)

                interactive_server_args = ['interactive server'] + tool_storage_args + pairing_server_args

                if test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    interactive_server_args = interactive_server_args + ['--interface-id', '-1']

                server_args = (
                    '--server_path', str(apps.chip_tool.path),
                    '--server_arguments', ' '.join(interactive_server_args))

                pairing_cmd = pairing_cmd.with_args(*server_args)
                test_cmd = test_cmd.with_args(*server_args)

                if dry_run:
                    log.info(shlex.join(pairing_cmd))
                    log.info(shlex.join(test_cmd))
                else:
                    runner.RunSubprocess(pairing_cmd,
                                         name='PAIR', dependencies=[apps_register])
                    runner.RunSubprocess(
                        test_cmd,
                        name='TEST', dependencies=[apps_register],
                        timeout_seconds=timeout_seconds)

        except Exception:
            log.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            loggedCapturedLogs = True
            raise
        finally:
            ok = apps_register.killAll()
            apps_register.factoryResetAll()
            apps_register.removeAll()
            if tool_storage_dir is not None:
                shutil.rmtree(tool_storage_dir, ignore_errors=True)
            # If loggedCapturedLogs then we are already throwing, so no need to
            # try to trigger test failure due to our abnormal termination.
            if not ok and not loggedCapturedLogs:
                log.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
                runner.capture_delegate.LogContents()
                raise Exception('Subprocess terminated abnormally')
