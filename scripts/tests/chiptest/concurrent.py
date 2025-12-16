from __future__ import annotations

import itertools
import logging
import queue
import subprocess
import sys
import tempfile
import threading
import time
from abc import ABC, abstractmethod
from contextlib import suppress
from dataclasses import dataclass
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from pathlib import Path
from statistics import mean
from typing import Callable, Protocol, TypeAlias, TypeVar

import chiptest
if sys.platform == "linux":
    import chiptest.linux as linux
elif sys.platform == "darwin":
    import chiptest.darwin as darwin

from .accessories import AppsRegister
from .mp_utils import PROCESS_EXIT_STATES, LogConfig, WrappedProcess, WrappedProcessPoolContext, mp_wrapped_spawn_context
from .runner import Executor
from .test_definition import TestDefinition, TestRunTime

log = logging.getLogger(__name__)

WORKER_START_TIMEOUT = 15
WORKER_STOP_TIMEOUT = 10

if sys.platform == "linux":
    # We have a private /run as we're running in unshare, so we can place it in any place under /run. We don't want it in /tmp, as
    # we remount it to worker-specific scratchpad.
    SYNC_MANAGER_PATH = "/run/python_pool_manager.sock"
else:
    # Other platforms will fall back to their default.
    SYNC_MANAGER_PATH = None


class WorkerError(RuntimeError):
    pass


@dataclass
class WorkerConfig:
    """Worker configuration which is a subset of command line options."""
    ble_wifi_enable: bool
    concurrency: int
    concurrency_status: float
    concurrency_fast: bool
    dry_run: bool
    logconfig: LogConfig
    paths: chiptest.ApplicationPaths
    pics_file: Path
    runtime: TestRunTime
    test_timeout_seconds: int | None

    def __post_init__(self):
        self.tmp_dir_default = Path(tempfile.gettempdir())
        self.tmp_dir_worker_base = self.tmp_dir_default / "matter_test_suite"


@dataclass
class WorkerResult:
    test: TestDefinition
    worker_id: int | None = None
    runtime: float | None = None
    exception: BaseException | None = None

    def summarize(self) -> bool:
        if self.exception is None:
            # The test result was already printed by the worker so we don't need to print anything.
            return True

        if isinstance(self.exception, KeyboardInterrupt):
            log.debug("Task '%s' got cancelled", self.test.name)
        elif self.worker_id is not None:
            log.error("Encountered exception while executing the task '%s' on runner %i: %r",
                      self.test.name, self.worker_id, self.exception)
        else:
            log.error("Encountered exception while executing the task '%s': %r", self.test.name, self.exception)
        return False


class Terminable(Protocol):
    def terminate(self) -> None: ...


