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

    std::vector<EndpointId> endpointIds   = { endpointId };
    std::vector<ClusterId> clusterIds     = { clusterId };
    std::vector<AttributeId> attributeIds = { attributeId };
    return InteractionModelReports::ReadAttribute(device, endpointIds, clusterIds, attributeIds, Optional<bool>(fabricFiltered));
}

CHIP_ERROR InteractionModel::ReadEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                       const Optional<EventNumber> & eventNumber)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };
    return InteractionModelReports::ReadEvent(device, endpointIds, clusterIds, eventIds, eventNumber);
}

CHIP_ERROR InteractionModel::SubscribeAttribute(const char * identity, EndpointId endpointId, ClusterId clusterId,
                                                AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                                bool fabricFiltered)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds   = { endpointId };
    std::vector<ClusterId> clusterIds     = { clusterId };
    std::vector<AttributeId> attributeIds = { attributeId };
    return InteractionModelReports::SubscribeAttribute(device, endpointIds, clusterIds, attributeIds, minInterval, maxInterval,
                                                       Optional<bool>(fabricFiltered));
}

CHIP_ERROR InteractionModel::SubscribeEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                            uint16_t minInterval, uint16_t maxInterval, const Optional<EventNumber> & eventNumber)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };
    return InteractionModelReports::SubscribeEvent(device, endpointIds, clusterIds, eventIds, minInterval, maxInterval,
                                                   eventNumber);
}

void InteractionModel::Shutdown()
{
    InteractionModelReports::Shutdown();
    InteractionModelWriter::Shutdown();
    InteractionModelCommands::Shutdown();
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
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

void InteractionModel::OnSubscriptionEstablished(SubscriptionId subscriptionId)
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
    if (mCommandSender.size())
    {
        mCommandSender.front().reset();
        mCommandSender.erase(mCommandSender.begin());
    }

    // If the command is repeated N times, wait for all the responses to comes in
    // before exiting.
    if (!mCommandSender.size())
    {
        ContinueOnChipMainThread(CHIP_NO_ERROR);
    }
}

