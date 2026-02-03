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
from collections.abc import Iterable
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum, StrEnum, auto
from pathlib import Path
from types import MappingProxyType

from .accessories import AppsRegister
from .runner import LogPipe, Runner, SubprocessInfo, SubprocessKind

log = logging.getLogger(__name__)

TEST_NODE_ID = '0x12344321'
TEST_DISCRIMINATOR = '3840'
TEST_PASSCODE = '20202021'
TEST_SETUP_QR_CODE = 'MT:-24J042C00KA0648G00'
TEST_THREAD_DATASET = '0e08000000000001000000030000104a0300001635060004001fffe0020884fa18779329ac770708fd269658e44aa21a030f4f70656e5468726561642d32386335010228c50c0402a0f7f8051000112233445566778899aabbccddeeff041000112233445566778899aabbccddeeff'


class App:
    def __init__(self, runner: Runner, subproc: SubprocessInfo):
        self.process: subprocess.Popen[bytes] | None = None
        self.outpipe: LogPipe | None = None
        self.runner = runner
        self.subproc = subproc
        self.cv_stopped = threading.Condition()
        self.stopped = True
        self.lastLogIndex = 0
        self.kvsPathSet = {'/tmp/chip_kvs'}
        self.options: dict[str, str] | None = None
        self.killed = False
        self.setupCode: str | None = None

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

    def start(self, options: dict[str, str] | None = None) -> bool:
        if not self.process:
            # Cache command line options to be used for reboots
            if options:
                self.options = options
            # Make sure to assign self.process before we do any operations that
            # might fail, so attempts to kill us on failure actually work.
            self.process, self.outpipe, _ = self.__startServer()
            self.waitForApplicationUp()
            self.__updateSetUpCode()
            with self.cv_stopped:
                self.stopped = False
                self.cv_stopped.notify()
            return True
        return False

    def stop(self) -> bool:
        if self.process:
            with self.cv_stopped:
                self.stopped = True
                self.cv_stopped.notify()
            ok = self.__terminateProcess()
            if not ok:
                # For now just raise an exception; no other way to get tests to fail in this situation.
                raise RuntimeError('Stopped subprocess terminated abnormally')
            return True
        return False

    def factoryReset(self) -> bool:
        wasRunning = (not self.killed) and self.stop()

        for kvs in self.kvsPathSet:
            if os.path.exists(kvs):
                os.unlink(kvs)

        if wasRunning:
            return self.start()

        return True

    def waitForApplicationUp(self):
        # Watch for both mDNS advertisement start as well as event loop start.
        # These two messages can appear in any order depending on the implementation.
        # Waiting for both makes the startup detection more robust.
        #
        # For a Thread-only node, the mDNS advertisement is not started immediately
        # after start, so it is skipped for Thread node.
        assert self.process is not None and self.outpipe is not None, "waitForApplicationUp can be called only after start()"

        what = ["APP STATUS: Starting event loop"]
        if not any(arg.startswith("--thread-node-id=") for arg in self.subproc.args):
            what += ["mDNS service published:"]

        self.__waitFor(what)

    def waitForMessage(self, message: str, timeoutInSeconds: float = 10):
        self.__waitFor([message], timeoutInSeconds=timeoutInSeconds)
        return True

    def kill(self) -> bool:
        ok = self.__terminateProcess()
        self.killed = True
        return ok

    def wait(self, timeout: float | None = None) -> int:
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

    def __startServer(self) -> tuple[subprocess.Popen[bytes], LogPipe, LogPipe]:
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

    def __waitFor(self, patterns: Iterable[str], timeoutInSeconds: float = 10):
        """
        Wait for all provided pattern strings to appear in the process output pipe (capture log).
        """
        assert self.process is not None and self.outpipe is not None, "__waitFor can be called only after start()"
        log.debug('Waiting for all patterns %r', patterns)

        start_time = time.monotonic()

        def allPatternsFound() -> int | None:
            assert self.outpipe is not None
            lastLogIndex = self.lastLogIndex
            for p in patterns:
                found, index = self.outpipe.CapturedLogContains(p, self.lastLogIndex)
                if not found:
                    return None
                lastLogIndex = max(lastLogIndex, index)

            return lastLogIndex

        lastLogIndex = allPatternsFound()
        while lastLogIndex is None:
            if self.process.poll() is not None:
                died_str = f'Server died while waiting for {patterns!r}, returncode {self.process.returncode}'
                log.error(died_str)
                raise RuntimeError(died_str)
            if time.monotonic() - start_time > timeoutInSeconds:
                raise TimeoutError(f'Timeout while waiting for {patterns!r}')
            time.sleep(0.1)

            lastLogIndex = allPatternsFound()

        self.lastLogIndex = lastLogIndex + 1
        log.debug('Success waiting for: %r', patterns)

    def __updateSetUpCode(self):
        assert self.outpipe is not None, "__updateSetUpCode needs to happen after __startServer"
        qrLine = self.outpipe.FindLastMatchingLine('.*SetupQRCode: *\\[(.*)]')
        if not qrLine:
            raise RuntimeError("Unable to find QR code")
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
            self.lastLogIndex = 0
        return True


