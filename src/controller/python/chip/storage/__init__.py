#
#    Copyright (c) 2021 Project CHIP Authors
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

# Needed to use types in type hints before they are fully defined.
from __future__ import annotations

import ctypes
from dataclasses import dataclass, field
from typing import *
from ctypes import *
from rich.pretty import pprint
import json
import logging
import base64
import chip.exceptions
import copy
import chip.native
import builtins

_SyncSetKeyValueCbFunct = CFUNCTYPE(
    None, py_object, c_char_p, POINTER(c_char),  c_uint16)
_SyncGetKeyValueCbFunct = CFUNCTYPE(
    None, py_object, c_char_p, POINTER(c_char), POINTER(c_uint16), POINTER(c_bool))
_SyncDeleteKeyValueCbFunct = CFUNCTYPE(None, py_object, c_char_p)


@_SyncSetKeyValueCbFunct
def _OnSyncSetKeyValueCb(storageObj, key: str, value, size):
    storageObj.SetSdkKey(key.decode("utf-8"), ctypes.string_at(value, size))


@_SyncGetKeyValueCbFunct
def _OnSyncGetKeyValueCb(storageObj, key: str, value, size, is_found):
    ''' This does not adhere to the API requirements of
    PersistentStorageDelegate::SyncGetKeyValue, but that is okay since
    the C++ storage binding layer is capable of adapting results from
    this method to the requirements of
    PersistentStorageDelegate::SyncGetKeyValue.
    '''
    keyValue = storageObj.GetSdkKey(key.decode("utf-8"))
    if (keyValue is not None):
        sizeOfValue = size[0]
        sizeToCopy = min(sizeOfValue, len(keyValue))

        count = 0

        for idx, val in enumerate(keyValue):
            if sizeToCopy == count:
                break
            value[idx] = val
            count = count + 1

        # As mentioned above, we are intentionally not returning
        # sizeToCopy as one might expect because the caller
        # will use the value in size[0] to determine if it should
        # return CHIP_ERROR_BUFFER_TOO_SMALL.
        size[0] = len(keyValue)
        is_found[0] = True
    else:
        is_found[0] = False
        size[0] = 0


@_SyncDeleteKeyValueCbFunct
def _OnSyncDeleteKeyValueCb(storageObj, key):
    storageObj.DeleteSdkKey(key.decode("utf-8"))


