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
                                           AttributeId attributeId, bool fabricFiltered, const Optional<DataVersion> & dataVersion)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds   = { endpointId };
    std::vector<ClusterId> clusterIds     = { clusterId };
    std::vector<AttributeId> attributeIds = { attributeId };

    Optional<std::vector<DataVersion>> dataVersions = Optional<std::vector<DataVersion>>();
    if (dataVersion.HasValue())
    {
        dataVersions.Value().push_back(dataVersion.Value());
    }

    return InteractionModelReports::ReadAttribute(device, endpointIds, clusterIds, attributeIds, Optional<bool>(fabricFiltered),
                                                  dataVersions);
}

CHIP_ERROR InteractionModel::ReadEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                       bool fabricFiltered, const Optional<EventNumber> & eventNumber)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };
    return InteractionModelReports::ReadEvent(device, endpointIds, clusterIds, eventIds, Optional<bool>(fabricFiltered),
                                              eventNumber);
}

CHIP_ERROR InteractionModel::SubscribeAttribute(const char * identity, EndpointId endpointId, ClusterId clusterId,
                                                AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                                bool fabricFiltered, const Optional<DataVersion> & dataVersion,
                                                const Optional<bool> & keepSubscriptions)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds   = { endpointId };
    std::vector<ClusterId> clusterIds     = { clusterId };
    std::vector<AttributeId> attributeIds = { attributeId };

    Optional<std::vector<DataVersion>> dataVersions = Optional<std::vector<DataVersion>>();
    if (dataVersion.HasValue())
    {
        dataVersions.Value().push_back(dataVersion.Value());
    }

    return InteractionModelReports::SubscribeAttribute(device, endpointIds, clusterIds, attributeIds, minInterval, maxInterval,
                                                       Optional<bool>(fabricFiltered), dataVersions, keepSubscriptions);
}

CHIP_ERROR InteractionModel::SubscribeEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                            uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered,
                                            const Optional<EventNumber> & eventNumber, const Optional<bool> & keepSubscriptions)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };
    return InteractionModelReports::SubscribeEvent(device, endpointIds, clusterIds, eventIds, minInterval, maxInterval,
                                                   Optional<bool>(fabricFiltered), eventNumber, keepSubscriptions);
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

