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
import re
from abc import ABC, abstractmethod
from configparser import ConfigParser
from ctypes import CFUNCTYPE, POINTER, c_bool, c_char, c_char_p, c_uint16, c_void_p, py_object
from typing import Any, Dict, Optional

from ..native import GetLibraryHandle

LOGGER = logging.getLogger(__name__)

_SetKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p, POINTER(c_char),  c_uint16)
_GetKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p, POINTER(c_char), POINTER(c_uint16), POINTER(c_bool))
_DeleteKeyValueCbFunc = CFUNCTYPE(None, py_object, c_char_p)


class PersistentStorage(ABC):
    """Abstract base class for persistent storage.

    This class provides persistent storage interface for both the Matter SDK
    and the native Python configuration storage.

    It interfaces with a C++ adapter that implements the PersistentStorageDelegate
    interface and can be passed into C++ logic that needs an instance of that
    interface.

    This object must be available throughout the lifetime of the Matter stack.
    """

    @_GetKeyValueCbFunc
    def _OnGetKeyValueCb(self, key: bytes, value, size, is_found):
        # This does not adhere to the API requirements of PersistentStorageDelegate::SyncGetKeyValue,
        # but that is okay since the C++ storage binding layer is capable of adapting results from
        # this method to the requirements of PersistentStorageDelegate::SyncGetKeyValue.
        retrievedValue = self.GetSdkKey(key.decode("utf-8"))
        if retrievedValue is not None:
            sizeToCopy = min(size[0], len(retrievedValue))
            for count, val in enumerate(retrievedValue):
                if sizeToCopy == count:
                    break
                value[count] = val
            # As mentioned above, we are intentionally not returning
            # sizeToCopy as one might expect because the caller
            # will use the value in size[0] to determine if it should
            # return CHIP_ERROR_BUFFER_TOO_SMALL.
            size[0] = len(retrievedValue)
            is_found[0] = True
        else:
            is_found[0] = False
            size[0] = 0

    @_SetKeyValueCbFunc
    def _OnSetKeyValueCb(self, key: bytes, value, size):
        self.SetSdkKey(key.decode("utf-8"), ctypes.string_at(value, size))

    @_DeleteKeyValueCbFunc
    def _OnDeleteKeyValueCb(self, key):
        self.DeleteSdkKey(key.decode("utf-8"))

    def __init__(self):
        self._handle = GetLibraryHandle()
        self._handle.pychip_Storage_InitializeStorageAdapter.restype = c_void_p
        self._handle.pychip_Storage_InitializeStorageAdapter.argtypes = [ctypes.py_object,
                                                                         _SetKeyValueCbFunc,
                                                                         _GetKeyValueCbFunc,
                                                                         _DeleteKeyValueCbFunc]
        self._closure = self._handle.pychip_Storage_InitializeStorageAdapter(
            ctypes.py_object(self), self._OnSetKeyValueCb, self._OnGetKeyValueCb, self._OnDeleteKeyValueCb)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.Shutdown()

    def __del__(self):
        if hasattr(self, '_closure'):
            LOGGER.warning("PersistentStorage object is being deleted without explicit shutdown. "
                           "Calling Shutdown() to clean up resources.")
            # The caller should call Shutdown() explicitly or use a context manager
            # to ensure that the object is cleaned up properly. However, we should
            # try our best not to leak any resources...
            self.Shutdown()

    def GetSdkStorageObject(self):
        """Return a ctypes reference to the SDK-side adapter instance."""
        return self._closure

    def Shutdown(self):
        """Shut down the object by freeing up the associated adapter instance.

        You cannot interact with this object there-after.

        This should only be called after the CHIP stack has shutdown (i.e
        after calling pychip_DeviceController_StackShutdown()).
        """
        if hasattr(self, '_closure'):
            self._handle.pychip_Storage_ShutdownAdapter.argtypes = [c_void_p]
            # Since the stack is not running at this point, we can safely call
            # C++ method directly on the current execution context without worrying
            # about race conditions.
            self._handle.pychip_Storage_ShutdownAdapter(self._closure)
            del self._closure

    @abstractmethod
    def GetKey(self, key: str) -> Any | None:
        """Retrieve the value of a key or None if it does not exist."""
        pass

    @abstractmethod
    def SetKey(self, key: str, value: Any):
        """Set the value of a key.

        If the key does not exist, it shall be created.

        After setting the key, the changes shall be committed using the
        Commit() method.
        """
        pass

    @abstractmethod
    def DeleteKey(self, key: str):
        """Delete a key.

        If the key does not exist, this method should do nothing.

        After deleting the key, the changes shall be committed using the
        Commit() method.
        """
        pass

    @abstractmethod
    def GetSdkKey(self, key: str) -> bytes | None:
        """Retrieve the value of an SDK key or None if it does not exist."""
        pass

    @abstractmethod
    def SetSdkKey(self, key: str, value: bytes):
        """Set the value of an SDK key.

        If the key does not exist, it shall be created.

        After setting the key, the changes shall be committed using the
        Commit() method.
        """
        pass

    @abstractmethod
    def DeleteSdkKey(self, key: str):
        """Delete an SDK key.

        If the key does not exist, this method should do nothing.

        After deleting the key, the changes shall be committed using the
        Commit() method.
        """
        pass

    @abstractmethod
    def Commit(self):
        """Commit the changes to persistent storage."""
        pass


