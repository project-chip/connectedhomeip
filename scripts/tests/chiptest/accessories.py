#
#    Copyright (c) 2021 Project CHIP Authors
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

from __future__ import annotations

import contextlib
import filecmp
import functools
import logging
import multiprocessing
import queue
import stat
import subprocess
import sys
import tempfile
import threading
from pathlib import Path
from typing import TYPE_CHECKING, Any, Callable, Concatenate, ParamSpec, TypeVar
from xmlrpc.server import SimpleXMLRPCServer

if TYPE_CHECKING:
    from .test_definition import App

log = logging.getLogger(__name__)

_DEFAULT_CHIP_ROOT = Path(__file__).parent.parent.parent.parent.absolute()

PORT = 9000
if sys.platform == 'linux':
    IP = '10.10.10.5'
else:
    IP = '127.0.0.1'

# Create RPC server multiprocessing context so that we can drop RPC server into a network namespace with a custom wrapper.
_ctx_rpc = multiprocessing.get_context("spawn")

if TYPE_CHECKING:
    # Mypy doesn't seem to like custom contexts.
    BaseProc = multiprocessing.Process
else:
    BaseProc = _ctx_rpc.Process


S = TypeVar("S", bound="AppsRegister")
P = ParamSpec("P")
R = TypeVar("R")


def with_accessories_lock(fn: Callable[Concatenate[S, P], R]) -> Callable[Concatenate[S, P], R]:
    """Decorator to acquire self._accessories_lock around instance method calls.

    As _accessories might be accessed either from the chiptest itself and from outside
    via the XMLRPC server it's good to have it available only under mutex.
    """
    @functools.wraps(fn)
    def wrapper(self: S, *args: P.args, **kwargs: P.kwargs) -> R:
        if (lock := getattr(self, "_accessories_lock", None)) is None:
            return fn(self, *args, **kwargs)
        with lock:
            return fn(self, *args, **kwargs)
    return wrapper