@dataclass
class TestTarget:
    name: str

    # command to execute. MUST be a placeholder like tv or lock
    command: str

    # arguments to pass in to the command to execute
    arguments: list[str] = field(default_factory=list)


@dataclass
class KnownSubprocessEntry:
    kind: SubprocessKind
    target_name: str | None = None


BUILTIN_SUBPROC_DATA = MappingProxyType({
    # Matter applications
    'all-clusters': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-all-clusters-app'),
    'all-devices': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='all-devices-app'),
    'air-purifier': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-air-purifier-app'),
    'bridge': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-bridge-app'),
    'camera': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-camera-app'),
    'camera-controller': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-camera-controller'),
    'closure': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='closure-app'),
    'energy-gateway': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-energy-gateway-app'),
    'evse': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-evse-app'),
    'fabric-bridge': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='fabric-bridge-app'),
    'fabric-admin': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='fabric-admin'),
    'fabric-sync': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='fabric-sync'),
    'jf-control': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='jfc-app'),
    'jf-admin': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='jfa-app'),
    'light': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-lighting-app'),
    'lit-icd': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='lit-icd-app'),
    'lock': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-lock-app'),
    'microwave-oven': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-microwave-oven-app'),
    'network-manager': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='matter-network-manager-app'),
    'ota-provider': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-ota-provider-app'),
    'ota-requestor': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-ota-requestor-app'),
    'rvc': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-rvc-app'),
    'terms-and-conditions': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-terms-and-conditions-app'),
    'tv': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='chip-tv-app'),
    'water-leak-detector': KnownSubprocessEntry(kind=SubprocessKind.APP, target_name='water-leak-detector-app'),

    # Tools
    'chip-tool': KnownSubprocessEntry(kind=SubprocessKind.TOOL, target_name='chip-tool'),
    'darwin-framework-tool': KnownSubprocessEntry(kind=SubprocessKind.TOOL, target_name='darwin-framework-tool'),
    'matter-repl-yaml-tester': KnownSubprocessEntry(kind=SubprocessKind.TOOL, target_name='yamltest_with_matter_repl_tester.py'),

    # No target_name as this is either chiptool.py or darwinframework.py depending on the selected TestRunTime
    'chip-tool-with-python': KnownSubprocessEntry(kind=SubprocessKind.TOOL)
})


class PathsFinderProto(typing.Protocol):
    def get(self, target_name: str) -> Path | None:
        pass


