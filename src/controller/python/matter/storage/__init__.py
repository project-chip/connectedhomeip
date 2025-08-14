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

import base64
import copy
import ctypes
import json
import logging
from configparser import ConfigParser
from ctypes import CFUNCTYPE, POINTER, c_bool, c_char, c_char_p, c_uint16, c_void_p, py_object
from typing import Dict, Optional

from ..native import GetLibraryHandle

LOGGER = logging.getLogger(__name__)

_SetKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p, POINTER(c_char),  c_uint16)
_GetKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p, POINTER(c_char), POINTER(c_uint16), POINTER(c_bool))
_DeleteKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p)


class PersistentStorage:
    ''' Class that provided persistent storage to back both native Python and
        SDK configuration key/value pairs.

        This class does not provide any persistence storage on its own. It keeps
        the configuration in memory. Please use a dedicated subclass to provide non-volatile
        persistence storage, such as PersistentStorageJSON.

        Configuration native to the Python libraries is organized under the top-level
        'repl-config' key while configuration native to the SDK and owned by the various
        C++ logic is organized under the top-level 'sdk-config' key.

        This interfaces with a C++ adapter that implements the PersistentStorageDelegate interface
        and can be passed into C++ logic that needs an instance of that interface.

        Object must be resident before the Matter stack starts up and last past its shutdown.
    '''

    @_SetKeyValueCbFunc
    def _OnSetKeyValueCb(self, key: bytes, value, size):
        self.SetSdkKey(key.decode("utf-8"), ctypes.string_at(value, size))

    @_GetKeyValueCbFunc
    def _OnGetKeyValueCb(self, key: bytes, value, size, is_found):
        ''' This does not adhere to the API requirements of
        PersistentStorageDelegate::SyncGetKeyValue, but that is okay since
        the C++ storage binding layer is capable of adapting results from
        this method to the requirements of
        PersistentStorageDelegate::SyncGetKeyValue.
        '''
        keyValue = self.GetSdkKey(key.decode("utf-8"))
        if keyValue is not None:
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

    @_DeleteKeyValueCbFunc
    def _OnDeleteKeyValueCb(self, key):
        self.DeleteSdkKey(key.decode("utf-8"))

    def __init__(self, data: Dict = {}):
        ''' Initializes the persistent storage with provided data.
        '''
        self._data = copy.deepcopy(data)
        self._handle = GetLibraryHandle()
        self._isActive = True

        if 'sdk-config' not in self._data:
            LOGGER.warning("No valid SDK configuration present")
            self._data['sdk-config'] = {}

        if 'repl-config' not in self._data:
            LOGGER.warning("No valid REPL configuration present")
            self._data['repl-config'] = {}

        self._handle.pychip_Storage_InitializeStorageAdapter.restype = c_void_p
        self._handle.pychip_Storage_InitializeStorageAdapter.argtypes = [ctypes.py_object,
                                                                         _SetKeyValueCbFunc,
                                                                         _GetKeyValueCbFunc,
                                                                         _DeleteKeyValueCbFunc]
        self._closure = self._handle.pychip_Storage_InitializeStorageAdapter(
            ctypes.py_object(self), self._OnSetKeyValueCb, self._OnGetKeyValueCb, self._OnDeleteKeyValueCb)

    def GetSdkStorageObject(self):
        ''' Returns a ctypes c_void_p reference to the SDK-side adapter instance.
        '''
        return self._closure

    def Commit(self):
        ''' Commits the cached configuration.
        '''
        pass

    def SetReplKey(self, key: str, value):
        ''' Set a REPL key to a specific value. Creates the key if one doesn't exist already.
        '''
        LOGGER.debug("SetReplKey: %s = %s", key, value)

        if not key:
            raise ValueError("Invalid Key")

        if value is None:
            self._data['repl-config'].pop(key, None)
        else:
            self._data['repl-config'][key] = value

        self.Commit()

    def GetReplKey(self, key: str):
        ''' Retrieves the value of a REPL key. Returns 'None' if the key
            doesn't exist.
        '''
        return copy.deepcopy(self._data['repl-config'].get(key, None))

    def SetSdkKey(self, key: str, value: bytes):
        ''' Set an SDK key to a specific value. Creates the key if one doesn't exist already.
        '''
        LOGGER.debug("SetSdkKey: %s = %s", key, value)

        if not key:
            raise ValueError("Invalid Key")
        if value is None:
            raise ValueError('Value is not expected to be None')

        self._data['sdk-config'][key] = base64.b64encode(value).decode("utf-8")
        self.Commit()

    def GetSdkKey(self, key: str):
        ''' Returns the SDK key if one exist. Otherwise, returns 'None'.
        '''
        if value := self._data['sdk-config'].get(key, None):
            return base64.b64decode(value)
        return None

    def DeleteSdkKey(self, key: str):
        ''' Deletes an SDK key if one exists.
        '''
        LOGGER.debug("DeleteSdkKey: %s", key)

        self._data['sdk-config'].pop(key, None)
        self.Commit()

    def Shutdown(self):
        ''' Shuts down the object by free'ing up the associated adapter instance.
            You cannot interact with this object there-after.

            This should only be called after the CHIP stack has shutdown (i.e
            after calling pychip_DeviceController_StackShutdown()).
        '''
        if self._isActive:
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
        return copy.deepcopy(self._data)

    def __del__(self):
        self.Shutdown()