class PersistentStorageBase(PersistentStorage):
    """Base class for persistent storage implementations.

    This class keeps the configuration in the dictionary _data attribute with
    two top-level keys 'repl-config' and 'sdk-config' respectively for the REPL
    and SDK configurations.
    """

    def __init__(self, data: Dict = {}, sdkData: Dict = {}):
        """Initializes the persistent storage with provided data."""
        super().__init__()
        self._data = copy.deepcopy(data)
        self._sdkData = copy.deepcopy(sdkData)

    def GetKey(self, key: str) -> Any | None:
        return copy.deepcopy(self._data.get(key, None))

    def SetKey(self, key: str, value: Any):
        if not key:
            raise ValueError("Invalid key")
        LOGGER.debug("Set key: %s = %s", key, value)
        self._data[key] = value
        self.Commit()

    def DeleteKey(self, key: str):
        LOGGER.debug("Delete key: %s", key)
        self._data.pop(key, None)
        self.Commit()

    def GetSdkKey(self, key: str) -> bytes | None:
        if value := self._sdkData.get(key, None):
            return base64.b64decode(value)
        return None

    def SetSdkKey(self, key: str, value: bytes):
        if not key:
            raise ValueError("Invalid SDK key")
        if value is None:
            raise ValueError("SDK key value is not expected to be None")
        LOGGER.debug("Set SDK key: %s = hex:%s", key, value.hex())
        self._sdkData[key] = base64.b64encode(value).decode("utf-8")
        self.Commit()

    def DeleteSdkKey(self, key: str):
        LOGGER.debug("Delete SDK key: %s", key)
        self._sdkData.pop(key, None)
        self.Commit()


class VolatileTemporaryPersistentStorage(PersistentStorageBase):
    """In-memory temporary persistent storage.

    This class does not provide any persistence storage on its own. It keeps
    the configuration in memory. Please use a dedicated subclass to provide
    a non-volatile persistence storage, such as PersistentStorageJSON or
    PersistentStorageINI.
    """

    def Commit(self):
        pass


# Regular expression to match CA keys.
_caKeyMatch = re.compile(r'(ExampleCAIntermediateCert|ExampleCARootCert|ExampleOpCredsCAKey|ExampleOpCredsICAKey)(\d+)')


