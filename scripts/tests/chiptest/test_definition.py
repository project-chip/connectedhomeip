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
import threading
import typing
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum, auto

from .application_paths import ApplicationPaths
from .applications import ChipReplCommissionerApp, ChipToolCommissionerApp, CommissioneeApp


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
    CHIP_REPL_PYTHON = auto()       # use the python yaml test runner


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

    def Run(self, runner, apps_register, paths: ApplicationPaths, pics_file: str,
            timeout_seconds: typing.Optional[int], dry_run=False, test_runtime: TestRunTime = TestRunTime.CHIP_TOOL_PYTHON):
        """
        Executes the given test case using the provided runner for execution.
        """
        runner.capture_delegate = ExecutionCapture()

        try:
            setupCode = '${SETUP_PAYLOAD}'
            restore_state = test_runtime != TestRunTime.CHIP_REPL_PYTHON

            if not dry_run:
                self._setup_apps(runner, apps_register, paths, restore_state)
                # Only start and pair the default app
                app = apps_register.get('default')
                app.start()
                setupCode = app.setupCode

            extra_pairing_args = []
            extra_test_args = []

            controller = None

            if test_runtime == TestRunTime.CHIP_REPL_PYTHON:
                controller = ChipReplCommissionerApp(runner, apps_register, paths, dry_run)
            elif test_runtime == TestRunTime.CHIP_TOOL_PYTHON:
                if self.target == TestTarget.LIT_ICD:
                    extra_pairing_args = ['--icd-registration', 'true']
                controller = ChipToolCommissionerApp(runner, apps_register, paths, dry_run)
            elif test_runtime == TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON:
                controller = ChipToolCommissionerApp(runner, apps_register, paths, dry_run)
            else:
                raise Exception("Unknown test runtime - "
                                "don't know which application to run")

            controller.pair(setupCode, extra_pairing_args)
            controller.test(self.run_name, pics_file, timeout_seconds, extra_test_args)

        except Exception:
            logging.error("!!!!!!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!!!!!!!!")
            runner.capture_delegate.LogContents()
            raise
        finally:
            apps_register.killAll()
            apps_register.factoryResetAll()
            apps_register.removeAll()
            if controller:
                controller.factoryReset()

    def _setup_apps(self, runner, apps_register, paths: ApplicationPaths, restore_state):
        if self.target == TestTarget.ALL_CLUSTERS:
            target_app = paths.all_clusters_app
        elif self.target == TestTarget.TV:
            target_app = paths.tv_app
        elif self.target == TestTarget.LOCK:
            target_app = paths.lock_app
        elif self.target == TestTarget.FABRIC_SYNC:
            target_app = paths.fabric_bridge_app
        elif self.target == TestTarget.OTA:
            target_app = paths.ota_requestor_app
        elif self.target == TestTarget.BRIDGE:
            target_app = paths.bridge_app
        elif self.target == TestTarget.LIT_ICD:
            target_app = paths.lit_icd_app
        elif self.target == TestTarget.MWO:
            target_app = paths.microwave_oven_app
        elif self.target == TestTarget.RVC:
            target_app = paths.rvc_app
        elif self.target == TestTarget.NETWORK_MANAGER:
            target_app = paths.network_manager_app
        else:
            raise Exception("Unknown test target - "
                            "don't know which application to run")

        for path, key in paths.items_with_key():
            # Do not add chip-tool or chip-repl-yaml-tester-cmd to the register
            if path == paths.chip_tool or path == paths.chip_repl_yaml_tester_cmd or path == paths.chip_tool_with_python_cmd:
                continue

            # Skip items where we don't actually have a path.  This can
            # happen if the relevant application does not exist.  It's
            # non-fatal as long as we are not trying to run any tests that
            # need that application.
            if path[-1] is None:
                continue

            # For the app indicated by self.target, give it the 'default' key to add to the register
            app_id = key
            if path == target_app:
                key = 'default'

            app = CommissioneeApp(f'{app_id}', runner, path)
            # Add the App to the register immediately, so if it fails during
            # start() we will be able to clean things up properly.
            apps_register.add(key, app)
            # Remove server application storage (factory reset),
            # so it will be commissionable again.
            if restore_state:
                app.factoryResetOrRestoreState()
            else:
                app.factoryReset()

            # It may sometimes be useful to run the same app multiple times depending
            # on the implementation. So this code creates a duplicate entry but with a different
            # key.
            app = CommissioneeApp(f'{app_id}#2', runner, path)
            apps_register.add(f'{key}#2', app)
            if restore_state:
                app.factoryResetOrRestoreState()
            else:
                app.factoryReset()
