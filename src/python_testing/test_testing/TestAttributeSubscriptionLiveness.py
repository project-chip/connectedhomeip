#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
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
#

"""Unit tests for AttributeSubscriptionHandler.await_first_value_asserting_no_forbidden.

Covers the stall-watchdog / liveness / attribute-filter extensions used by the OTA
tests (TC_SU_2_2) to wait for device-dependent operations of unknown duration.
No DUT is required: reports are injected directly into the handler's queue from a
feeder thread.
"""

import threading
import time
from datetime import UTC, datetime

from mobly import asserts, signals

import matter.clusters as Clusters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeValue, MatterBaseTest
from matter.testing.runner import default_matter_test_main

UpdateState = Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
UpdateStateProgress = Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress
UpdateStateEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum


def _report(attribute, value) -> AttributeValue:
    return AttributeValue(endpoint_id=0, attribute=attribute, value=value, timestamp_utc=datetime.now(UTC))


def _feed(handler: AttributeSubscriptionHandler, timed_reports: list[tuple[float, AttributeValue]]) -> threading.Thread:
    """Enqueue each report into the handler's queue at its (absolute) delay from now."""
    t_start = time.time()

    def run():
        for delay_sec, report in timed_reports:
            time.sleep(max(0.0, t_start + delay_sec - time.time()))
            handler.attribute_queue.put(report)

    feeder = threading.Thread(target=run, daemon=True)
    feeder.start()
    return feeder


def _cluster_wide_handler() -> AttributeSubscriptionHandler:
    return AttributeSubscriptionHandler(
        expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
        expected_attribute=None)


class TestAttributeSubscriptionLiveness(MatterBaseTest):
    # Pure unit test, no DUT: never attempt the framework's background subscription.
    disable_wildcard_subscription = True

    def _expect_failure(self, fn, expected_substring: str):
        try:
            fn()
        except signals.TestFailure as e:
            asserts.assert_in(expected_substring, str(e),
                              f"Failure message should contain '{expected_substring}', got: {e}")
            return
        asserts.fail(f"Expected TestFailure containing '{expected_substring}', but call succeeded")

    def test_target_value_returns(self):
        """Regression: plain target wait (no new parameters) still succeeds."""
        handler = _cluster_wide_handler()
        _feed(handler, [(0.1, _report(UpdateState, UpdateStateEnum.kApplying))])
        t = handler.await_first_value_asserting_no_forbidden(
            target_value=UpdateStateEnum.kApplying,
            forbidden_values=set(),
            timeout_sec=5.0)
        asserts.assert_true(t <= time.time(), "Returned timestamp should be in the past")

    def test_forbidden_value_fails(self):
        """Regression: forbidden value before target still fails."""
        handler = _cluster_wide_handler()
        _feed(handler, [(0.1, _report(UpdateState, UpdateStateEnum.kDownloading))])
        self._expect_failure(
            lambda: handler.await_first_value_asserting_no_forbidden(
                target_value=UpdateStateEnum.kApplying,
                forbidden_values={UpdateStateEnum.kDownloading},
                timeout_sec=5.0),
            "Forbidden state")

    def test_attribute_filter_ignores_other_attributes(self):
        """expected_attribute must prevent the IntEnum collision on cluster-wide queues.

        UpdateStateProgress reports carry plain ints; cluster enums compare equal to
        ints, so a progress report numerically equal to the target enum must not be
        accepted as the target (nor trip the forbidden check).
        """
        handler = _cluster_wide_handler()
        _feed(handler, [
            # Numerically equals UpdateStateEnum.kApplying but is a progress report.
            (0.1, _report(UpdateStateProgress, int(UpdateStateEnum.kApplying))),
            # Numerically equals the forbidden kDownloading but is a progress report.
            (0.2, _report(UpdateStateProgress, int(UpdateStateEnum.kDownloading))),
            (0.6, _report(UpdateState, UpdateStateEnum.kApplying)),
        ])
        t_call_start = time.time()
        handler.await_first_value_asserting_no_forbidden(
            target_value=UpdateStateEnum.kApplying,
            forbidden_values={UpdateStateEnum.kDownloading},
            timeout_sec=5.0,
            expected_attribute=UpdateState)
        elapsed = time.time() - t_call_start
        asserts.assert_greater_equal(
            elapsed, 0.5, "Method must have waited for the genuine UpdateState report, "
            "not matched the numerically-equal progress report")

    def test_attribute_filter_timeout_without_genuine_target(self):
        """With the filter set, a numerically-matching foreign report alone must time out."""
        handler = _cluster_wide_handler()
        _feed(handler, [(0.1, _report(UpdateStateProgress, int(UpdateStateEnum.kApplying)))])
        self._expect_failure(
            lambda: handler.await_first_value_asserting_no_forbidden(
                target_value=UpdateStateEnum.kApplying,
                forbidden_values=set(),
                timeout_sec=1.0,
                expected_attribute=UpdateState),
            "budget exhausted")

    def test_stall_without_reports_fails(self):
        """No liveness reports for longer than stall_timeout_sec must fail with a stall message."""
        handler = _cluster_wide_handler()
        t_call_start = time.time()
        self._expect_failure(
            lambda: handler.await_first_value_asserting_no_forbidden(
                target_value=UpdateStateEnum.kApplying,
                forbidden_values=set(),
                timeout_sec=30.0,
                stall_timeout_sec=0.5),
            "Stall detected")
        # Stall must fire long before the overall timeout (queue poll granularity is 1s).
        asserts.assert_less(time.time() - t_call_start, 5.0,
                            "Stall detection should not wait for the overall timeout")

    def test_liveness_extends_wait_beyond_stall_timeout(self):
        """Progress reports must keep resetting the stall timer until the target arrives."""
        handler = _cluster_wide_handler()
        progress_reports = [(0.3 * i, _report(UpdateStateProgress, i)) for i in range(1, 6)]
        _feed(handler, progress_reports + [(1.8, _report(UpdateState, UpdateStateEnum.kApplying))])

        last_progress = [None]

        def progress_liveness(report):
            if report.attribute != UpdateStateProgress:
                return False
            if report.value is None or report.value == last_progress[0]:
                return False
            last_progress[0] = report.value
            return True

        # Total wait (~1.8s) far exceeds stall_timeout_sec: only liveness resets allow success.
        handler.await_first_value_asserting_no_forbidden(
            target_value=UpdateStateEnum.kApplying,
            forbidden_values=set(),
            timeout_sec=30.0,
            expected_attribute=UpdateState,
            stall_timeout_sec=0.6,
            liveness_matcher=progress_liveness)

    def test_budget_exhausted_despite_liveness(self):
        """Live progress must not extend the wait past the overall timeout_sec budget."""
        handler = _cluster_wide_handler()
        _feed(handler, [(0.2 * i, _report(UpdateStateProgress, i)) for i in range(1, 15)])

        def any_progress(report):
            return report.attribute == UpdateStateProgress

        self._expect_failure(
            lambda: handler.await_first_value_asserting_no_forbidden(
                target_value=UpdateStateEnum.kApplying,
                forbidden_values=set(),
                timeout_sec=1.0,
                expected_attribute=UpdateState,
                stall_timeout_sec=0.5,
                liveness_matcher=any_progress),
            "budget exhausted")


if __name__ == "__main__":
    default_matter_test_main()
