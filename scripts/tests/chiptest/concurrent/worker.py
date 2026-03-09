import dataclasses
import json
import logging
import os
import queue
import shlex
import subprocess
import sys
import time
from abc import ABC, abstractmethod
from pathlib import Path
from typing import Protocol, TypeVar
import datetime
import enum

import chiptest

if sys.platform == "linux":
    import chiptest.linux as linux
elif sys.platform == "darwin":
    import chiptest.darwin as darwin

from chiptest.accessories import AppsRegister
from chiptest.concurrent.config import WorkerConfig
from chiptest.mp_utils.process import ProcessState, WrappedProcess
from chiptest.runner import Executor
from chiptest.test_definition import TEST_THREAD_DATASET, TestDefinition

log = logging.getLogger(__name__)


class WorkerError(RuntimeError):
    pass


@dataclasses.dataclass
class WorkerJob:
    iteration: int
    test: TestDefinition


@dataclasses.dataclass
class WorkerResult:
    job: WorkerJob
    worker_id: int | None = None
    runtime: float | None = None
    exception: BaseException | None = None


# TODO: Merge with result thread result.
class TestStatus(enum.Enum):
    PASSED = "passed"
    FAILED = "failed"
    DRY_RUN = "dry_run"


# TODO: Merge with result thread result.
@dataclasses.dataclass
class TestResult:
    name: str
    iteration: int
    status: TestStatus
    duration_seconds: float


# TODO: Merge with result thread result.
@dataclasses.dataclass
class RunSummary:
    run_timestamp: datetime.datetime
    iterations: int
    total_runs: int = 0
    passed: int = 0
    failed: int = 0
    results: list[TestResult] = dataclasses.field(default_factory=list)

    def record(self, name: str, iteration: int, status: TestStatus, duration: float) -> None:
        self.results.append(TestResult(name=name, iteration=iteration, status=status, duration_seconds=round(duration, 3)))
        if status == TestStatus.PASSED:
            self.passed += 1
        elif status == TestStatus.FAILED:
            self.failed += 1

    def write_json(self, path: Path) -> None:
        data = dataclasses.asdict(self)
        data["run_timestamp"] = self.run_timestamp.isoformat()
        # Convert Enum to string for JSON serialization
        for result in data["results"]:
            result["status"] = result["status"].value
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(data, indent=2))
        log.info("Test run summary written to %s", path)


class Terminable(Protocol):
    def terminate(self) -> None: ...


ToCleanT = TypeVar("ToCleanT", bound=Terminable)


class WorkerProcess(WrappedProcess[WorkerConfig, WorkerJob, WorkerResult], ABC):
    def _add_to_clean(self, component: ToCleanT) -> ToCleanT | None:
        """Add a component to be cleaned up on process termination.

        Returning Optional to hint that in case of cleanup the reference will be set to None. These fields should be explicily
        checked for None before use.
        """
        self._to_clean.put(component)
        return component

    def _proc_init(self) -> None:
        self._to_clean: queue.LifoQueue[Terminable] = queue.LifoQueue()
        self.mgmt_ns_wrapper: str | None = None
        self.ble_controller_app: int | None = None
        self.ble_controller_tool: int | None = None
        self.thread_ba_host: str | None = None
        self.thread_ba_port: int | None = None

        # Initialize platform-specific executor.
        self.executor = self._add_to_clean(executor := self._platform_init())

        # Finalize common parts.
        self.runner = chiptest.runner.Runner(executor)
        self.apps_register = self._add_to_clean(apps := AppsRegister(self.mgmt_ns_wrapper, self._config.log_config))
        apps.init()

        # TODO: Make shared (probably by moving to the result thread)
        self.run_summary = RunSummary(
            run_timestamp=datetime.datetime.now(datetime.timezone.utc),
            iterations=self._config.iterations,
        )

    @abstractmethod
    def _platform_init(self) -> Executor:
        """Initialize platform-specific executor."""

    def _proc_work(self) -> None:
        while True:
            work = self.work_queue.req_get_or_cancel(req_queue_id=self._config.id)
            self.state.phase = ProcessState.Phase.WORKING
            self.work_queue.rsp_put(result := self._run_test(work))
            if isinstance(result.exception, KeyboardInterrupt):
                raise result.exception
            self.state.phase = ProcessState.Phase.READY

    def _run_test(self, job: WorkerJob) -> WorkerResult:
        result = WorkerResult(job, self._config.id)
        test_start = time.monotonic()  # Initialize here to also cover the case when the test fails to start.
        try:
            if self.apps_register is None:
                raise RuntimeError("Invalid state of the worker")

            self._config.log_config.set_log_fmt(task=job.test.name)

            log.info("Would run test" if self._config.dry_run else "Starting test")

            # TODO: Make it into a command line option.
            # TODO: Create a context manager for the logger.
            self._config.log_config.set_log_fmt(task=job.test.name, log_level=logging.INFO)
            test_start = time.monotonic()
            job.test.Run(self.runner, self.apps_register, self._config.subproc_info_repo, self._config.pics_file,
                         self._config.test_timeout_seconds, self._config.dry_run, self._config.runtime,
                         ble_controller_app=self.ble_controller_app,
                         ble_controller_tool=self.ble_controller_tool,
                         op_network='Thread' if self._config.thread_required else 'WiFi',
                         thread_ba_host=self.thread_ba_host,
                         thread_ba_port=self.thread_ba_port)
            result.runtime = time.monotonic() - test_start
            self._config.log_config.set_log_fmt(task=job.test.name)

            if self._config.dry_run:
                self.run_summary.record(job.test.name, job.iteration, TestStatus.DRY_RUN, result.runtime)
            else:
                log.info("✅ Completed in %0.2f seconds", result.runtime)
                self.run_summary.record(job.test.name, job.iteration, TestStatus.PASSED, result.runtime)
        except BaseException as e:
            result.runtime = time.monotonic() - test_start
            result.exception = e

            # TODO: Use proper path construction.
            if Path('/tmp/thread.pcap').exists():
                os.system("echo 'base64 -d - >/tmp/thread.pcap <<EOF' && base64 /tmp/thread.pcap && echo EOF")

            if isinstance(e, KeyboardInterrupt):
                log.info("❔ Cancelled after %0.2f seconds", result.runtime)
                # TODO: Add run_summary.record for cancelled tests.
            else:
                log.exception("❌ Failed in %0.2f seconds", result.runtime)
                self.run_summary.record(job.test.name, job.iteration, TestStatus.FAILED, result.runtime)
        finally:
            self._config.log_config.set_log_fmt(task=None)
            return result

    def _proc_cleanup(self):
        while True:
            # During termination treat exceptions as warnings to ensure that all items get a chance to terminate.
            try:
                item = self._to_clean.get_nowait()

                # Get member name if exists.
                if (name := next((attr for attr, val in vars(self).items() if val == item), None)) is not None:
                    log.debug("Cleaning up %s (%s)", name, item.__class__.__name__)
                    setattr(self, name, None)
                else:
                    log.debug("Cleaning up %s", item.__class__.__name__)

                # Terminate the item.
                item.terminate()
            except queue.Empty:
                # No more items to clean.
                break
            except Exception as e:
                log.warning("Exception during cleanup: %r", e)

        # TODO: Move to result thread.
        if self._config.summary_file is not None:
            self.run_summary.total_runs = len(self.run_summary.results)
            self.run_summary.write_json(self._config.summary_file)


