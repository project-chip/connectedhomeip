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

CHIP_ERROR DeviceProxy::SendCommands(app::CommandSender * commandObj)
{
    VerifyOrReturnLogError(IsSecureConnected(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(GetSecureSession().Value());
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

void DeviceProxy::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                                   Callback::Cancelable * onReportCallback, app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddReportCallback(GetDeviceId(), endpoint, cluster, attribute, onReportCallback, tlvDataFilter);
}

CHIP_ERROR DeviceProxy::SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                                 Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter)
{
    VerifyOrReturnLogError(IsSecureConnected(), CHIP_ERROR_INCORRECT_STATE);

    app::ReadClient * readClient = nullptr;
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewReadClient(
        &readClient, app::ReadClient::InteractionType::Read, &GetInteractionModelDelegate()->GetBufferedCallback()));

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddIMResponseHandler(readClient, onSuccessCallback, onFailureCallback, aTlvDataFilter);
    }
    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    chip::app::ReadPrepareParams readPrepareParams(GetSecureSession().Value());
    readPrepareParams.mpAttributePathParamsList    = &aPath;
    readPrepareParams.mAttributePathParamsListSize = 1;

    CHIP_ERROR err = readClient->SendReadRequest(readPrepareParams);

    if (err != CHIP_NO_ERROR)
    {
        CancelIMResponseHandler(readClient);
    }
    return err;
}

CHIP_ERROR DeviceProxy::SendSubscribeAttributeRequest(app::AttributePathParams aPath, uint16_t mMinIntervalFloorSeconds,
                                                      uint16_t mMaxIntervalCeilingSeconds, Callback::Cancelable * onSuccessCallback,
                                                      Callback::Cancelable * onFailureCallback)
{
    VerifyOrReturnLogError(IsSecureConnected(), CHIP_ERROR_INCORRECT_STATE);

    uint8_t seqNum = GetNextSequenceNumber();

    app::AttributePathParams * path = GetInteractionModelDelegate()->AllocateAttributePathParam(1, seqNum);

    VerifyOrReturnError(path != nullptr, CHIP_ERROR_NO_MEMORY);

    *path = aPath;

    app::ReadClient * readClient = nullptr;
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewReadClient(
        &readClient, app::ReadClient::InteractionType::Subscribe, &GetInteractionModelDelegate()->GetBufferedCallback()));

    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    VerifyOrReturnError(GetSecureSession().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    app::ReadPrepareParams params(GetSecureSession().Value());
    params.mpAttributePathParamsList    = path;
    params.mAttributePathParamsListSize = 1;
    params.mMinIntervalFloorSeconds     = mMinIntervalFloorSeconds;
    params.mMaxIntervalCeilingSeconds   = mMaxIntervalCeilingSeconds;
    params.mKeepSubscriptions           = false;

    CHIP_ERROR err = readClient->SendSubscribeRequest(params);
    if (err != CHIP_NO_ERROR)
    {
        GetInteractionModelDelegate()->FreeAttributePathParam(reinterpret_cast<uint64_t>(readClient));
        readClient->Shutdown();
        return err;
    }

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddIMResponseHandler(readClient, onSuccessCallback, onFailureCallback);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceProxy::SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                                  Callback::Cancelable * onFailureCallback)
{
    VerifyOrReturnLogError(IsSecureConnected(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;

    app::WriteClient * writeClient = aHandle.Get();

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddIMResponseHandler(writeClient, onSuccessCallback, onFailureCallback);
    }
    if ((err = aHandle.SendWriteRequest(GetSecureSession().Value())) != CHIP_NO_ERROR)
    {
        CancelIMResponseHandler(writeClient);
    }
    return err;
}

} // namespace chip
