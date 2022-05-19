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

#include <app/tests/suites/commands/interaction_model/InteractionModel.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

class ReportCommand : public InteractionModelReports, public ModelCommand, public chip::app::ReadClient::Callback
{
public:
    ReportCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelReports(this), ModelCommand(commandName, credsIssuerConfig)
    {}

    virtual void OnSubscription(){};

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
        InteractionModelReports::Shutdown();
        SetCommandExitStatus(mError);
    }

    void OnSubscriptionEstablished(chip::SubscriptionId subscriptionId) override { OnSubscription(); }

protected:
    // Use a 3x-longer-than-default timeout because wildcard reads can take a
    // while.
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return mTimeout.HasValue() ? chip::System::Clock::Seconds16(mTimeout.Value()) : (ModelCommand::GetWaitDuration() * 3);
    }

    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class ReadAttribute : public ReportCommand
{
public:
    ReadAttribute(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                  CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read", credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        ReportCommand::AddArguments();
    }

    ~ReadAttribute() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReadAttribute(device, endpointIds, mClusterIds, mAttributeIds, mFabricFiltered, mDataVersion);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
    chip::Optional<bool> mFabricFiltered;
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
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
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
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::SubscribeAttribute(device, endpointIds, mClusterIds, mAttributeIds, mMinInterval, mMaxInterval,
                                                 mFabricFiltered, mDataVersion, mKeepSubscriptions);
    }

    void OnSubscription() override
    {
        // The ReadClient instance can not be released directly into the OnSubscription
        // callback since it happens to be called by ReadClient itself which is doing additional
        // work after that.
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto * command = reinterpret_cast<SubscribeAttribute *>(arg);
                if (!command->IsInteractive())
                {
                    command->InteractionModelReports::Shutdown();
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
    chip::Optional<bool> mFabricFiltered;
    chip::Optional<std::vector<chip::DataVersion>> mDataVersion;
    chip::Optional<bool> mKeepSubscriptions;
};

class ReadEvent : public ReportCommand
{
public:
    ReadEvent(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-event-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
              CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("event-name", eventName);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReportCommand::AddArguments();
    }

    ~ReadEvent() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::ReadEvent(device, endpointIds, mClusterIds, mEventIds, mFabricFiltered, mEventNumber);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::EventId> mEventIds;
    chip::Optional<bool> mFabricFiltered;
    chip::Optional<chip::EventNumber> mEventNumber;
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
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event-by-id", credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
                   CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("event-name", eventName, "Event name.");
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval,
                    "The requested minimum interval between reports. Sets MinIntervalFloor in the Subscribe Request.");
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval,
                    "The requested maximum interval between reports. Sets MaxIntervalCeiling in the Subscribe Request.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions,
                    "false - Terminate existing subscriptions from initiator.\n  true - Leave existing subscriptions in place.");
        ReportCommand::AddArguments();
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReportCommand::SubscribeEvent(device, endpointIds, mClusterIds, mEventIds, mMinInterval, mMaxInterval,
                                             mFabricFiltered, mEventNumber, mKeepSubscriptions);
    }

    void OnSubscription() override
    {
        // The ReadClient instance can not be released directly into the OnEventSubscription
        // callback since it happens to be called by ReadClient itself which is doing additional
        // work after that.
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                auto * command = reinterpret_cast<SubscribeEvent *>(arg);
                if (!command->IsInteractive())
                {
                    command->InteractionModelReports::Shutdown();
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
    chip::Optional<bool> mFabricFiltered;
    chip::Optional<chip::EventNumber> mEventNumber;
    chip::Optional<bool> mKeepSubscriptions;
};