CHIP_ERROR InteractionModelReports::ReportAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                                                    std::vector<chip::ClusterId> clusterIds,
                                                    std::vector<chip::AttributeId> attributeIds,
                                                    chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval,
                                                    uint16_t maxInterval, const chip::Optional<bool> & fabricFiltered,
                                                    const chip::Optional<std::vector<chip::DataVersion>> & dataVersions,
                                                    const chip::Optional<bool> & keepSubscriptions)
{
    const size_t clusterCount      = clusterIds.size();
    const size_t attributeCount    = attributeIds.size();
    const size_t endpointCount     = endpointIds.size();
    const size_t dataVersionsCount = dataVersions.HasValue() ? dataVersions.Value().size() : 0;

    VerifyOrReturnError(clusterCount > 0 && clusterCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(attributeCount > 0 && attributeCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(endpointCount > 0 && endpointCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(dataVersionsCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);

    const bool hasSameIdsCount = (clusterCount == attributeCount) && (clusterCount == endpointCount) &&
        (dataVersionsCount == 0 || clusterCount == dataVersionsCount);
    const bool multipleClusters =
        clusterCount > 1 && attributeCount == 1 && endpointCount == 1 && (dataVersionsCount == 0 || dataVersionsCount == 1);
    const bool multipleAttributes =
        attributeCount > 1 && clusterCount == 1 && endpointCount == 1 && (dataVersionsCount == 0 || dataVersionsCount == 1);
    const bool multipleEndpoints =
        endpointCount > 1 && clusterCount == 1 && attributeCount == 1 && (dataVersionsCount == 0 || dataVersionsCount == 1);
    const bool multipleDataVersions = dataVersionsCount > 1 && clusterCount == 1 && attributeCount == 1 && endpointCount == 1;

    size_t pathsCount = 0;
    if (hasSameIdsCount)
    {
        pathsCount = clusterCount;
    }
    else if (multipleClusters)
    {
        pathsCount = clusterCount;
    }
    else if (multipleAttributes)
    {
        pathsCount = attributeCount;
    }
    else if (multipleEndpoints)
    {
        pathsCount = endpointCount;
    }
    else if (multipleDataVersions)
    {
        pathsCount = dataVersionsCount;
    }
    else
    {
        ChipLogError(chipTool,
                     "\n%sAttribute commands targetting multiple paths needs to have: \n \t * One element with multiple ids (for "
                     "example 1 cluster id, 1 attribute id, 2 endpoint ids)\n\t * Or the same "
                     "number of ids (for examples 2 cluster ids, 2 attribute ids and 2 endpoint ids).\n The current command has %u "
                     "cluster ids, %u attribute ids, %u endpoint ids.",
                     interactionType == chip::app::ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read",
                     static_cast<unsigned int>(clusterCount), static_cast<unsigned int>(attributeCount),
                     static_cast<unsigned int>(endpointCount));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ChipLogProgress(chipTool, "Sending %sAttribute to:",
                    interactionType == chip::app::ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read");

    chip::app::AttributePathParams attributePathParams[kMaxAllowedPaths];
    chip::app::DataVersionFilter dataVersionFilter[kMaxAllowedPaths];
    for (size_t i = 0; i < pathsCount; i++)
    {
        chip::ClusterId clusterId     = clusterIds.at((hasSameIdsCount || multipleClusters) ? i : 0);
        chip::AttributeId attributeId = attributeIds.at((hasSameIdsCount || multipleAttributes) ? i : 0);
        chip::EndpointId endpointId   = endpointIds.at((hasSameIdsCount || multipleEndpoints) ? i : 0);

        ChipLogProgress(chipTool, "\tcluster " ChipLogFormatMEI ", attribute: " ChipLogFormatMEI ", endpoint %u",
                        ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId), endpointId);

        if (clusterId != kInvalidClusterId)
        {
            attributePathParams[i].mClusterId = clusterId;
        }

        if (attributeId != kInvalidAttributeId)
        {
            attributePathParams[i].mAttributeId = attributeId;
        }

        if (endpointId != kInvalidEndpointId)
        {
            attributePathParams[i].mEndpointId = endpointId;
        }

        if (dataVersions.HasValue())
        {
            chip::DataVersion dataVersion    = dataVersions.Value().at((hasSameIdsCount || multipleDataVersions) ? i : 0);
            dataVersionFilter[i].mEndpointId = endpointId;
            dataVersionFilter[i].mClusterId  = clusterId;
            dataVersionFilter[i].mDataVersion.SetValue(dataVersion);
        }
    }

    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = pathsCount;

    if (fabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = fabricFiltered.Value();
    }

    if (dataVersions.HasValue())
    {
        params.mpDataVersionFilterList    = dataVersionFilter;
        params.mDataVersionFilterListSize = pathsCount;
    }

    if (interactionType == chip::app::ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = minInterval;
        params.mMaxIntervalCeilingSeconds = maxInterval;
        if (keepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = keepSubscriptions.Value();
        }
    }

    auto & client = interactionType == chip::app::ReadClient::InteractionType::Subscribe ? mSubscribeClient : mReadClient;
    client = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                     mBufferedReadAdapter, interactionType);
    return client->SendRequest(params);
}

CHIP_ERROR InteractionModelReports::ReportEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                                                std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds,
                                                chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval,
                                                uint16_t maxInterval, const chip::Optional<chip::EventNumber> & eventNumber,
                                                const chip::Optional<bool> & keepSubscriptions)
{
    const size_t clusterCount  = clusterIds.size();
    const size_t eventCount    = eventIds.size();
    const size_t endpointCount = endpointIds.size();

    VerifyOrReturnError(clusterCount > 0 && clusterCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(eventCount > 0 && eventCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(endpointCount > 0 && endpointCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);

    const bool hasSameIdsCount   = (clusterCount == eventCount) && (clusterCount == endpointCount);
    const bool multipleClusters  = clusterCount > 1 && eventCount == 1 && endpointCount == 1;
    const bool multipleEvents    = eventCount > 1 && clusterCount == 1 && endpointCount == 1;
    const bool multipleEndpoints = endpointCount > 1 && clusterCount == 1 && eventCount == 1;

    size_t pathsCount = 0;
    if (hasSameIdsCount)
    {
        pathsCount = clusterCount;
    }
    else if (multipleClusters)
    {
        pathsCount = clusterCount;
    }
    else if (multipleEvents)
    {
        pathsCount = eventCount;
    }
    else if (multipleEndpoints)
    {
        pathsCount = endpointCount;
    }
    else
    {
        ChipLogError(chipTool,
                     "\n%sEvent command targetting multiple paths needs to have: \n \t * One element with multiple ids (for "
                     "example 1 cluster id, 1 event id, 2 endpoint ids)\n\t * Or the same "
                     "number of ids (for examples 2 cluster ids, 2 event ids and 2 endpoint ids).\n The current command has %u "
                     "cluster ids, %u event ids, %u endpoint ids.",
                     interactionType == chip::app::ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read",
                     static_cast<unsigned int>(clusterCount), static_cast<unsigned int>(eventCount),
                     static_cast<unsigned int>(endpointCount));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    chip::app::EventPathParams eventPathParams[kMaxAllowedPaths];

    ChipLogProgress(chipTool, "Sending %sEvent to:",
                    interactionType == chip::app::ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read");
    for (size_t i = 0; i < pathsCount; i++)
    {
        chip::ClusterId clusterId   = clusterIds.at((hasSameIdsCount || multipleClusters) ? i : 0);
        chip::EventId eventId       = eventIds.at((hasSameIdsCount || multipleEvents) ? i : 0);
        chip::EndpointId endpointId = endpointIds.at((hasSameIdsCount || multipleEndpoints) ? i : 0);

        ChipLogProgress(chipTool, "\tcluster " ChipLogFormatMEI ", event: " ChipLogFormatMEI ", endpoint %u",
                        ChipLogValueMEI(clusterId), ChipLogValueMEI(eventId), endpointId);
        if (clusterId != chip::kInvalidClusterId)
        {
            eventPathParams[i].mClusterId = clusterId;
        }

        if (eventId != chip::kInvalidEventId)
        {
            eventPathParams[i].mEventId = eventId;
        }

        if (endpointId != chip::kInvalidEndpointId)
        {
            eventPathParams[i].mEndpointId = endpointId;
        }
    }

    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = pathsCount;
    params.mEventNumber                 = eventNumber;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;

    if (interactionType == chip::app::ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = minInterval;
        params.mMaxIntervalCeilingSeconds = maxInterval;
        if (keepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = keepSubscriptions.Value();
        }
    }

    auto & client = interactionType == chip::app::ReadClient::InteractionType::Subscribe ? mSubscribeClient : mReadClient;
    client = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                     mBufferedReadAdapter, interactionType);
    return client->SendRequest(params);
}
