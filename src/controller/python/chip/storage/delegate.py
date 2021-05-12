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

#
#    @file
#      The file engine for controller persistent storage.
#

import sqlite3
from dataclasses import dataclass, field
from ctypes import CFUNCTYPE, c_char, c_char_p, c_void_p, c_size_t, c_uint16, c_uint32, c_uint64, c_uint8
import ctypes
import chip.native
import base64
import threading
from urllib import parse as urlparse

@dataclass
class StorageOptions:
    path: str = ":memory:"
    params: dict = field(default_factory=dict)

# typedef void (*GetKeyValueFunct)(const uint8_t * key, void * value, uint16_t * size);
# typedef void (*SetKeyValueFunct)(const uint8_t * key, const void * value, uint16_t size);
# typedef void (*DeleteKeyValueFunct)(const uint8_t * key);
_ControllerGetKeyValueFunct = CFUNCTYPE(None, c_char_p, c_size_t, c_void_p, ctypes.POINTER(c_uint16))
_ControllerSetKeyValueFunct = CFUNCTYPE(None, c_char_p, c_size_t, c_void_p, c_uint16)
_ControllerDeleteKeyValueFunct = CFUNCTYPE(None, c_char_p, c_size_t)

_CONTROLLER_STORAGE_DOMAIN = "controller"
_CONTROLLER_STORAGE_FIELD = "default"

class SqlStorage:
    def __init__(self, options: StorageOptions):
        self.lock = threading.Lock()
        self.con = sqlite3.connect(options.path, check_same_thread=False)
        self.cache = dict()
        cur = self.con.cursor()
        cur.executescript('''
        CREATE TABLE IF NOT EXISTS storage (
            domain text NOT NULL,
            field text NOT NULL,
            key text NOT NULL,
            value text NOT NULL
        );
        CREATE UNIQUE INDEX IF NOT EXISTS storage_index ON storage (
            domain, field, key
        );
        ''')
        self.con.commit()

    def Set(self, domain:str, field:str, key:str, value:bytes)->None:
        with self.lock:
            cur = self.con.cursor()
            valueStr = base64.standard_b64encode(value).decode('utf-8')
            cur.execute('INSERT OR REPLACE INTO storage (domain, field, key, value) VALUES (?, ?, ?, ?)', (domain, field, key, valueStr))
            self.con.commit()

    def Get(self, domain:str, field:str, key:str)->bytes:
        with self.lock:
            cur = self.con.cursor()
            cur.execute("SELECT value FROM storage WHERE domain = ? AND field = ? AND key = ? LIMIT 1", (domain, field, key))
            res = cur.fetchone()
            return base64.standard_b64decode(res[0]) if res is not None else None

    def Delete(self, domain:str, field:str, key:str)->None:
        with self.lock:
            cur = self.con.cursor()
            cur.execute("DELETE FROM storage WHERE domain = ? AND field = ? AND key = ?", (domain, field, key))
            self.con.commit()

_controllerStorage = None

@_ControllerGetKeyValueFunct
def ControllerGetKeyValueFunct(key:c_char_p, keyLen:int, valPtr:c_void_p, sizePtr:ctypes.POINTER(c_uint16)):
    global _controllerStorage
    accessKey = ctypes.string_at(key, keyLen).decode("utf-8")
    data = _controllerStorage.Get(_CONTROLLER_STORAGE_DOMAIN, _CONTROLLER_STORAGE_FIELD, accessKey)
    sizeRead = ctypes.pointer(c_uint16(len(data))) if data is not None else ctypes.pointer(c_uint16(0xffff))
    if data is not None and valPtr != 0:
        copySize = min(len(data), sizePtr.contents.value)
        ctypes.memmove(valPtr, ctypes.c_char_p(data), copySize)
    ctypes.memmove(sizePtr, sizeRead, ctypes.sizeof(c_uint16))

@_ControllerSetKeyValueFunct
def ControllerSetKeyValueFunct(key:c_char_p, keyLen: int, valPtr:c_void_p, size:int):
    global _controllerStorage
    accessKey = ctypes.string_at(key, keyLen).decode("utf-8")
    data = ctypes.string_at(valPtr, size)
    _controllerStorage.Set(_CONTROLLER_STORAGE_DOMAIN, _CONTROLLER_STORAGE_FIELD, accessKey, data)

@_ControllerDeleteKeyValueFunct
def ControllerDeleteKeyValueFunct(key:c_char_p, keyLen: int):
    global _controllerStorage
    accessKey = ctypes.string_at(key, keyLen).decode("utf-8")
    _controllerStorage.Delete(_CONTROLLER_STORAGE_DOMAIN, _CONTROLLER_STORAGE_FIELD, accessKey)

def _InitControllerStorageInternal(storageClass, options: StorageOptions):
    global _controllerStorage
    handle = chip.native.GetLibraryHandle()
    _controllerStorage = storageClass(options)
    if not handle.pychip_PythonPersistentStorageDelegate_SetCallbacks.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)
        setter.Set("pychip_PythonPersistentStorageDelegate_SetCallbacks", None, [_ControllerGetKeyValueFunct, _ControllerSetKeyValueFunct, _ControllerDeleteKeyValueFunct])

        handle.pychip_PythonPersistentStorageDelegate_SetCallbacks(ControllerGetKeyValueFunct, ControllerSetKeyValueFunct, ControllerDeleteKeyValueFunct)

def InitControllerStorage(storageUrl: str):
    storageMap = {
        "sqlite3": SqlStorage
    }
    storage = urlparse.urlparse(storageUrl)
    storageClass = storageMap.get(storage.scheme, None)
    if not storageClass:
        raise Exception("Unknown storage: {}".format(storage.scheme))
    options = StorageOptions(path=storage.path, params=urlparse.parse_qs(storage.query))
    _InitControllerStorageInternal(storageClass, options)
