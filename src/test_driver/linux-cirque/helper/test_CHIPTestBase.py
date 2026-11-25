"""
Copyright (c) 2026 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import http.server
import json
import logging
import os
import signal
import socket
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path
from unittest import mock

import requests
import urllib3


class _DaemonThreadingHTTPServer(http.server.ThreadingHTTPServer):
    daemon_threads = True


def _make_handler(stop_event: threading.Event, state: dict):
    class _CirqueStubHandler(http.server.BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):
            state["requests"].append(self.path)

            if self.path.startswith("/fast_json"):
                body = json.dumps({"status": "ok", "count": 2}).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
                return

            if self.path.startswith("/chunked_json"):
                parts = [b'{"status": ', b'"chunked"}']
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Transfer-Encoding", "chunked")
                self.end_headers()
                for part in parts:
                    self.wfile.write(f"{len(part):x}\r\n".encode("ascii") + part + b"\r\n")
                    self.wfile.flush()
                self.wfile.write(b"0\r\n\r\n")
                self.wfile.flush()
                return

            if self.path.startswith("/non_json"):
                body = b"<html>502 Bad Gateway</html>"
                self.send_response(502)
                self.send_header("Content-Type", "text/html")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
                return

            if self.path.startswith("/device_log/"):
                mode = state.get("log_mode", "fast")
                if mode == "fast":
                    body = b"booting...\nready for action\n"
                    self.send_response(200)
                    self.send_header("Content-Length", str(len(body)))
                    self.end_headers()
                    self.wfile.write(body)
                    return

                if mode == "stalled_headers":
                    stop_event.wait(timeout=state.get("stall_sec", 2.0))
                    return

                if mode == "stalled_body":
                    self.send_response(200)
                    self.send_header("Content-Length", "100")
                    self.end_headers()
                    try:
                        self.wfile.write(b"partial")
                        self.wfile.flush()
                    except OSError:
                        return
                    stop_event.wait(timeout=state.get("stall_sec", 2.0))
                    return

                if mode == "trickle_fixed":
                    total = state.get("trickle_count", 40)
                    interval = state.get("trickle_interval", 0.05)
                    self.send_response(200)
                    self.send_header("Content-Length", str(total))
                    self.end_headers()
                    for _ in range(total):
                        if stop_event.wait(timeout=interval):
                            return
                        try:
                            self.wfile.write(b"x")
                            self.wfile.flush()
                        except OSError:
                            return
                    return

                if mode == "trickle_chunked":
                    total = state.get("trickle_count", 40)
                    interval = state.get("trickle_interval", 0.05)
                    self.send_response(200)
                    self.send_header("Transfer-Encoding", "chunked")
                    self.end_headers()
                    for _ in range(total):
                        if stop_event.wait(timeout=interval):
                            return
                        try:
                            self.wfile.write(b"1\r\nx\r\n")
                            self.wfile.flush()
                        except OSError:
                            return
                    try:
                        self.wfile.write(b"0\r\n\r\n")
                        self.wfile.flush()
                    except OSError:
                        return
                    return

                if mode == "sigterm_on_first_then_stall":
                    call_idx = sum(1 for p in state["requests"] if p.startswith("/device_log/"))
                    if call_idx == 1 and state.get("sigterm_pid"):
                        os.kill(state["sigterm_pid"], signal.SIGTERM)
                    stop_event.wait(timeout=state.get("stall_sec", 2.0))
                    return

            if self.path.startswith("/destroy_home/"):
                state["destroy_calls"].append(time.monotonic())
                delay = state.get("destroy_delay", 0.0)
                if delay > 0 and stop_event.wait(timeout=delay):
                    return
                body = json.dumps("destroyed").encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                try:
                    self.wfile.write(body)
                except OSError:
                    return
                return

            self.send_response(404)
            self.send_header("Content-Length", "0")
            self.end_headers()

        def log_message(self, fmt, *args):
            # Silence BaseHTTPRequestHandler stderr logging during tests.
            pass

    return _CirqueStubHandler


class TestCHIPTestBase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cirque_dir = str(Path(__file__).resolve().parent.parent)
        if cirque_dir not in sys.path:
            sys.path.insert(0, cirque_dir)
        from helper import CHIPTestBase

        cls.mod = CHIPTestBase

    def setUp(self):
        self.stop_event = threading.Event()
        self.state = {
            "requests": [],
            "destroy_calls": [],
            "log_mode": "fast",
            "stall_sec": 2.0,
            "trickle_count": 40,
            "trickle_interval": 0.05,
            "destroy_delay": 0.0,
            "sigterm_pid": None,
        }
        self.server = _DaemonThreadingHTTPServer(("127.0.0.1", 0), _make_handler(self.stop_event, self.state))
        self.server_thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.server_thread.start()
        self.base_url = f"http://127.0.0.1:{self.server.server_address[1]}/"

    def tearDown(self):
        self.stop_event.set()
        self.server.shutdown()
        self.server.server_close()
        self.server_thread.join(timeout=2.0)

    def _make_home(self, cirque_url=None, num_devices=1):
        cfg = {
            f"device{i}": {"type": "CHIPEndDevice", "base_image": "@default", "capability": []}
            for i in range(num_devices)
        }
        home = self.mod.CHIPVirtualHome(cirque_url or self.base_url, cfg)
        home.logger = logging.getLogger("test_chiptestbase")
        home.logger.addHandler(logging.NullHandler())
        home.home_id = "home-1"
        home.non_ap_devices = [
            {"id": f"dev-{i:08d}-xxxx", "type": "CHIPEndDevice"}
            for i in range(num_devices)
        ]
        return home

    def test_wait_for_device_output_matches_fast_response(self):
        home = self._make_home()
        self.assertEqual(home.query_api("fast_json"), {"status": "ok", "count": 2})
        self.assertEqual(home.query_api("chunked_json"), {"status": "chunked"})
        self.assertTrue(home.wait_for_device_output("dev-0", "ready for action", timeout=1.0))

    def test_query_api_non_json_raises_requests_json_decode_error(self):
        home = self._make_home()
        with self.assertRaises(requests.exceptions.JSONDecodeError) as ctx:
            home.query_api("non_json")
        self.assertIsInstance(ctx.exception, requests.exceptions.RequestException)

    def test_wait_for_device_output_stalled_headers(self):
        self.state["log_mode"] = "stalled_headers"
        home = self._make_home()
        t0 = time.monotonic()
        ok = home.wait_for_device_output("dev-0", "ready", timeout=0.3)
        elapsed = time.monotonic() - t0
        self.assertFalse(ok)
        self.assertGreaterEqual(elapsed, 0.25)
        self.assertLess(elapsed, 1.0)

    def test_wait_for_device_output_stalled_body(self):
        self.state["log_mode"] = "stalled_body"
        home = self._make_home()
        t0 = time.monotonic()
        ok = home.wait_for_device_output("dev-0", "ready", timeout=0.3)
        elapsed = time.monotonic() - t0
        self.assertFalse(ok)
        self.assertGreaterEqual(elapsed, 0.25)
        self.assertLess(elapsed, 1.0)

    def test_wait_for_device_output_slow_trickling_body_fixed_length(self):
        self.state["log_mode"] = "trickle_fixed"
        self.state["trickle_count"] = 40
        self.state["trickle_interval"] = 0.05
        home = self._make_home()
        t0 = time.monotonic()
        ok = home.wait_for_device_output("dev-0", "ready", timeout=0.3)
        elapsed = time.monotonic() - t0
        self.assertFalse(ok)
        self.assertGreaterEqual(elapsed, 0.25)
        self.assertLess(elapsed, 0.8)

    def test_wait_for_device_output_slow_trickling_body_chunked(self):
        self.state["log_mode"] = "trickle_chunked"
        self.state["trickle_count"] = 40
        self.state["trickle_interval"] = 0.05
        home = self._make_home()
        t0 = time.monotonic()
        ok = home.wait_for_device_output("dev-0", "ready", timeout=0.3)
        elapsed = time.monotonic() - t0
        self.assertFalse(ok)
        self.assertGreaterEqual(elapsed, 0.25)
        self.assertLess(elapsed, 0.8)

    def test_wait_for_device_output_propagates_connection_refused(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
            probe.bind(("127.0.0.1", 0))
            unused_port = probe.getsockname()[1]
        home = self._make_home(cirque_url=f"http://127.0.0.1:{unused_port}/")
        with self.assertRaises(requests.exceptions.ConnectionError):
            home.wait_for_device_output("dev-0", "ready", timeout=0.5)

    def test_sigterm_cleanup_shares_single_deadline_across_five_devices(self):
        self.state["log_mode"] = "stalled_headers"
        self.state["stall_sec"] = 3.0
        self.state["destroy_delay"] = 0.05

        class _SigtermTest(self.mod.CHIPVirtualHome):
            def setup(self):
                os.kill(os.getpid(), signal.SIGTERM)

            def test_routine(self):
                pass

        with tempfile.TemporaryDirectory() as tmpdir, \
                mock.patch.dict(os.environ, {"DEVICE_LOG_DIR": tmpdir}), \
                mock.patch.object(self.mod, "CLEANUP_TOTAL_TIMEOUT_SEC", 1.0), \
                mock.patch.object(self.mod, "DESTROY_HOME_TIMEOUT_SEC", 0.4):
            home = _SigtermTest(
                self.base_url,
                {f"device{i}": {"type": "CHIPEndDevice", "base_image": "@default", "capability": []} for i in range(5)},
            )
            home.logger = logging.getLogger("test_sigterm_5dev")
            home.logger.addHandler(logging.NullHandler())
            home.home_id = "home-sigterm"
            home.non_ap_devices = [
                {"id": f"dev-{i:08d}-xxxx", "type": "CHIPEndDevice"}
                for i in range(5)
            ]

            t0 = time.monotonic()
            with self.assertRaises(SystemExit) as ctx:
                home.run_test(save_logs=True)
            elapsed = time.monotonic() - t0

        self.assertEqual(ctx.exception.code, 143)
        self.assertIsNone(home.home_id)
        log_reqs = [p for p in self.state["requests"] if p.startswith("/device_log/")]
        self.assertEqual(len(log_reqs), 1)
        self.assertEqual(len(self.state["destroy_calls"]), 1)
        self.assertLess(elapsed, 1.0)
        self.assertGreaterEqual(elapsed, 0.55)

    def test_sigterm_during_save_device_logs_still_bounds_destroy_home(self):
        self.state["log_mode"] = "sigterm_on_first_then_stall"
        self.state["sigterm_pid"] = os.getpid()
        self.state["stall_sec"] = 3.0
        self.state["destroy_delay"] = 2.0

        class _MidCleanupSigtermTest(self.mod.CHIPVirtualHome):
            def setup(self):
                pass

            def test_routine(self):
                pass

        with tempfile.TemporaryDirectory() as tmpdir, \
                mock.patch.dict(os.environ, {"DEVICE_LOG_DIR": tmpdir}), \
                mock.patch.object(self.mod, "CLEANUP_TOTAL_TIMEOUT_SEC", 0.5), \
                mock.patch.object(self.mod, "DESTROY_HOME_TIMEOUT_SEC", 0.2):
            home = _MidCleanupSigtermTest(
                self.base_url,
                {"device0": {"type": "CHIPEndDevice", "base_image": "@default", "capability": []}},
            )
            home.logger = logging.getLogger("test_sigterm_mid_logs")
            home.logger.addHandler(logging.NullHandler())
            home.home_id = "home-mid-sigterm"
            home.non_ap_devices = [{"id": "dev-00000000-xxxx", "type": "CHIPEndDevice"}]

            t0 = time.monotonic()
            with self.assertRaises(SystemExit) as ctx:
                home.run_test(save_logs=True)
            elapsed = time.monotonic() - t0

        self.assertEqual(ctx.exception.code, 143)
        self.assertIsNone(home.home_id)
        self.assertEqual(len(self.state["destroy_calls"]), 1)
        self.assertGreaterEqual(elapsed, 0.45)
        self.assertLess(elapsed, 1.0)

    def test_is_read_timeout_connection_error_classification(self):
        pool = urllib3.HTTPConnectionPool("127.0.0.1", port=1)
        read_timeout = urllib3.exceptions.ReadTimeoutError(pool, "/", "read timed out")
        self.assertTrue(
            self.mod.CHIPVirtualHome._is_read_timeout_connection_error(
                requests.exceptions.ConnectionError(read_timeout)
            )
        )
        ctx_err = requests.exceptions.ConnectionError("wrapped")
        ctx_err.__context__ = TimeoutError("timed out")
        self.assertTrue(self.mod.CHIPVirtualHome._is_read_timeout_connection_error(ctx_err))
        self.assertFalse(
            self.mod.CHIPVirtualHome._is_read_timeout_connection_error(
                requests.exceptions.ConnectionError(ConnectionRefusedError("refused"))
            )
        )

    def test_read_response_chunk_urllib3_v1_fallback(self):
        class _LegacyRaw:
            def read(self, amt, decode_content=True):
                return f"v1-{amt}-{decode_content}".encode("ascii")

        class _BufferedRaw:
            def read1(self, amt):
                return f"no-kw-{amt}".encode("ascii")

            def read(self, amt):
                return f"plain-{amt}".encode("ascii")

        self.assertEqual(
            self.mod.CHIPVirtualHome._read_response_chunk(_LegacyRaw()),
            b"v1-65536-True",
        )
        self.assertEqual(
            self.mod.CHIPVirtualHome._read_response_chunk(_BufferedRaw()),
            b"plain-65536",
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
