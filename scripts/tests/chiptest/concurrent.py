from __future__ import annotations

import itertools
import logging
import multiprocessing
import os
import queue
import signal
import subprocess
import sys
import tempfile
import threading
import time
from abc import ABC, abstractmethod
from contextlib import contextmanager, suppress
from dataclasses import dataclass, field, replace
from multiprocessing.managers import SyncManager
from pathlib import Path
from statistics import mean
from types import TracebackType
from typing import TYPE_CHECKING, Any, Iterator, Literal, Protocol, TypeAlias, TypeVar

import chiptest
import chiptest.darwin as darwin
import chiptest.linux as linux

from .accessories import AppsRegister
from .mp_utils import LogConfig, mp_wrapped_spawn_context
from .runner import Executor
from .test_definition import TestDefinition, TestRunTime

log = logging.getLogger(__name__)
worker_state: WorkerState | None = None

WORKER_INIT_BARRIER_TIMEOUT = 30
WORKER_TERMINATE_TIMEOUT = 5

if sys.platform == "linux":
    # We have a private /run as we're running in unshare, so we can place it in any place under /run. We don't want it in /tmp, as
    # we remount it to worker-specific scratchpad.
    SYNC_MANAGER_PATH = "/run/python_pool_manager.sock"
else:
    # Other platforms will fall back to their default.
    SYNC_MANAGER_PATH = None

# Create worker multiprocessing context so that we can drop RPC server into a network namespace with a custom wrapper.
_ctx = multiprocessing.get_context("spawn")

if TYPE_CHECKING:
    # Mypy doesn't seem to like custom contexts.
    BaseProcess = multiprocessing.Process
else:
    BaseProcess = _ctx.Process


class WorkerError(RuntimeError):
    pass


@contextmanager
def defer_keyboard_interrupt() -> Iterator[None]:
    """Defer KeyboardInterrupt in a context, and trigger the handler only once the context ends."""
    original_handler = signal.getsignal(signal.SIGINT)
    interrupted: list[tuple[int, Any]] = []

    def handler(signum, frame):
        log.warning("Caught a user interrupt. Waiting for the task to complete...")
        interrupted.append((signum, frame))

    signal.signal(signal.SIGINT, handler)
    try:
        yield
    finally:
        # Restore original handler and call it with captured signal.
        signal.signal(signal.SIGINT, original_handler)
        if interrupted and callable(original_handler):
            original_handler(*interrupted[0])


@dataclass
class WorkerConfig:
    """Worker configuration which is a subset of command line options."""
    ble_wifi_enable: bool
    concurrency: int
    concurrency_status: float
    concurrency_fast: bool
    dry_run: bool
    log_config: LogConfig
    paths: chiptest.ApplicationPaths
    pics_file: Path
    runtime: TestRunTime
    test_timeout_seconds: int | None

    def __post_init__(self):
        self.tmp_dir_default = Path(tempfile.gettempdir())
        self.tmp_dir_worker_base = self.tmp_dir_default / "matter_test_suite"


class WorkerShared:
    def __init__(self, manager: SyncManager, config: WorkerConfig):
        # Barrier used to synchronize initialization event between workers and the main process.
        # TODO: Remove init_barrier and base the general status on worker_working (which will be worker_status)
        self.init_barrier = manager.Barrier(config.concurrency + 1)

        # Event used to signal cancellation of all activity either because of error or no tasks left.
        self.cancel_event = manager.Event()

        # TODO: Replace with a proper runner status.
        self.worker_working = manager.list([manager.Event() for _ in range(config.concurrency)])

        # Indicate that one of the workers finished a job.
        self.job_done_event = manager.Event()

    @property
    def worker_utilization(self) -> int:
        return sum(status.is_set() for status in self.worker_working)


