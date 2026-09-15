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

from __future__ import annotations

import itertools
import logging
import sys
from abc import ABC, abstractmethod
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager

from chiptest.concurrency.context import StartStopContextMixin
from chiptest.concurrency.process_pool import WrappedProcessPool
from chiptest.concurrency.worker import GenericWorkerProcess, WorkerConfig, WorkerJob
from chiptest.results import ResultProcessingThread, TestResult
from chiptest.test_definition import TestConcurrencySchedulerType

log = logging.getLogger(__name__)


# Select the appropriate WorkerProcessCls based on the platform.
if sys.platform == "linux":
    import chiptest.linux
    WorkerProcessCls = chiptest.linux.LinuxWorkerProcess
elif sys.platform == 'darwin':
    import chiptest.darwin
    WorkerProcessCls = chiptest.darwin.DarwinWorkerProcess
else:
    WorkerProcessCls = GenericWorkerProcess


class WorkerProcessPool(WrappedProcessPool[WorkerProcessCls, WorkerConfig, WorkerJob, TestResult], StartStopContextMixin, ABC):
    """An abstract class for pool of platform-specific worker processes that can be managed collectively."""

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, result_thread: ResultProcessingThread,
                 config: WorkerConfig) -> None:
        super().__init__(
            WorkerProcessCls, mp_context, mp_manager, result_thread.result_queue, config.concurrency, "Test Pool", config)
        self._results_thread = result_thread
        self.state.register_exception_holder(result_thread)
        self.new_iteration()

    def _schedule_to_worker(self, worker_id: int, job: WorkerJob) -> None:
        """Schedule a job to a specific worker.

        Should be called by overrides of the schedule() method in subclasses, which implement different scheduling strategies.
        """
        self.collect_exceptions()
        self._pool[worker_id].work_queue.put(job)

    @abstractmethod
    def schedule(self, job: WorkerJob) -> None:
        """Schedule a job to a worker. Implement this method in subclasses."""

    def new_iteration(self) -> None:
        """Prepare the pool for a new iteration of tests.

        Optionally, subclasses can override this method to implement specific behavior for different scheduling strategies.
        """


class FastWorkerProcessPool(WorkerProcessPool):
    """Schedule jobs to workers as soon as they are ready, without any specific order or reproducibility guarantees."""

    def schedule(self, job: WorkerJob) -> None:
        self._schedule_to_worker(self.state.process_ready_queue.get(), job)


class ReproducibleWorkerProcessPool(WorkerProcessPool):
    """Schedule jobs to workers in a reproducible order, ensuring that the same sequence of jobs is executed across runs."""

    def new_iteration(self) -> None:
        self._worker_id_cycle = itertools.cycle(range(len(self._pool)))

    def schedule(self, job: WorkerJob) -> None:
        self._schedule_to_worker(next(self._worker_id_cycle), job)


def worker_pool_class(scheduler_type: TestConcurrencySchedulerType) -> type[WorkerProcessPool]:
    """Return the appropriate WorkerProcessPool subclass for the given scheduler type."""
    match scheduler_type:
        case TestConcurrencySchedulerType.FAST:
            return FastWorkerProcessPool
        case TestConcurrencySchedulerType.REPRODUCIBLE:
            return ReproducibleWorkerProcessPool
        case _:
            raise ValueError(f"Unknown scheduler type: {scheduler_type}")
