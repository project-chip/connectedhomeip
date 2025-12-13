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

# TODO: Double check the process creation flow.

from __future__ import annotations

import filecmp
import functools
import logging
import multiprocessing
import queue
import subprocess
import sys
import threading
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from pathlib import Path
from typing import TYPE_CHECKING, Any, Callable, Concatenate, ParamSpec, TypeVar
from xmlrpc.server import SimpleXMLRPCServer

from .mp_utils import LogConfig, WrappedMultiprocessingProcessContext, mp_wrapped_spawn_context

if TYPE_CHECKING:
    from .test_definition import App

log = logging.getLogger(__name__)

_DEFAULT_CHIP_ROOT = Path(__file__).parent.parent.parent.parent.absolute()

PORT = 9000
if sys.platform == 'linux':
    IP = '10.10.10.5'
else:
    IP = '127.0.0.1'

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
    def __init__(self, net_ns: str | None = None, log_config: LogConfig = LogConfig()) -> None:
        self._accessories: dict[str, App] = {}
        self._accessories_lock = threading.RLock()

        self._net_ns = net_ns
        self._log_config = log_config

        self._server_thread_cancel = threading.Event()
        self._server_thread_init_done = threading.Event()
        self._server_thread = threading.Thread(target=self._run_server, name="XmlRpcServerManager")

    def _run_server(self) -> None:
        log.debug("Starting server process")
        with (multiprocessing.Manager() as manager,
              mp_wrapped_spawn_context(f"ip netns exec {self._net_ns}") as ctx,
              AppsXmlRpcServer(ctx, manager, self._log_config) as server):
            self._server_thread_init_done.set()
            log.debug("XMLRPC Server process started")

            while not self._server_thread_cancel.is_set():
                try:
                    # Get a command from queue.
                    name, args = server.cmd_queue.get(timeout=1)
                    assert isinstance(name, str) and isinstance(args, tuple), "Wrong command format"

                    # Execute function with arguments and push return value to the response queue.
                    if (func := getattr(self, name, None)) is None or not callable(func):
                        raise RuntimeError(f'Function "{name}" does not exist in AppsRegister')
                    if func.__name__ not in APPS_RPC_FUNCS:
                        raise RuntimeError(f'Function "{name}" is not registered as an RPC function')
                    server.rsp_queue.put(func(*args))  # pyright: ignore[reportArgumentType]
                except queue.Empty:
                    # No command received within timeout. Continue polling.
                    pass
                except Exception as e:
                    # On error, pass the exception to the XMLRPC server process and continue.
                    log.error("Error in XMLRPC Manager: %s", e, exc_info=True)
                    server.rsp_queue.put(e)
            log.debug("Caught a cancel event in XMLRPC Manager")

    def init(self) -> None:
        if self._server_thread.is_alive():
            log.debug("XMLRPC server is already running")
            return

        log.debug("Starting XMLRPC Manager")
        self._server_thread.start()
        self._server_thread_init_done.wait(AppsXmlRpcServer.DEFAULT_START_TIMEOUT)
        log.debug("XMLRPC Manager started")

    def uninit(self) -> None:
        if not self._server_thread.is_alive():
            log.debug("XMLRPC server is already down")
            return

        log.debug("Stopping XMLRPC Manager")
        self._server_thread_cancel.set()
        self._server_thread.join(AppsXmlRpcServer.DEFAULT_STOP_TIMEOUT)
        log.debug("XMLRPC Manager stopped")

    def terminate(self):
        self.uninit()

    @property
    @with_accessories_lock
    def accessories(self):
        """List of registered accessory applications."""
        return self._accessories.values()

    @with_accessories_lock
    def add(self, name: str, accessory: App) -> None:
        self._accessories[name] = accessory

    @with_accessories_lock
    def remove(self, name: str) -> None:
        self._accessories.pop(name)

    @with_accessories_lock
    def remove_all(self) -> None:
        self._accessories.clear()

    @with_accessories_lock
    def get(self, name: str) -> App:
        return self._accessories[name]

    @with_accessories_lock
    def kill(self, name: str) -> bool:
        if accessory := self._accessories[name]:
            return accessory.kill()
        return False

    @with_accessories_lock
    def kill_all(self) -> bool:
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
    def factory_reset_all(self) -> None:
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

    def create_ota_image(self, otaImageFilePath: str, rawImageFilePath: str, rawImageContent: str, vid: str = '0xDEAD',
                         pid: str = '0xBEEF') -> bool:
        # Write the raw image content
        Path(rawImageFilePath).write_text(rawImageContent)

        # Add an OTA header to the raw file
        otaImageTool = _DEFAULT_CHIP_ROOT / 'src/app/ota_image_tool.py'
        cmd = [str(otaImageTool), 'create', '-v', vid, '-p', pid, '-vn', '2',
               '-vs', "2.0", '-da', 'sha256', rawImageFilePath, otaImageFilePath]
        s = subprocess.Popen(cmd)
        # We need to have some timeout so that in case the process hangs we don't wait infinitely in CI. 60 seconds is large enough
        # for the OTA tool.
        s.wait(60)
        if s.returncode != 0:
            raise RuntimeError('Cannot create OTA image file')
        return True

    def compare_files(self, file1: str | Path, file2: str | Path) -> bool:
        if not filecmp.cmp(file1, file2, shallow=False):
            raise RuntimeError(f'Files {file1} and {file2} do not match')
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


APPS_RPC_FUNCS = tuple(func.__name__ for func in (
    AppsRegister.start, AppsRegister.stop, AppsRegister.reboot, AppsRegister.factory_reset, AppsRegister.wait_for_message,
    AppsRegister.compare_files, AppsRegister.create_ota_image, AppsRegister.create_file, AppsRegister.delete_file
))


class AppsXmlRpcServer(WrappedMultiprocessingProcessContext):
    CommandQueue = queue.Queue[tuple[str, tuple[Any, ...]]]
    ResponseQueue = queue.Queue[bool | Exception]

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, log_config: LogConfig) -> None:
        proc_name_short = f"{log_config.process_name}/XMLRPC" if log_config.process_name is not None else "XMLRPC"
        super().__init__(mp_context, mp_manager, "XML RPC Server", proc_name_short, log_config)

        self.cmd_queue: AppsXmlRpcServer.CommandQueue = mp_manager.Queue()
        self.rsp_queue: AppsXmlRpcServer.ResponseQueue = mp_manager.Queue()

        self._server: SimpleXMLRPCServer | None = None
        self._server_thread: threading.Thread | None = None

    def _call(self, name: str, *args: Any) -> bool:
        self.cmd_queue.put((name, args))
        if isinstance(rsp := self.rsp_queue.get(), Exception):
            raise rsp
        return rsp

    def create_func(self, name: str):
        return lambda *args: self._call(name, *args)

    @staticmethod
    def to_camel_case(name: str):
        s = name.split("_")
        return s[0] + "".join(word.capitalize() for word in s[1:])

    def _proc_init(self) -> None:
        self._server = SimpleXMLRPCServer((IP, PORT))
        for func in APPS_RPC_FUNCS:
            self._server.register_function(self.create_func(func), self.to_camel_case(func))

        self._server_thread = threading.Thread(target=self._server.serve_forever)
        self._server_thread.start()

    def _proc_cleanup(self):
        if self._server is not None:
            log.debug("Stopping XMLRPC Server")
            self._server.shutdown()
        if self._server_thread is not None:
            self._server_thread.join()