@dataclass
class WorkerResult:
    test: TestDefinition
    worker_id: int | None = None
    runtime: float | None = None
    exception: BaseException | None = None

    def summarize(self) -> bool:
        if self.exception is None:
            # The test result was already printed by the worker.
            return True

        if isinstance(self.exception, KeyboardInterrupt):
            log.debug("Task '%s' got cancelled", self.test.name)
        elif self.worker_id is not None:
            log.error("Encountered exception while executing the task '%s' on runner %i: %r",
                      self.test.name, self.worker_id, self.exception)
        else:
            log.error("Encountered exception while executing the task '%s': %r", self.test.name, self.exception)
        return False

    @staticmethod
    def summarize_all(result: WorkerResult | list[WorkerResult]) -> bool:
        if isinstance(result, list):
            results = [r.summarize() for r in result]
            return all(results)
        return result.summarize()


class Terminatable(Protocol):
    def terminate(self) -> None: ...


class WorkerState(ABC):
    def __init__(self, id: int, config: WorkerConfig, shared: WorkerShared) -> None:
        self.to_clean: queue.LifoQueue[Terminatable] = queue.LifoQueue()

        self.id = id
        self.config = config
        self.shared = shared
        self.rpc_ns: str | None = None
        self.ble_controller_app: int | None = None
        self.ble_controller_tool: int | None = None

        self.log_config = log_config = replace(self.config.log_config, process_name=f"W{id:0{len(str(config.concurrency))}}")
        log_config.set_log_fmt()

        try:
            # Initialize platform-specific executor.
            self.executor = self.add_to_clean(executor := self._platform_init())

            # Finalize common parts.
            self.runner = chiptest.runner.Runner(executor)
            self.apps_register = self.add_to_clean(apps := AppsRegister(self.rpc_ns, log_config))
            apps.init()
        except BaseException:
            self.terminate()
            raise

    ToCleanT = TypeVar("ToCleanT", bound=Terminatable)

    def add_to_clean(self, component: ToCleanT) -> ToCleanT | None:
        self.to_clean.put(component)
        return component

    @abstractmethod
    def _platform_init(self) -> Executor:
        """Initialize platform-specific executor."""

    def terminate(self) -> None:
        """Cleanup worker state."""
        log.debug("Cleaning up state of the worker")

        while not self.to_clean.empty():
            # During termination treat exceptions as warnings to ensure that all items get a chance to terminate.
            try:
                item = self.to_clean.get_nowait()

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


