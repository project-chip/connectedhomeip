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
from pathlib import Path

from chiptest.accessories import AppsRegister
from chiptest.runner import SubprocessInfo, SubprocessKind

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
        if self.outpipe:
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
    ALL_CLUSTERS = 'all-clusters'
    TV = 'tv'
    LOCK = 'lock'
    OTA = 'ota-requestor'
    BRIDGE = 'bridge'
    LIT_ICD = 'lit-icd'
    FABRIC_SYNC = 'fabric-sync'
    MWO = 'microwave-oven'
    RVC = 'rvc'
    NETWORK_MANAGER = 'network-manager'
    ENERGY_GATEWAY = 'energy-gateway'
    ENERGY_MANAGEMENT = 'energy-management'
    CLOSURE = 'closure'


@dataclass
class SubprocEntry:
    kind: SubprocessKind
    path_lookup: str | None = None
    wrapper: tuple[str] | None = None


BUILTIN_SUBPROC_KNOWHOW = {
    # Matter applications
    'all-clusters': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-all-clusters-app'),
    'lock': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-lock-app'),
    'fabric-bridge': SubprocEntry(kind=SubprocessKind.APP, path_lookup='fabric-bridge-app'),
    'ota-provider': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-ota-provider-app'),
    'ota-requestor': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-ota-requestor-app'),
    'tv': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-tv-app'),
    'bridge': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-bridge-app'),
    'lit-icd': SubprocEntry(kind=SubprocessKind.APP, path_lookup='lit-icd-app'),
    'microwave-oven': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-microwave-oven-app'),
    'rvc': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-rvc-app'),
    'network-manager': SubprocEntry(kind=SubprocessKind.APP, path_lookup='matter-network-manager-app'),
    'energy-gateway': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-energy-gateway-app'),
    'energy-management': SubprocEntry(kind=SubprocessKind.APP, path_lookup='chip-energy-management-app'),
    'closure': SubprocEntry(kind=SubprocessKind.APP, path_lookup='closure-app'),

    # Tools
    'chip-tool': SubprocEntry(kind=SubprocessKind.TOOL, path_lookup='chip-tool'),
    'darwin-framework-tool': SubprocEntry(kind=SubprocessKind.TOOL, path_lookup='darwin-framework-tool'),
    'matter-repl-yaml-tester': SubprocEntry(kind=SubprocessKind.TOOL, path_lookup='yamltest_with_matter_repl_tester.py'),

    # No path_lookup as this is either chiptool.py or darwinframework.py depending on the platform
    'chip-tool-with-python': SubprocEntry(kind=SubprocessKind.TOOL)
}


class SubprocessInfoRepo(dict):
    # We don't want to explicitly reference PathsFinder type because we don't want to create a dependency on
    # diskcache which PathsFinder imports. Instead we just want a dict-like object
    def __init__(self, paths: dict[str, Path],
                 subproc_knowhow: dict[str, SubprocEntry] = BUILTIN_SUBPROC_KNOWHOW,
                 *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.paths = paths
        self.subproc_knowhow = subproc_knowhow

    def addSpec(self, spec: str, kind: SubprocessKind | None = None):
        """Add a path to the repo as specified on the command line"""

        el = spec.split(':')
        if len(el) == 3:
            # <kind>:<key>:<path>
            kind_s, key, path = el
            kind = SubprocessKind[kind_s]
            path = Path(path)
        elif len(el) == 2:
            # <key>:<path>
            key, path = el
            path = Path(path)
            if kind is None:
                kind = self.subproc_knowhow[key].kind
        else:
            raise RuntimeError("Cannot parse path spec '%s'", spec)

        s = SubprocessInfo(kind=kind, path=path)
        if path.suffix == '.py':
            s = s.wrap_with('python3')

        self[key] = s

    def require(self, key: str, kind: SubprocessKind | None = None, path_lookup: str | None = None):
        """
        Indicate that a subprocess path is required. Throw exception if it's not already in the repo
        and can't be discovered using the paths finder.
        """
        if key in self:
            return

        if kind is None:
            kind = self.subproc_knowhow[key].kind
        if path_lookup is None:
            path_lookup = self.subproc_knowhow[key].path_lookup
            if path_lookup is None:
                raise RuntimeError(f"No path lookup provided nor present in knowhow for key {key}")

        if (path := self.paths.get(path_lookup)) is None:
            raise RuntimeError("Cannot find path for '%s'", key)

        logging.info("Discovered '%s' path '%s'", key, path)
        s = SubprocessInfo(kind=kind, path=path)
        if path.suffix == '.py':
            s = s.wrap_with('python3')

        self[key] = s


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

    def Run(self, runner, apps_register: AppsRegister, subproc_info_repo: SubprocessInfoRepo, pics_file: str,
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
            target_key = self.target.value
            subproc_info_repo.require(target_key)
            if not dry_run:
                for key, subproc in subproc_info_repo.items():
                    # Do not add tools to the app register
                    if subproc.kind == 'tool':
                        continue

                    # For the app indicated by self.target, give it the 'default' key to add to the register
                    if key == target_key:
                        key = 'default'
                    if ble_controller_app is not None:
                        subproc = subproc.with_args("--ble-controller", str(ble_controller_app), "--wifi")
                    app = App(runner, subproc)
                    # Add the App to the register immediately, so if it fails during
                    # start() we will be able to clean things up properly.
                    apps_register.add(key, app)
                    # Remove server application storage (factory reset),
                    # so it will be commissionable again.
                    app.factoryReset()

                    # It may sometimes be useful to run the same app multiple times depending
                    # on the implementation. So this code creates a duplicate entry but with a different
                    # key.
                    app = App(runner, subproc)
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
                repl_cmd = subproc_info_repo['matter-repl-yaml-tester'].with_args(
                    '--setup-code', setupCode, '--yaml-path', self.run_name, "--pics-file", pics_file)

                if dry_run:
                    log.info(shlex.join(repl_cmd))
                else:
                    runner.RunSubprocess(repl_cmd, name='MATTER_REPL_YAML_TESTER',
                                         dependencies=[apps_register], timeout_seconds=timeout_seconds)
            else:
                pairing_server_args = []

                if ble_controller_tool is not None:
                    pairing_cmd = subproc_info_repo['chip-tool-with-python'].with_args(
                        "pairing", "code-wifi", TEST_NODE_ID, "MatterAP", "MatterAPPassword", TEST_SETUP_QR_CODE)
                    pairing_server_args = ["--ble-controller", str(ble_controller_tool)]
                else:
                    pairing_cmd = subproc_info_repo['chip-tool-with-python'].with_args('pairing', 'code', TEST_NODE_ID, setupCode)

                if self.target == TestTarget.LIT_ICD and test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    pairing_cmd = pairing_cmd.with_args('--icd-registration', 'true')

                test_cmd = subproc_info_repo['chip-tool-with-python'].with_args('tests', self.run_name, '--PICS', pics_file)

                interactive_server_args = ['interactive server'] + tool_storage_args + pairing_server_args

                if test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    interactive_server_args = interactive_server_args + ['--interface-id', '-1']

                server_args = (
                    '--server_path', str(subproc_info_repo['chip-tool'].path),
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
