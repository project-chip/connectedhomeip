from __future__ import annotations

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
from contextlib import suppress
import dataclasses
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from pathlib import Path
from statistics import mean
from typing import Callable, ClassVar, Protocol, TypeVar, TypeAlias

import chiptest
if sys.platform == "linux":
    import chiptest.linux as linux
elif sys.platform == "darwin":
    import chiptest.darwin as darwin

from .accessories import AppsRegister
from .mp_utils.process import ProcessConfigTemplate, ProcessState, WorkQueueCancelled, WrappedProcess, mp_wrapped_spawn_context, ProcessConfigMixin
from .mp_utils.queue import WorkQueue
from .mp_utils.pool import WrappedProcessPool
from .mp_utils.log_utils import LogConfig
from .mp_utils.common import StartStopContextMixin
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


@dataclasses.dataclass
class TestConfig:
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

    def __post_init__(self):
        self.tmp_dir_default = Path(tempfile.gettempdir())
        self.tmp_dir_worker_base = self.tmp_dir_default / "matter_test_suite"


@dataclasses.dataclass
class WorkerConfigTemplate(ProcessConfigTemplate, TestConfig):
    WORKER_START_TIMEOUT: ClassVar[float] = 15
    WORKER_STOP_TIMEOUT: ClassVar[float] = 10

    @classmethod
    def from_test_config(cls, log_config: LogConfig, config: TestConfig):
        return cls(**dataclasses.asdict(config), name_short="W{id:0{len(str(config.concurrency))}}", name_long="Worker {id}",
                   log_config=log_config, start_timeout=WorkerConfigTemplate.WORKER_START_TIMEOUT,
                   stop_timeout=WorkerConfigTemplate.WORKER_STOP_TIMEOUT)


@dataclasses.dataclass
class WorkerConfig(WorkerConfigTemplate, ProcessConfigMixin):
    pass


WorkerJob: TypeAlias = TestDefinition | None


@dataclasses.dataclass
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


class WorkerProcess(WrappedProcess[WorkerConfig, WorkerJob, WorkerResult], ABC):
    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, config: WorkerConfig,
                 worker_ready_queue: queue.Queue[int], work_queue: WorkQueue[WorkerJob, WorkerResult],
                 state_changed_cond: threading.Condition) -> None:
        super().__init__(mp_context, mp_manager, config, work_queue, state_changed_cond)

        self.active = mp_manager.Event()  # TODO: Move to process state.
        self.worker_ready_queue = worker_ready_queue  # TODO: Move to WrappedProcess.

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
        self.worker_ready_queue.put(self._config.id)

    @abstractmethod
    def _platform_init(self) -> Executor:
        """Initialize platform-specific executor."""

    ToCleanT = TypeVar("ToCleanT", bound=Terminable)

    def _add_to_clean(self, component: ToCleanT) -> ToCleanT | None:
        self._to_clean.put(component)
        return component

    def _proc_work(self) -> None:
        while True:
            # Get the work item and check for end of work signal.
            if (work := self.work_queue.get_req_or_cancel()) is None:
                log.info("Cleaning up as there are no more jobs to process")
                break

            # Perform the test.
            self.active.set()
            self.work_queue.put_rsp(self._run_test(work))
            self.worker_ready_queue.put(self._config.id)
            self.active.clear()

    def _proc_cleanup(self):
        log.debug("Cleaning up state of the worker")

        # TODO: Change _to_clean to deque.
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
        result = WorkerResult(test, self._config.id)
        test_start = time.monotonic()
        try:
            if self.apps_register is None:
                raise RuntimeError("Invalid state of the worker")

            self._config.log_config.set_log_fmt(log, test.name)

            log.info("Would run test" if self._config.dry_run else "Starting test")
            # TODO: Potentially intercept stdout/stderr to output it in one block.
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

            if Path('thread.pcap').exists():
                os.system("echo 'base64 -d - >thread.pcap <<EOF' && base64 thread.pcap && echo EOF")

            if isinstance(e, KeyboardInterrupt):
                raise
            log.exception("❌ FAILED in %0.2f seconds", result.runtime, exc_info=True)
        finally:
            self._config.log_config.set_log_fmt(log, None)
            return result


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
                self.ble_controller_app = 0  # Bind app to the first BLE controller
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


WorkerPoolProcessT = TypeVar("WorkerPoolProcessT", bound=WorkerProcess)


class WorkerPool(WrappedProcessPool[WorkerPoolProcessT, WorkerConfig, WorkerJob, WorkerResult], StartStopContextMixin):
    def __init__(self, process_cls: type[WorkerPoolProcessT], mp_context: SpawnContext, mp_manager: SyncManager, log_config: LogConfig, config: TestConfig) -> None:
        self.config = config
        self.worker_ready_queue: queue.Queue[int] = mp_manager.Queue()

        super().__init__(process_cls, mp_context, mp_manager, config.concurrency, "Test Pool", config_cls=WorkerConfig,
                         config_template=WorkerConfigTemplate.from_test_config(log_config, config))

    @property
    def worker_utilization(self) -> int:
        return sum(worker.active.is_set() for worker in self._pool)

    def get_first_free_worker(self) -> int | None:
        return next((wid for wid, working in enumerate(worker.active for worker in self._pool) if not working.is_set()), None)

    def finalize(self):
        return self.work_queue.put_req(None, req_queue_id=None)


@dataclasses.dataclass
class TestPoolManager:
    log_config: LogConfig
    config: WorkerConfigTemplate
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
                self.status_periodicity = (0.5 if self.log_config.log_level_int <= logging.DEBUG else
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
              WorkerPool(WorkerProcessCls, ctx, manager, self.log_config, self.config) as pool):
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
                # TODO: As exception group
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

                while not all(state == ProcessState.Phase.CLOSED for state in pool.state):
                    # TODO: Wait for all and check if error at the end with exception group
                    with pool.state_changed:
                        pool.state_changed.wait()
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
                with pool.state_changed:
                    result_thread_cancel.set()
                    pool.state_changed.notify_all()
                result_thread.join()

                log.debug("Finalized worker pool")
                self.print_summary()

    def _scheduler_fast(self, pool: WorkerPool, tests: list[TestDefinition], check_if_error: Callable[[], None]):
        for test in tests:
            while True:
                with pool.state_changed:
                    # TODO: Wait for worker_ready_queue or
                    pool.state_changed.wait()
                check_if_error()  # TODO: You sure?
                with suppress(queue.Empty):
                    worker_id = pool.worker_ready_queue.get_nowait()
                    break
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            pool.work_queue.put_req(test, worker_id)

    def _scheduler_reproducible(self, pool: WorkerPool, tests: list[TestDefinition]):
        for test, worker_id in zip(tests, itertools.cycle(range(len(pool)))):
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            pool.work_queue.put_req(test, worker_id)

    def _process_result_queue(self, pool: WorkerPool, cancel_event: threading.Event,
                              exception_queue: queue.Queue[Exception]) -> None:
        while True:
            try:
                result = pool.work_queue.get_rsp_or_cancel()
            except WorkQueueCancelled:
                break

            if not isinstance(result, WorkerResult):
                # Can happen if the mp_manager is already shut down, which can happen in some unclean cancellation cases.
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
