from __future__ import annotations

from collections.abc import Iterable
import dataclasses
import itertools
import logging
import os
import queue
import subprocess
import sys
import tempfile
import threading
import time
from abc import ABC, abstractmethod
from multiprocessing.managers import SyncManager
from pathlib import Path
from statistics import mean
from typing import Callable, ClassVar, Protocol, TypeVar

import chiptest

if sys.platform == "linux":
    import chiptest.linux as linux
elif sys.platform == "darwin":
    import chiptest.darwin as darwin

from .accessories import AppsRegister
from .mp_utils.common import StartStopContextMixin
from .mp_utils.log_utils import LogConfig
from .mp_utils.pool import WrappedProcessPool
from .mp_utils.process import ProcessConfigTemplate, ProcessState, WorkQueueCancelled, WrappedProcess, mp_wrapped_spawn_context
from .runner import Executor
from .test_definition import SubprocessInfoRepo, TestDefinition, TestRunTime

log = logging.getLogger(__name__)


if sys.platform == "linux":
    # We have a private /run as we're running in unshare, so we can place it in any place under /run. We don't want it in /tmp, as
    # we remount it to worker-specific scratchpad.
    SYNC_MANAGER_PATH = "/run/python_pool_manager.sock"
else:
    # Other platforms will fall back to their default.
    SYNC_MANAGER_PATH = None


class WorkerError(RuntimeError):
    pass


@dataclasses.dataclass(frozen=True)
class TestJobConfig:
    """Worker configuration which is a subset of command line options."""
    wifi_required: bool
    thread_required: bool
    commissioning_method: str
    concurrency: int
    concurrency_status: float
    concurrency_fast: bool
    dry_run: bool
    subproc_info_repo: SubprocessInfoRepo
    pics_file: Path
    runtime: TestRunTime
    test_timeout_seconds: int | None
    iterations: int
    keep_going: bool
    expected_failures: int


@dataclasses.dataclass(frozen=True)
class WorkerConfig(ProcessConfigTemplate, TestJobConfig):
    WORKER_START_TIMEOUT: ClassVar[float] = 15
    WORKER_STOP_TIMEOUT: ClassVar[float] = 10

    tmp_dir_default: ClassVar[Path] = Path(tempfile.gettempdir())
    tmp_dir_worker_base: ClassVar[Path] = Path(tempfile.gettempdir()) / "matter_test_suite"

    @classmethod
    def from_test_job_config(cls, log_config: LogConfig, config: TestJobConfig):
        return cls(**dataclasses.asdict(config),
                   name_short=f"W{{id:0{len(str(config.concurrency))}}}",
                   name_long="Worker {id}",
                   log_config=log_config,
                   start_timeout=WorkerConfig.WORKER_START_TIMEOUT,
                   stop_timeout=WorkerConfig.WORKER_STOP_TIMEOUT)


@dataclasses.dataclass
class WorkerResult:
    test: TestDefinition
    worker_id: int | None = None
    runtime: float | None = None
    exception: BaseException | None = None


class Terminable(Protocol):
    def terminate(self) -> None: ...


ToCleanT = TypeVar("ToCleanT", bound=Terminable)


