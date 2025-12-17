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

import filecmp
import functools
import logging
import subprocess
import sys
import threading
from pathlib import Path
from typing import Callable, Concatenate, ParamSpec, TypeVar
from xmlrpc.server import SimpleXMLRPCServer

log = logging.getLogger(__name__)

_DEFAULT_CHIP_ROOT = Path(__file__).parent.parent.parent.parent.absolute()

IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'


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
    def __init__(self) -> None:
        self._accessories = {}
        self._accessories_lock = threading.RLock()

    def init(self):
        self._start_xmlrpc_server()

    def uninit(self):
        self._stop_xmlrpc_server()

    @property
    @with_accessories_lock
    def accessories(self):
        """List of registered accessory applications."""
        return self._accessories.values()

    @with_accessories_lock
    def add(self, name, accessory):
        self._accessories[name] = accessory

    @with_accessories_lock
    def remove(self, name):
        self._accessories.pop(name)

    @with_accessories_lock
    def remove_all(self):
        self._accessories.clear()

    @with_accessories_lock
    def get(self, name):
        return self._accessories[name]

    @with_accessories_lock
    def kill(self, name):
        if accessory := self._accessories[name]:
            return accessory.kill()
        return False

    @with_accessories_lock
    def kill_all(self):
        # Make sure to do kill() on all of our apps, even if some of them returned False
        results = [accessory.kill() for accessory in self._accessories.values()]
        return all(results)

    @with_accessories_lock
    def start(self, name, args):
        if accessory := self._accessories[name]:
            # The args param comes directly from the sys.argv[2:] of Start.py and should contain a list of strings in
            # key-value pair, e.g. [option1, value1, option2, value2, ...]
            return accessory.start(self._create_command_line_options(args))
        return False

    @with_accessories_lock
    def stop(self, name):
        if accessory := self._accessories[name]:
            return accessory.stop()
        return False

    @with_accessories_lock
    def reboot(self, name):
        if accessory := self._accessories[name]:
            return accessory.stop() and accessory.start()
        return False

    @with_accessories_lock
    def factory_reset_all(self):
        for accessory in self._accessories.values():
            accessory.factoryReset()

    @with_accessories_lock
    def factory_reset(self, name):
        if accessory := self._accessories[name]:
            return accessory.factoryReset()
        return False

    @with_accessories_lock
    def wait_for_message(self, name, message, timeoutInSeconds=10):
        if accessory := self._accessories[name]:
            # The message param comes directly from the sys.argv[2:] of WaitForMessage.py and should contain a list of strings that
            # comprise the entire message to wait for
            return accessory.waitForMessage(' '.join(message), timeoutInSeconds)
        return False

    def create_ota_image(self, otaImageFilePath, rawImageFilePath, rawImageContent, vid='0xDEAD', pid='0xBEEF'):
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

    def compare_files(self, file1, file2):
        if not filecmp.cmp(file1, file2, shallow=False):
            raise RuntimeError(f'Files {file1} and {file2} do not match')
        return True

    def create_file(self, filePath, fileContent):
        Path(filePath).write_text(fileContent)
        return True

    def delete_file(self, filePath):
        Path(filePath).unlink(missing_ok=True)
        return True

    def _start_xmlrpc_server(self):
        self.server = SimpleXMLRPCServer((IP, PORT))

        self.server.register_function(self.start, 'start')
        self.server.register_function(self.stop, 'stop')
        self.server.register_function(self.reboot, 'reboot')
        self.server.register_function(self.factory_reset, 'factoryReset')
        self.server.register_function(self.wait_for_message, 'waitForMessage')
        self.server.register_function(self.compare_files, 'compareFiles')
        self.server.register_function(self.create_ota_image, 'createOtaImage')
        self.server.register_function(self.create_file, 'createFile')
        self.server.register_function(self.delete_file, 'deleteFile')

        self.server_thread = threading.Thread(target=self.server.serve_forever)
        self.server_thread.start()

    def _stop_xmlrpc_server(self):
        self.server.shutdown()

    @staticmethod
    def _create_command_line_options(args):
        try:
            # Create a dictionary from the key-value pair list
            return dict(zip(args[::2], args[1::2], strict=True))
        except ValueError:
            # args should contain a list of strings in key-value pair, e.g. [option1, value1, option2, value2, ...]
            log.warning("Unexpected command line options %r - not key/value pairs (odd length)", args)
            return {}
