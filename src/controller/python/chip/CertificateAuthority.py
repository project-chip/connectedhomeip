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
from chip.native import PyChipError
from rich.pretty import pprint
import json
import logging
import builtins
import base64
import chip.exceptions
from chip import ChipDeviceCtrl
from chip import ChipStack
from chip import FabricAdmin
from chip.storage import PersistentStorage


class CertificateAuthority:
    '''  This represents an operational Root Certificate Authority (CA) with a root key key pair with associated public key (i.e "Root PK") . This manages
         a set of FabricAdmin objects, each administering a fabric identified by a unique FabricId scoped to it.

         Each CertificateAuthority instance is tied to a 'CA index' that is used to look-up the list of fabrics already setup previously
         in the provided PersistentStorage object.

         >> C++ Binding Details

         Each CertificateAuthority instance is associated with a single instance of the OperationalCredentialsAdapter. This adapter instance implements
         the OperationalCredentialsDelegate and is meant to provide a Python adapter to the functions in that delegate. It relies on the in-built
         ExampleOperationalCredentialsIssuer to then generate certificate material for the CA.  This instance also uses the 'CA index' to
         store/look-up the associated credential material from the provided PersistentStorage object.
    '''
    @classmethod
    def _Handle(cls):
        return chip.native.GetLibraryHandle()

    @classmethod
    def logger(cls):
        return logging.getLogger('CertificateAuthority')

    def __init__(self, chipStack: ChipStack.ChipStack, caIndex: int, persistentStorage: PersistentStorage = None):
        '''  Initializes the CertificateAuthority. This will set-up the associated C++ OperationalCredentialsAdapter
             as well.

             Arguments:
                chipStack:          A reference to a chip.ChipStack object.
                caIndex:            An index used to look-up details about stored credential material and fabrics from persistent storage.
                persistentStorage:  An optional reference to a PersistentStorage object. If one is provided, it will pick that over
                                    the default PersistentStorage object retrieved from the chipStack.
        '''
        self.logger().warning(f"New CertificateAuthority at index {caIndex}")

        self._chipStack = chipStack
        self._caIndex = caIndex

        self._Handle().pychip_OpCreds_InitializeDelegate.restype = c_void_p
        self._Handle().pychip_OpCreds_InitializeDelegate.argtypes = [ctypes.py_object, ctypes.c_uint32, ctypes.c_void_p]

        self._Handle().pychip_OpCreds_SetMaximallyLargeCertsUsed.restype = PyChipError
        self._Handle().pychip_OpCreds_SetMaximallyLargeCertsUsed.argtypes = [ctypes.c_void_p, ctypes.c_bool]

        if (persistentStorage is None):
            persistentStorage = self._chipStack.GetStorageManager()

        self._persistentStorage = persistentStorage
        self._maximizeCertChains = False

        self._closure = self._chipStack.Call(
            lambda: self._Handle().pychip_OpCreds_InitializeDelegate(
                ctypes.py_object(self), ctypes.c_uint32(self._caIndex), self._persistentStorage.GetSdkStorageObject())
        )

        if (self._closure is None):
            raise ValueError("Encountered error initializing OpCreds adapter")

        self._isActive = True
        self._activeAdmins = []

    def LoadFabricAdminsFromStorage(self):
        ''' If FabricAdmins had been setup previously, this re-creates them using information from persistent storage.
            Otherwise, it initializes the REPL keys in persistent storage to sane defaults. This includes a top-level
            key identifying the CA (using the associated CA Index) initialized to an empty list.

            This expects a 'caList' key to be present in the REPL config.

            Each FabricAdmin that is added there-after will insert a dictionary item into that list containing
            'fabricId' and 'vendorId' keys.
        '''
        if (not(self._isActive)):
            raise RuntimeError("Object isn't active")

        self.logger().warning("Loading fabric admins from storage...")

        caList = self._persistentStorage.GetReplKey(key='caList')
        if (str(self._caIndex) not in caList):
            caList[str(self._caIndex)] = []
            self._persistentStorage.SetReplKey(key='caList', value=caList)

        fabricAdminMetadataList = self._persistentStorage.GetReplKey(key='caList')[str(self._caIndex)]
        for adminMetadata in fabricAdminMetadataList:
            self.NewFabricAdmin(vendorId=int(adminMetadata['vendorId']), fabricId=int(adminMetadata['fabricId']))

    def NewFabricAdmin(self, vendorId: int, fabricId: int):
        ''' Creates a new FabricAdmin object initialized with the provided vendorId and fabricId values.

            This will update the REPL keys in persistent storage IF a 'caList' key is present. If it isn't,
            will avoid making any updates.
        '''
        if (not(self._isActive)):
            raise RuntimeError(
                f"CertificateAuthority object was previously shutdown and is no longer valid!")

        if (vendorId is None or fabricId is None):
            raise ValueError(f"Invalid values for fabricId and vendorId")

        for existingAdmin in self._activeAdmins:
            if (existingAdmin.fabricId == fabricId):
                raise ValueError(f"Provided fabricId of {fabricId} collides with an existing FabricAdmin instance!")

        fabricAdmin = FabricAdmin.FabricAdmin(self, vendorId=vendorId, fabricId=fabricId)

        caList = self._persistentStorage.GetReplKey('caList')
        if (caList is not None):
            replFabricEntry = {'fabricId': fabricId, 'vendorId': vendorId}

            if (replFabricEntry not in caList[str(self._caIndex)]):
                caList[str(self._caIndex)].append(replFabricEntry)

            self._persistentStorage.SetReplKey(key='caList', value=caList)

        self._activeAdmins.append(fabricAdmin)

        return fabricAdmin

    def Shutdown(self):
        ''' Shuts down all active FabricAdmin objects managed by this CertificateAuthority before
            shutting itself down.

            You cannot interact with this object there-after.
        '''
        if (self._isActive):
            for admin in self._activeAdmins:
                admin.Shutdown()

            self._activeAdmins = []
            self._Handle().pychip_OpCreds_FreeDelegate.argtypes = [ctypes.c_void_p]
            self._chipStack.Call(
                lambda: self._Handle().pychip_OpCreds_FreeDelegate(
                    ctypes.c_void_p(self._closure))
            )

            self._isActive = False

    def GetOpCredsContext(self):
        ''' Returns a pointer to the underlying C++ OperationalCredentialsAdapter.
        '''
        if (not(self._isActive)):
            raise RuntimeError("Object isn't active")

        return self._closure

    @property
    def caIndex(self) -> int:
        return self._caIndex

    @property
    def adminList(self) -> list[FabricAdmin.FabricAdmin]:
        return self._activeAdmins

    @property
    def maximizeCertChains(self) -> bool:
        return self._maximizeCertChains

    @maximizeCertChains.setter
    def maximizeCertChains(self, enabled: bool):
        self._chipStack.Call(
            lambda: self._Handle().pychip_OpCreds_SetMaximallyLargeCertsUsed(ctypes.c_void_p(self._closure), ctypes.c_bool(enabled))
        ).raise_on_error()

        self._maximizeCertChains = enabled

    def __del__(self):
        self.Shutdown()


