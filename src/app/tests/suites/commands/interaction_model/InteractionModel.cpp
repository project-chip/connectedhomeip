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

CHIP_ERROR InteractionModel::ReadAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                           chip::AttributeId attributeId, bool fabricFiltered)
{
    chip::DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId  = endpointId;
    attributePathParams[0].mClusterId   = clusterId;
    attributePathParams[0].mAttributeId = attributeId;

    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = 1;
    params.mIsFabricFiltered            = fabricFiltered;

    mReadClient =
        std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                mBufferedReadAdapter, chip::app::ReadClient::InteractionType::Read);
    return mReadClient->SendRequest(params);
}

CHIP_ERROR InteractionModel::ReadEvent(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                       chip::EventId eventId)
{
    chip::DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    chip::app::EventPathParams eventPathParams[1];
    eventPathParams[0].mEndpointId = endpointId;
    eventPathParams[0].mClusterId  = clusterId;
    eventPathParams[0].mEventId    = eventId;

    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = 1;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;

    // TODO: Add data version supports
    mReadClient =
        std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                mBufferedReadAdapter, chip::app::ReadClient::InteractionType::Subscribe);
    VerifyOrReturnError(mReadClient != nullptr, CHIP_ERROR_NO_MEMORY);

    return mReadClient->SendRequest(params);
}

CHIP_ERROR InteractionModel::SubscribeAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                chip::AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                                bool fabricFiltered)
{
    chip::DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    chip::app::AttributePathParams attributePathParams[1];
    attributePathParams[0].mEndpointId  = endpointId;
    attributePathParams[0].mClusterId   = clusterId;
    attributePathParams[0].mAttributeId = attributeId;

    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = 1;
    params.mMinIntervalFloorSeconds     = minInterval;
    params.mMaxIntervalCeilingSeconds   = maxInterval;
    params.mIsFabricFiltered            = fabricFiltered;

    mSubscribeClient =
        std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                mBufferedReadAdapter, chip::app::ReadClient::InteractionType::Subscribe);
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
void InteractionModel::OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                                       const chip::app::StatusIB & status)
{
    OnResponse(status, data);
}

void InteractionModel::OnEventData(const chip::app::EventHeader & eventHeader, chip::TLV::TLVReader * data,
                                   const chip::app::StatusIB * status)
{
    OnResponse(*status, data);
}

void InteractionModel::OnError(CHIP_ERROR error)
{
    chip::app::StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone()
{
    mReadClient.reset();
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

void InteractionModel::OnSubscriptionEstablished(uint64_t subscriptionId)
{
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

/////////// WriteClient Callback Interface /////////
void InteractionModel::OnResponse(const chip::app::WriteClient * client, const chip::app::ConcreteDataAttributePath & path,
                                  chip::app::StatusIB status)
{
    OnResponse(status, nullptr);
}

void InteractionModel::OnError(const chip::app::WriteClient * client, CHIP_ERROR error)
{
    chip::app::StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone(chip::app::WriteClient * client)
{
    mWriteClient.reset();
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

/////////// CommandSender Callback Interface /////////
void InteractionModel::OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                                  const chip::app::StatusIB & status, chip::TLV::TLVReader * data)
{
    OnResponse(status, data);
}

void InteractionModel::OnError(const chip::app::CommandSender * client, CHIP_ERROR error)
{
    chip::app::StatusIB status(error);
    OnResponse(status, nullptr);
}

void InteractionModel::OnDone(chip::app::CommandSender * client)
{
    mCommandSender.reset();
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}