class AppsRegister:
    def __init__(self, net_ns: str | None = None) -> None:
        self._accessories: dict[str, App] = {}
        self._accessories_lock = threading.RLock()

        self._net_ns = net_ns

        self._server_thread_cancel = threading.Event()
        self._server_thread = threading.Thread(target=self._run_server)

    def _run_server_loop(self, cmd_queue: AppsXmlRpcServer.CommandQueue, rsp_queue: AppsXmlRpcServer.ResponseQueue) -> None:
        while True:
            try:
                # Get a command from queue.
                name, args = cmd_queue.get(timeout=1)
                assert isinstance(name, str) and isinstance(args, tuple), "Wrong command format"

                # Execute function with arguments and push return value to the response queue.
                if (func := getattr(self, name, None)) is None or not callable(func):
                    raise RuntimeError(f'Function "{name}" doesn\'t exist in AppsRegister')
                if func.__name__ not in APPS_RPC_FUNCS:
                    raise RuntimeError(f'Function "{name}" isn\'t registered as an RPC function')
                rsp_queue.put(func(*args))  # pyright: ignore[reportArgumentType]
            except queue.Empty:
                # No command received within timeout. Continue polling.
                pass
            except Exception as e:
                # On error, pass the exception to the XMLRPC server process and continue.
                log.error("Error in XMLRPC Manager: %s", e, exc_info=True)
                rsp_queue.put(e)
            except KeyboardInterrupt:
                break

            if self._server_thread_cancel.is_set():
                log.debug("XMLRPC Manager: cancel event")
                break

    def _run_server(self) -> None:
        log.debug("XMLRPC Manager: starting server process")
        with multiprocessing.Manager() as manager:
            cmd_queue: AppsXmlRpcServer.CommandQueue = manager.Queue()
            rsp_queue: AppsXmlRpcServer.ResponseQueue = manager.Queue()
            inner_cancel = manager.Event()

            mp_wrapper_name: Path | None = None
            process: AppsXmlRpcServer | None = None
            try:
                if sys.platform == "linux" and self._net_ns is not None:
                    # On Linux we have a separate network namespace for the RPC server. By
                    # setting multiprocessing context executable to a small wrapper, we can
                    # drop only the XMLRPC server to this namespace, leaving the main
                    # process without any restrictions. Mind that this requires having a
                    # "spawn" mp context to work properly.
                    with tempfile.NamedTemporaryFile("w", encoding="utf8", delete=False, delete_on_close=False) as wrapper:
                        mp_wrapper_name = Path(wrapper.name)
                        wrapper.write(f'#!/bin/sh\nexec ip netns exec {self._net_ns} python3 "$@"')
                    mp_wrapper_name.chmod(mp_wrapper_name.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
                    _ctx_rpc.set_executable(str(mp_wrapper_name))

                process = AppsXmlRpcServer(cmd_queue, rsp_queue, inner_cancel)
                process.start()
                log.debug("XMLRPC manager: process started")

                self._run_server_loop(cmd_queue, rsp_queue)
            finally:
                if mp_wrapper_name is not None:
                    mp_wrapper_name.unlink()

                inner_cancel.set()
                if process is not None:
                    process.join()

    def init(self):
        if self._server_thread.is_alive():
            log.debug("XMLRPC server is already running.")
            return

        log.debug("AppsRegister: Starting XMLRPC Manager")
        self._server_thread.start()
        log.debug("AppsRegister: XMLRPC Manager started")

    def uninit(self):
        log.debug("AppsRegister: Stopping XMLRPC Manager")
        self._server_thread_cancel.set()
        self._server_thread.join()
        log.debug("AppsRegister: XMLRPC Manager stopped")

    def terminate(self):
        self.uninit()

    @property
    @with_accessories_lock
    def accessories(self):
        """List of registered accessory applications."""
        return self._accessories.values()

    @with_accessories_lock
    def add(self, name: str, accessory: App):
        self._accessories[name] = accessory

    @with_accessories_lock
    def remove(self, name: str):
        self._accessories.pop(name)

    @with_accessories_lock
    def remove_all(self):
        self._accessories.clear()

    @with_accessories_lock
    def get(self, name: str):
        return self._accessories[name]

    @with_accessories_lock
    def kill(self, name: str):
        if accessory := self._accessories[name]:
            accessory.kill()

    @with_accessories_lock
    def kill_all(self):
        # Make sure to do kill() on all of our apps, even if some of them returned False
        results = [accessory.kill() for accessory in self._accessories.values()]
        return all(results)

    @with_accessories_lock
    def start(self, name: str, args: list[str]) -> bool:
        if accessory := self._accessories[name]:
            # The args param comes directly from the sys.argv[2:] of Start.py and should contain a list of strings in
            # key-value pair, e.g. [option1, value1, option2, value2, ...]
            return accessory.start(self._create_command_line_options(args))
        return False

    @with_accessories_lock
    def stop(self, name: str) -> bool:
        if accessory := self._accessories[name]:
            return accessory.stop()
        return False

    @with_accessories_lock
    def reboot(self, name: str) -> bool:
        if accessory := self._accessories[name]:
            return accessory.stop() and accessory.start()
        return False

    @with_accessories_lock
    def factory_reset_all(self):
        for accessory in self._accessories.values():
            accessory.factoryReset()

    @with_accessories_lock
    def factory_reset(self, name: str) -> bool:
        if accessory := self._accessories[name]:
            return accessory.factoryReset()
        return False

    @with_accessories_lock
    def wait_for_message(self, name: str, message: list[str], timeoutInSeconds: float = 10) -> bool:
        if accessory := self._accessories[name]:
            # The message param comes directly from the sys.argv[2:] of WaitForMessage.py and should contain a list of strings that
            # comprise the entire message to wait for
            return accessory.waitForMessage(' '.join(message), timeoutInSeconds)
        return False

    def create_ota_image(self, otaImageFilePath: str, rawImageFilePath: str, rawImageContent: str,
                         vid: str = '0xDEAD', pid: str = '0xBEEF') -> bool:
        # Write the raw image content
        with open(rawImageFilePath, 'w') as rawFile:
            rawFile.write(rawImageContent)

        # Add an OTA header to the raw file
        otaImageTool = _DEFAULT_CHIP_ROOT / 'src/app/ota_image_tool.py'
        cmd = [str(otaImageTool), 'create', '-v', vid, '-p', pid, '-vn', '2',
               '-vs', "2.0", '-da', 'sha256', rawImageFilePath, otaImageFilePath]
        s = subprocess.Popen(cmd)
        s.wait()
        if s.returncode != 0:
            raise Exception('Cannot create OTA image file')
        return True

    def compare_files(self, file1: str | Path, file2: str | Path) -> bool:
        if not filecmp.cmp(file1, file2, shallow=False):
            raise Exception('Files %s and %s do not match' % (file1, file2))
        return True

    def create_file(self, filePath: str | Path, fileContent: str) -> bool:
        Path(filePath).write_text(fileContent)
        return True

    def delete_file(self, filePath: str | Path) -> bool:
        Path(filePath).unlink(missing_ok=True)
        return True

    @staticmethod
    def _create_command_line_options(args: list[str]) -> dict[str, str]:
        try:
            # Create a dictionary from the key-value pair list
            return dict(zip(args[::2], args[1::2], strict=True))
        except ValueError:
            # args should contain a list of strings in key-value pair, e.g. [option1, value1, option2, value2, ...]
            log.warning("Unexpected command line options %r - not key/value pairs (odd length)", args)
            return {}


APPS_RPC_FUNCS = [func.__name__ for func in [AppsRegister.start, AppsRegister.stop, AppsRegister.reboot, AppsRegister.factory_reset,
                                             AppsRegister.wait_for_message, AppsRegister.compare_files, AppsRegister.create_ota_image,
                                             AppsRegister.create_file, AppsRegister.delete_file]]


class AppsXmlRpcServer(BaseProc):
    CommandQueue = queue.Queue[tuple[str, tuple[Any, ...]]]
    ResponseQueue = queue.Queue[bool | Exception]

    def __init__(self, cmd_queue: CommandQueue, rsp_queue: ResponseQueue, cancel_event: threading.Event) -> None:
        super().__init__()
        self._cmd_queue = cmd_queue
        self._rsp_queue = rsp_queue
        self._cancel_event = cancel_event

    def run(self):
        with contextlib.suppress(KeyboardInterrupt):
            self.start_server()
            self._cancel_event.wait()
        self.stop_server()

    def start_server(self) -> None:
        log.debug("XMLRPC server process: initializing")
        self._server = SimpleXMLRPCServer((IP, PORT))

        def call(name: str, *args: Any) -> bool:
            self._cmd_queue.put((name, args))
            if isinstance(rsp := self._rsp_queue.get(), Exception):
                raise rsp
            return rsp

        def create_func(name: str):
            return lambda *args: call(name, *args)

        def to_camel_case(name: str):
            s = name.split("_")
            return s[0] + "".join(word.capitalize() for word in s[1:])

        for func in APPS_RPC_FUNCS:
            self._server.register_function(create_func(func), to_camel_case(func))

        self._server_thread = threading.Thread(target=self._server.serve_forever)
        self._server_thread.start()
        log.debug("XMLRPC server process: started")

    def stop_server(self):
        log.debug("XMLRPC server process: stopping")
        self._server.shutdown()
