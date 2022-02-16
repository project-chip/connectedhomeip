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

import logging
import time
import threading
import sys
from random import randrange
from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.client import ServerProxy

IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'


class AppsRegister:
    _instance = None
    __accessories = {}

    def init(self):
        self.__startXMLRPCServer()

    def uninit(self):
        self.__stopXMLRPCServer()

    def add(self, name, accessory):
        self.__accessories[name] = accessory

    def remove(self, name):
        self.__accessories.pop(name)

    def removeAll(self):
        self.__accessories = {}

    def poll(self):
        for accessory in self.__accessories.values():
            status = accessory.poll()
            if status is not None:
                return status
        return None

    def kill(self, name):
        accessory = self.__accessories[name]
        if accessory:
            accessory.kill()

    def killAll(self):
        for accessory in self.__accessories.values():
            accessory.kill()

    def start(self, name, discriminator):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.start(discriminator)
        return False

    def stop(self, name):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.stop()
        return False

    def reboot(self, name, discriminator):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.stop() and accessory.start(discriminator)
        return False

    def factoryResetAll(self):
        for accessory in self.__accessories.values():
            accessory.factoryReset()

    def factoryReset(self, name):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.factoryReset()
        return False

    def waitForCommissionableAdvertisement(self, name):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.waitForCommissionableAdvertisement()
        return False

    def waitForOperationalAdvertisement(self, name):
        accessory = self.__accessories[name]
        if accessory:
            return accessory.waitForOperationalAdvertisement()
        return False

    def ping(self):
        return True

    def __startXMLRPCServer(self):
        self.server = SimpleXMLRPCServer((IP, PORT))

        self.server.register_function(self.start, 'start')
        self.server.register_function(self.stop, 'stop')
        self.server.register_function(self.reboot, 'reboot')
        self.server.register_function(self.factoryReset, 'factoryReset')
        self.server.register_function(
            self.waitForCommissionableAdvertisement, 'waitForCommissionableAdvertisement')
        self.server.register_function(
            self.waitForOperationalAdvertisement, 'waitForOperationalAdvertisement')
        self.server.register_function(self.ping, 'ping')

        self.server_thread = threading.Thread(target=self.__handle_request)
        self.server_thread.start()

    def __handle_request(self):
        self.__should_handle_requests = True
        while self.__should_handle_requests:
            self.server.handle_request()

    def __stopXMLRPCServer(self):
        self.__should_handle_requests = False
        # handle_request will wait until it receives a message, so let's send a ping to the server
        client = ServerProxy('http://' + IP + ':' +
                             str(PORT) + '/', allow_none=True)
        client.ping()