class CertificateAuthorityManager:
    ''' Manages a set of CertificateAuthority instances.
    '''
    @classmethod
    def _Handle(cls):
        return chip.native.GetLibraryHandle()

    @classmethod
    def logger(cls):
        return logging.getLogger('CertificateAuthorityManager')

    def __init__(self, chipStack: ChipStack.ChipStack, persistentStorage: PersistentStorage = None):
        ''' Initializes the manager.

            chipStack:          Reference to a chip.ChipStack object that is used to initialize
                                CertificateAuthority instances.

            persistentStorage:  If provided, over-rides the default instance in the provided chipStack
                                when initializing CertificateAuthority instances.
        '''
        self._activeCaIndexList = []
        self._chipStack = chipStack

        if (persistentStorage is None):
            persistentStorage = self._chipStack.GetStorageManager()

        self._persistentStorage = persistentStorage
        self._activeCaList = []
        self._isActive = True

    def _AllocateNextCaIndex(self):
        ''' Allocate the next un-used CA index.
        '''
        nextCaIndex = 1
        for ca in self._activeCaList:
            nextCaIndex = ca.caIndex + 1
        return nextCaIndex

    def LoadAuthoritiesFromStorage(self):
        ''' Loads any existing CertificateAuthority instances present in persistent storage.
            If the 'caList' key is not present in the REPL config, it will create one.
        '''
        if (not(self._isActive)):
            raise RuntimeError("Object is not active")

        self.logger().warning("Loading certificate authorities from storage...")

        #
        # Persist details to storage (read modify write).
        #
        caList = self._persistentStorage.GetReplKey('caList')
        if (caList is None):
            caList = {}

        for caIndex in caList:
            ca = self.NewCertificateAuthority(int(caIndex))
            ca.LoadFabricAdminsFromStorage()

    def NewCertificateAuthority(self, caIndex: int = None, maximizeCertChains: bool = False):
        ''' Creates a new CertificateAuthority instance with the provided CA Index and the PersistentStorage
            instance previously setup in the constructor.

            This will write to the REPL keys in persistent storage to setup an empty list for the 'CA Index'
            item.
        '''
        if (not(self._isActive)):
            raise RuntimeError("Object is not active")

        if (caIndex is None):
            caIndex = self._AllocateNextCaIndex()

        #
        # Persist details to storage (read modify write).
        #
        caList = self._persistentStorage.GetReplKey('caList')
        if (caList is None):
            caList = {}

        if (str(caIndex) not in caList):
            caList[str(caIndex)] = []
            self._persistentStorage.SetReplKey(key='caList', value=caList)

        ca = CertificateAuthority(chipStack=self._chipStack, caIndex=caIndex, persistentStorage=self._persistentStorage)
        ca.maximizeCertChains = maximizeCertChains
        self._activeCaList.append(ca)

        return ca

    def Shutdown(self):
        ''' Shuts down all active CertificateAuthority instances tracked by this manager, before shutting itself down.

            You cannot interact with this object there-after.
        '''
        for ca in self._activeCaList:
            ca.Shutdown()

        self._activeCaList = []
        self._isActive = False

    @property
    def activeCaList(self) -> List[CertificateAuthority]:
        return self._activeCaList

    def __del__(self):
        self.Shutdown()
