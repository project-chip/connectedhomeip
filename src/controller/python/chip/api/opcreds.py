#
#    Copyright (c) 2023 Project CHIP Authors
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

from ctypes import POINTER, Structure, byref, c_bool, c_char_p, c_uint8, c_uint32, c_void_p
from typing import List

from chip import native


class chip_Controller_DeviceCommissioner(c_void_p):
    pass


class pychip_OpCreds_AllocateControllerParams(Structure):
    _fields_ = [
        ('adminVendorId', native.VendorId),
        ('enableServerInteractions', c_bool),
        ('operationalKey', native.pychip_P256Keypair),
        ('noc', POINTER(c_uint8)),
        ('nocLen', c_uint32),
        ('icac', POINTER(c_uint8)),
        ('icacLen', c_uint32),
        ('rcac', POINTER(c_uint8)),
        ('rcacLen', c_uint32),
        ('ipk', POINTER(c_uint8)),
        ('ipkLen', c_uint32),
    ]


class pychip_OpCreds_AllocateCommissionerParams(Structure):
    _fields_ = [
        ('opCredsContext', c_void_p),
        ('fabricId', native.FabricId),
        ('nodeId', native.NodeId),
        ('adminVendorId', native.VendorId),
        ('paaTrustStorePath', c_char_p),
        ('useTestCommissioner', c_bool),
        ('enableServerInteractions', c_bool),
        ('caseAuthTags', POINTER(native.CASEAuthTag)),
        ('caseAuthTagsLen', c_uint32),
        ('operationalKey', native.pychip_P256Keypair),
    ]


@native.native_function
def pychip_OpCreds_AllocateCommissioner(param: POINTER(pychip_OpCreds_AllocateControllerParams), ret_commissioner: POINTER(
    chip_Controller_DeviceCommissioner)) -> native.PyChipError: ...


@native.native_function
def pychip_OpCreds_AllocateController(param: POINTER(pychip_OpCreds_AllocateControllerParams), ret_commissioner: POINTER(
    chip_Controller_DeviceCommissioner)) -> native.PyChipError: ...


def allocate_commissioner(fabricId: int, nodeId: int, adminVendorId: int,
                          opCredsContext: c_void_p,
                          caseAuthTags: List[int] = [],
                          paaTrustStorePath: str = "", useTestCommissioner: bool = True,
                          operationalKey: native.pychip_P256Keypair = None) -> chip_Controller_DeviceCommissioner:
    params = pychip_OpCreds_AllocateCommissionerParams()
    params.opCredsContext = opCredsContext
    params.fabricId = fabricId
    params.nodeId = nodeId
    params.adminVendorId = adminVendorId
    params.caseAuthTags = (c_uint32 * len(caseAuthTags))(*caseAuthTags)
    params.caseAuthTagsLen = len(caseAuthTags)
    params.paaTrustStorePath = paaTrustStorePath.encode()
    params.useTestCommissioner = useTestCommissioner
    params.operationalKey = operationalKey

    ret = chip_Controller_DeviceCommissioner()

    err = pychip_OpCreds_AllocateCommissioner(byref(params), byref(ret))
    err.raise_on_error()

    return ret


def allocate_controller(noc: bytes, icac: bytes, rcac: bytes, ipk: bytes, operationalKey: native.pychip_P256Keypair, adminVendorId: native.VendorId) -> chip_Controller_DeviceCommissioner:
    params = pychip_OpCreds_AllocateControllerParams()
    params.adminVendorId = adminVendorId
    params.noc = noc
    params.nocLen = len(noc)
    params.icac = icac
    params.icacLen = len(icac) if icac else 0
    params.rcac = rcac
    params.rcacLen = len(rcac)
    params.ipk = ipk
    params.ipkLen = len(ipk)
    params.operationalKey = operationalKey

    ret = chip_Controller_DeviceCommissioner()

    err = pychip_OpCreds_AllocateController(byref(params), byref(ret))
    err.raise_on_error()

    return ret