class SubprocessInfoRepo(dict):
    # We don't want to explicitly reference PathsFinder type because we
    # don't want to create a dependency on the diskcache module which PathsFinder imports.
    # Instead we just want a dict-like object
    def __init__(self, paths: PathsFinderProto,
                 subproc_knowhow: MappingProxyType[str, KnownSubprocessEntry] = BUILTIN_SUBPROC_DATA,
                 *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.paths = paths
        self.subproc_knowhow = subproc_knowhow

    def addSpec(self, spec: str, kind: SubprocessKind | None = None):
        """Add a path to the repo as specified on the command line"""
        el = spec.split(':')
        if len(el) == 2:
            # <key>:<path>
            key, path_s = el
            path = Path(path_s)
            if kind is None:
                if key not in self.subproc_knowhow:
                    raise ValueError(f"Kind not provided for key '{key}' and not specified in know-how")
                kind = self.subproc_knowhow[key].kind
        else:
            raise ValueError(f"Cannot parse path spec '{spec}'")

        if key not in self.subproc_knowhow:
            log.warning("Key '%s' is not present in the subprocess repo know-how, possible typo", key)

        s = SubprocessInfo(kind=kind, path=path)
        if path.suffix == '.py':
            s = s.wrap_with('python3')
        self[key] = s

    def missing_keys(self):
        """
        Return a list of keys for tools or apps missing (not specified) based on the
        know-how dictionary.
        """
        return [k for k in self.subproc_knowhow if k not in self]

    def discover(self):
        """
        Try to discover paths to all apps and tools in the know-how which we are still missing.
        Reuse the `require` method but ignore failures, we expect the test-cases to fail if they
        depend on missing paths.
        """
        log.info("Discovering missing paths")
        start_ts = time.time()
        discovered_count = 0
        for key in self.missing_keys():
            try:
                self.require(key)
                discovered_count += 1
            except (LookupError, ValueError) as e:
                # Stack trace is not needed here
                log.warning("Exception while trying to discover '%s': %r", key, e)
        log.info("Discovery of %d paths took %.2f seconds", discovered_count, time.time() - start_ts)

    def require(self, key: str, kind: SubprocessKind | None = None, target_name: str | None = None):
        """
        Indicate that a subprocess path is required. Throw exception if it's not already in the repo
        and can't be discovered using the paths finder.
        """
        if key in self:
            return self[key]
        if kind is None:
            if key not in self.subproc_knowhow:
                raise ValueError(f"Key '{key}': kind neither provided nor specified in know-how")
            kind = self.subproc_knowhow[key].kind
        if target_name is None:
            if key not in self.subproc_knowhow:
                raise ValueError(f"Key '{key}': target name neither provided nor specified in know-how")
            target_name = self.subproc_knowhow[key].target_name
            if target_name is None:
                raise ValueError(f"Key '{key}': Key exists in know-how but no target name specified")
        if (path := self.paths.get(target_name)) is None:
            raise LookupError(f"Cannot find path for required {kind} key '{key}'")
        log.info("Discovered required key '%s' path '%s'", key, path)
        s = SubprocessInfo(kind=kind, path=path)
        if path.suffix == '.py':
            s = s.wrap_with('python3')
        self[key] = s
        return self[key]


@dataclass
class CaptureLine:
    when: datetime
    source: str
    line: str


class ExecutionCapture:
    """
    Keeps track of output lines in a process, to help debug failures.
    """

    def __init__(self) -> None:
        self.lock = threading.Lock()
        self.captures: list[CaptureLine] = []

    def Log(self, source: str, line: str):
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


class TestTag(StrEnum):
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
        raise KeyError(f"Unknown tag: {self!r}")


class TestRunTime(Enum):
    CHIP_TOOL_PYTHON = auto()  # use the python yaml test parser with chip-tool
    DARWIN_FRAMEWORK_TOOL_PYTHON = auto()  # use the python yaml test parser with chip-tool
    MATTER_REPL_PYTHON = auto()       # use the python yaml test runner


@dataclass
class TestDefinition:
    name: str
    run_name: str
    targets: list[TestTarget]
    tags: set[TestTag] = field(default_factory=set)

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

    def Run(self, runner: Runner, apps_register: AppsRegister, subproc_info_repo: SubprocessInfoRepo,
            pics_file: Path, timeout_seconds: int | None, dry_run: bool = False,
            test_runtime: TestRunTime = TestRunTime.CHIP_TOOL_PYTHON,
            ble_controller_app: int | None = None,
            ble_controller_tool: int | None = None,
            op_network: str = 'WiFi',
            ):
        """
        Executes the given test case using the provided runner for execution.
        Will iterate and execute every target.
        """
        for target in self.targets:
            log.info('Executing %s::%s', self.name, target.name)
            self._RunImpl(target, runner, apps_register, subproc_info_repo, pics_file, timeout_seconds, dry_run,
                          test_runtime, ble_controller_app, ble_controller_tool, op_network)

    def _RunImpl(self, target: TestTarget, runner: Runner, apps_register: AppsRegister, subproc_info_repo: SubprocessInfoRepo,
                 pics_file: Path, timeout_seconds: int | None, dry_run: bool = False,
                 test_runtime: TestRunTime = TestRunTime.CHIP_TOOL_PYTHON,
                 ble_controller_app: int | None = None,
                 ble_controller_tool: int | None = None,
                 op_network: str = 'WiFi'):
        runner.capture_delegate = ExecutionCapture()

        tool_storage_dir = None

        loggedCapturedLogs = False
        try:
            if target.command not in subproc_info_repo:
                log.warning("Path to default target '%s' for test '%s' is not known, test will likely fail",
                            target.command, self.name)
            if not dry_run:
                for key, subproc in subproc_info_repo.items():
                    # Do not add tools to the register
                    if subproc.kind == SubprocessKind.TOOL:
                        continue

                    # For the app indicated by target, give it the 'default' key to add to the register
                    if key == target.command:
                        key = 'default'
                        for arg in target.arguments:
                            subproc = subproc.with_args(arg)

                    if ble_controller_app is not None:
                        subproc = subproc.with_args("--ble-controller", str(ble_controller_app))
                        if op_network == 'WiFi':
                            subproc = subproc.with_args("--wifi")
                        elif op_network == 'Thread':
                            # The node id must not conflict with ThreadBorderRouter.NODE_ID
                            subproc = subproc.with_args("--thread-node-id=2")

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
                assert app.setupCode is not None, "Setup code should have been set in app.start()"
                setupCode = app.setupCode

            if test_runtime == TestRunTime.MATTER_REPL_PYTHON:
                assert 'matter-repl-yaml-tester' in subproc_info_repo, \
                    "Matter REPL YAML tester should have been set for selected test runtime"
                python_cmd = subproc_info_repo['matter-repl-yaml-tester'].with_args(
                    '--setup-code', setupCode, '--yaml-path', self.run_name, "--pics-file", str(pics_file))

                if dry_run:
                    log.info(shlex.join(python_cmd.to_cmd()))
                else:
                    runner.RunSubprocess(python_cmd, name='MATTER_REPL_YAML_TESTER',
                                         dependencies=[apps_register], timeout_seconds=timeout_seconds)
            else:  # CHIP_TOOL_PYTHON
                assert 'chip-tool' in subproc_info_repo, \
                    "Chip tool should have been set for selected test runtime"
                assert 'chip-tool-with-python' in subproc_info_repo, \
                    "Chip tool with Python should have been set for selected test runtime"
                pairing_server_args = []

                pairing_cmd = subproc_info_repo['chip-tool-with-python']
                if ble_controller_tool is not None:
                    if op_network == 'WiFi':
                        pairing_cmd = pairing_cmd.with_args(
                            "pairing", "code-wifi", TEST_NODE_ID, "MatterAP", "MatterAPPassword", TEST_SETUP_QR_CODE)
                        pairing_server_args = ["--ble-controller", str(ble_controller_tool)]
                    elif op_network == 'Thread':
                        pairing_cmd = pairing_cmd.with_args(
                            "pairing", "code-thread", TEST_NODE_ID, f"hex:{TEST_THREAD_DATASET}", TEST_SETUP_QR_CODE)
                        pairing_server_args = ["--ble-controller", str(ble_controller_tool)]
                else:
                    pairing_cmd = pairing_cmd.with_args('pairing', 'code', TEST_NODE_ID, setupCode)

                if target.command == 'lit-icd' and test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    pairing_cmd = pairing_cmd.with_args('--icd-registration', 'true')

                test_cmd = subproc_info_repo['chip-tool-with-python'].with_args('tests', self.run_name, '--PICS', str(pics_file))

                interactive_server_args = ['interactive server'] + tool_storage_args + pairing_server_args

                if test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                    interactive_server_args = interactive_server_args + ['--interface-id', '-1']

                server_args = (
                    '--server_path', str(subproc_info_repo['chip-tool'].path),
                    '--server_arguments', ' '.join(interactive_server_args))

                pairing_cmd = pairing_cmd.with_args(*server_args)
                test_cmd = test_cmd.with_args(*server_args)

                if dry_run:
                    log.info("Pairing command: %s", shlex.join(pairing_cmd.to_cmd()))
                    log.info("Testcase command: %s", shlex.join(test_cmd.to_cmd()))
                else:
                    runner.RunSubprocess(pairing_cmd,
                                         name='PAIR', dependencies=[apps_register])
                    runner.RunSubprocess(
                        test_cmd,
                        name='TEST', dependencies=[apps_register],
                        timeout_seconds=timeout_seconds)

        except BaseException:
            log.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            loggedCapturedLogs = True
            raise
        finally:
            ok = apps_register.kill_all()
            apps_register.factory_reset_all()
            apps_register.remove_all()
            if tool_storage_dir is not None:
                shutil.rmtree(tool_storage_dir, ignore_errors=True)
            # If loggedCapturedLogs then we are already throwing, so no need to
            # try to trigger test failure due to our abnormal termination.
            if not ok and not loggedCapturedLogs:
                log.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
                runner.capture_delegate.LogContents()
                raise RuntimeError('Subprocess terminated abnormally')
