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
 *  @file
 *    This file contains implementation of Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#include <app/DeviceProxy.h>

#include <app/CommandSender.h>
#include <app/ReadPrepareParams.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::Callback;

namespace chip {

CHIP_ERROR DeviceProxy::SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout)
{
    VerifyOrReturnLogError(IsSecureConnected(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(GetSecureSession().Value(), timeout);
}

void DeviceProxy::AddIMResponseHandler(void * commandObj, Callback::Cancelable * onSuccessCallback,
                                       Callback::Cancelable * onFailureCallback, app::TLVDataFilter tlvDataFilter)
{
    // Interaction model uses the object instead of a sequence number as the identifier of transactions.
    // Since the objects can be identified by its pointer which fits into a uint64 value (the type of NodeId), we use it for the
    // "node id" field in callback manager.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.AddResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */, onSuccessCallback,
                                      onFailureCallback, tlvDataFilter);
}

void DeviceProxy::CancelIMResponseHandler(void * commandObj)
{
    // Interaction model uses the object instead of a sequence number as the identifier of transactions.
    // Since the objects can be identified by its pointer which fits into a uint64 value (the type of NodeId), we use it for the
    // "node id" field in callback manager.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.CancelResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */);
}

} // namespace chip
