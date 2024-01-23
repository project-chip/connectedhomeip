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

namespace chip {
namespace test_utils {
void BusyWaitMillis(uint16_t busyWaitForMs)
{
    auto & clock = chip::System::SystemClock();
    auto start   = clock.GetMonotonicTimestamp();
    chip::System::Clock::Milliseconds32 durationInMs(busyWaitForMs);
    while (clock.GetMonotonicTimestamp() - start < durationInMs)
    {
        // nothing to do.
    };
}
} // namespace test_utils
} // namespace chip

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

    InteractionModelReports::ResetOptions();
    InteractionModelReports::SetFabricFiltered(fabricFiltered);
    InteractionModelReports::SetDataVersions(dataVersions);

    return InteractionModelReports::ReadAttribute(device, endpointIds, clusterIds, attributeIds);
}

CHIP_ERROR InteractionModel::ReadEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                       bool fabricFiltered, const Optional<EventNumber> & eventNumber)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };

    InteractionModelReports::ResetOptions();
    InteractionModelReports::SetFabricFiltered(fabricFiltered);
    InteractionModelReports::SetEventNumber(eventNumber);

    return InteractionModelReports::ReadEvent(device, endpointIds, clusterIds, eventIds);
}

CHIP_ERROR InteractionModel::SubscribeAttribute(const char * identity, EndpointId endpointId, ClusterId clusterId,
                                                AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                                bool fabricFiltered, const Optional<DataVersion> & dataVersion,
                                                const Optional<bool> & keepSubscriptions, const Optional<bool> & autoResubscribe)
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

    InteractionModelReports::ResetOptions();
    InteractionModelReports::SetMinInterval(minInterval);
    InteractionModelReports::SetMaxInterval(maxInterval);
    InteractionModelReports::SetFabricFiltered(fabricFiltered);
    InteractionModelReports::SetDataVersions(dataVersions);
    InteractionModelReports::SetKeepSubscriptions(keepSubscriptions);
    InteractionModelReports::SetAutoResubscribe(autoResubscribe);

    return InteractionModelReports::SubscribeAttribute(device, endpointIds, clusterIds, attributeIds);
}