class PersistentStorageJSON(PersistentStorage):
    """Persistent storage back-end which stores data in a JSON file."""

    def __init__(self, path: str):
        LOGGER.info("Loading configuration from JSON file: %s", path)
        self._path = path
        try:
            with open(self._path) as f:
                LOGGER.info("Loading configuration from JSON file")
                data = json.loads(f.read() or '{}')
        except FileNotFoundError:
            LOGGER.info("Configuration file not found, using empty configuration")
            data = {}
        except Exception as ex:
            LOGGER.critical("Could not load configuration from JSON file: %s", ex)
            data = {}
        super().__init__(data)

    def Commit(self):
        try:
            with open(self._path, 'w') as f:
                json.dump(self._data, f, ensure_ascii=True, indent=4)
        except Exception as ex:
            LOGGER.critical("Could not save configuration to JSON file: %s", ex)


class PersistentStorageINI(PersistentStorage):
    """Persistent storage back-end which stores data in an INI file.

    This persistent storage is compatible with the chip-tool implementation.
    """

    class ConfigParser(ConfigParser):
        """Parser that preserves key case and does not use interpolation."""

        def __init__(self, defaults=None):
            # Set the default section to 'NO-DEFAULT' to treat the default section
            # as a regular section, so other sections will not inherit from it.
            super().__init__(defaults=defaults, default_section='NO-DEFAULT', interpolation=None)
            # SDK configuration is stored in the 'Default' (mind the case) section.
            self.add_section('Default')

        def optionxform(self, option: str) -> str:
            """Preserves case of keys."""
            return option

    def __init__(self, path: str, chipToolFabricStoragePath: Optional[str] = None):
        """Initializes the persistent storage from an INI file.

        Optionally, a chipToolFabricStoragePath can be provided to load fabric CA
        from a separate file. This allows to load chip-tool persistent storage which
        stores fabric CA in a separate file.
        """
        try:
            data = {}
            config = PersistentStorageINI.ConfigParser()
            LOGGER.info("Loading configuration from INI file: %s", path)
            config.read(path)
            if config.has_section('Default'):
                data['sdk-config'] = dict(config.items('Default'))
            if config.has_section('REPL'):
                data['repl-config'] = {
                    k: json.loads(v) if v else None
                    for k, v in config.items('REPL')
                }
            if chipToolFabricStoragePath:
                # Compatibility layer with chip-tool persistent storage.
                LOGGER.info("Loading fabric configuration from INI file: %s", chipToolFabricStoragePath)
                config.read(chipToolFabricStoragePath)
                if value := config.get('Default', 'ExampleCAIntermediateCert0', fallback=None):
                    data['sdk-config']['ExampleCAIntermediateCert1'] = value
                if value := config.get('Default', 'ExampleCARootCert0', fallback=None):
                    data['sdk-config']['ExampleCARootCert1'] = value
                if value := config.get('Default', 'ExampleOpCredsCAKey0', fallback=None):
                    data['sdk-config']['ExampleOpCredsCAKey1'] = value
                if value := config.get('Default', 'ExampleOpCredsICAKey0', fallback=None):
                    data['sdk-config']['ExampleOpCredsICAKey1'] = value
        except Exception as ex:
            LOGGER.critical("Could not load configuration from INI file: %s", ex)
        self._path = path
        self._chipToolFabricStoragePath = chipToolFabricStoragePath
        super().__init__(data)

    def Commit(self):
        # Get a copy of the SDK configuration so that we can modify it.
        sdkConfig = self._data['sdk-config'].copy()
        if self._chipToolFabricStoragePath:
            # Compatibility layer with chip-tool persistent storage.
            configFabric = PersistentStorageINI.ConfigParser()
            if value := sdkConfig.pop('ExampleCAIntermediateCert1', None):
                configFabric['Default']['ExampleCAIntermediateCert0'] = value
            if value := sdkConfig.pop('ExampleCARootCert1', None):
                configFabric['Default']['ExampleCARootCert0'] = value
            if value := sdkConfig.pop('ExampleOpCredsCAKey1', None):
                configFabric['Default']['ExampleOpCredsCAKey0'] = value
            if value := sdkConfig.pop('ExampleOpCredsICAKey1', None):
                configFabric['Default']['ExampleOpCredsICAKey0'] = value
            try:
                with open(self._chipToolFabricStoragePath, 'w') as f:
                    configFabric.write(f)
            except Exception as ex:
                LOGGER.critical("Could not save fabric configuration to INI file: %s", ex)
        config = PersistentStorageINI.ConfigParser()
        config['Default'] = sdkConfig
        config['REPL'] = {
            k: json.dumps(v, separators=(',', ':'))
            for k, v in self._data['repl-config'].items()
        }
        try:
            with open(self._path, 'w') as f:
                config.write(f)
        except Exception as ex:
            LOGGER.critical("Could not save configuration to INI file: %s", ex)
