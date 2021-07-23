/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file defines Base class for a CHIP IM Command
 *
 */

#include "DemuxedInvokeInitiator.h"

namespace chip {
namespace app {

DemuxedInvokeInitiator::DemuxedInvokeInitiator(onDoneFuncDef onDoneFunc)
   : mOnDoneFunc(onDoneFunc)
{
}

CHIP_ERROR DemuxedInvokeInitiator::AddCommand(IEncodableElement *request, CommandParams params,
                  std::function<void (DemuxedInvokeInitiator& invokeInitiator, CommandParams&)> onDataFunc,
                  std::function<void (DemuxedInvokeInitiator& invokeInitiator, CHIP_ERROR error, StatusResponse *response)> onErrorFunc)
{
    auto onDataClosure = [onDataFunc](DemuxedInvokeInitiator& initiator, CommandParams &pparams, TLV::TLVReader *reader) {
        onDataFunc(initiator, pparams);
    };

    mHandlers.push_back({onDataClosure, onErrorFunc, params.ClusterId, params.CommandId});
    return mInitiator.AddRequest(params, request);
}

void DemuxedInvokeInitiator::OnResponse(InvokeInitiator &initiator, CommandParams &params, TLV::TLVReader *payload)
{
    bool foundMatch = false;

    for (auto iter : mHandlers) {
        if (iter.clusterId == params.ClusterId && iter.commandId == params.CommandId) {
            iter.onDataClosure(*this, params, payload);
            return;
        }
    }

    if (!foundMatch) {
        ChipLogProgress(DataManagement, "Could not find a matching demuxed handler for command! (ClusterId = %" PRIx32 ", Endpoint = %lu, Command = %lu)",
                        params.ClusterId, (unsigned long)params.EndpointId, (unsigned long)params.CommandId);
    }
}

void DemuxedInvokeInitiator::OnError(InvokeInitiator &initiator, CommandParams *params, CHIP_ERROR error, StatusResponse *statusResponse)
{
    for (auto iter : mHandlers) {
        if ((params && (iter.clusterId == params->ClusterId && iter.commandId == params->CommandId)) || (!params)) {
            iter.onErrorFunc(*this, error, statusResponse);
            return;
        }
    }
}

void DemuxedInvokeInitiator::OnEnd(InvokeInitiator &initiator)
{
    mOnDoneFunc(*this);
}

} // namespace app
} // namespace chip