class PersistentStorage:
    ''' Class that provided persistent storage to back both native Python and
        SDK configuration key/value pairs.

        Configuration native to the Python libraries is organized under the top-level
        'repl-config' key while configuration native to the SDK and owned by the various
        C++ logic is organized under the top-level 'sdk-config' key.

        This interfaces with a C++ adapter that implements the PersistentStorageDelegate interface
        and can be passed into C++ logic that needs an instance of that interface.

        Object must be resident before the Matter stack starts up and last past its shutdown.
    '''
    @classmethod
    def logger(cls):
        return logging.getLogger('PersistentStorage')

    def __init__(self, path: str = None, jsonData: Dict = None):
        ''' Initializes the object with either a path to a JSON file that contains the configuration OR
            a JSON dictionary that contains an in-memory representation of the configuration.

            In either case, if there are no valid configurations that already exist, empty Python
            and SDK configuration records will be created upon construction.
        '''
        if (path is None and jsonData is None):
            raise ValueError("Need to provide at least one of path or jsonData")

        if (path is not None and jsonData is not None):
            raise ValueError("Can't provide both a valid path and jsonData")

        if (path is not None):
            self.logger().warn(f"Initializing persistent storage from file: {path}")
        else:
            self.logger().warn(f"Initializing persistent storage from dict")

        self._handle = chip.native.GetLibraryHandle()
        self._isActive = True
        self._path = path

        if (self._path):
            try:
                self._file = open(path, 'r')
                self._file.seek(0, 2)
                size = self._file.tell()
                self._file.seek(0)

                if (size != 0):
                    self.logger().warn(f"Loading configuration from {path}...")
                    self._jsonData = json.load(self._file)
                else:
                    self._jsonData = {}

            except Exception as ex:
                logging.error(ex)
                logging.critical(f"Could not load configuration from {path} - resetting configuration...")
                self._jsonData = {}
        else:
            self._jsonData = jsonData

        if ('sdk-config' not in self._jsonData):
            logging.warn(f"No valid SDK configuration present - clearing out configuration")
            self._jsonData['sdk-config'] = {}

        if ('repl-config' not in self._jsonData):
            logging.warn(f"No valid REPL configuration present - clearing out configuration")
            self._jsonData['repl-config'] = {}

        # Clear out the file so that calling 'Commit' will re-open the file at that time in write mode.
        self._file = None

        self._handle.pychip_Storage_InitializeStorageAdapter.restype = c_void_p
        self._handle.pychip_Storage_InitializeStorageAdapter.argtypes = [ctypes.py_object,
                                                                         _SyncSetKeyValueCbFunct, _SyncGetKeyValueCbFunct, _SyncDeleteKeyValueCbFunct]

        self._closure = self._handle.pychip_Storage_InitializeStorageAdapter(ctypes.py_object(
            self), _OnSyncSetKeyValueCb, _OnSyncGetKeyValueCb, _OnSyncDeleteKeyValueCb)

    def GetSdkStorageObject(self):
        ''' Returns a ctypes c_void_p reference to the SDK-side adapter instance.
        '''
        return self._closure

    def Commit(self):
        ''' Commits the cached JSON configuration to file (if one was provided in the constructor).
            Otherwise, this is a no-op.
        '''
        self.logger().info("Committing...")

        if (self._path is None):
            return

        if (self._file is None):
            try:
                self._file = open(self._path, 'w')
            except Exception as ex:
                logging.warn(
                    f"Could not open {self._path} for writing configuration. Error:")
                logging.warn(ex)
                return

        self._file.seek(0)
        json.dump(self._jsonData, self._file, ensure_ascii=True, indent=4)
        self._file.truncate()
        self._file.flush()

    def SetReplKey(self, key: str, value):
        ''' Set a REPL key to a specific value. Creates the key if one doesn't exist already.
        '''
        self.logger().info(f"SetReplKey: {key} = {value}")

        if (key is None or key == ''):
            raise ValueError("Invalid Key")

        if (value is None):
            del(self._jsonData['repl-config'][key])
        else:
            self._jsonData['repl-config'][key] = value

        self.Commit()

    def GetReplKey(self, key: str):
        ''' Retrieves the value of a REPL key. Returns 'None' if the key
            doesn't exist.
        '''
        if (key not in self._jsonData['repl-config']):
            return None

        return copy.deepcopy(self._jsonData['repl-config'][key])

    def SetSdkKey(self, key: str, value: bytes):
        ''' Set an SDK key to a specific value. Creates the key if one doesn't exist already.
        '''
        self.logger().info(f"SetSdkKey: {key} = {value}")

        if (key is None or key == ''):
            raise ValueError("Invalid Key")

        if (value is None):
            raise ValueError('value is not expected to be None')
        else:
            self._jsonData['sdk-config'][key] = base64.b64encode(
                value).decode("utf-8")

        self.Commit()

    def GetSdkKey(self, key: str):
        ''' Returns the SDK key if one exist. Otherwise, returns 'None'.
        '''
        if (key not in self._jsonData['sdk-config']):
            return None

        return base64.b64decode(self._jsonData['sdk-config'][key])

    def DeleteSdkKey(self, key: str):
        ''' Deletes an SDK key if one exists.
        '''
        self.logger().info(f"DeleteSdkKey: {key}")

        del(self._jsonData['sdk-config'][key])
        self.Commit()

    def Shutdown(self):
        ''' Shuts down the object by free'ing up the associated adapter instance.
            You cannot interact with this object there-after.

            This should only be called after the CHIP stack has shutdown (i.e
            after calling pychip_DeviceController_StackShutdown()).
        '''
        if (self._isActive):
            self._handle.pychip_Storage_ShutdownAdapter.argtypes = [c_void_p]

            #
            # Since the stack is not running at this point, we can safely call
            # C++ method directly on the current execution context without worrying
            # about race conditions.
            #
            self._handle.pychip_Storage_ShutdownAdapter(self._closure)
            self._isActive = False

    @property
    def jsonData(self) -> Dict:
        ''' Returns a copy of the internal cached JSON data.
        '''
        return copy.deepcopy(self._jsonData)

    def __del__(self):
        self.Shutdown()
