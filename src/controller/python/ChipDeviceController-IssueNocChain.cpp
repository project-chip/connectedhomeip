/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <map>
#include <string>

#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

typedef void PyObject;

using namespace chip;

extern "C" {

typedef void (*pychip_DeviceController_IssueNOCChainCallbackPythonCallback)(
    PyObject * context, ChipError::StorageType status, const uint8_t * noc, size_t nocLen, const uint8_t * icac, size_t icacLen,
    const uint8_t * rcac, size_t rcacLen, const uint8_t * ipk, size_t ipkLen, NodeId adminSubject);

static pychip_DeviceController_IssueNOCChainCallbackPythonCallback pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct;

void pychip_DeviceController_SetIssueNOCChainCallbackPythonCallback(
    pychip_DeviceController_IssueNOCChainCallbackPythonCallback callback)
{
    pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct = callback;
}

ChipError::StorageType pychip_DeviceController_IssueNOCChain(chip::Controller::DeviceCommissioner * devCtrl,
                                                             PyObject * pythonContext, uint8_t * NOCSRElements,
                                                             size_t NOCSRElementsLen, NodeId nodeId);
}

void pychip_DeviceController_IssueNOCChainCallback(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                                   const ByteSpan & rcac, Optional<Crypto::AesCcm128KeySpan> ipk,
                                                   Optional<NodeId> adminSubject)
{
    if (pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct != nullptr)
    {
        Crypto::AesCcm128KeySpan ipkData;
        ipkData = ipk.ValueOr(Crypto::AesCcm128KeySpan());
        pychip_DeviceController_IssueNOCChainCallbackPythonCallbackFunct(
            context, status.AsInteger(), noc.data(), noc.size(), icac.data(), icac.size(), rcac.data(), rcac.size(), ipkData.data(),
            ipk.HasValue() ? ipkData.size() : 0, adminSubject.ValueOr(kUndefinedNodeId));
    }
}

ChipError::StorageType pychip_DeviceController_IssueNOCChain(chip::Controller::DeviceCommissioner * devCtrl,
                                                             PyObject * pythonContext, uint8_t * NOCSRElements,
                                                             size_t NOCSRElementsLen, NodeId nodeId)
{
    return devCtrl
        ->IssueNOCChain(
            ByteSpan(NOCSRElements, NOCSRElementsLen), nodeId,
            /* Note: Memory leak here. This is a quick and a bit dirty PoC */
            new Callback::Callback<Controller::OnNOCChainGeneration>(pychip_DeviceController_IssueNOCChainCallback, pythonContext))
        .AsInteger();
}
