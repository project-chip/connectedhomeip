/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "InteractionModel.h"

using namespace chip;
using namespace chip::app;

CHIP_ERROR InteractionModel::ReadAttribute(const char * identity, EndpointId endpointId, ClusterId clusterId,
                                           AttributeId attributeId, bool fabricFiltered)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    AttributePathParams attributePathParams[1];
    if (endpointId != kInvalidEndpointId)
    {
        attributePathParams[0].mEndpointId = endpointId;
    }

    if (clusterId != kInvalidClusterId)
    {
        attributePathParams[0].mClusterId = clusterId;
    }

    if (attributeId != kInvalidAttributeId)
    {
        attributePathParams[0].mAttributeId = attributeId;
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = 1;
    params.mIsFabricFiltered            = fabricFiltered;

    mReadClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, ReadClient::InteractionType::Read);
    return mReadClient->SendRequest(params);
}

CHIP_ERROR InteractionModel::ReadEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                       bool fabricFiltered)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    EventPathParams eventPathParams[1];
    if (endpointId != kInvalidEndpointId)
    {
        eventPathParams[0].mEndpointId = endpointId;
    }

    if (clusterId != kInvalidClusterId)
    {
        eventPathParams[0].mClusterId = clusterId;
    }

    if (eventId != kInvalidEventId)
    {
        eventPathParams[0].mEventId = eventId;
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = 1;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;
    params.mIsFabricFiltered            = fabricFiltered;

    // TODO: Add data version supports
    mReadClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, ReadClient::InteractionType::Read);
    VerifyOrReturnError(mReadClient != nullptr, CHIP_ERROR_NO_MEMORY);

    return mReadClient->SendRequest(params);
}

CHIP_ERROR InteractionModel::SubscribeAttribute(const char * identity, EndpointId endpointId, ClusterId clusterId,
                                                AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                                bool fabricFiltered)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    AttributePathParams attributePathParams[1];
    if (endpointId != kInvalidEndpointId)
    {
        attributePathParams[0].mEndpointId = endpointId;
    }

    if (clusterId != kInvalidClusterId)
    {
        attributePathParams[0].mClusterId = clusterId;
    }

    if (attributeId != kInvalidAttributeId)
    {
        attributePathParams[0].mAttributeId = attributeId;
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = 1;
    params.mMinIntervalFloorSeconds     = minInterval;
    params.mMaxIntervalCeilingSeconds   = maxInterval;
    params.mIsFabricFiltered            = fabricFiltered;

    mSubscribeClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                    mBufferedReadAdapter, ReadClient::InteractionType::Subscribe);
    VerifyOrReturnError(mSubscribeClient != nullptr, CHIP_ERROR_NO_MEMORY);

    return mSubscribeClient->SendRequest(params);
}

CHIP_ERROR InteractionModel::SubscribeEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                            uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    EventPathParams eventPathParams[1];

    if (endpointId != kInvalidEndpointId)
    {
        eventPathParams[0].mEndpointId = endpointId;
    }

    if (clusterId != kInvalidClusterId)
    {
        eventPathParams[0].mClusterId = clusterId;
    }

    if (eventId != kInvalidEventId)
    {
        eventPathParams[0].mEventId = eventId;
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = 1;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;
    params.mMinIntervalFloorSeconds     = minInterval;
    params.mMaxIntervalCeilingSeconds   = maxInterval;
    params.mIsFabricFiltered            = fabricFiltered;

    mSubscribeClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                    mBufferedReadAdapter, ReadClient::InteractionType::Subscribe);
    VerifyOrReturnError(mSubscribeClient != nullptr, CHIP_ERROR_NO_MEMORY);

    return mSubscribeClient->SendRequest(params);
}

void InteractionModel::Shutdown()
{
    mReadClient.reset();
    mSubscribeClient.reset();
    mWriteClient.reset();
    mCommandSender.reset();
}

/////////// ReadClient Callback Interface /////////
void InteractionModel::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    OnResponse(status, data);
}

void InteractionModel::OnEventData(const EventHeader & eventHeader, TLV::TLVReader * data, const StatusIB * status)
{
    OnResponse(status == nullptr ? StatusIB() : *status, data);
}

void InteractionModel::OnError(CHIP_ERROR error)
{
    StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone()
{
    mReadClient.reset();
    // TODO: Close the subscribe client
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

void InteractionModel::OnSubscriptionEstablished(uint64_t subscriptionId)
{
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

/////////// WriteClient Callback Interface /////////
void InteractionModel::OnResponse(const WriteClient * client, const ConcreteDataAttributePath & path, StatusIB status)
{
    OnResponse(status, nullptr);
}

void InteractionModel::OnError(const WriteClient * client, CHIP_ERROR error)
{
    StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone(WriteClient * client)
{
    mWriteClient.reset();
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

/////////// CommandSender Callback Interface /////////
void InteractionModel::OnResponse(CommandSender * client, const ConcreteCommandPath & path, const StatusIB & status,
                                  TLV::TLVReader * data)
{
    OnResponse(status, data);
}

void InteractionModel::OnError(const CommandSender * client, CHIP_ERROR error)
{
    StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone(CommandSender * client)
{
    mCommandSender.reset();
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}
