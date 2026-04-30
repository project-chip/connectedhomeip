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

import dataclasses
import logging
import threading

from chiptest.log_config import LogMessageCounter
from chiptest.results import RunSummary

log = logging.getLogger(__name__)


@dataclasses.dataclass(eq=False)
class PeriodicStatusThread(threading.Thread):
    """
    Thread that periodically prints the status of test execution.

    Periodicity is defined as a number of log messages printed between status updates.
    """

    run_summary: RunSummary
    log_counter: LogMessageCounter
    periodicity: int

    def __post_init__(self) -> None:
        super().__init__(name="Status", daemon=True)

    def run(self) -> None:
        if self.periodicity == 0:
            log.debug("Periodic status overview is disabled")
            return

        log.debug("Starting periodic status overview thread with periodicity of %i log lines", self.periodicity)
        target_count = self.log_counter.total
        while not self.log_counter.cancelled:
            target_count += self.periodicity
            self.log_counter.wait_for_count_or_cancel(target_count)
            # We want to print the status one more time after cancellation to show the final result before termination.

            with self.run_summary:
                current_iteration = self.run_summary.current_iteration
                iterations = self.run_summary.iterations
                successful_tests = self.run_summary.passed
                failed_tests = self.run_summary.failed
                expected_test_count = self.run_summary.expected_test_count

            test_status: list[str] = []
            if successful_tests > 0:
                test_status.append(f"{successful_tests} successful")
            if failed_tests > 0:
                test_status.append(f"{failed_tests} failed")
            if not test_status:
                test_status.append("no tests completed")

            status_message = (
                f"Iteration {current_iteration}/{iterations}: "
                f"{successful_tests + failed_tests}/{expected_test_count} tests ({', '.join(test_status)})"
            )
            log.info("", extra={"status": status_message, "count": False})

        log.debug("Status overview thread has stopped")

    def terminate(self) -> None:
        # The status thread is the only consumer of the log counter, so we can use it to cancel the thread.
        self.log_counter.cancel()
