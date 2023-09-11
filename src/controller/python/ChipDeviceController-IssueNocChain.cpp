/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <map>
#include <string>

#include <controller/CHIPDeviceController.h>
#include <controller/python/chip/native/PyChipError.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::Credentials;

typedef void PyObject;

using namespace chip;

extern "C" {

typedef void (*pychip_DeviceController_IssueNOCChainCallbackPythonCallback)(
    PyObject * context, PyChipError status, const uint8_t * noc, size_t nocLen, const uint8_t * icac, size_t icacLen,
    const uint8_t * rcac, size_t rcacLen, const uint8_t * ipk, size_t ipkLen, NodeId adminSubject);

static pychip_DeviceController_IssueNOCChainCallbackPythonCallback pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct;

void pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback(
    pychip_DeviceController_IssueNOCChainCallbackPythonCallback callback)
{
    pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct = callback;
}

PyChipError pychip_DeviceController_IssueNOCChain(chip::Controller::DeviceCommissioner * devCtrl, PyObject * pythonContext,
                                                  uint8_t * NOCSRElements, size_t NOCSRElementsLen, NodeId nodeId);
}

void pychip_DeviceController_IssueNOCChainCallback(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                                   const ByteSpan & rcac, Optional<Crypto::IdentityProtectionKeySpan> ipk,
                                                   Optional<NodeId> adminSubject)
{
    if (pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct == nullptr)
    {
        return;
    }

    chip::Platform::ScopedMemoryBuffer<uint8_t> chipNoc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> chipIcac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> chipRcac;
    MutableByteSpan chipNocSpan;
    MutableByteSpan chipIcacSpan;
    MutableByteSpan chipRcacSpan;

    Crypto::IdentityProtectionKeySpan ipkData;
    ipkData = ipk.ValueOr(Crypto::IdentityProtectionKeySpan());

    CHIP_ERROR err = status;
    if (err != CHIP_NO_ERROR)
    {
        ExitNow();
    }
    VerifyOrExit(chipNoc.Alloc(Credentials::kMaxCHIPCertLength), err = CHIP_ERROR_NO_MEMORY);
    chipNocSpan = MutableByteSpan(chipNoc.Get(), Credentials::kMaxCHIPCertLength);

    VerifyOrExit(chipIcac.Alloc(Credentials::kMaxCHIPCertLength), err = CHIP_ERROR_NO_MEMORY);
    chipIcacSpan = MutableByteSpan(chipIcac.Get(), Credentials::kMaxCHIPCertLength);

    VerifyOrExit(chipRcac.Alloc(Credentials::kMaxCHIPCertLength), err = CHIP_ERROR_NO_MEMORY);
    chipRcacSpan = MutableByteSpan(chipRcac.Get(), Credentials::kMaxCHIPCertLength);

    SuccessOrExit(err = ConvertX509CertToChipCert(noc, chipNocSpan));
    SuccessOrExit(err = ConvertX509CertToChipCert(icac, chipIcacSpan));
    SuccessOrExit(err = ConvertX509CertToChipCert(rcac, chipRcacSpan));

exit:
    if (err == CHIP_NO_ERROR)
    {
        pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct(
            context, ToPyChipError(err), chipNocSpan.data(), chipNocSpan.size(), chipIcacSpan.data(), chipIcacSpan.size(),
            chipRcacSpan.data(), chipRcacSpan.size(), ipkData.data(), ipk.HasValue() ? ipkData.size() : 0,
            adminSubject.ValueOr(kUndefinedNodeId));
    }
    else
    {
        pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct(context, ToPyChipError(err), nullptr, 0, nullptr, 0,
                                                                         nullptr, 0, nullptr, 0, 0);
    }
}

PyChipError pychip_DeviceController_IssueNOCChain(chip::Controller::DeviceCommissioner * devCtrl, PyObject * pythonContext,
                                                  uint8_t * NOCSRElements, size_t NOCSRElementsLen, NodeId nodeId)
{
    return ToPyChipError(devCtrl->IssueNOCChain(
        ByteSpan(NOCSRElements, NOCSRElementsLen), nodeId,
        /* Note: Memory leak here. This is a quick and a bit dirty PoC */
        new Callback::Callback<Controller::OnNOCChainGeneration>(pychip_DeviceController_IssueNOCChainCallback, pythonContext)));
}
