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

import dataclasses
import filecmp
import functools
import logging
import multiprocessing
import subprocess
import sys
import threading
from pathlib import Path
from typing import TYPE_CHECKING, Any, Callable, Concatenate, ParamSpec, TypeAlias, TypeVar
from xmlrpc.server import SimpleXMLRPCServer

from chiptest.concurrency.context import StartStopContextMixin, mp_wrapped_spawn_context
from chiptest.concurrency.process import ProcessConfig, WrappedProcess, with_annotated_exception
from chiptest.concurrency.work_queue import CancellableQueue, QueueCancelled
from chiptest.log_config import LogConfig

if TYPE_CHECKING:
    from .test_definition import App

log = logging.getLogger(__name__)

_DEFAULT_CHIP_ROOT = Path(__file__).parent.parent.parent.parent.absolute()

PORT = 9000
if sys.platform == 'linux':
    IP = '10.10.10.5'
else:
    IP = '127.0.0.1'


@dataclasses.dataclass(frozen=True)
class XmlRpcFuncCall:
    """Represents a call to an XML-RPC function, with the function name and arguments."""
    name: str
    args: tuple[Any, ...]


XmlRpcFuncRet: TypeAlias = bool | Exception


class XmlRpcServerProcess(WrappedProcess[XmlRpcFuncCall, XmlRpcFuncRet], StartStopContextMixin):
    """
    Wrapped subprocess that hosts the XML-RPC endpoint for `AppsRegister`.

    This class provides the XML-RPC-facing side of the accessory control flow:
    - `_proc_init()` creates `SimpleXMLRPCServer`, registers RPC methods from `APPS_RPC_FUNCS` (snake_case converted to camelCase),
      and runs the server loop in a dedicated thread.
    - Incoming RPC calls are converted into `XmlRpcFuncCall` requests and pushed into `self._work_queue`.
    - The manager thread executes the requested `AppsRegister` method and sends either a `bool` result or an `Exception` through
      `self._rsp_queue`.
    - `_call()` reads that response and re-raises exceptions so XML-RPC receives the original failure.
    """
    _server_manager: SimpleXMLRPCServer
    _server_thread_manager: threading.Thread

    def _call(self, name: str, *args: Any) -> bool:
        log.debug("Call: %s%r", name, args)
        self._work_queue.put(XmlRpcFuncCall(name, args))

        # Get the result from the response queue. If there was an exception, propagate it to the RPC server process, so that it can
        # be handled by it, and in turn propagated back to the caller of the RPC function.
        rsp = self._rsp_queue.get()
        log.debug("Response for %s: %r", name, rsp)
        if isinstance(rsp, Exception):
            raise rsp
        return rsp

    def _create_func(self, name: str):
        return lambda *args: self._call(name, *args)

    @staticmethod
    def _to_camel_case(name: str):
        s = name.split("_")
        return s[0] + "".join(word.capitalize() for word in s[1:])

    def _proc_init(self) -> None:
        # Create the XMLRPC server and register functions.
        self._server_manager = SimpleXMLRPCServer((IP, PORT))
        for func in APPS_RPC_FUNCS:
            self._server_manager.register_function(self._create_func(func), self._to_camel_case(func))

        # Set the log level for the process with the dedicated log level for RPC.
        self._config.log_config.set_fmt(level=self._config.log_config.level_rpc)

        # Run the server loop in a separate thread.
        self._server_thread_manager = threading.Thread(target=self._server_manager.serve_forever, name="Server")
        self._server_thread_manager.start()

    def _proc_cleanup(self):
        if hasattr(self, '_server_manager'):
            log.debug("Stopping XMLRPC Server thread")
            self._server_manager.shutdown()

        if hasattr(self, '_server_thread_manager'):
            log.debug("Waiting for XMLRPC Server thread to stop")
            self._server_thread_manager.join(timeout=self._config.stop_timeout_sec)
            if self._server_thread_manager.is_alive():
                log.error("XMLRPC Server thread failed to stop")
            log.debug("XMLRPC Server thread stopped")


