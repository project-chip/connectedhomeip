#
#    Copyright (c) 2025 Project CHIP Authors
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

"""Unit tests for PromptCoordinator and the on_timeout extension in AttributeSubscriptionHandler."""

import queue
import threading
import time
import unittest
from unittest.mock import MagicMock, patch

from mobly import signals

from matter.testing.prompt_coordinator import PromptCoordinator

try:
    from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler
    from matter.testing.matter_testing import AttributeValue
    HAS_MATTER_SDK = True
except ImportError:
    HAS_MATTER_SDK = False


class TestPromptCoordinatorNonInteractive(unittest.TestCase):
    """PromptCoordinator in non-interactive (CI) mode always returns False."""

    def setUp(self):
        self.coordinator = PromptCoordinator()

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_returns_false_when_not_a_tty(self, mock_stdin):
        mock_stdin.isatty.return_value = False
        result = self.coordinator.ask_user("some timeout", elapsed_sec=100, extension_sec=60)
        self.assertFalse(result)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_does_not_print_when_not_interactive(self, mock_stdin):
        mock_stdin.isatty.return_value = False
        with patch("builtins.print") as mock_print:
            self.coordinator.ask_user("some timeout", elapsed_sec=100, extension_sec=60)
            mock_print.assert_not_called()

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_prompt_time_not_accumulated_in_ci_mode(self, mock_stdin):
        mock_stdin.isatty.return_value = False
        self.coordinator.ask_user("some timeout", elapsed_sec=100, extension_sec=60)
        self.assertEqual(self.coordinator.total_prompt_seconds, 0.0)


class TestPromptCoordinatorInteractive(unittest.TestCase):
    """PromptCoordinator in interactive mode."""

    def _make_interactive_coordinator(self, answer: str):
        """Return a coordinator whose stdin returns ``answer``."""
        coordinator = PromptCoordinator()
        mock_stdin = MagicMock()
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = answer + "\n"
        return coordinator, mock_stdin

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_yes_returns_true(self, mock_stdin):
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "y\n"
        result = PromptCoordinator().ask_user("test", elapsed_sec=50, extension_sec=60)
        self.assertTrue(result)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_yes_long_form_returns_true(self, mock_stdin):
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "yes\n"
        result = PromptCoordinator().ask_user("test", elapsed_sec=50, extension_sec=60)
        self.assertTrue(result)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_no_returns_false(self, mock_stdin):
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "n\n"
        result = PromptCoordinator().ask_user("test", elapsed_sec=50, extension_sec=60)
        self.assertFalse(result)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_prompt_time_is_accumulated(self, mock_stdin):
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "y\n"
        coordinator = PromptCoordinator()
        coordinator.ask_user("test", elapsed_sec=50, extension_sec=60)
        self.assertGreater(coordinator.total_prompt_seconds, 0.0)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_still_needed_false_skips_prompt(self, mock_stdin):
        """If still_needed() returns False after acquiring the lock, no prompt is shown."""
        mock_stdin.isatty.return_value = True
        coordinator = PromptCoordinator()
        with patch("builtins.print") as mock_print:
            result = coordinator.ask_user(
                "test", elapsed_sec=50, extension_sec=60,
                still_needed=lambda: False
            )
        mock_print.assert_not_called()
        self.assertTrue(result)  # resolved — treated as extended

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_still_needed_true_shows_prompt(self, mock_stdin):
        """If still_needed() returns True, the prompt is displayed."""
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "y\n"
        coordinator = PromptCoordinator()
        with patch("builtins.print") as mock_print:
            result = coordinator.ask_user(
                "test", elapsed_sec=50, extension_sec=60,
                still_needed=lambda: True
            )
        mock_print.assert_called()
        self.assertTrue(result)


class TestPromptCoordinatorSerialisation(unittest.TestCase):
    """Concurrent prompts are serialised — only one at a time."""

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_concurrent_prompts_are_serialised(self, mock_stdin):
        mock_stdin.isatty.return_value = True

        prompt_order = []
        lock = threading.Lock()

        def slow_readline():
            time.sleep(0.05)  # simulate user thinking
            with lock:
                prompt_order.append(threading.current_thread().name)
            return "y\n"

        mock_stdin.readline.side_effect = slow_readline

        coordinator = PromptCoordinator()
        results = []

        def run(name):
            threading.current_thread().name = name
            r = coordinator.ask_user(name, elapsed_sec=1, extension_sec=10)
            results.append(r)

        t1 = threading.Thread(target=run, args=("thread-1",))
        t2 = threading.Thread(target=run, args=("thread-2",))
        t1.start()
        t2.start()
        t1.join()
        t2.join()

        # Both should succeed (user said yes to each)
        self.assertEqual(results, [True, True])
        # Prompts were issued in sequence, not overlapping
        self.assertEqual(len(prompt_order), 2)

    @patch("matter.testing.prompt_coordinator.sys.stdin")
    def test_wait_time_accumulates_in_total_prompt_seconds(self, mock_stdin):
        """Time waiting for the lock is counted as prompt time."""
        mock_stdin.isatty.return_value = True
        mock_stdin.readline.return_value = "y\n"

        coordinator = PromptCoordinator()
        coordinator._lock.acquire()  # hold the lock to simulate another prompt in progress

        start = time.time()
        release_at = [start + 0.1]

        def release():
            time.sleep(0.1)
            coordinator._lock.release()

        threading.Thread(target=release, daemon=True).start()

        coordinator.ask_user("test", elapsed_sec=5, extension_sec=10)

        # total_prompt_seconds must include the wait (~0.1 s) plus interaction time
        self.assertGreaterEqual(coordinator.total_prompt_seconds, 0.1)


