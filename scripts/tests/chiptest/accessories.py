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
import logging
import os
import subprocess
import sys
import threading
from xmlrpc.server import SimpleXMLRPCServer

log = logging.getLogger(__name__)

_DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'


class AppsRegister:
    def __init__(self) -> None:
        self._accessories = {}

    def init(self):
        self._start_xmlrpc_server()

    def uninit(self):
        self._stop_xmlrpc_server()

    @property
    def accessories(self):
        """List of registered accessory applications."""
        return self._accessories.values()

    def add(self, name, accessory):
        self._accessories[name] = accessory

    def remove(self, name):
        self._accessories.pop(name)

    def remove_all(self):
        self._accessories = {}

    def get(self, name):
        return self._accessories[name]

    def kill(self, name):
        accessory = self._accessories[name]
        if accessory:
            accessory.kill()

    def kill_all(self):
        ok = True
        for accessory in self._accessories.values():
            # make sure to do kill() on all of our apps, even if some of them returned False
            ok = accessory.kill() and ok
        return ok

    def start(self, name, args):
        accessory = self._accessories[name]
        if accessory:
            # The args param comes directly from the sys.argv[2:] of Start.py and should contain a list of strings in
            # key-value pair, e.g. [option1, value1, option2, value2, ...]
            options = self._create_command_line_options(args)
            return accessory.start(options)
        return False

    def stop(self, name):
        accessory = self._accessories[name]
        if accessory:
            return accessory.stop()
        return False

    def reboot(self, name):
        accessory = self._accessories[name]
        if accessory:
            return accessory.stop() and accessory.start()
        return False

    def factory_reset_all(self):
        for accessory in self._accessories.values():
            accessory.factoryReset()

    def factory_reset(self, name):
        accessory = self._accessories[name]
        if accessory:
            return accessory.factoryReset()
        return False

    def wait_for_message(self, name, message, timeoutInSeconds=10):
        accessory = self._accessories[name]
        if accessory:
            # The message param comes directly from the sys.argv[2:] of WaitForMessage.py and should contain a list of strings that
            # comprise the entire message to wait for
            return accessory.waitForMessage(' '.join(message), timeoutInSeconds)
        return False

    def create_ota_image(self, otaImageFilePath, rawImageFilePath, rawImageContent, vid='0xDEAD', pid='0xBEEF'):
        # Write the raw image content
        with open(rawImageFilePath, 'w') as rawFile:
            rawFile.write(rawImageContent)

        # Add an OTA header to the raw file
        otaImageTool = _DEFAULT_CHIP_ROOT + '/src/app/ota_image_tool.py'
        cmd = [otaImageTool, 'create', '-v', vid, '-p', pid, '-vn', '2',
               '-vs', "2.0", '-da', 'sha256', rawImageFilePath, otaImageFilePath]
        s = subprocess.Popen(cmd)
        s.wait()
        if s.returncode != 0:
            raise Exception('Cannot create OTA image file')
        return True

    def compare_files(self, file1, file2):
        if filecmp.cmp(file1, file2, shallow=False) is False:
            raise Exception('Files %s and %s do not match' % (file1, file2))
        return True

    def create_file(self, filePath, fileContent):
        with open(filePath, 'w') as rawFile:
            rawFile.write(fileContent)
        return True

    def delete_file(self, filePath):
        if os.path.exists(filePath):
            os.remove(filePath)
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

    def _create_command_line_options(self, args):
        if not args:
            return {}

        # args should contain a list of strings in key-value pair, e.g. [option1, value1, option2, value2, ...]
        if (len(args) % 2) != 0:
            log.warning("Unexpected command line options %r - not key/value pairs (odd length)", args)
            return {}

        # Create a dictionary from the key-value pair list
        return {args[i]: args[i+1] for i in range(0, len(args), 2)}