class WorkerProcess(WrappedProcess, ABC):
    WorkQueueT: TypeAlias = queue.Queue[TestDefinition | None]
    ResultQueueT: TypeAlias = queue.Queue[WorkerResult]

    def __init__(self, id: int, mp_context: SpawnContext, mp_manager: SyncManager,
                 config: WorkerConfig, state_changed: threading.Condition, work_cancel_error: threading.Condition,
                 cancel_event: threading.Event, worker_ready_queue: queue.Queue[int], resp_queue: ResultQueueT) -> None:
        super().__init__(mp_context, mp_manager, f"Worker {id}", f"W{id:0{len(str(config.concurrency))}}", config.logconfig,
                         state_changed, work_cancel_error, cancel_event)
        self.id = id
        self.config = config

        self.active = mp_manager.Event()
        self.work_queue = mp_manager.Queue()
        self.worker_ready_queue = worker_ready_queue
        self.resp_queue = resp_queue

    def _proc_init(self) -> None:
        self._to_clean: queue.LifoQueue[Terminable] = queue.LifoQueue()
        self.rpc_ns: str | None = None
        self.ble_controller_app: int | None = None
        self.ble_controller_tool: int | None = None

        # Initialize platform-specific executor.
        self.executor = self._add_to_clean(executor := self._platform_init())

        # Finalize common parts.
        self.runner = chiptest.runner.Runner(executor)
        self.apps_register = self._add_to_clean(apps := AppsRegister(self.rpc_ns, self.log_config))
        apps.init()
        with self.work_cancel_error:
            self.worker_ready_queue.put(self.id)
            self.work_cancel_error.notify_all()

    @abstractmethod
    def _platform_init(self) -> Executor:
        """Initialize platform-specific executor."""

    ToCleanT = TypeVar("ToCleanT", bound=Terminable)

    def _add_to_clean(self, component: ToCleanT) -> ToCleanT | None:
        self._to_clean.put(component)
        return component

    def _proc_work(self, work_cancel_error: threading.Condition, cancel_event: threading.Event):
        while not cancel_event.is_set():
            try:
                work = self.work_queue.get_nowait()
            except queue.Empty:
                continue

            # Check for end of work signal.
            if work is None:
                log.info("Cleaning up as there are no more jobs to process")
                break

            # Perform the test.
            self.active.set()
            self.resp_queue.put(self._run_test(work))
            with self.work_cancel_error:
                self.worker_ready_queue.put(self.id)
                self.work_cancel_error.notify_all()
            self.active.clear()

            with work_cancel_error:
                work_cancel_error.wait()

    def _proc_cleanup(self):
        log.debug("Cleaning up state of the worker")

        while not self._to_clean.empty():
            # During termination treat exceptions as warnings to ensure that all items get a chance to terminate.
            try:
                item = self._to_clean.get_nowait()

                # Get member name if exists.
                name = next((attr for attr, val in vars(self).items() if val == item), None)

                if name is not None:
                    log.debug("Cleaning up %s (%s)", name, item.__class__.__name__)
                    setattr(self, name, None)
                else:
                    log.debug("Cleaning up %s", item.__class__.__name__)
                item.terminate()
            except Exception as e:
                log.warning("Exception during cleanup: %s", e)
        log.debug("Worker terminated")

    def _run_test(self, test: TestDefinition) -> WorkerResult:
        result = WorkerResult(test, self.id)
        test_start = time.monotonic()
        try:
            if self.apps_register is None:
                raise RuntimeError("Invalid state of the worker")

            self.log_config.set_log_fmt(test.name)

            log.info("Would run test" if self.config.dry_run else "Starting test")
            # TODO: Potentially intercept stdout/stderr to output it in one block.
            test_start = time.monotonic()
            test.Run(self.runner, self.apps_register, self.config.paths, self.config.pics_file, self.config.test_timeout_seconds,
                     self.config.dry_run, self.config.runtime, self.ble_controller_app, self.ble_controller_tool)
            result.runtime = time.monotonic() - test_start

            if not self.config.dry_run:
                log.info("✅ Completed in %0.2f seconds", result.runtime)
        except BaseException as e:
            result.runtime = time.monotonic() - test_start
            result.exception = e
            if isinstance(e, KeyboardInterrupt):
                raise
            log.exception("❌ FAILED in %0.2f seconds", result.runtime, exc_info=True)
        finally:
            self.log_config.set_log_fmt(None)
            return result


