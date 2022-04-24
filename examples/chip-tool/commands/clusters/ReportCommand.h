/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include <app/ReadClient.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

constexpr uint8_t kMaxAllowedPaths = 10;

class ReportCommand : public ModelCommand, public chip::app::ReadClient::Callback
{
public:
    ReportCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand(commandName, credsIssuerConfig), mBufferedReadAdapter(*this)
    {}

    virtual void OnAttributeSubscription(){};
    virtual void OnEventSubscription(){};

    /////////// ReadClient Callback Interface /////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
            return;
        }

        if (data == nullptr)
        {
            ChipLogError(chipTool, "Response Failure: No Data");
            mError = CHIP_ERROR_INTERNAL;
            return;
        }

        error = DataModelLogger::LogAttribute(path, data);
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: Can not decode Data");
            mError = error;
            return;
        }
    }

    void OnEventData(const chip::app::EventHeader & eventHeader, chip::TLV::TLVReader * data,
                     const chip::app::StatusIB * status) override
    {
        if (status != nullptr)
        {
            CHIP_ERROR error = status->ToChipError();
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
                mError = error;
                return;
            }
        }

        if (data == nullptr)
        {
            ChipLogError(chipTool, "Response Failure: No Data");
            mError = CHIP_ERROR_INTERNAL;
            return;
        }

        CHIP_ERROR error = DataModelLogger::LogEvent(eventHeader, data);
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: Can not decode Data");
            mError = error;
            return;
        }
    }

    void OnError(CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    void OnDone() override
    {
        mReadClient.reset();
        SetCommandExitStatus(mError);
    }

    void OnSubscriptionEstablished(uint64_t subscriptionId) override { OnAttributeSubscription(); }

protected:
    CHIP_ERROR ReportAttribute(ChipDevice * device, std::vector<chip::EndpointId> endpointIds,
                               std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                               chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval = 0,
                               uint16_t maxInterval                                                = 0,
                               const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional)
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
            ChipLogError(
                chipTool,
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
            attributePathParams[i].mClusterId   = clusterId;
            attributePathParams[i].mAttributeId = attributeId;
            attributePathParams[i].mEndpointId  = endpointId;

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

        if (mFabricFiltered.HasValue())
        {
            params.mIsFabricFiltered = mFabricFiltered.Value();
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
            if (mKeepSubscriptions.HasValue())
            {
                params.mKeepSubscriptions = mKeepSubscriptions.Value();
            }
        }

        mReadClient = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
                                                              device->GetExchangeManager(), mBufferedReadAdapter, interactionType);
        return mReadClient->SendRequest(params);
    }

    CHIP_ERROR ReportEvent(ChipDevice * device, std::vector<chip::EndpointId> endpointIds, std::vector<chip::ClusterId> clusterIds,
                           std::vector<chip::EventId> eventIds, chip::app::ReadClient::InteractionType interactionType,
                           uint16_t minInterval = 0, uint16_t maxInterval = 0)
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
            eventPathParams[i].mClusterId  = clusterId;
            eventPathParams[i].mEventId    = eventId;
            eventPathParams[i].mEndpointId = endpointId;
        }

        chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
        params.mpEventPathParamsList        = eventPathParams;
        params.mEventPathParamsListSize     = pathsCount;
        params.mEventNumber                 = mEventNumber;
        params.mpAttributePathParamsList    = nullptr;
        params.mAttributePathParamsListSize = 0;

        if (interactionType == chip::app::ReadClient::InteractionType::Subscribe)
        {
            params.mMinIntervalFloorSeconds   = minInterval;
            params.mMaxIntervalCeilingSeconds = maxInterval;
            if (mKeepSubscriptions.HasValue())
            {
                params.mKeepSubscriptions = mKeepSubscriptions.Value();
            }
        }

        mReadClient = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
                                                              device->GetExchangeManager(), mBufferedReadAdapter, interactionType);
        return mReadClient->SendRequest(params);
    }

    // Use a 3x-longer-than-default timeout because wildcard reads can take a
    // while.
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return mTimeout.HasValue() ? chip::System::Clock::Seconds16(mTimeout.Value()) : (ModelCommand::GetWaitDuration() * 3);
    }

    std::unique_ptr<chip::app::ReadClient> mReadClient;
    chip::app::BufferedReadCallback mBufferedReadAdapter;

    // mFabricFiltered is really only used by the attribute commands, but we end
    // up needing it in our class's shared code.
    chip::Optional<bool> mFabricFiltered;

    // mKeepSubscriptions is really only used by the subscribe commands, but we end
    // up needing it in our class's shared code.
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<chip::EventNumber> mEventNumber;

    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class ReadAttribute : public ReportCommand
{
public:
    ReadAttribute(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                  CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read", credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ReportCommand::AddArguments();
    }

    ~ReadAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReportAttribute(device, endpointIds, mClusterIds, mAttributeIds,
                                              chip::app::ReadClient::InteractionType::Read, 0, 0, mDataVersion);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
    chip::Optional<std::vector<chip::DataVersion>> mDataVersion;
};

class SubscribeAttribute : public ReportCommand
{
public:
    SubscribeAttribute(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("subscribe-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                       CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe", credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReportAttribute(device, endpointIds, mClusterIds, mAttributeIds,
                                              chip::app::ReadClient::InteractionType::Subscribe, mMinInterval, mMaxInterval,
                                              mDataVersion);
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return ReportCommand::GetWaitDuration(); }

    void OnAttributeSubscription() override
    {
        // The ReadClient instance can not be released directly into the OnAttributeSubscription
        // callback since it happens to be called by ReadClient itself which is doing additional
        // work after that.
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto * command = reinterpret_cast<SubscribeAttribute *>(arg);
                if (!command->IsInteractive())
                {
                    command->mReadClient.reset();
                }
                command->SetCommandExitStatus(CHIP_NO_ERROR);
            },
            reinterpret_cast<intptr_t>(this));
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;

    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    chip::Optional<std::vector<chip::DataVersion>> mDataVersion;
};

class ReadEvent : public ReportCommand
{
public:
    ReadEvent(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-event-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
              CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("event-name", eventName);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ~ReadEvent() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReportEvent(device, endpointIds, mClusterIds, mEventIds,
                                          chip::app::ReadClient::InteractionType::Read);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::EventId> mEventIds;
};

class SubscribeEvent : public ReportCommand
{
public:
    SubscribeEvent(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("subscribe-event-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
                   CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("attr-name", eventName);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReportEvent(device, endpointIds, mClusterIds, mEventIds,
                                          chip::app::ReadClient::InteractionType::Subscribe, mMinInterval, mMaxInterval);
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return ReportCommand::GetWaitDuration(); }

    void OnEventSubscription() override
    {
        // The ReadClient instance can not be released directly into the OnEventSubscription
        // callback since it happens to be called by ReadClient itself which is doing additional
        // work after that.
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto * command = reinterpret_cast<SubscribeEvent *>(arg);
                if (!command->IsInteractive())
                {
                    command->mReadClient.reset();
                }
                command->SetCommandExitStatus(CHIP_NO_ERROR);
            },
            reinterpret_cast<intptr_t>(this));
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::EventId> mEventIds;

    uint16_t mMinInterval;
    uint16_t mMaxInterval;
};