class PersistentStorageJSON(PersistentStorageBase):
    """Persistent storage back-end which stores data in a JSON file."""

    def _caKeysBackwardCompatibilityRewrite(self, data: Dict, sdkData: Dict):
        """Rewrites CA keys if the index does not start from 0.

        This rewrite is a backward compatibility patch for old CertificateAuthority
        class which used 1-based indexing for CA keys. The new implementation uses
        0-based indexing to be compatible with chip-tool implementation.
        """
        caKeys = {}
        for key in sdkData:
            if match := _caKeyMatch.fullmatch(key):
                caKeys[key] = (match.group(1), int(match.group(2)))

        if not caKeys or min([v[1] for v in caKeys.values()]) == 0:
            # No CA keys found or they are 0-based indexed - nothing to rewrite.
            return data, sdkData

        LOGGER.info("Rewriting CA keys to 0-based indexing")

        caKeysRewritten = {}
        for key, (prefix, index) in caKeys.items():
            caKeysRewritten[f"{prefix}{index - 1}"] = sdkData.pop(key)
        sdkData.update(caKeysRewritten)

        caListRewritten = {}
        for key, value in data.get('caList', {}).items():
            caListRewritten[str(int(key) - 1)] = value
        data['caList'] = caListRewritten

        return data, sdkData

    def __init__(self, path: str):
        LOGGER.info("Loading configuration from JSON file: %s", path)
        self._path = path
        try:
            with open(self._path) as f:
                jsonData = json.loads(f.read() or '{}')
                data = jsonData.get('repl-config', {})
                sdkData = jsonData.get('sdk-config', {})
                # TODO: Remove this compatibility layer when all JSON files are rewritten.
                data, sdkData = self._caKeysBackwardCompatibilityRewrite(data, sdkData)
        except FileNotFoundError:
            LOGGER.info("Configuration file not found, using empty configuration")
            data, sdkData = {}, {}
        except Exception as ex:
            LOGGER.critical("Could not load configuration from JSON file: %s", ex)
            data, sdkData = {}, {}
        super().__init__(data, sdkData)

    def Commit(self):
        try:
            with open(self._path, 'w') as f:
                json.dump({
                    'repl-config': self._data,
                    'sdk-config': self._sdkData,
                }, f, ensure_ascii=True, indent=4)
        except Exception as ex:
            LOGGER.critical("Could not save configuration to JSON file: %s", ex)


class PersistentStorageINI(PersistentStorageBase):
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
            # Preserves case of keys.
            return option

    def __init__(self, path: str, chipToolFabricStoragePath: Optional[str] = None):
        """Initializes the persistent storage from an INI file.

        Optionally, a chipToolFabricStoragePath can be provided to load fabric CA
        from a separate file. This allows to load chip-tool persistent storage which
        stores fabric CA in a separate file.
        """
        LOGGER.info("Loading configuration from INI file: %s", path)
        self._chipToolFabricStoragePath = chipToolFabricStoragePath
        self._path = path
        try:
            data, sdkData = {}, {}
            config = PersistentStorageINI.ConfigParser()
            config.read(path)
            if config.has_section('Default'):
                sdkData = dict(config.items('Default'))
            if config.has_section('REPL'):
                data = {
                    k: json.loads(v) if v else None
                    for k, v in config.items('REPL')
                }
            if chipToolFabricStoragePath:
                LOGGER.info("Loading fabric configuration from INI file: %s", chipToolFabricStoragePath)
                config.read(chipToolFabricStoragePath)
                for key, value in config.items('Default'):
                    sdkData[key] = value
        except Exception as ex:
            LOGGER.critical("Could not load configuration from INI file: %s", ex)
        super().__init__(data, sdkData)

    def Commit(self):
        # Get a copy of the SDK configuration so that we can modify it.
        sdkConfig = self._sdkData.copy()
        if self._chipToolFabricStoragePath:
            # Compatibility layer with chip-tool persistent storage.
            configFabric = PersistentStorageINI.ConfigParser()
            for key in self._sdkData:
                # Move CA keys to the separate fabric configuration file.
                if _caKeyMatch.fullmatch(key):
                    configFabric['Default'][key] = sdkConfig.pop(key)
            try:
                with open(self._chipToolFabricStoragePath, 'w') as f:
                    configFabric.write(f)
            except Exception as ex:
                LOGGER.critical("Could not save fabric configuration to INI file: %s", ex)
        config = PersistentStorageINI.ConfigParser()
        config['Default'] = sdkConfig
        config['REPL'] = {
            k: json.dumps(v, separators=(',', ':'))
            for k, v in self._data.items()
        }
        try:
            with open(self._path, 'w') as f:
                config.write(f)
        except Exception as ex:
            LOGGER.critical("Could not save configuration to INI file: %s", ex)