if sys.platform == "linux":

    class LinuxWorkerProcess(WorkerProcess):
        def _platform_init(self) -> Executor:
            log.debug("Initializing Linux test executor.")

            # Create a virtual /tmp.
            tmp_dir_default = self.config.tmp_dir_default
            tmp_dir = self.config.tmp_dir_worker_base / str(self.id)
            tmp_dir.mkdir(parents=True, exist_ok=True)

            log.info("Remounting %s as %s for the worker", tmp_dir, tmp_dir_default)
            if subprocess.run(["mount", "-o", "bind", str(tmp_dir), str(tmp_dir_default)]).returncode != 0:
                raise RuntimeError(f"Failed to mount a virtual {tmp_dir_default}")

            self.net_ns = self._add_to_clean(net_ns := linux.IsolatedNetworkNamespace(
                index=self.id,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                app_link_up=not self.config.ble_wifi_enable,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet
                # depending on the commissioning method used.
                app_link_name='wlx-app' if self.config.ble_wifi_enable else 'eth-app',
                wait_for_dad=False))
            net_ns.wait_for_duplicate_address_detection()
            self.rpc_ns = net_ns.rpc_ns

            if self.config.ble_wifi_enable:
                self.dbus = self._add_to_clean(linux.DBusTestSystemBus())
                self.bluetooth = self._add_to_clean(linux.BluetoothMock())
                self.wifi = self._add_to_clean(linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", net_ns))
                self.ble_controller_app = 0  # Bind app to the first BLE controller
                self.ble_controller_tool = 1  # Bind tool to the second BLE controller

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


WorkerPoolProcessT = TypeVar("WorkerPoolProcessT", bound=WorkerProcess)


class WorkerPool(WrappedProcessPoolContext[WorkerPoolProcessT]):
    def __init__(self, process_cls: type[WorkerPoolProcessT], mp_context: SpawnContext, mp_manager: SyncManager, config: WorkerConfig) -> None:
        self.config = config
        self.work_cancel_error = mp_manager.Condition()
        self.result_queue: WorkerProcess.ResultQueueT = mp_manager.Queue()
        self.worker_ready_queue: queue.Queue[int] = mp_manager.Queue()

        super().__init__(process_cls, mp_context, mp_manager, config.concurrency, "Test Pool", WORKER_START_TIMEOUT, WORKER_STOP_TIMEOUT)

    def _init_process(self, process_cls: type[WorkerPoolProcessT], id: int, mp_context: SpawnContext, mp_manager: SyncManager,
                      state_changed: threading.Condition, work_cancel_error: threading.Condition,
                      cancel_event: threading.Event) -> WorkerPoolProcessT:
        return process_cls(id, mp_context, mp_manager, self.config, state_changed, work_cancel_error, cancel_event,
                           self.worker_ready_queue, self.result_queue)

    @property
    def worker_utilization(self) -> int:
        return sum(worker.active.is_set() for worker in self._pool)

    def get_first_free_worker(self) -> int | None:
        return next((wid for wid, working in enumerate(worker.active for worker in self._pool) if not working.is_set()), None)

    def put(self, id: int | None, item: TestDefinition | None, block: bool = True, timeout: float | None = None) -> None:
        if id is None:
            for id in range(len(self)):
                self.put(id, item, block, timeout)
            return

        if id > self.config.concurrency:
            raise ValueError(f"No worker with ID {id}")
        with self.work_cancel_error:
            self._pool[id].work_queue.put(item, block, timeout)
            self.work_cancel_error.notify_all()

    def finalize(self):
        return self.put(None, None)


@dataclass
class TestPoolManager:
    config: WorkerConfig
    iterations: int
    keep_going: bool
    expected_failures: int

    def __post_init__(self) -> None:
        self.test_results: dict[str, list[WorkerResult]] = {}
        self.iteration_test_failures: list[int] = []

        # Calculate periodicity of status overview thread.
        self.status_periodicity: float | None
        match self.config.concurrency_status:
            case num if num < 0:
                # "Automatic" periodicity. Could be improved with checking activity of logger.
                self.status_periodicity = (0.5 if self.config.logconfig.log_level_int <= logging.DEBUG else
                                           2 if self.config.concurrency > 4 else
                                           5 if self.config.concurrency > 1 else
                                           10)
            case 0:
                self.status_periodicity = None
            case num:
                self.status_periodicity = num

    def run_tests(self, tests: list[TestDefinition]):
        self.test_results = {test.name: [] for test in tests}

        with (SyncManager(address=SYNC_MANAGER_PATH) as manager,
              mp_wrapped_spawn_context("unshare --map-root-user -n -m") as ctx,
              WorkerPool(WorkerProcessCls, ctx, manager, self.config) as pool):
            # Set up the periodic status overview thread.
            status_thread_cancel = threading.Event()
            status_thread = threading.Thread(target=self._print_periodic_status,
                                             args=(pool, status_thread_cancel, self.status_periodicity))

            # Set up result queue processor thread.
            result_thread_cancel = threading.Event()
            result_thread_exception: queue.Queue[Exception] = queue.Queue(1)
            result_thread = threading.Thread(target=self._process_result_queue,
                                             args=(pool, result_thread_cancel, result_thread_exception))

            def check_if_error():
                pool.check_if_error(raise_error=True)
                with suppress(queue.Empty):
                    raise result_thread_exception.get_nowait()

            try:
                if self.status_periodicity is not None:
                    log.debug("Launching periodic status overview thread.")
                    status_thread.start()

                result_thread.start()

                # Perform the tests.
                for i in range(self.iterations):
                    log.info("Scheduling iteration %d", i+1)

                    if self.config.concurrency_fast:
                        self._scheduler_fast(pool, tests, check_if_error)
                    else:
                        self._scheduler_reproducible(pool, tests)

                    # If this is the last iteration schedule finalization event.
                    if i+1 == self.iterations:
                        pool.finalize()

                    check_if_error()

                log.info("All jobs scheduled")

                while not all(state in PROCESS_EXIT_STATES for state in pool.state):
                    with pool.work_cancel_error:
                        pool.work_cancel_error.wait()
                    check_if_error()
            except BaseException as e:
                if isinstance(e, KeyboardInterrupt):
                    log.warning("Interrupting execution per user request")
                else:
                    log.error("Interrupting execution: %r", e)
                raise
            finally:
                log.debug("Stopping job status thread")
                status_thread_cancel.set()
                status_thread.join()

                log.info("Stopping worker pool")
                pool.stop()

                log.debug("Stopping result processing thread")
                with pool.work_cancel_error:
                    result_thread_cancel.set()
                    pool.work_cancel_error.notify_all()
                result_thread.join()

                log.debug("Finalized worker pool")
                self.print_summary()

    def _scheduler_fast(self, pool: WorkerPool, tests: list[TestDefinition], check_if_error: Callable[[], None]):
        for test in tests:
            while True:
                with pool.work_cancel_error:
                    pool.work_cancel_error.wait()
                check_if_error()
                with suppress(queue.Empty):
                    worker_id = pool.worker_ready_queue.get_nowait()
                    break
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            pool.put(worker_id, test)

    def _scheduler_reproducible(self, pool: WorkerPool, tests: list[TestDefinition]):
        for test, wid in zip(tests, itertools.cycle(range(len(pool)))):
            log.debug("Enqueuing test %s to worker %i", test.name, wid)
            pool.put(wid, test)

    def _process_result_queue(self, pool: WorkerPool, cancel_event: threading.Event,exception_queue: queue.Queue[Exception]) -> None:
        while not cancel_event.is_set():
            with pool.work_cancel_error:
                pool.work_cancel_error.wait()

            try:
                result = pool.result_queue.get_nowait()
            except queue.Empty:
                continue

            if not isinstance(result, WorkerResult):
                log.warning("Wrong work result: %r", result)
                continue

            self.test_results[result.test.name].append(result)

            iteration = len(self.test_results[result.test.name])
            if len(self.iteration_test_failures) < iteration:
                self.iteration_test_failures.append(0)

            if result.summarize():
                continue

            if not self.keep_going:
                exception_queue.put(WorkerError("Task failed and --keep-going flag is not set."))
                return

            last_result_from_iteration = min(len(row) for row in self.test_results.values()) == iteration
            observed_failures = self.iteration_test_failures[iteration-1] + 1
            if last_result_from_iteration and observed_failures != self.expected_failures:
                exception_queue.put(WorkerError(
                    f"Iteration {iteration}: expected failure count {self.expected_failures}, but got {observed_failures}"))
                return

            self.iteration_test_failures[iteration-1] = observed_failures

    def _print_periodic_status(self, pool: WorkerPool, cancel_event: threading.Event, periodicity: float) -> None:
        """Periodically print current status overview."""
        while not cancel_event.is_set():
            start = time.monotonic()
            try:
                if (utilization := pool.worker_utilization) == 0:
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
                log.info("\033[34;1mIteration %i/%i: %i/%i tests (%s)%s\033[0m", current_iteration, self.iterations,
                         successful_tests + failed_tests, self.total_tests, ", ".join(test_status), worker_utilization)
            finally:
                cancel_event.wait(max(0, periodicity - (time.monotonic() - start)))

    def print_summary(self) -> None:
        if self.iterations == 0 or not self.test_results:
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
        return self.iterations * len(self.test_results)
