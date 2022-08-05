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
    try:
        keyValue = storageObj.GetSdkKey(key.decode("utf-8"))
    except Exception as ex:
        keyValue = None

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

    def __init__(self, path: str):
        self._path = path
        self._handle = chip.native.GetLibraryHandle()
        self._isActive = True

        try:
            self._file = open(path, 'r')
            self._file.seek(0, 2)
            size = self._file.tell()
            self._file.seek(0)

            if (size != 0):
                logging.critical(f"Loading configuration from {path}...")
                self.jsonData = json.load(self._file)
            else:
                logging.warn(
                    f"No valid configuration present at {path} - clearing out configuration")
                self.jsonData = {'repl-config': {}, 'sdk-config': {}}

        except Exception as ex:
            logging.error(ex)
            logging.warn(
                f"Could not load configuration from {path} - resetting configuration...")
            self.jsonData = {'repl-config': {}, 'sdk-config': {}}

        self._file = None
        self._handle.pychip_Storage_InitializeStorageAdapter(ctypes.py_object(
            self), _OnSyncSetKeyValueCb, _OnSyncGetKeyValueCb, _OnSyncDeleteKeyValueCb)

    def Sync(self):
        if (self._file is None):
            try:
                self._file = open(self._path, 'w')
            except Exception as ex:
                logging.warn(
                    f"Could not open {self._path} for writing configuration. Error:")
                logging.warn(ex)
                return

        self._file.seek(0)
        json.dump(self.jsonData, self._file, ensure_ascii=True, indent=4)
        self._file.truncate()
        self._file.flush()

    def SetReplKey(self, key: str, value):
        logging.info(f"SetReplKey: {key} = {value}")

        if (key is None or key == ''):
            raise ValueError("Invalid Key")

        if (value is None):
            del(self.jsonData['repl-config'][key])
        else:
            self.jsonData['repl-config'][key] = value

        self.Sync()

    def GetReplKey(self, key: str):
        return copy.deepcopy(self.jsonData['repl-config'][key])

    def SetSdkKey(self, key: str, value: bytes):
        logging.info(f"SetSdkKey: {key} = {value}")

        if (key is None or key == ''):
            raise ValueError("Invalid Key")

        if (value is None):
            raise ValueError('value is not expected to be None')
        else:
            self.jsonData['sdk-config'][key] = base64.b64encode(
                value).decode("utf-8")

        self.Sync()

    def GetSdkKey(self, key: str):
        return base64.b64decode(self.jsonData['sdk-config'][key])

    def DeleteSdkKey(self, key: str):
        del(self.jsonData['sdk-config'][key])
        self.Sync()

    def GetUnderlyingStorageAdapter(self):
        return self._storageAdapterObj

    def Shutdown(self):
        builtins.chipStack.Call(
            lambda: self._handle.pychip_Storage_ShutdownAdapter()
        )

        self._isActive = False

    def __del__(self):
        if (self._isActive):
            builtins.chipStack.Call(
                lambda: self._handle.pychip_Storage_ShutdownAdapter()
            )