CHIP_ERROR InteractionModel::SubscribeEvent(const char * identity, EndpointId endpointId, ClusterId clusterId, EventId eventId,
                                            uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered,
                                            const Optional<EventNumber> & eventNumber, const Optional<bool> & keepSubscriptions,
                                            const Optional<bool> & autoResubscribe)
{
    DeviceProxy * device = GetDevice(identity);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<EndpointId> endpointIds = { endpointId };
    std::vector<ClusterId> clusterIds   = { clusterId };
    std::vector<EventId> eventIds       = { eventId };

    InteractionModelReports::ResetOptions();
    SetMinInterval(minInterval);
    SetMaxInterval(maxInterval);
    SetFabricFiltered(fabricFiltered);
    SetEventNumber(eventNumber);
    SetKeepSubscriptions(keepSubscriptions);
    SetAutoResubscribe(autoResubscribe);

    return InteractionModelReports::SubscribeEvent(device, endpointIds, clusterIds, eventIds);
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

void InteractionModel::OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams)
{
    InteractionModelReports::OnDeallocatePaths(std::move(aReadPrepareParams));
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

    pathsConfig.count               = pathsCount;
    pathsConfig.attributePathParams = std::make_unique<AttributePathParams[]>(pathsCount);
    pathsConfig.dataVersionFilter   = std::make_unique<DataVersionFilter[]>(pathsCount);

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
                                                    ReadClient::InteractionType interactionType)
{
    ChipLogProgress(chipTool,
                    "Sending %sAttribute to:", interactionType == ReadClient::InteractionType::Subscribe ? "Subscribe" : "Read");

    InteractionModelConfig::AttributePathsConfig pathsConfig;
    ReturnErrorOnFailure(
        InteractionModelConfig::GetAttributePaths(endpointIds, clusterIds, attributeIds, mDataVersions, pathsConfig));

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = nullptr;
    params.mEventPathParamsListSize     = 0;
    params.mpAttributePathParamsList    = pathsConfig.attributePathParams.get();
    params.mAttributePathParamsListSize = pathsConfig.count;

    if (mFabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = mFabricFiltered.Value();
    }

    if (mDataVersions.HasValue())
    {
        params.mpDataVersionFilterList    = pathsConfig.dataVersionFilter.get();
        params.mDataVersionFilterListSize = pathsConfig.count;
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = mMinInterval;
        params.mMaxIntervalCeilingSeconds = mMaxInterval;
        if (mKeepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = mKeepSubscriptions.Value();
        }
        params.mIsPeerLIT = mIsPeerLIT;
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    if (interactionType == ReadClient::InteractionType::Read)
    {
        ReturnErrorOnFailure(client->SendRequest(params));
    }
    else if (mAutoResubscribe.ValueOr(false))
    {
        pathsConfig.attributePathParams.release();
        if (mDataVersions.HasValue())
        {
            pathsConfig.dataVersionFilter.release();
        }
        ReturnErrorOnFailure(client->SendAutoResubscribeRequest(std::move(params)));
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
                                                chip::app::ReadClient::InteractionType interactionType)
{
    const size_t clusterCount  = clusterIds.size();
    const size_t eventCount    = eventIds.size();
    const size_t endpointCount = endpointIds.size();
    const size_t isUrgentCount = mIsUrgents.HasValue() ? mIsUrgents.Value().size() : 0;

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

    auto eventPathParams = std::make_unique<EventPathParams[]>(pathsCount);

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

        if (mIsUrgents.HasValue() && mIsUrgents.Value().size() > i)
        {
            eventPathParams[i].mIsUrgentEvent = mIsUrgents.Value().at(i);
        }
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams.get();
    params.mEventPathParamsListSize     = pathsCount;
    params.mEventNumber                 = mEventNumber;
    params.mpAttributePathParamsList    = nullptr;
    params.mAttributePathParamsListSize = 0;

    if (mFabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = mFabricFiltered.Value();
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = mMinInterval;
        params.mMaxIntervalCeilingSeconds = mMaxInterval;
        if (mKeepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = mKeepSubscriptions.Value();
        }
        params.mIsPeerLIT = mIsPeerLIT;
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    if (mAutoResubscribe.ValueOr(false))
    {
        eventPathParams.release();
        ReturnErrorOnFailure(client->SendAutoResubscribeRequest(std::move(params)));
    }
    else
    {
        ReturnErrorOnFailure(client->SendRequest(params));
    }
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}

void InteractionModelReports::CleanupReadClient(ReadClient * aReadClient)
{
    mReadClients.erase(
        std::remove_if(mReadClients.begin(), mReadClients.end(), [aReadClient](auto & item) { return item.get() == aReadClient; }),
        mReadClients.end());
}

CHIP_ERROR InteractionModelReports::ReportNone(chip::DeviceProxy * device, chip::app::ReadClient::InteractionType interactionType)
{
    AttributePathParams attributePathParams[kMaxAllowedPaths];
    EventPathParams eventPathParams[kMaxAllowedPaths];

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = 0;
    params.mEventNumber                 = mEventNumber;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = 0;

    if (mFabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = mFabricFiltered.Value();
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = mMinInterval;
        params.mMaxIntervalCeilingSeconds = mMaxInterval;
        if (mKeepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = mKeepSubscriptions.Value();
        }
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    ReturnErrorOnFailure(client->SendRequest(params));
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelReports::ReportAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                                              std::vector<chip::EventId> eventIds,
                                              chip::app::ReadClient::InteractionType interactionType)
{
    const size_t endpointCount  = endpointIds.size();
    const size_t clusterCount   = clusterIds.size();
    const size_t attributeCount = attributeIds.size();
    const size_t eventCount     = eventIds.size();

    // TODO Add data version supports
    // TODO Add isUrgents supports

    VerifyOrReturnError(endpointCount > 0 && endpointCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(clusterCount > 0 && clusterCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(attributeCount > 0 && attributeCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(eventCount > 0 && eventCount <= kMaxAllowedPaths, CHIP_ERROR_INVALID_ARGUMENT);

    const bool hasSameIdsCount = (clusterCount == (attributeCount + eventCount)) && (clusterCount == endpointCount);
    if (!hasSameIdsCount)
    {
        ChipLogError(chipTool,
                     "\nCommand targetting a combination of attribute and event paths needs to have has many clusters and "
                     "endpoints than the number of attribute and events combined.\n"
                     "For example if there are 2 attributes and 1 event, the command expects 3 clusters and 3 endpoints.\n"
                     "Clusters and endpoints ids will be consumed first to populate the attribute paths of the request, and then "
                     "to populate the event paths of the request.\n\n"
                     "For example the following arguments:\n"
                     "\tcluster-ids: 6,6,0X28\n"
                     "\tendpoint-ids: 1,1,0\n"
                     "\tattribute-ids: 0,0x4001\n"
                     "\tevent-ids: 0\n"
                     "\n"
                     "will create the following paths:\n"
                     "\t{cluster: 6, endpoint: 1, attribute: 0}\n"
                     "\t{cluster: 6, endpoint: 1, attribute: 0x4001}\n"
                     "\t{cluster: 0x28, endpoint: 0, event: 0}\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    AttributePathParams attributePathParams[kMaxAllowedPaths];
    EventPathParams eventPathParams[kMaxAllowedPaths];

    size_t attributeIndex = 0;
    size_t eventIndex     = 0;
    size_t pathsCount     = clusterCount;
    for (size_t i = 0; i < pathsCount; i++)
    {
        auto clusterId  = clusterIds.at(i);
        auto endpointId = endpointIds.at(i);

        if (attributeIndex < attributeIds.size())
        {
            auto attributeId = attributeIds.at(attributeIndex);

            if (endpointId != kInvalidEndpointId)
            {
                attributePathParams[attributeIndex].mEndpointId = endpointId;
            }

            if (clusterId != kInvalidClusterId)
            {
                attributePathParams[attributeIndex].mClusterId = clusterId;
            }

            if (attributeId != kInvalidAttributeId)
            {
                attributePathParams[attributeIndex].mAttributeId = attributeId;
            }

            attributeIndex++;
        }
        else if (eventIndex < eventIds.size())
        {
            auto eventId = eventIds.at(eventIndex);

            if (endpointId != kInvalidEndpointId)
            {
                eventPathParams[eventIndex].mEndpointId = endpointId;
            }

            if (clusterId != kInvalidClusterId)
            {
                eventPathParams[eventIndex].mClusterId = clusterId;
            }

            if (eventId != kInvalidEventId)
            {
                eventPathParams[eventIndex].mEventId = eventId;
            }

            eventIndex++;
        }
    }

    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpEventPathParamsList        = eventPathParams;
    params.mEventPathParamsListSize     = eventCount;
    params.mEventNumber                 = mEventNumber;
    params.mpAttributePathParamsList    = attributePathParams;
    params.mAttributePathParamsListSize = attributeCount;

    if (mFabricFiltered.HasValue())
    {
        params.mIsFabricFiltered = mFabricFiltered.Value();
    }

    if (interactionType == ReadClient::InteractionType::Subscribe)
    {
        params.mMinIntervalFloorSeconds   = mMinInterval;
        params.mMaxIntervalCeilingSeconds = mMaxInterval;
        if (mKeepSubscriptions.HasValue())
        {
            params.mKeepSubscriptions = mKeepSubscriptions.Value();
        }
        params.mIsPeerLIT = mIsPeerLIT;
    }

    auto client = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                               mBufferedReadAdapter, interactionType);
    ReturnErrorOnFailure(client->SendRequest(params));
    mReadClients.push_back(std::move(client));
    return CHIP_NO_ERROR;
}

void InteractionModelReports::OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams)
{
    if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
    {
        delete[] aReadPrepareParams.mpAttributePathParamsList;
    }

    if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
    {
        delete[] aReadPrepareParams.mpDataVersionFilterList;
    }

    if (aReadPrepareParams.mpEventPathParamsList != nullptr)
    {
        delete[] aReadPrepareParams.mpEventPathParamsList;
    }
}