void InteractionModel::OnDone(ReadClient * aReadClient)
{
    InteractionModelReports::CleanupReadClient(aReadClient);
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

void InteractionModel::OnSubscriptionEstablished(SubscriptionId subscriptionId)
{
    ContinueOnChipMainThread(CHIP_NO_ERROR);
}

void InteractionModel::OnResubscriptionAttempt(CHIP_ERROR aTerminationCause, uint32_t aNextResubscribeIntervalMsec) {}

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

CHIP_ERROR InteractionModelConfig::GetAttributePaths(std::vector<EndpointId> endpointIds, std::vector<ClusterId> clusterIds,
                                                     std::vector<AttributeId> attributeIds,
                                                     const Optional<std::vector<DataVersion>> & dataVersions,
                                                     AttributePathsConfig & pathsConfig)
{
    const size_t endpointCount     = endpointIds.size();
    const size_t clusterCount      = clusterIds.size();
    const size_t attributeCount    = attributeIds.size();
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
                     "\nCommand targetting multiple paths needs to have: \n \t * One element with multiple ids (for "
                     "example 1 cluster id, 1 attribute id, 2 endpoint ids)\n\t * Or the same "
                     "number of ids (for examples 2 cluster ids, 2 attribute ids and 2 endpoint ids).\n The current command has %u "
                     "cluster ids, %u attribute ids, %u endpoint ids.",
                     static_cast<unsigned int>(clusterCount), static_cast<unsigned int>(attributeCount),
                     static_cast<unsigned int>(endpointCount));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    pathsConfig.count = pathsCount;

    for (size_t i = 0; i < pathsCount; i++)
    {
        ClusterId clusterId     = clusterIds.at((hasSameIdsCount || multipleClusters) ? i : 0);
        AttributeId attributeId = attributeIds.at((hasSameIdsCount || multipleAttributes) ? i : 0);
        EndpointId endpointId   = endpointIds.at((hasSameIdsCount || multipleEndpoints) ? i : 0);

        ChipLogProgress(chipTool, "\tcluster " ChipLogFormatMEI ", attribute: " ChipLogFormatMEI ", endpoint %u",
                        ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId), endpointId);

        if (clusterId != kInvalidClusterId)
        {
            pathsConfig.attributePathParams[i].mClusterId = clusterId;
        }

        if (attributeId != kInvalidAttributeId)
        {
            pathsConfig.attributePathParams[i].mAttributeId = attributeId;
        }

        if (endpointId != kInvalidEndpointId)
        {
            pathsConfig.attributePathParams[i].mEndpointId = endpointId;
        }

        if (dataVersions.HasValue())
        {
            DataVersion dataVersion = dataVersions.Value().at((hasSameIdsCount || multipleDataVersions) ? i : 0);
            pathsConfig.dataVersionFilter[i].mEndpointId = endpointId;
            pathsConfig.dataVersionFilter[i].mClusterId  = clusterId;
            pathsConfig.dataVersionFilter[i].mDataVersion.SetValue(dataVersion);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelReports::ReportAttribute(DeviceProxy * device, std::vector<EndpointId> endpointIds,
                                                    std::vector<ClusterId> clusterIds, std::vector<AttributeId> attributeIds,
                                                    ReadClient::InteractionType interactionType, uint16_t minInterval,
                                                    uint16_t maxInterval, const Optional<bool> & fabricFiltered,
                                                    const Optional<std::vector<DataVersion>> & dataVersions,
                                                    const Optional<bool> & keepSubscriptions)
{
    InteractionModelConfig::AttributePathsConfig pathsConfig;
    ReturnErrorOnFailure(
        InteractionModelConfig::GetAttributePaths(endpointIds, clusterIds, attributeIds, dataVersions, pathsConfig));

    ChipLogProgress(chipTool,
                    "Sending %sAttribute to:", interactionType == ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read");

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = pathsConfig.attributePathParams;
    params.mAttributePathParamsListSize = pathsConfig.count;

    if (fabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = fabricFiltered.Value();
    }

    if (dataVersions.HasValue())
    {
        params.mpDataVersionFilterList    = pathsConfig.dataVersionFilter;
        params.mDataVersionFilterListSize = pathsConfig.count;
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = minInterval;
        params.mMaxIntervalCeilingSeconds = maxInterval;
        if (keepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = keepSubscriptions.Value();
        }
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    if (interactionType == ReadClient::InteractionType::Read)
    {
        ReturnErrorOnFailure(client->SendRequest(params));
    }
    else
    {
        // We want to allow certain kinds of spec-invalid subscriptions so we
        // can test how the server reacts to them.
        ReturnErrorOnFailure(client->SendSubscribeRequestWithoutValidation(params));
    }
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelReports::ReportEvent(DeviceProxy * device, std::vector<EndpointId> endpointIds,
                                                std::vector<ClusterId> clusterIds, std::vector<EventId> eventIds,
                                                ReadClient::InteractionType interactionType, uint16_t minInterval,
                                                uint16_t maxInterval, const Optional<bool> & fabricFiltered,
                                                const Optional<EventNumber> & eventNumber, const Optional<bool> & keepSubscriptions,
                                                const Optional<std::vector<bool>> & isUrgents)
{
    const size_t clusterCount  = clusterIds.size();
    const size_t eventCount    = eventIds.size();
    const size_t endpointCount = endpointIds.size();
    const size_t isUrgentCount = isUrgents.HasValue() ? isUrgents.Value().size() : 0;

    VerifyOrReturnError(clusterCount > 0 && clusterCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(eventCount > 0 && eventCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(endpointCount > 0 && endpointCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isUrgentCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);

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
                     interactionType == ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read",
                     static_cast<unsigned int>(clusterCount), static_cast<unsigned int>(eventCount),
                     static_cast<unsigned int>(endpointCount));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    EventPathParams eventPathParams[kMaxAllowedPaths];

    ChipLogProgress(chipTool,
                    "Sending %sEvent to:", interactionType == ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read");
    for (size_t i = 0; i < pathsCount; i++)
    {
        ClusterId clusterId   = clusterIds.at((hasSameIdsCount || multipleClusters) ? i : 0);
        EventId eventId       = eventIds.at((hasSameIdsCount || multipleEvents) ? i : 0);
        EndpointId endpointId = endpointIds.at((hasSameIdsCount || multipleEndpoints) ? i : 0);

        ChipLogProgress(chipTool, "\tcluster " ChipLogFormatMEI ", event: " ChipLogFormatMEI ", endpoint %u",
                        ChipLogValueMEI(clusterId), ChipLogValueMEI(eventId), endpointId);
        if (clusterId != kInvalidClusterId)
        {
            eventPathParams[i].mClusterId = clusterId;
        }

        if (eventId != kInvalidEventId)
        {
            eventPathParams[i].mEventId = eventId;
        }

        if (endpointId != kInvalidEndpointId)
        {
            eventPathParams[i].mEndpointId = endpointId;
        }

        if (isUrgents.HasValue() && isUrgents.Value().size() > i)
        {
            eventPathParams[i].mIsUrgentEvent = isUrgents.Value().at(i);
        }
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = pathsCount;
    params.mEventNumber                 = eventNumber;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;

    if (fabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = fabricFiltered.Value();
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = minInterval;
        params.mMaxIntervalCeilingSeconds = maxInterval;
        if (keepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = keepSubscriptions.Value();
        }
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    ReturnErrorOnFailure(client->SendRequest(params));
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}

void InteractionModelReports::CleanupReadClient(ReadClient * aReadClient)
{
    mReadClients.erase(
        std::remove_if(mReadClients.begin(), mReadClients.end(), [aReadClient](auto & item) { return item.get() == aReadClient; }),
        mReadClients.end());
}

CHIP_ERROR InteractionModelReports::ReadAll(DeviceProxy * device, std::vector<EndpointId> endpointIds,
                                            const Optional<bool> & fabricFiltered)
{
    AttributePathParams attributePathParams[kMaxAllowedPaths];
    EventPathParams eventPathParams[kMaxAllowedPaths];

    auto pathsCount = endpointIds.size();
    VerifyOrReturnError(pathsCount > 0 && pathsCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);

    for (size_t i = 0; i < pathsCount; i++)
    {
        auto endpointId                    = endpointIds.at(i);
        attributePathParams[i].mEndpointId = endpointId;
        eventPathParams[i].mEndpointId     = endpointId;
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = pathsCount;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = pathsCount;

    if (fabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = fabricFiltered.Value();
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, ReadClient::InteractionType::Read);
    ReturnErrorOnFailure(client->SendRequest(params));
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}
