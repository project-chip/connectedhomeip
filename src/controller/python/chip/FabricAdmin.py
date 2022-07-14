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
import ipdb
import json
import logging
import builtins
import base64
import chip.exceptions
from chip import ChipDeviceCtrl
import copy


class FabricAdmin:
    ''' Administers a given fabric instance. Each admin is associated with
        a specific RCAC and ICAC as well as Fabric ID and Index.

        There can only be one admin instance for a given fabric in a single
        Python process instance.

        The admin also persists to storage its detail automatically to permit
        easy loading of its information later.

        >> C++ Binding Details

        Each instance of the fabric admin is associated with a single instance
        of the OperationalCredentialsAdapter. This adapter instance implements
        the OperationalCredentialsDelegate and is meant to provide a Python 
        adapter to the functions in that delegate so that the fabric admin
        can in turn, provide users the ability to generate their own NOCs for devices
        on the network (not implemented yet). For now, it relies on the in-built 
        ExampleOperationalCredentialsIssuer to do that.

        TODO: Add support for FabricAdmin to permit callers to hook up their own GenerateNOC
              logic.

        >> Persistence

        Each instance persists its fabric ID and index to storage. This is in addition to the
        persistence built into the ExampleOperationalCredentialsIssuer that persists details
        about the RCAC/ICAC as well. This facilitates re-construction of a fabric admin on subsequent
        boot for a given fabric and ensuring it automatically picks up the right ICAC/RCAC details as well.
    '''

    _handle = chip.native.GetLibraryHandle()
    _isActive = False
    activeFabricIndexList = set()
    activeFabricIdList = set()
    activeAdmins = set()
    vendorId = None

    def AllocateNextFabricIndex(self):
        ''' Allocate the next un-used fabric index.
        '''
        nextFabricIndex = 1
        while nextFabricIndex in FabricAdmin.activeFabricIndexList:
            nextFabricIndex = nextFabricIndex + 1
        return nextFabricIndex

    def AllocateNextFabricId(self):
        ''' Allocate the next un-used fabric ID.
        '''
        nextFabricId = 1

        while nextFabricId in FabricAdmin.activeFabricIdList:
            nextFabricId = nextFabricId + 1
        return nextFabricId

    def __init__(self, vendorId: int, rcac: bytes = None, icac: bytes = None, fabricIndex: int = None, fabricId: int = None):
        ''' Creates a valid FabricAdmin object with valid RCAC/ICAC, and registers itself as an OperationalCredentialsDelegate
            for other parts of the system (notably, DeviceController) to vend NOCs.

            vendorId:       Valid operational Vendor ID associated with this fabric.
            rcac, icac:     Specify the RCAC and ICAC to be used with this fabric (not-supported). If not specified, an RCAC and ICAC will
                            be automatically generated.

            fabricIndex:    Local fabric index to be associated with this fabric. If omitted, one will be automatically assigned.
            fabricId:       Local fabric ID to be associated with this fabric. If omitted, one will be automatically assigned.
        '''
        if (rcac is not None or icac is not None):
            raise ValueError(
                "Providing valid rcac/icac values is not supported right now!")

        if (vendorId is None or vendorId == 0):
            raise ValueError(
                f"Invalid VendorID ({vendorId}) provided!")

        self.vendorId = vendorId

        if (fabricId is None):
            self._fabricId = self.AllocateNextFabricId()
        else:
            if (fabricId in FabricAdmin.activeFabricIdList):
                raise ValueError(
                    f"FabricId {fabricId} is already being managed by an existing FabricAdmin object!")

            self._fabricId = fabricId

        if (fabricIndex is None):
            self._fabricIndex = self.AllocateNextFabricIndex()
        else:
            if (fabricIndex in FabricAdmin.activeFabricIndexList):
                raise ValueError(
                    f"FabricIndex {fabricIndex} is already being managed by an existing FabricAdmin object!")

            self._fabricIndex = fabricIndex

        # Add it to the tracker to prevent future FabricAdmins from managing the same fabric.
        FabricAdmin.activeFabricIdList.add(self._fabricId)
        FabricAdmin.activeFabricIndexList.add(self._fabricIndex)

        print(
            f"New FabricAdmin: FabricId: {self._fabricId}({self._fabricIndex}), VendorId = {hex(self.vendorId)}")
        self._handle.pychip_OpCreds_InitializeDelegate.restype = c_void_p

        self.closure = builtins.chipStack.Call(
            lambda: self._handle.pychip_OpCreds_InitializeDelegate(
                ctypes.py_object(self), ctypes.c_uint32(self._fabricIndex))
        )

        if (self.closure is None):
            raise ValueError("Encountered error initializing OpCreds adapter")

        #
        # Persist details to storage (read modify write).
        #
        try:
            adminList = builtins.chipStack.GetStorageManager().GetReplKey('fabricAdmins')
        except KeyError:
            adminList = {str(self._fabricIndex): {'fabricId': self._fabricId}}
            builtins.chipStack.GetStorageManager().SetReplKey('fabricAdmins', adminList)

        adminList[str(self._fabricIndex)] = {'fabricId': self._fabricId, 'vendorId': self.vendorId}
        builtins.chipStack.GetStorageManager().SetReplKey('fabricAdmins', adminList)

        self._isActive = True
        self.nextControllerId = 1

        FabricAdmin.activeAdmins.add(self)

    def NewController(self, nodeId: int = None, paaTrustStorePath: str = "", useTestCommissioner: bool = False):
        ''' Vend a new controller on this fabric seeded with the right fabric details.
        '''
        if (not(self._isActive)):
            raise RuntimeError(
                f"FabricAdmin object was previously shutdown and is no longer valid!")

        if (nodeId is None):
            nodeId = self.nextControllerId
            self.nextControllerId = self.nextControllerId + 1

        print(
            f"Allocating new controller with FabricId: {self._fabricId}({self._fabricIndex}), NodeId: {nodeId}")
        controller = ChipDeviceCtrl.ChipDeviceController(
            self.closure, self._fabricId, self._fabricIndex, nodeId, self.vendorId, paaTrustStorePath, useTestCommissioner)
        return controller

    def ShutdownAll():
        ''' Shuts down all active fabrics, but without deleting them from storage.
        '''
        activeAdmins = copy.copy(FabricAdmin.activeAdmins)

        for admin in activeAdmins:
            admin.Shutdown(False)

        FabricAdmin.activeAdmins.clear()

    def Shutdown(self, deleteFromStorage: bool = True):
        ''' Shutdown this fabric and free up its resources. This is important since relying
            solely on the destructor will not guarantee relishining of C++-side resources.

            deleteFromStorage:      Whether to delete this fabric's details from persistent storage.
        '''
        if (self._isActive):
            builtins.chipStack.Call(
                lambda: self._handle.pychip_OpCreds_FreeDelegate(
                    ctypes.c_void_p(self.closure))
            )

            FabricAdmin.activeFabricIdList.remove(self._fabricId)
            FabricAdmin.activeFabricIndexList.remove(self._fabricIndex)

            if (deleteFromStorage):
                adminList = builtins.chipStack.GetStorageManager().GetReplKey('fabricAdmins')
                del(adminList[str(self._fabricIndex)])
                if (len(adminList) == 0):
                    adminList = None

                builtins.chipStack.GetStorageManager().SetReplKey('fabricAdmins', adminList)

            FabricAdmin.activeAdmins.remove(self)
            self._isActive = False

    def __del__(self):
        self.Shutdown(False)