class XmlRpcServerProcessManager(threading.Thread):
    """
    Thread used to orchestrate the XML-RPC subprocess and dispatch loop.

    The manager bridges three domains:
    - Parent test code that starts/stops XML-RPC support.
    - `XmlRpcServerProcess` (run in network namespace on Linux), which exposes XML-RPC and forwards calls as queue requests.
    - `AppsRegister`, where requested operations are executed.

    Lifecycle behavior:
    - `start()` launches the manager thread and waits for initialization (`_init_done`) or failure (`_exception`).
    - `run()` creates the wrapped spawn context, starts `XmlRpcServerProcess`, then consumes requests and executes validated
      `AppsRegister` methods.
    - `stop()` signals shutdown by cancelling the work and response queues, joins the thread, and always shuts down the
      multiprocessing manager.

    Return values and exceptions from `AppsRegister` execution are written to the response queue so the XML-RPC subprocess can
    propagate them back to the RPC caller.
    """

    def __init__(self, apps: AppsRegister, net_ns_wrapper: str | None, log_config: LogConfig) -> None:
        super().__init__(name="XmlRpcMgr", daemon=True)

        self.apps = apps
        self.net_ns_wrapper = net_ns_wrapper
        self.log_config = log_config

        self._init_done = threading.Event()
        self._exception: BaseException | None = None
        self._stopped = False

        self._mp_manager = multiprocessing.Manager()
        self._proc_config = ProcessConfig(name="XmlRpcProc", log_config=log_config).with_formatted_name()
        self._work_queue: CancellableQueue[XmlRpcFuncCall] = CancellableQueue(self._mp_manager)
        self._rsp_queue: CancellableQueue[XmlRpcFuncRet] = CancellableQueue(self._mp_manager)

    @with_annotated_exception
    def start(self) -> None:
        if self._stopped:
            raise RuntimeError("Cannot start XMLRPC Manager after it has been stopped")

        super().start()

        try:
            if not self._init_done.wait(timeout=self._proc_config.start_timeout_sec):
                raise TimeoutError("Failed to start within timeout")

            if self._exception is not None:
                raise RuntimeError("XMLRPC Manager initialization failed") from self._exception
        except BaseException as start_exc:
            log.error("Stopping XMLRPC Manager after init error: %r", start_exc)
            self.stop(raise_on_proc_error=False)
            raise

    @with_annotated_exception
    def stop(self, raise_on_proc_error: bool = True) -> None:
        if self._stopped:
            log.debug("XMLRPC Manager is already stopped")
            return

        try:
            # Cancel all queues to signal the subprocess and the manager thread to stop.
            self._work_queue.cancel()
            self._rsp_queue.cancel()

            # Stopping the queues should result in the subprocess exiting, and in turn the manager thread.
            self.join(timeout=self._proc_config.stop_timeout_sec)

            # Propagate the exception risen in the thread.
            if raise_on_proc_error and self._exception is not None:
                raise RuntimeError("XMLRPC Manager failed") from self._exception

            if self.is_alive():
                raise TimeoutError("XMLRPC Manager failed to stop within timeout")
        except BaseException as e:
            log.error("Error when stopping XMLRPC Manager: %r", e)
            raise
        finally:
            self._mp_manager.shutdown()
            self._stopped = True

    def run(self) -> None:
        try:
            with (mp_wrapped_spawn_context(wrapper_linux=self.net_ns_wrapper) as ctx,
                  XmlRpcServerProcess(ctx, self._mp_manager, self._proc_config, self._work_queue, self._rsp_queue) as server):
                # Process is already successfully started when the context is entered.
                self._init_done.set()

                while True:
                    try:
                        server._rsp_queue.put(self._proc_work(server._work_queue.get()))
                    except QueueCancelled:
                        log.debug("Stopping on a cancel event")
                        break
        except Exception as e:
            self._exception = e
            self._init_done.set()

    def _proc_work(self, func_call: XmlRpcFuncCall) -> XmlRpcFuncRet:
        try:
            # Check if the function exists and is registered as an RPC function.
            if (func := getattr(self.apps, func_call.name, None)) is None or not callable(func):
                raise RuntimeError(f'Function "{func_call.name}" does not exist in AppsRegister')
            if func.__name__ not in APPS_RPC_FUNCS:
                raise RuntimeError(f'Function "{func_call.name}" is not registered as an RPC function')

            # Execute the function with given arguments and propagate the return value.
            if not isinstance(ret := func(*func_call.args), bool):
                raise RuntimeError(f'Function "{func_call.name}" returned invalid type {type(ret)}')
            return ret
        except Exception as e:
            # Pass the exception to the XMLRPC server process. The server process will handle it and return it to the caller, so
            # that the caller can also handle it.
            return e


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
    def __init__(self, net_ns_wrapper: str | None = None, log_config: LogConfig | None = None) -> None:
        self._accessories: dict[str, App] = {}
        self._accessories_lock = threading.RLock()

        self._net_ns_wrapper = net_ns_wrapper
        self._log_config = log_config if log_config is not None else LogConfig()
        self._server_manager: XmlRpcServerProcessManager | None = None

    def init(self) -> None:
        if self._server_manager is None:
            self._server_manager = XmlRpcServerProcessManager(self, self._net_ns_wrapper, self._log_config)

        if self._server_manager.is_alive():
            log.debug("XMLRPC server is already running")
            return

        log.debug("Starting XMLRPC Manager")
        self._server_manager.start()
        log.debug("XMLRPC Manager started")

    def uninit(self) -> None:
        if self._server_manager is None:
            log.debug("XMLRPC server is already down")
            return

        log.debug("Stopping XMLRPC Manager")
        self._server_manager.stop()
        self._server_manager = None
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
        try:
            s.communicate(timeout=60)
        except subprocess.TimeoutExpired:
            s.kill()
            raise RuntimeError('OTA image tool timed out')
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
"""Functions of AppsRegister that are exposed via XMLRPC. The function names will be converted to camelCase for XMLRPC."""