@unittest.skipUnless(HAS_MATTER_SDK, "Matter SDK not available — skipping AttributeSubscriptionHandler tests")
class TestAwaitAllExpectedReportMatchesOnTimeout(unittest.TestCase):
    """on_timeout callback integration in await_all_expected_report_matches."""

    def _make_handler_with_no_reports(self):
        """Return an AttributeSubscriptionHandler whose report store is always empty."""
        from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler
        handler = AttributeSubscriptionHandler.__new__(AttributeSubscriptionHandler)
        handler._attribute_reports = {}
        handler._attribute_report_counts = {}
        handler._lock = threading.Lock()
        handler._q = queue.Queue()
        handler._endpoint_id = 0
        return handler, AttributeMatcher

    def test_on_timeout_extend_then_succeed(self):
        """on_timeout returns True once → deadline extended; report arrives → success."""
        handler = AttributeSubscriptionHandler.__new__(AttributeSubscriptionHandler)
        handler._attribute_report_counts = {}
        handler._lock = threading.Lock()
        handler._q = queue.Queue()
        handler._endpoint_id = 0

        handler._attribute_reports = {}
        called = [0]

        fake_attr = MagicMock()
        fake_report = MagicMock(spec=AttributeValue)
        fake_report.value = 42

        def on_timeout(pending_descriptions, elapsed_sec, extension_sec):
            called[0] += 1
            # Inject a matching report so the next loop iteration finds it
            handler._attribute_reports = {fake_attr: [fake_report]}
            return True  # extend

        matcher = AttributeMatcher.from_callable(
            description="wait for 42",
            matcher=lambda r: r.value == 42
        )

        handler.await_all_expected_report_matches(
            [matcher],
            timeout_sec=0.05,
            on_timeout=on_timeout,
            extension_sec=10.0,
        )

        self.assertEqual(called[0], 1)

    def test_on_timeout_abort_fails_test(self):
        """on_timeout returns False → asserts.fail is raised."""
        handler = AttributeSubscriptionHandler.__new__(AttributeSubscriptionHandler)
        handler._attribute_reports = {}
        handler._attribute_report_counts = {}
        handler._lock = threading.Lock()
        handler._q = queue.Queue()
        handler._endpoint_id = 0

        matcher = AttributeMatcher.from_callable(
            description="never matches",
            matcher=lambda r: False
        )

        with self.assertRaises(signals.TestFailure):
            handler.await_all_expected_report_matches(
                [matcher],
                timeout_sec=0.05,
                on_timeout=lambda **kw: False,
            )

    def test_no_callback_fails_immediately(self):
        """Without on_timeout the method fails on timeout as before."""
        handler = AttributeSubscriptionHandler.__new__(AttributeSubscriptionHandler)
        handler._attribute_reports = {}
        handler._attribute_report_counts = {}
        handler._lock = threading.Lock()
        handler._q = queue.Queue()
        handler._endpoint_id = 0

        matcher = AttributeMatcher.from_callable(
            description="never matches",
            matcher=lambda r: False
        )

        with self.assertRaises(signals.TestFailure):
            handler.await_all_expected_report_matches([matcher], timeout_sec=0.05)


@unittest.skipUnless(HAS_MATTER_SDK, "Matter SDK not available — skipping AttributeSubscriptionHandler tests")
class TestAwaitFirstValueOnTimeout(unittest.TestCase):
    """on_timeout callback integration in await_first_value_asserting_no_forbidden."""

    def _make_handler(self):
        from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
        handler = AttributeSubscriptionHandler.__new__(AttributeSubscriptionHandler)
        handler._attribute_reports = {}
        handler._attribute_report_counts = {}
        handler._lock = threading.Lock()
        handler._q = queue.Queue()
        handler._endpoint_id = 0
        return handler

    def test_extend_then_value_arrives(self):
        """on_timeout extends deadline; target value arrives → returns timestamp."""
        handler = self._make_handler()
        called = [0]

        fake_report = MagicMock()
        fake_report.value = 99

        def on_timeout(pending_descriptions, elapsed_sec, extension_sec):
            called[0] += 1
            handler._q.put(fake_report)
            return True

        t = handler.await_first_value_asserting_no_forbidden(
            target_value=99,
            forbidden_values=set(),
            timeout_sec=0.05,
            on_timeout=on_timeout,
            extension_sec=10.0,
        )
        self.assertEqual(called[0], 1)
        self.assertIsInstance(t, float)

    def test_abort_fails_test(self):
        handler = self._make_handler()
        with self.assertRaises(signals.TestFailure):
            handler.await_first_value_asserting_no_forbidden(
                target_value=99,
                forbidden_values=set(),
                timeout_sec=0.05,
                on_timeout=lambda **kw: False,
            )

    def test_no_callback_fails_immediately(self):
        handler = self._make_handler()
        with self.assertRaises(signals.TestFailure):
            handler.await_first_value_asserting_no_forbidden(
                target_value=99,
                forbidden_values=set(),
                timeout_sec=0.05,
            )


if __name__ == "__main__":
    unittest.main()