class LinuxWorkerState(WorkerState):
    def _platform_init(self) -> Executor:
        log.debug("Initializing Linux test executor.")

        # Create a virtual /tmp.
        tmp_dir_default = self.config.tmp_dir_default
        tmp_dir = self.config.tmp_dir_worker_base / str(self.id)
        tmp_dir.mkdir(parents=True, exist_ok=True)

        log.info("Remounting %s as %s for the worker", tmp_dir, tmp_dir_default)
        if subprocess.run(["mount", "-o", "bind", str(tmp_dir), str(tmp_dir_default)]).returncode != 0:
            raise RuntimeError(f"Failed to mount a virtual {tmp_dir_default}")

        self.net_ns = self.add_to_clean(net_ns := linux.IsolatedNetworkNamespace(
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
            self.dbus = self.add_to_clean(linux.DBusTestSystemBus())
            self.bluetooth = self.add_to_clean(linux.BluetoothMock())
            self.wifi = self.add_to_clean(linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", net_ns))
            self.ble_controller_app = 0  # Bind app to the first BLE controller
            self.ble_controller_tool = 1  # Bind tool to the second BLE controller

        return linux.LinuxNamespacedExecutor(net_ns)


class DarwinWorkerState(WorkerState):
    def _platform_init(self) -> Executor:
        log.debug("Initializing Darwin test executor.")
        return darwin.DarwinExecutor()


class GenericWorkerState(WorkerState):
    def _platform_init(self) -> Executor:
        log.warning("No platform-specific executor for '%s'", sys.platform)
        return Executor()


class WorkerProcess(BaseProcess):
    # TODO: Merge with WorkerState
    WorkQueueT: TypeAlias = queue.Queue[TestDefinition | None]
    ResultQueueT: TypeAlias = queue.Queue[WorkerResult]

    def __init__(self, id: int, config: WorkerConfig, shared: WorkerShared, work_queue: WorkQueueT, resp_queue: ResultQueueT) -> None:
        super().__init__(name=f"TestWorker{id}")
        self.id = id
        self.config = config
        self.shared = shared
        self.work_queue = work_queue
        self.resp_queue = resp_queue

        self.worker_state: WorkerState | None = None

    def run(self) -> None:
        # Initialize the worker.
        try:
            WorkerStateCls: type[WorkerState]
            match sys.platform:
                case "linux":
                    WorkerStateCls = LinuxWorkerState
                case "darwin":
                    WorkerStateCls = DarwinWorkerState
                case _:
                    WorkerStateCls = GenericWorkerState

            # Don't interrupt during worker initialization.
            with defer_keyboard_interrupt():
                self.worker_state = WorkerStateCls(self.id, self.config, self.shared)
                self.shared.init_barrier.wait()

            # Perform work. Will be stopped eventually with a KeyboardInterrupt.
            while not self.shared.cancel_event.is_set():
                try:
                    if (work := self.work_queue.get(timeout=1)) is None:
                        log.info("Cleaning up as there are no more jobs to process")
                        break
                    self.shared.worker_working[self.id].set()
                    self.resp_queue.put(self.run_test(work))
                    self.shared.worker_working[self.id].clear()
                    self.shared.job_done_event.set()
                except queue.Empty:
                    continue
        except BaseException as e:
            if isinstance(e, KeyboardInterrupt):
                log.warning("Interrupting work")
            else:
                self.shared.cancel_event.set()
                log.critical("Cleaning up after error: %r", e, exc_info=True)
        finally:
            # 1. If for some reason the barrier is broken at this point, we don't really care.
            # 2. We also don't care about KeyboardInterrupt anymore, but we also don't want KeyboardInterrupt to interrupt the
            #    termination flow.
            with suppress(KeyboardInterrupt), defer_keyboard_interrupt():
                log.debug("Terminating worker")
                if self.worker_state is not None:
                    self.worker_state.terminate()
                log.debug("Worker terminated")

    def run_test(self, test: TestDefinition) -> WorkerResult:
        result = WorkerResult(test)
        test_start = time.monotonic()
        try:
            if self.worker_state is None or self.worker_state.apps_register is None:
                raise RuntimeError("Invalid state of the worker")

            result.worker_id = self.worker_state.id
            config = self.worker_state.config
            self.worker_state.log_config.set_log_fmt(test.name)

            if config.dry_run:
                log.info("Would run test")
            else:
                log.info("Starting test")

            # TODO: Potentially intercept stdout/stderr to output it in one block.
            test_start = time.monotonic()
            test.Run(self.worker_state.runner, self.worker_state.apps_register, config.paths, config.pics_file, config.test_timeout_seconds,
                     config.dry_run, config.runtime, self.worker_state.ble_controller_app, self.worker_state.ble_controller_tool)
            result.runtime = time.monotonic() - test_start

            if not config.dry_run:
                log.info("✅ Completed in %0.2f seconds", result.runtime)
        except BaseException as e:
            result.runtime = time.monotonic() - test_start
            result.exception = e
            if isinstance(e, KeyboardInterrupt):
                raise
            log.exception("❌ FAILED in %0.2f seconds", result.runtime, exc_info=True)
        finally:
            if self.worker_state is not None:
                self.worker_state.log_config.set_log_fmt(None)
            return result


@dataclass
class PoolStatus:
    config: WorkerConfig
    iter_count: int
    iter_cur: int = 0
    test_results: dict[str, list[WorkerResult]] = field(default_factory=dict)
    running: bool = False

    _lock: threading.Lock = field(default_factory=threading.Lock, init=False, repr=False)

    def __enter__(self) -> PoolStatus:
        self._lock.acquire()
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> Literal[False]:
        self._lock.release()
        # Returning False means exceptions are not suppressed.
        return False

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
        return self.iter_count * len(self.test_results)

    def print_summary(self) -> None:
        if self.iter_count == 0 or not self.test_results:
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

# TODO: Test errors in tests
# TODO: Test errors with keep-going


class TestPoolManager:
    def __init__(self, config: WorkerConfig, iterations: int, keep_going: bool, expected_failures: int) -> None:
        self._config = config
        self._status = PoolStatus(config, iterations)
        self._shared: WorkerShared | None = None
        self._work_queues: tuple[WorkerProcess.WorkQueueT, ...] = ()
        self._pool: tuple[WorkerProcess, ...] = ()

        self.iterations = iterations
        self.keep_going = keep_going
        self.expected_failures = expected_failures

        periodicity: float | None = None
        match self._config.concurrency_status:
            case num if num < 0:
                # "Automatic" periodicity. Could be improved with checking activity of logger.
                periodicity = (0.5 if self._config.log_config.log_level_int <= logging.DEBUG else
                               2 if self._config.concurrency > 4 else
                               5 if self._config.concurrency > 1 else
                               10)
            case 0:
                periodicity = None
            case num:
                periodicity = num

        if periodicity is not None:
            log.debug("Launching periodic status overview thread.")
            self._status_thread = threading.Thread(target=self._print_periodic_status, args=(periodicity,), daemon=True)
            self._status_thread.start()

    @property
    def is_pool_alive(self) -> bool:
        return all(worker.is_alive() for worker in self._pool)

    def wait_for_pool_termination(self, timeout: float = WORKER_TERMINATE_TIMEOUT):
        start_time = time.monotonic()
        while (time.monotonic() - start_time) < timeout:
            if not self.is_pool_alive:
                return True
            time.sleep(0.5)
        return False

    def _terminate_pool(self):
        try:
            if self.wait_for_pool_termination():
                return
            log.warning("Timeout when waiting for some workers. Finalizing the jobs forcefully")

            # Sending SIGINT should trigger KeyboardInterrupt in case the test is still running, which should result in worker.
            for worker in self._pool:
                if worker.is_alive() and worker.pid is not None:
                    # TODO Python 3.14: worker.interrupt()
                    os.kill(worker.pid, signal.SIGINT)

            if self.wait_for_pool_termination():
                return
            log.warning("Timeout when waiting for graceful job termination. Terminating the worker processes")

            # Send SIGTERM
            for worker in self._pool:
                if worker.is_alive():
                    worker.terminate()

            if self.wait_for_pool_termination():
                return
            log.warning("Timeout when waiting for forced job termination. Killing the worker processes")

            # Send SIGKILL
            for worker in self._pool:
                if worker.is_alive():
                    worker.kill()
        finally:
            for worker in self._pool:
                if worker.is_alive():
                    worker.join(WORKER_TERMINATE_TIMEOUT)

    def run_tests(self, selected_tests: list[TestDefinition]):
        log.info("Initializing worker pool for concurrency of %i workers", self._config.concurrency)
        with self._status as status:
            status.test_results = {test.name: [] for test in selected_tests}

        with SyncManager(address=SYNC_MANAGER_PATH) as manager, mp_wrapped_spawn_context("unshare --map-root-user -n -m", _ctx):
            self._shared = shared = WorkerShared(manager, self._config)
            self._work_queues = tuple(manager.Queue() for _ in range(self._config.concurrency))
            results: WorkerProcess.ResultQueueT = manager.Queue()
            self._pool = pool = tuple(WorkerProcess(id, self._config, shared, work_queue, results)
                                      for id, work_queue in enumerate(self._work_queues))
            try:
                # Initialize the pool.
                for worker in pool:
                    worker.start()

                # Wait for all workers to pass the initializer, while ensuring that user can't interrupt (same as in workers).
                # Don't interrupt during worker initialization.
                with defer_keyboard_interrupt():
                    log.info("Waiting for all workers to initialize")
                    shared.init_barrier.wait(WORKER_INIT_BARRIER_TIMEOUT)
                    if shared.cancel_event.is_set():
                        raise WorkerError("Error during worker initialization")

                # All workers are in a valid state, so we can spawn tasks.
                for i in range(self.iterations):
                    log.info("Starting iteration %d", i+1)
                    observed_failures = 0
                    with self._status as status:
                        status.running = True
                        status.iter_cur = i

                    # TODO: Move to concurrency mode function.
                    if self._config.concurrency_fast:
                        for test in selected_tests:
                            # Get first free worker.
                            while (free_worker := next((wid for wid, working in enumerate(shared.worker_working) if not working.is_set()), None)) is None:
                                # When no workers are available process results and wait for any job.
                                observed_failures += self._process_result_queue(results)
                                shared.job_done_event.wait()
                                shared.job_done_event.clear()

                            log.debug("Enqueuing test %s to worker %i", test.name, free_worker)
                            self._work_queues[free_worker].put(test)

                            # We want to set it here as well, so that the next iteration doesn't double-book a worker because of
                            # a slight delay between enqueuing a job and the flag being set.
                            shared.worker_working[free_worker].set()

                        # If this is the last iteration schedule finalization event.
                        if i+1 == self.iterations:
                            for work_queue in self._work_queues:
                                work_queue.put(None)

                        # Wait for all jobs to finish.
                        while shared.worker_utilization > 0:
                            observed_failures += self._process_result_queue(results)
                            shared.job_done_event.wait()
                            shared.job_done_event.clear()
                    else:
                        # Enqueue all jobs.
                        for test, (wid, work_queue) in zip(selected_tests, itertools.cycle(enumerate(self._work_queues))):
                            log.debug("Enqueuing test %s to worker %i", test.name, wid)
                            work_queue.put(test)

                        # Enqueue finalize events on the last iteration.
                        if i+1 == self.iterations:
                            for work_queue in self._work_queues:
                                work_queue.put(None)
                        observed_failures += self._process_result_queue(results, expected_results=len(selected_tests))

                    if observed_failures != self.expected_failures:
                        raise WorkerError(
                            f"Iteration {i+1}: expected failure count {self.expected_failures}, but got {observed_failures}")
            except (KeyboardInterrupt, Exception) as e:
                if isinstance(e, KeyboardInterrupt):
                    log.warning("Interrupting execution per user request")
                else:
                    log.error("Interrupting execution: %r", e)
                raise
            finally:
                # Set cancel event so that all workers initialize termination.
                shared.cancel_event.set()

                # Wait until all workers finish cleanup.
                log.info("Waiting until all workers terminate")
                self._terminate_pool()

                with self._status as status:
                    status.running = False

                # Process all remaining results.
                self._process_result_queue(results, ignore_failures=True)

                log.debug("Finalized worker pool")
                with self._status as status:
                    status.print_summary()

    def _process_result_queue(self, queue: WorkerProcess.ResultQueueT, expected_results: int | None = None, ignore_failures: bool = False) -> int:
        observed_failures = 0
        result_count = 0
        while not queue.empty() if expected_results is None else (result_count < expected_results):
            if not isinstance(result := queue.get(), WorkerResult):
                log.warning("Wrong work result: %s", str(result))
                continue

            with self._status as status:
                status.test_results[result.test.name].append(result)
            if not result.summarize():
                observed_failures += 1
                # TODO: Move keep_going to ignore_failures.
                if not ignore_failures and not self.keep_going:
                    raise WorkerError("Task failed and --keep-going flag is not set.")
            result_count += 1
        return observed_failures

    def _print_periodic_status(self, periodicity: float) -> None:
        """Periodically print current status overview."""
        while True:
            start = time.monotonic()
            try:
                with self._status as status:
                    if not status.running:
                        continue

                    test_status: list[str] = []
                    worker_utilization = ""
                    if (successful_tests := len(status.successful_tests)) > 0:
                        test_status.append(f"{successful_tests} successful")
                    if (failed_tests := len(status.failed_tests)) > 0:
                        test_status.append(f"{failed_tests} failed")
                    if not test_status:
                        test_status.append("no tests completed")
                    if self._config.concurrency > 1 and self._shared is not None:
                        worker_utilization = f", worker utilization: {self._shared.worker_utilization}/{len(self._work_queues)}"

                    # Style: bold, blue
                    log.info("\033[34;1mIteration %i/%i: %i/%i tests (%s)%s\033[0m", status.iter_cur + 1, status.iter_count,
                             successful_tests + failed_tests, status.total_tests, ", ".join(test_status), worker_utilization)
            finally:
                time.sleep(max(0, periodicity - (time.monotonic() - start)))
