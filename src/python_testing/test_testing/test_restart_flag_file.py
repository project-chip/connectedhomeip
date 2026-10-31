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

import asyncio
import builtins
import contextlib
import importlib.util
import io
import os
import stat
import sys
import tempfile
import textwrap
import threading
import time
import types
import unittest
from pathlib import Path
from unittest import mock

from mobly import signals

from matter.testing import matter_testing
from matter.testing.matter_testing import MatterBaseTest

CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(CHIP_ROOT / "src" / "python_testing"))
import TC_SUTestBase  # noqa: E402 isort:skip


def _load_run_python_test():
    spec = importlib.util.spec_from_file_location("run_python_test", CHIP_ROOT / "scripts" / "tests" / "run_python_test.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


run_python_test = _load_run_python_test()


class TestRestartFlagReader(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.flag = os.path.join(self.dir.name, "chip_test_restart_app_unittest")
        self.config = run_python_test.TestRunConfig("app", "--KVS kvs1", "--storage-path admin.json", None, None)
        self.app_manager = mock.patch.object(run_python_test, "AppProcessManager").start()
        self.reset = mock.patch.object(run_python_test, "factory_reset_config_removal").start()
        self.monitor = run_python_test.AppRestartMonitor(self.flag)

    def tearDown(self):
        self.monitor.stop()
        mock.patch.stopall()
        self.dir.cleanup()

    def _start_monitor(self):
        self.monitor.start([mock.Mock()], threading.Lock(), self.config)

    def _wait_consumed(self):
        deadline = time.monotonic() + 10
        while os.path.exists(self.flag) and time.monotonic() < deadline:
            time.sleep(0.05)
        self.assertFalse(os.path.exists(self.flag), "monitor did not consume the flag file")

    def test_factory_reset_written_after_file_creation(self):
        # The file exists empty across several monitor polls (0.5 s) before the request text lands.
        with open(self.flag, "w") as f:
            self._start_monitor()
            time.sleep(1.5)
            self.assertTrue(os.path.exists(self.flag), "monitor removed a flag file it had not acted on")
            with open(self.flag) as current:
                self.assertEqual(current.read(), "")
            self.app_manager.assert_not_called()
            f.write("factory reset app only")
        self._wait_consumed()
        self.reset.assert_called_once_with("--KVS kvs1", "--storage-path admin.json", run_python_test.FactoryResetType.AppOnly)
        self.app_manager.assert_called_once_with(self.config)

    def test_known_requests(self):
        for text, reset_type in (("restart\n", None),
                                 ("factory reset", run_python_test.FactoryResetType.AppAndController),
                                 ("factory reset app only", run_python_test.FactoryResetType.AppOnly)):
            with self.subTest(text=text):
                self.app_manager.reset_mock()
                self.reset.reset_mock()
                self.monitor = run_python_test.AppRestartMonitor(self.flag)
                with open(self.flag, "w") as f:
                    f.write(text)
                self._start_monitor()
                self._wait_consumed()
                self.monitor.stop()
                if reset_type is None:
                    self.reset.assert_not_called()
                else:
                    self.reset.assert_called_once_with("--KVS kvs1", "--storage-path admin.json", reset_type)
                self.app_manager.assert_called_once_with(self.config)

    def test_partial_request_keeps_polling(self):
        with open(self.flag, "w") as f:
            f.write("factory reset app")
            f.flush()
            self._start_monitor()
            time.sleep(1.0)
            with open(self.flag) as current:
                self.assertEqual(current.read(), "factory reset app")
            self.app_manager.assert_not_called()
            f.write(" only")
        self._wait_consumed()
        self.reset.assert_called_once_with("--KVS kvs1", "--storage-path admin.json", run_python_test.FactoryResetType.AppOnly)
        self.app_manager.assert_called_once_with(self.config)

    def _assert_rejected(self, text, shown):
        monitor_open = getattr(run_python_test, "open", builtins.open)
        unknown_reads = []

        def counted_open(file, *args, **kwargs):
            if file == self.flag:
                with contextlib.suppress(FileNotFoundError), builtins.open(file, "rb") as raw:
                    if not raw.read().startswith(b"rejected: "):
                        unknown_reads.append(file)
            return monitor_open(file, *args, **kwargs)

        start = time.monotonic()
        with self.assertLogs(run_python_test.log, "ERROR") as logs, \
                mock.patch.object(run_python_test, "open", counted_open, create=True):
            with open(self.flag + ".new", "wb") as f:
                f.write(text)
            unknown_reads.clear()
            os.replace(self.flag + ".new", self.flag)
            with self.assertRaisesRegex(signals.TestFailure, f"rejected by the test runner \\(rejected: {shown}\\)"):
                asyncio.run(MatterBaseTest.wait_for_restart_flag_file_removal(None, self.flag, "factory reset", timeout_sec=10))
            self.assertLessEqual(len(unknown_reads), 5)
        self.assertLess(time.monotonic() - start, 5)
        self.assertIn(f"'{shown}'", logs.output[0])

    def test_unknown_request_fails_writer(self):
        self._start_monitor()
        self._assert_rejected(b"factory res3t", "factory res3t")
        time.sleep(0.5)
        with open(self.flag) as f:
            self.assertEqual(f.read(), "rejected: factory res3t")
        self._assert_rejected(b"\xff", "\ufffd")
        self.app_manager.assert_not_called()
        self.reset.assert_not_called()
        matter_testing.write_restart_flag_file(self.flag, "restart")
        self._wait_consumed()
        self.app_manager.assert_called_once_with(self.config)
        self.assertEqual(os.listdir(self.dir.name), [])

    def test_rejection_under_ascii_locale(self):
        def ascii_default(real_open):
            def ascii_open(file, mode="r", *args, encoding=None, **kwargs):
                return real_open(file, mode, *args, encoding=encoding or (None if "b" in mode else "ascii"), **kwargs)
            return ascii_open

        with mock.patch.object(run_python_test, "open", ascii_default(builtins.open), create=True), \
                mock.patch.object(matter_testing, "open", ascii_default(builtins.open), create=True):
            self._start_monitor()
            self._assert_rejected(b"factory r\xc3\xa9s\xc3\xa9t\xff", "factory r\u00e9s\u00e9t\ufffd")
        self.assertTrue(self.monitor.thread.is_alive())

    def test_failed_rejection_is_retried(self):
        real_replace = os.replace
        failures = [OSError("replace failed")]

        def flaky_replace(src, dst):
            if dst == self.flag and src.endswith(".rejected") and failures:
                raise failures.pop()
            return real_replace(src, dst)

        with mock.patch.object(run_python_test.os, "replace", flaky_replace):
            self._start_monitor()
            self._assert_rejected(b"factory res3t", "factory res3t")
        self.assertEqual(failures, [])

    def _poll_sequence(self, reads):
        real_exists, real_open, real_unlink = os.path.exists, open, os.unlink
        drained = threading.Event()

        def exists(path):
            if path != self.flag:
                return real_exists(path)
            if not reads:
                drained.set()
                return real_exists(path)
            if reads[0] is None:
                reads.pop(0)
                return False
            return True

        def scripted_open(file, *args, **kwargs):
            if file != self.flag or not reads:
                return real_open(file, *args, **kwargs)
            if (content := reads.pop(0)) is FileNotFoundError:
                raise FileNotFoundError(file)
            return io.StringIO(content)

        self.monitor = run_python_test.AppRestartMonitor(self.flag)
        with mock.patch.object(run_python_test.os.path, "exists", exists), \
                mock.patch.object(run_python_test.os, "unlink", lambda path: path == self.flag or real_unlink(path)), \
                mock.patch.object(run_python_test, "open", scripted_open, create=True):
            self._start_monitor()
            self.assertTrue(drained.wait(10))
            self.monitor.stop()

    def test_unknown_polls_are_consecutive(self):
        x, y = "factory res3t", "factory res4t"
        for name, reads in (("flag absent", [x, x, None, x, x]),
                            ("flag removed during read", [x, x, FileNotFoundError, x, x]),
                            ("content changed", [x, x, y, y]),
                            ("partial request", [x, x, "factory", x, x]),
                            ("served request", [x, x, "restart", x, x]),
                            ("new request after removal", [x, x, None, y, y])):
            with self.subTest(name), self.assertNoLogs(run_python_test.log, "ERROR"):
                self._poll_sequence(reads)
                self.assertFalse(os.path.exists(self.flag))
        with self.assertLogs(run_python_test.log, "ERROR"):
            self._poll_sequence([x, x, None, y, y, y])
        with open(self.flag) as f:
            self.assertEqual(f.read(), f"rejected: {y}")
        self.app_manager.assert_called_once_with(self.config)

    def test_runner_removes_rejection_marker(self):
        script = os.path.join(self.dir.name, "rejected_writer.py")
        with open(script, "w") as f:
            f.write(textwrap.dedent("""
                import sys
                flag = sys.argv[sys.argv.index("--restart-flag-file") + 1]
                for path in (flag, flag + ".rejected", sys.argv[0] + ".out"):
                    with open(path, "w") as f:
                        f.write(flag)
            """))
        run_python_test.main_impl("", False, False, "", "", None, script, "", False, False, Path(self.dir.name), 60, True, "run1")
        with open(script + ".out") as f:
            flag = f.read()
        self.assertFalse(os.path.exists(flag))
        self.assertFalse(os.path.exists(flag + ".rejected"))

    def test_transient_unknown_read_is_tolerated(self):
        real_open = open
        stale_reads = [io.StringIO("factory res3t"), io.StringIO("factory res3t")]

        def flaky_open(file, *args, **kwargs):
            if file == self.flag and stale_reads:
                return stale_reads.pop()
            return real_open(file, *args, **kwargs)

        with mock.patch.object(run_python_test, "open", flaky_open, create=True):
            for stale in ([], [io.StringIO("factory res3t")]):
                stale_reads.extend(stale)
                with open(self.flag, "w") as f:
                    f.write("restart")
                if self.monitor.thread is None:
                    self._start_monitor()
                self._wait_consumed()
        self.assertEqual(self.app_manager.call_count, 2)

    def test_writer_wait_tolerates_removal_during_check(self):
        with mock.patch.object(matter_testing.os.path, "exists", side_effect=[True, False]):
            asyncio.run(MatterBaseTest.wait_for_restart_flag_file_removal(None, self.flag, "restart"))

    def test_flag_removed_between_check_and_read(self):
        real_exists = os.path.exists
        stale_checks = [True, True]

        def exists(path):
            if path == self.flag and stale_checks:
                return stale_checks.pop()
            return real_exists(path)

        with mock.patch.object(run_python_test.os.path, "exists", exists):
            self._start_monitor()
            time.sleep(0.3)
        self.assertTrue(self.monitor.thread.is_alive())
        with open(self.flag, "w") as f:
            f.write("restart")
        self._wait_consumed()
        self.app_manager.assert_called_once_with(self.config)

    def test_stop_while_waiting_for_request(self):
        open(self.flag, "w").close()
        self._start_monitor()
        time.sleep(0.3)
        start = time.monotonic()
        self.monitor.stop()
        self.assertFalse(self.monitor.thread.is_alive())
        self.assertLess(time.monotonic() - start, 1.0)
        self.assertTrue(os.path.exists(self.flag))
        self.app_manager.assert_not_called()


class TestRestartFlagWriter(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.flag = os.path.join(self.dir.name, "chip_test_restart_app_unittest")
        self.seen = set()
        self.flag_opened_for_write = []

    def tearDown(self):
        self.dir.cleanup()

    def _record(self):
        if os.path.exists(self.flag):
            with open(self.flag) as f:
                self.seen.add(f.read())

    def _trace(self, frame, event, arg):
        self._record()
        return self._trace

    def _observe(self, fn):
        real_open, real_os_open = builtins.open, os.open

        def checked_open(file, mode="r", *args, **kwargs):
            if file == self.flag and mode != "r":
                self.flag_opened_for_write.append(mode)
            return real_open(file, mode, *args, **kwargs)

        def checked_os_open(path, flags, *args, **kwargs):
            if path == self.flag:
                self.flag_opened_for_write.append(flags)
            return real_os_open(path, flags, *args, **kwargs)

        # Check the flag path on every traced line, so a reader polling at any moment is covered.
        previous_trace = sys.gettrace()
        with mock.patch.object(builtins, "open", checked_open), mock.patch.object(os, "open", checked_os_open):
            sys.settrace(self._trace)
            try:
                fn()
            finally:
                sys.settrace(previous_trace)
        self._record()
        self.assertEqual(self.flag_opened_for_write, [], "flag file was written in place")
        self.assertEqual(os.listdir(self.dir.name), [os.path.basename(self.flag)])

    def _fake_test(self):
        async def no_wait(*args):
            pass
        return types.SimpleNamespace(
            get_restart_flag_file=lambda: self.flag,
            _fail_if_wildcard_subscription_active=lambda operation: None,
            _expire_sessions_on_all_controllers=lambda: None,
            wait_for_restart_flag_file_removal=no_wait,
            controller=mock.Mock(),
            requestor_node_id=1,
        )

    def test_factory_reset_request(self):
        self._observe(lambda: asyncio.run(MatterBaseTest.request_device_factory_reset(self._fake_test())))
        self.assertEqual(self.seen, {"factory reset app only"})

    def test_factory_reset_ctrl_request(self):
        self._observe(lambda: asyncio.run(MatterBaseTest.request_device_factory_reset(self._fake_test(), reset_ctrl=True)))
        self.assertEqual(self.seen, {"factory reset"})

    def test_reboot_request(self):
        self._observe(lambda: asyncio.run(MatterBaseTest.request_device_reboot(self._fake_test())))
        self.assertEqual(self.seen, {"restart"})

    def test_software_update_restart_request(self):
        with mock.patch.object(TC_SUTestBase, "sleep"):
            self._observe(lambda: TC_SUTestBase.SoftwareUpdateBaseTest.restart_requestor(self._fake_test()))
        self.assertEqual(self.seen, {"restart"})

    def test_flag_file_mode_matches_open(self):
        reference = os.path.join(self.dir.name, "reference")
        previous_umask = os.umask(0o027)
        try:
            matter_testing.write_restart_flag_file(self.flag, "restart")
            open(reference, "w").close()
        finally:
            os.umask(previous_umask)
        self.assertEqual(stat.S_IMODE(os.stat(reference).st_mode), 0o640)
        self.assertEqual(stat.S_IMODE(os.stat(self.flag).st_mode), 0o640)

    def test_failed_write_leaves_no_temp_file(self):
        with mock.patch.object(matter_testing.os, "replace", side_effect=OSError("replace failed")), self.assertRaises(OSError):
            matter_testing.write_restart_flag_file(self.flag, "restart")
        self.assertEqual(os.listdir(self.dir.name), [])

    def test_existing_temp_name_is_not_removed(self):
        with mock.patch.object(matter_testing.uuid, "uuid4", return_value=mock.Mock(hex="taken")), self.assertRaises(FileExistsError):
            open(self.flag + ".taken", "w").close()
            matter_testing.write_restart_flag_file(self.flag, "restart")
        self.assertEqual(os.listdir(self.dir.name), [os.path.basename(self.flag) + ".taken"])

    def test_failed_cleanup_keeps_original_error(self):
        with mock.patch.object(matter_testing.os, "replace", side_effect=OSError("replace failed")), \
                mock.patch.object(matter_testing.os, "unlink", side_effect=OSError("unlink failed")), \
                self.assertRaisesRegex(OSError, "replace failed"):
            matter_testing.write_restart_flag_file(self.flag, "restart")


if __name__ == "__main__":
    unittest.main()