class WorkerProcess(WrappedProcess[WorkerConfig, TestDefinition, WorkerResult], ABC):
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

    @abstractmethod
    def _platform_init(self) -> Executor:
        """Initialize platform-specific executor."""

    def _proc_work(self) -> None:
        while True:
            work = self.work_queue.get_req_or_cancel()
            self.state.phase = ProcessState.Phase.WORKING
            self.work_queue.put_rsp(self._run_test(work))
            self.state.phase = ProcessState.Phase.READY

    def _run_test(self, test: TestDefinition) -> WorkerResult:
        result = WorkerResult(test, self._config.id)
        test_start = time.monotonic()  # Initialize here to also cover the case when the test fails to start.
        try:
            if self.apps_register is None:
                raise RuntimeError("Invalid state of the worker")

            self._config.log_config.set_log_fmt(log, test.name)

            log.info("Would run test" if self._config.dry_run else "Starting test")
            test_start = time.monotonic()
            test.Run(self.runner, self.apps_register, self._config.subproc_info_repo, self._config.pics_file,
                     self._config.test_timeout_seconds, self._config.dry_run, self._config.runtime,
                     ble_controller_app=self.ble_controller_app,
                     ble_controller_tool=self.ble_controller_tool,
                     op_network='Thread' if self._config.thread_required else 'WiFi',
                     thread_ba_host=self.thread_ba_host,
                     thread_ba_port=self.thread_ba_port)
            result.runtime = time.monotonic() - test_start

            if not self._config.dry_run:
                log.info("✅ Completed in %0.2f seconds", result.runtime)
        except BaseException as e:
            result.runtime = time.monotonic() - test_start
            result.exception = e

            if isinstance(e, KeyboardInterrupt):
                log.info("❔ Cancelled after %0.2f seconds", result.runtime)
                raise

            # TODO: Use proper path construction.
            if Path('/tmp/thread.pcap').exists():
                os.system("echo 'base64 -d - >/tmp/thread.pcap <<EOF' && base64 /tmp/thread.pcap && echo EOF")

            log.exception("❌ Failed in %0.2f seconds", result.runtime, exc_info=True)
        finally:
            self._config.log_config.set_log_fmt(log, None)
            return result

    def _proc_cleanup(self):
        log.debug("Cleaning up state of the worker")

        while not self._to_clean.empty():
            # During termination treat exceptions as warnings to ensure that all items get a chance to terminate.
            try:
                item = self._to_clean.get_nowait()

                # Get member name if exists.
                if (name := next((attr for attr, val in vars(self).items() if val == item), None)) is not None:
                    log.debug("Cleaning up %s (%s)", name, item.__class__.__name__)
                    setattr(self, name, None)
                else:
                    log.debug("Cleaning up %s", item.__class__.__name__)
                item.terminate()
            except BaseException as e:
                log.warning("Exception during cleanup: %s", e)