if sys.platform == "linux":

    class LinuxWorkerProcess(WorkerProcess):
        def _platform_init(self) -> Executor:
            log.debug("Initializing Linux test executor")

            # Create a virtual /tmp.
            tmp_dir_default = self._config.tmp_dir_default
            tmp_dir = self._config.tmp_dir_worker_base / str(self._config.id)
            tmp_dir.mkdir(parents=True, exist_ok=True)

            log.info("Remounting %s as %s for the worker", tmp_dir, tmp_dir_default)
            if subprocess.run(["mount", "-o", "bind", str(tmp_dir), str(tmp_dir_default)]).returncode != 0:
                raise RuntimeError(f"Failed to mount a virtual {tmp_dir_default}")

            self.net_ns = self._add_to_clean(net_ns := linux.IsolatedNetworkNamespace(
                index=self._config.id,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                app_link_up=not self._config.wifi_required,
                add_ula=not self._config.thread_required,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet
                # depending on the commissioning method used.
                app_link_name='wlx-app' if self._config.wifi_required else 'eth-app'))
            self.mgmt_ns_wrapper = shlex.join(net_ns.mgmt_ns.netns_cmd_wrapper)

            if self._config.commissioning_method == 'ble-wifi':
                self.dbus = self._add_to_clean(linux.DBusTestSystemBus())
                self.bluetooth = self._add_to_clean(linux.BluetoothMock())
                self.wifi = self._add_to_clean(linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", net_ns))
                self.ble_controller_app = 0   # Bind app to the first BLE controller
                self.ble_controller_tool = 1  # Bind tool to the second BLE controller
            elif self._config.commissioning_method == 'ble-thread':
                self.dbus = self._add_to_clean(linux.DBusTestSystemBus())
                self.bluetooth = self._add_to_clean(linux.BluetoothMock())
                self.thread = self._add_to_clean(linux.ThreadBorderRouter(TEST_THREAD_DATASET, net_ns))
                self.ble_controller_app = 0   # Bind app to the first BLE controller
                self.ble_controller_tool = 1  # Bind tool to the second BLE controller
            elif self._config.commissioning_method == 'thread-meshcop':
                self.thread = self._add_to_clean(thread := linux.ThreadBorderRouter(TEST_THREAD_DATASET, net_ns))
                self.thread_ba_host = thread.get_border_agent_host()
                self.thread_ba_port = thread.get_border_agent_port()

            return linux.LinuxNamespacedExecutor(net_ns)

    WorkerProcessCls = LinuxWorkerProcess

elif sys.platform == "darwin":

    class DarwinWorkerProcess(WorkerProcess):
        def _platform_init(self) -> Executor:
            log.debug("Initializing Darwin test executor.")
            return darwin.DarwinExecutor()

    WorkerProcessCls = DarwinWorkerProcess

else:

    class GenericWorkerProcess(WorkerProcess):
        def _platform_init(self) -> Executor:
            log.warning("No platform-specific executor for '%s'", sys.platform)
            return Executor()

    WorkerProcessCls = GenericWorkerProcess
