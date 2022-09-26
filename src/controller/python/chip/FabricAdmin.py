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
import builtins
import base64
import chip.exceptions
from chip import ChipDeviceCtrl
import copy
from .storage import PersistentStorage
from chip.CertificateAuthority import CertificateAuthority


class FabricAdmin:
    ''' Administers a fabric associated with a unique FabricID under a given CertificateAuthority
        instance.
    '''
    @classmethod
    def _Handle(cls):
        return chip.native.GetLibraryHandle()

    @classmethod
    def logger(cls):
        return logging.getLogger('FabricAdmin')

    def __init__(self, certificateAuthority: CertificateAuthority, vendorId: int, fabricId: int = 1):
        ''' Initializes the object.

            certificateAuthority:       CertificateAuthority instance that will be used to vend NOCs for both
                                        DeviceControllers and commissionable nodes on this fabric.
            vendorId:                   Valid operational Vendor ID associated with this fabric.
            fabricId:                   Fabric ID to be associated with this fabric.
        '''
        self._handle = chip.native.GetLibraryHandle()

        if (vendorId is None or vendorId == 0):
            raise ValueError(
                f"Invalid VendorID ({vendorId}) provided!")

        if (fabricId is None or fabricId == 0):
            raise ValueError(
                f"Invalid FabricId ({fabricId}) provided!")

        self._vendorId = vendorId
        self._fabricId = fabricId
        self._certificateAuthority = certificateAuthority

        self.logger().warning(f"New FabricAdmin: FabricId: 0x{self._fabricId:016X}, VendorId = 0x{self.vendorId:04X}")

        self._isActive = True
        self._activeControllers = []

    def NewController(self, nodeId: int = None, paaTrustStorePath: str = "", useTestCommissioner: bool = False, catTags: List[int] = []):
        ''' Create a new chip.ChipDeviceCtrl.ChipDeviceController instance on this fabric.

            When vending ChipDeviceController instances on a given fabric, each controller instance
            is associated with a unique fabric index local to the running process. In the underlying FabricTable, each FabricInfo
            instance can be treated as unique identities that can collide on the same logical fabric.

            nodeId:                 NodeID to be assigned to the controller. Automatically allocates one starting from 112233 if one
                                    is not provided.

            paaTrustStorePath:      Path to the PAA trust store. If one isn't provided, a suitable default is selected.
            useTestCommissioner:    If a test commmisioner is to be created.
            catTags:			    A list of 32-bit CAT tags that will added to the NOC generated for this controller.
        '''
        if (not(self._isActive)):
            raise RuntimeError(
                f"FabricAdmin object was previously shutdown and is no longer valid!")

        nodeIdList = [controller.nodeId for controller in self._activeControllers if controller.isActive]
        if (nodeId is None):
            if (len(nodeIdList) != 0):
                nodeId = max(nodeIdList) + 1
            else:
                nodeId = 112233
        else:
            if (nodeId in nodeIdList):
                raise RuntimeError(f"Provided NodeId {nodeId} collides with an existing controller instance!")

        self.logger().warning(
            f"Allocating new controller with CaIndex: {self._certificateAuthority.caIndex}, FabricId: 0x{self._fabricId:016X}, NodeId: 0x{nodeId:016X}, CatTags: {catTags}")

        controller = ChipDeviceCtrl.ChipDeviceController(opCredsContext=self._certificateAuthority.GetOpCredsContext(), fabricId=self._fabricId, nodeId=nodeId,
                                                         adminVendorId=self._vendorId, paaTrustStorePath=paaTrustStorePath, useTestCommissioner=useTestCommissioner, fabricAdmin=self, catTags=catTags)

        self._activeControllers.append(controller)
        return controller

    def Shutdown(self):
        ''' Shutdown all active controllers on the fabric before shutting down the fabric itself.

            You cannot interact with this object there-after.
        '''
        if (self._isActive):
            for controller in self._activeControllers:
                controller.Shutdown()

            self._isActive = False

    def __del__(self):
        self.Shutdown()

    @property
    def vendorId(self) -> int:
        return self._vendorId

    @property
    def fabricId(self) -> int:
        return self._fabricId

    @property
    def caIndex(self) -> int:
        return self._certificateAuthority.caIndex

    @property
    def certificateAuthority(self) -> CertificateAuthority:
        return self._certificateAuthority