if sys.platform == "linux":

    class LinuxWorkerProcess(WorkerProcess):
        def _platform_init(self) -> Executor:
            log.debug("Initializing Linux test executor.")

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
                app_name='wlx-app' if self._config.wifi_required else 'eth-app'))
            self.mgmt_ns_wrapper = net_ns.mgmt_ns.netns_cmd_wrapper

            if self._config.commissioning_method == 'ble-wifi':
                self.dbus = self._add_to_clean(linux.DBusTestSystemBus())
                self.bluetooth = self._add_to_clean(linux.BluetoothMock())
                self.wifi = self._add_to_clean(linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", net_ns))
                self.ble_controller_app = 0   # Bind app to the first BLE controller
                self.ble_controller_tool = 1  # Bind tool to the second BLE controller
            elif self._config.commissioning_method == 'ble-thread':
                self.dbus = self._add_to_clean(linux.DBusTestSystemBus())
                self.bluetooth = self._add_to_clean(linux.BluetoothMock())
                self.thread = self._add_to_clean(linux.ThreadBorderRouter(net_ns))
                self.ble_controller_app = 0   # Bind app to the first BLE controller
                self.ble_controller_tool = 1  # Bind tool to the second BLE controller
            elif self._config.commissioning_method == 'thread-meshcop':
                self.thread = self._add_to_clean(thread := linux.ThreadBorderRouter(net_ns))
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


class TestPool(WrappedProcessPool[WorkerProcessCls, WorkerConfig, TestDefinition, WorkerResult], StartStopContextMixin):
    pass


class TestPoolManager:
    def __init__(self, log_config: LogConfig, config: TestJobConfig) -> None:
        self.config = WorkerConfig.from_test_job_config(log_config, config)

        self.test_results: dict[str, list[WorkerResult]] = {}
        self.iteration_test_failures: list[int] = []
        self.result_thread_exception: WorkerError | None = None

        # Calculate periodicity of status overview thread.
        self.status_periodicity: float | None
        match self.config.concurrency_status:
            case num if num < 0:
                # "Automatic" periodicity. Could be improved with checking activity of logger.
                self.status_periodicity = (0.5 if self.config.log_config.log_level_int <= logging.DEBUG else
                                           2 if self.config.concurrency > 4 else
                                           5 if self.config.concurrency > 1 else
                                           10)
            case 0:
                self.status_periodicity = None
            case num:
                self.status_periodicity = num

    def run_tests(self, tests: list[TestDefinition]):
        self.test_results = {test.name: [] for test in tests}

        with (SyncManager(address=SYNC_MANAGER_PATH) as mp_manager,
              mp_wrapped_spawn_context("unshare --map-root-user -n -m") as mp_ctx,
              TestPool(WorkerProcessCls, mp_ctx, mp_manager, self.config.concurrency, "Test Pool", self.config) as pool):
            # Set up the periodic status overview thread.
            status_thread_cancel = threading.Event()
            status_thread = threading.Thread(target=self._print_periodic_status,
                                             args=(pool, status_thread_cancel, self.status_periodicity))

            # Set up result queue processor thread.
            result_thread = threading.Thread(target=self._process_result_queue, args=(pool,))

            if self.status_periodicity is not None:
                log.debug("Launching periodic status overview thread.")
                status_thread.start()

            result_thread.start()

            def collect_errors():
                pool.state.collect_exceptions()
                if self.result_thread_exception is not None:
                    raise self.result_thread_exception

            def wait_for_closed(states: Iterable[ProcessState]) -> bool:
                collect_errors()
                return all(state.phase == ProcessState.Phase.CLOSED for state in states)

            # Perform the tests.
            try:
                for i in range(self.config.iterations):
                    log.info("Scheduling iteration %d", i+1)

                    if self.config.concurrency_fast:
                        self._scheduler_fast(pool, tests, collect_errors)
                    else:
                        self._scheduler_reproducible(pool, tests, collect_errors)

                    # If this is the last iteration schedule finalization event.
                    if i+1 == self.config.iterations:
                        pool.work_queue.finalize_req()

                log.info("All jobs scheduled")
                pool.state.wait_for(wait_for_closed)
            except BaseException as e:
                if isinstance(e, KeyboardInterrupt):
                    log.warning("Interrupting execution per user request")
                else:
                    log.error("Interrupting execution after error: %r", e)
                raise
            finally:
                log.debug("Stopping job status thread")
                status_thread_cancel.set()
                status_thread.join()

                log.info("Stopping worker pool")
                pool.stop()

                log.debug("Waiting for result processing thread")
                result_thread.join()

                log.debug("Finalized worker pool")
                self.print_summary()

    def _scheduler_fast(self, pool: TestPool, tests: list[TestDefinition], collect_errors: Callable[[], None]):
        if pool.state.process_ready_queue is None:
            raise RuntimeError("Ready queue is not initialized in the pool state")

        for test in tests:
            worker_id = pool.state.process_ready_queue.get_or_cancel()
            collect_errors()
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            pool.work_queue.put_req(test, worker_id)

    def _scheduler_reproducible(self, pool: TestPool, tests: list[TestDefinition], collect_errors: Callable[[], None]):
        for test, worker_id in zip(tests, itertools.cycle(range(len(pool)))):
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            pool.work_queue.put_req(test, worker_id)

        collect_errors()

    def _process_result_queue(self, pool: TestPool) -> None:
        while True:
            try:
                result = pool.work_queue.get_rsp_or_cancel()
            except WorkQueueCancelled:
                log.debug("No more results to process, finishing result processing thread")
                break

            # Double check the result type. Mismatch can happen if the mp_manager is already shut down for some reason, which can
            # happen in some unclean cancellation cases.
            if not isinstance(result, WorkerResult):
                log.warning("Wrong work result: %r", result)
                continue

            self.test_results[result.test.name].append(result)

            # Prepare for the next iteration if needed.
            iteration = len(self.test_results[result.test.name])
            if len(self.iteration_test_failures) < iteration:
                self.iteration_test_failures.append(0)

            if result.exception is None:
                continue

            # If we're here that means the test has failed.
            if not self.config.keep_going:
                self.result_thread_exception = WorkerError("Task failed and --keep-going flag is not set.")
                break

            last_result_from_iteration = min(len(row) for row in self.test_results.values()) == iteration
            observed_failures = self.iteration_test_failures[iteration-1] + 1
            if last_result_from_iteration and observed_failures != self.config.expected_failures:
                self.result_thread_exception = WorkerError(
                    f"Iteration {iteration}: expected failure count {self.config.expected_failures}, but got {observed_failures}")
                break

            self.iteration_test_failures[iteration-1] = observed_failures

    def _print_periodic_status(self, pool: TestPool, cancel_event: threading.Event, periodicity: float) -> None:
        """Periodically print current status overview."""
        while not cancel_event.is_set():
            start = time.monotonic()
            try:
                if (utilization := pool.state.working_count) == 0:
                    continue

                current_iteration = len(self.iteration_test_failures)

                test_status: list[str] = []
                if (successful_tests := len(self.successful_tests)) > 0:
                    test_status.append(f"{successful_tests} successful")
                if (failed_tests := len(self.failed_tests)) > 0:
                    test_status.append(f"{failed_tests} failed")
                if not test_status:
                    test_status.append("no tests completed")

                worker_utilization = (f", worker utilization: {utilization}/{self.config.concurrency}"
                                      if self.config.concurrency > 1 else "")

                # Style: bold, blue
                log.info("\033[34;1mIteration %i/%i: %i/%i tests (%s)%s\033[0m", current_iteration, self.config.iterations,
                         successful_tests + failed_tests, self.total_tests, ", ".join(test_status), worker_utilization)
            finally:
                cancel_event.wait(max(0, periodicity - (time.monotonic() - start)))

    def print_summary(self) -> None:
        if self.config.iterations == 0 or not self.test_results:
            log.info("No test results available.")

        table: tuple[tuple[str, ...], ...] = (("Test", "Passed", "Time", "Worker ID", "Status"),) + tuple(
            (name,
             # Passed
             f"{sum((r.exception is None) for r in results)}/{len(results)}",
             # Average time
             (f"{mean(runtimes):.2f}" if (runtimes := tuple(r.runtime for r in results if r.runtime is not None)) else "—"),
             # Workers
             ", ".join(str(r.worker_id) for r in results if r.worker_id is not None),
             # Status
             (f"❌ {e!r}" if (e := next((r.exception for r in results if r.exception is not None), None)) is not None else
              "✅" if len(results) > 0 else
              "❔"))
            for name, results in self.test_results.items())
        # TODO: Generate weights file.
        # TODO: If weight = 0 warning that it's advised to regenerate weights file to improve speed
        # TODO: Remember to make it a dict, so that we only update and not remove

        # Remove workers column if only one worker is used.
        if self.config.concurrency == 1:
            table = tuple(row[:3] + row[4:] for row in table)

        # Compute max width for each column from headers and rows. Skip the last column, as it can be quite wide.
        widths = tuple(max(len(row[i]) for row in table) for i in range(len(table[0])-1))
        fmt = "  ".join(f"{{:{w}}}" for w in widths) + "  {}"

        # Print header, separator and rows.
        log.info(fmt.format(*table[0]))
        log.info("  ".join("-" * w for w in widths) + "  ------")
        for row in table[1:]:
            log.info(fmt.format(*row))

        # Print a note where to find /tmp for concurrent tests.
        if self.config.concurrency > 1:
            log.info("Note: You can find %s of each runner in %s/{Worker ID}",
                     self.config.tmp_dir_default, self.config.tmp_dir_worker_base)
        else:
            log.info("Note: You can find %s in %s/0", self.config.tmp_dir_default, self.config.tmp_dir_worker_base)

    @property
    def successful_tests(self) -> tuple[WorkerResult, ...]:
        return tuple(result
                     for _, results in self.test_results.items()
                     for result in results
                     if result.exception is None)

    @property
    def failed_tests(self) -> tuple[WorkerResult, ...]:
        return tuple(result
                     for _, results in self.test_results.items()
                     for result in results
                     if result.exception is not None)

    @property
    def total_tests(self) -> int:
        return self.config.iterations * len(self.test_results)
