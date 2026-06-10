# Copyright (c) 2026 Project CHIP Authors
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

import contextlib
import dataclasses
import logging
import shlex
import subprocess
import sys
import tempfile
from abc import ABC, abstractmethod
from pathlib import Path
from typing import ClassVar

import chiptest
from chiptest.accessories import AppsRegister
from chiptest.concurrency.context import StartStopContextMixin
from chiptest.log_config import LogConfig
from chiptest.results import TestResult
from chiptest.runner import Executor
from chiptest.test_definition import TEST_THREAD_DATASET, CommissioningMethod, TestDefinition, TestJobConfig

if sys.platform == "linux":
    import chiptest.linux as linux
elif sys.platform == "darwin":
    import chiptest.darwin as darwin

from .process import ProcessConfig, WrappedProcess

log = logging.getLogger(__name__)


@dataclasses.dataclass
class WorkerConfig(ProcessConfig, TestJobConfig):
    """Configuration for a worker process based on a common test job config."""

    WORKER_START_TIMEOUT: ClassVar[float] = 15
    WORKER_STOP_TIMEOUT: ClassVar[float] = 10

    tmp_dir_default: ClassVar[Path] = Path(tempfile.gettempdir())
    tmp_dir_worker_base: ClassVar[Path] = Path(tempfile.gettempdir()) / "matter_test_suite"

    @classmethod
    def from_test_job_config(cls, log_config: LogConfig, config: TestJobConfig):
        """Create a worker config from a test job config."""
        # Needs to be a shallow copy, so that we don't accidentally create unpicklable generators in the config.
        return cls(**{field.name: getattr(config, field.name) for field in dataclasses.fields(config)},
                   name=f"W{{id:0{len(str(config.concurrency))}}}",
                   log_config=log_config,
                   start_timeout_sec=WorkerConfig.WORKER_START_TIMEOUT,
                   stop_timeout_sec=WorkerConfig.WORKER_STOP_TIMEOUT)


@dataclasses.dataclass
class WorkerJob:
    """A unit of work for a worker process."""
    iteration: int
    test: TestDefinition


class WorkerProcess(WrappedProcess[WorkerConfig, WorkerJob, TestResult], StartStopContextMixin, ABC):
    """
    Abstract process class responsible for running tests in a subprocess.

    Each supported platform should have its own implementation of this class that initializes the platform-specific executor and
    resources in ``_platform_init()`` and set WorkerProcessCls.
    """

    def _proc_init(self, exit_stack: contextlib.ExitStack) -> None:
        self.mgmt_ns_wrapper: str | None = None
        self.thread_ba_host: str | None = None
        self.thread_ba_port: int | None = None

        # Initialize platform-specific executor.
        self.executor = exit_stack.enter_context(executor := self._platform_init(exit_stack))

        # Finalize common parts.
        self.runner = chiptest.runner.Runner(executor)
        self.apps_register: AppsRegister = exit_stack.enter_context(AppsRegister(self.mgmt_ns_wrapper, self._config.log_config))

    @abstractmethod
    def _platform_init(self, exit_stack: contextlib.ExitStack) -> Executor:
        """Initialize platform-specific executor."""

    def _proc_work(self) -> None:
        """Main loop of the worker process."""
        while True:
            work = self._work_queue.get()

            with (self.state.working_context(),
                  self._config.log_config.fmt_context(task=work.test.name, level=self._config.log_config.level_tests),
                  TestResult.measure_execution(work.test.name, self._config.id, work.iteration, self._config.dry_run) as result):
                work.test.Run(self.runner, self.apps_register, self._config, self.thread_ba_host, self.thread_ba_port)

            self._rsp_queue.put(result)
            if isinstance(result.exception, KeyboardInterrupt):
                raise result.exception


if sys.platform == "linux":

    class LinuxWorkerProcess(WorkerProcess):
        """Linux implementation of the worker process."""

        def _platform_init(self, exit_stack: contextlib.ExitStack) -> linux.LinuxNamespacedExecutor:
            log.debug("Initializing Linux test executor")

            # Create a virtual /tmp.
            tmp_dir_default = self._config.tmp_dir_default
            tmp_dir = self._config.tmp_dir_worker_base / str(self._config.id)
            tmp_dir.mkdir(parents=True, exist_ok=True)

            log.info("Remounting %s as %s for the worker", tmp_dir, tmp_dir_default)
            if subprocess.run(["mount", "-o", "bind", str(tmp_dir), str(tmp_dir_default)]).returncode != 0:
                raise RuntimeError(f"Failed to mount a virtual {tmp_dir_default}")

            commissioning_method = self._config.commissioning_method

            self.net_ns: linux.IsolatedNetworkNamespace = exit_stack.enter_context(linux.IsolatedNetworkNamespace(
                index=self._config.id,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                app_link_up=not commissioning_method.wifi_required,
                add_ula=not commissioning_method.thread_required,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet depending on the commissioning
                # method used.
                app_link_name=commissioning_method.app_link_name, tool_link_name=commissioning_method.tool_link_name))
            self.mgmt_ns_wrapper = shlex.join(self.net_ns.mgmt_ns.netns_cmd_wrapper)

            match commissioning_method:
                case CommissioningMethod.BLE_WIFI:
                    exit_stack.enter_context(linux.DBusTestSystemBus())
                    exit_stack.enter_context(linux.BluetoothMock())
                    exit_stack.enter_context(linux.WpaSupplicantMock([commissioning_method.app_link_name],
                                                                     "MatterAP", "MatterAPPassword", self.net_ns))
                case CommissioningMethod.BLE_THREAD:
                    exit_stack.enter_context(linux.DBusTestSystemBus())
                    exit_stack.enter_context(linux.BluetoothMock())
                    exit_stack.enter_context(linux.ThreadBorderRouter(TEST_THREAD_DATASET, self.net_ns))
                case CommissioningMethod.THREAD_MESHCOP:
                    exit_stack.enter_context(tbr := linux.ThreadBorderRouter(TEST_THREAD_DATASET, self.net_ns))
                    self.thread_ba_host = tbr.get_border_agent_host()
                    self.thread_ba_port = tbr.get_border_agent_port()
                case CommissioningMethod.WIFIPAF_WIFI:
                    exit_stack.enter_context(linux.DBusTestSystemBus())
                    exit_stack.enter_context(linux.WpaSupplicantMock(
                        [commissioning_method.app_link_name, commissioning_method.tool_link_name],
                        "MatterAP", "MatterAPPassword", self.net_ns))

            return linux.LinuxNamespacedExecutor(self.net_ns)

    WorkerProcessCls = LinuxWorkerProcess

elif sys.platform == "darwin":

    class DarwinWorkerProcess(WorkerProcess):
        """Darwin implementation of the worker process."""

        def _platform_init(self, exit_stack: contextlib.ExitStack) -> darwin.DarwinExecutor:
            log.debug("Initializing Darwin test executor.")
            return exit_stack.enter_context(darwin.DarwinExecutor())

    WorkerProcessCls = DarwinWorkerProcess

else:

    class GenericWorkerProcess(WorkerProcess):
        """Generic implementation of the worker process."""

        def _platform_init(self, exit_stack: contextlib.ExitStack) -> Executor:
            log.warning('No platform-specific executor for "%s"', sys.platform)
            return exit_stack.enter_context(Executor())

    WorkerProcessCls = GenericWorkerProcess
