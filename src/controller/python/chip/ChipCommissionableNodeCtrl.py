#
#    Copyright (c) 2020-2021 Project CHIP Authors
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
#      Python interface for ChipCommissionableNodeController
#

"""Chip Commissionable Node Controller interface
"""

from __future__ import absolute_import
from __future__ import print_function
from ctypes import *
from .ChipStack import *
from .exceptions import *
from .native import PyChipError

__all__ = ["ChipCommissionableNodeController"]


def _singleton(cls):
    instance = [None]

    def wrapper(*args, **kwargs):
        if instance[0] is None:
            instance[0] = cls(*args, **kwargs)
        return instance[0]

    return wrapper


@_singleton
class ChipCommissionableNodeController(object):
    def __init__(self, chipStack: ChipStack):
        self.commissionableNodeCtrl = None
        self._ChipStack = chipStack
        self._dmLib = None

        self._InitLib()

        commissionableNodeCtrl = c_void_p(None)
        self._dmLib.pychip_CommissionableNodeController_NewController(
            pointer(commissionableNodeCtrl)).raise_on_error()

        self.commissionableNodeCtrl = commissionableNodeCtrl
        self._ChipStack.commissionableNodeCtrl = commissionableNodeCtrl

    def __del__(self):
        if self.commissionableNodeCtrl != None:
            self._dmLib.pychip_CommissionableNodeController_DeleteController(
                self.commissionableNodeCtrl)
            self.commissionableNodeCtrl = None

    def PrintDiscoveredCommissioners(self):
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_CommissionableNodeController_PrintDiscoveredCommissioners(
                self.commissionableNodeCtrl)
        )

    def DiscoverCommissioners(self):
        self._ChipStack.Call(
            lambda: self._dmLib.pychip_CommissionableNodeController_DiscoverCommissioners(
                self.commissionableNodeCtrl)
        ).raise_on_error()

    # ----- Private Members -----
    def _InitLib(self):
        if self._dmLib is None:
            self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

            self._dmLib.pychip_CommissionableNodeController_NewController.argtypes = [
                POINTER(c_void_p)]
            self._dmLib.pychip_CommissionableNodeController_NewController.restype = PyChipError

            self._dmLib.pychip_CommissionableNodeController_DeleteController.argtypes = [
                c_void_p]
            self._dmLib.pychip_CommissionableNodeController_DeleteController.restype = PyChipError

            self._dmLib.pychip_CommissionableNodeController_DiscoverCommissioners.argtypes = [
                c_void_p]
            self._dmLib.pychip_CommissionableNodeController_DiscoverCommissioners.restype = PyChipError

            self._dmLib.pychip_CommissionableNodeController_PrintDiscoveredCommissioners.argtypes = [
                c_void_p]
