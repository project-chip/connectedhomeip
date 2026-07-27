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
    ReportCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig, const char * helpText = nullptr) :
        InteractionModelReports(this),
        ModelCommand(commandName, credsIssuerConfig, /* supportsMultipleEndpoints = */ true, helpText)
    {}

    /////////// ReadClient Callback Interface /////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(path, status));

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

        LogErrorOnFailure(RemoteDataModelLogger::LogAttributeAsJSON(path, data));

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
                LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(eventHeader, *status));

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

        LogErrorOnFailure(RemoteDataModelLogger::LogEventAsJSON(eventHeader, data));

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
        LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(error));

        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        InteractionModelReports::OnDeallocatePaths(std::move(aReadPrepareParams));
    }

    void Shutdown() override
    {
        // We don't shut down InteractionModelReports here; we leave it for
        // Cleanup to handle.
        mError = CHIP_NO_ERROR;
        ModelCommand::Shutdown();
    }

    void Cleanup() override { InteractionModelReports::Shutdown(); }

protected:
    // Use a 3x-longer-than-default timeout because wildcard reads can take a
    // while.
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return mTimeout.HasValue() ? chip::System::Clock::Seconds16(mTimeout.Value()) : (ModelCommand::GetWaitDuration() * 3);
    }

    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class ReadCommand : public ReportCommand
{
protected:
    ReadCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig, const char * helpText = nullptr) :
        ReportCommand(commandName, credsIssuerConfig, helpText)
    {}

    void OnDone(chip::app::ReadClient * aReadClient) override
    {
        InteractionModelReports::CleanupReadClient(aReadClient);
        SetCommandExitStatus(mError);
    }
};

class SubscribeCommand : public ReportCommand
{
protected:
    SubscribeCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig, const char * helpText = nullptr) :
        ReportCommand(commandName, credsIssuerConfig, helpText)
    {}

    void OnSubscriptionEstablished(chip::SubscriptionId subscriptionId) override
    {
        mSubscriptionEstablished = true;
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    void OnDone(chip::app::ReadClient * aReadClient) override
    {
        InteractionModelReports::CleanupReadClient(aReadClient);

        if (!mSubscriptionEstablished)
        {
            SetCommandExitStatus(mError);
        }
        // else we must be getting here from Cleanup(), which means we have
        // already done our exit status thing.
    }

    void Shutdown() override
    {
        mSubscriptionEstablished = false;
        ReportCommand::Shutdown();
    }

    // For subscriptions we always defer interactive cleanup.  Either our
    // ReadClients will terminate themselves (in which case they will be removed
    // from our list anyway), or they should hang around until shutdown.
    bool DeferInteractiveCleanup() override { return true; }

private:
    bool mSubscriptionEstablished = false;
};

class ReadAttribute : public ReadCommand
{
public:
    ReadAttribute(CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-by-id", credsIssuerConfig, "Read attributes for the given attribute path (which may include wildcards).")
    {
        AddArgument("cluster-ids", 0, UINT32_MAX, &mClusterIds,
                    "Comma-separated list of cluster ids to read from (e.g. \"6\" or \"8,0x201\").\n  Allowed to be 0xFFFFFFFF to "
                    "indicate a wildcard cluster.");
        AddAttributeIdArgument();
        AddCommonArguments();
        ReadCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-by-id", credsIssuerConfig, "Read attributes from this cluster; allows wildcard endpoint and attribute."),
        mClusterIds(1, clusterId)
    {
        AddAttributeIdArgument();
        AddCommonArguments();
        ReadCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                  CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read", credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddCommonArguments();
        ReadCommand::AddArguments();
    }

    ~ReadAttribute() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReadCommand::ReadAttribute(device, endpointIds, mClusterIds, mAttributeIds);
    }

private:
    void AddAttributeIdArgument()
    {
        AddArgument("attribute-ids", 0, UINT32_MAX, &mAttributeIds,
                    "Comma-separated list of attribute ids to read (e.g. \"0\" or \"1,0xFFFC,0xFFFD\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard attribute.");
    }

    void AddCommonArguments()
    {
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered read. Defaults to true.");
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersions,
                    "Comma-separated list of data versions for the clusters being read.");
    }

    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
};

class SubscribeAttribute : public SubscribeCommand
{
public:
    SubscribeAttribute(CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-by-id", credsIssuerConfig,
                         "Subscribe to attributes for the given attribute path (which may include wildcards).")
    {
        AddArgument("cluster-ids", 0, UINT32_MAX, &mClusterIds,
                    "Comma-separated list of cluster ids to subscribe to (e.g. \"6\" or \"8,0x201\").\n  Allowed to be 0xFFFFFFFF "
                    "to indicate a wildcard cluster.");
        AddAttributeIdArgument();
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-by-id", credsIssuerConfig,
                         "Subscribe to attributes from this cluster; allows wildcard endpoint and attribute."),
        mClusterIds(1, clusterId)
    {
        AddAttributeIdArgument();
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                       CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe", credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        SubscribeCommand::SetPeerLIT(IsPeerLIT());
        return SubscribeCommand::SubscribeAttribute(device, endpointIds, mClusterIds, mAttributeIds);
    }

private:
    void AddAttributeIdArgument()
    {
        AddArgument("attribute-ids", 0, UINT32_MAX, &mAttributeIds,
                    "Comma-separated list of attribute ids to subscribe to (e.g. \"0\" or \"1,0xFFFC,0xFFFD\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard attribute.");
    }

    void AddCommonArguments()
    {
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval,
                    "Server should not send a new report if less than this number of seconds has elapsed since the last report.");
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval,
                    "Server must send a report if this number of seconds has elapsed since the last report.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered subscription. Defaults to true.");
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersions,
                    "Comma-separated list of data versions for the clusters being subscribed to.");
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions,
                    "Boolean indicating whether to keep existing subscriptions when creating the new one. Defaults to false.");
        AddArgument("auto-resubscribe", 0, 1, &mAutoResubscribe,
                    "Boolean indicating whether the subscription should auto-resubscribe.  Defaults to false.");
    }

    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
};

class ReadEvent : public ReadCommand
{
public:
    ReadEvent(CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-event-by-id", credsIssuerConfig, "Read events for the given event path (which may include wildcards).")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReadCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-event-by-id", credsIssuerConfig, "Read events from this cluster; allows wildcard endpoint and event."),
        mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReadCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
              CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("event-name", eventName);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReadCommand::AddArguments();
    }

    ~ReadEvent() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReadCommand::ReadEvent(device, endpointIds, mClusterIds, mEventIds);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::EventId> mEventIds;
};

class SubscribeEvent : public SubscribeCommand
{
public:
    SubscribeEvent(CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-event-by-id", credsIssuerConfig,
                         "Subscribe to events for the given event path (which may include wildcards).")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterIds);
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-event-by-id", credsIssuerConfig,
                         "Subscribe to events from this cluster; allows wildcard endpoint and event."),
        mClusterIds(1, clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventIds);
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
                   CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-event", credsIssuerConfig),
        mClusterIds(1, clusterId), mEventIds(1, eventId)
    {
        AddArgument("event-name", eventName, "Event name.");
        AddCommonArguments();
        SubscribeCommand::AddArguments();
    }

    void AddCommonArguments()
    {
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval,
                    "The requested minimum interval between reports. Sets MinIntervalFloor in the Subscribe Request.");
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval,
                    "The requested maximum interval between reports. Sets MaxIntervalCeiling in the Subscribe Request.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions,
                    "false - Terminate existing subscriptions from initiator.\n  true - Leave existing subscriptions in place.");
        AddArgument(
            "is-urgent", 0, 1, &mIsUrgents,
            "Sets isUrgent in the Subscribe Request.\n"
            "  The queueing of any urgent event SHALL force an immediate generation of reports containing all events queued "
            "leading up to (and including) the urgent event in question.\n"
            "  This argument takes a comma separated list of true/false values.\n"
            "  If the number of paths exceeds the number of entries provided to is-urgent, then isUrgent will be false for the "
            "extra paths.");
        AddArgument("auto-resubscribe", 0, 1, &mAutoResubscribe,
                    "Boolean indicating whether the subscription should auto-resubscribe.  Defaults to false.");
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        SubscribeCommand::SetPeerLIT(IsPeerLIT());
        return SubscribeCommand::SubscribeEvent(device, endpointIds, mClusterIds, mEventIds);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::EventId> mEventIds;
};

class ReadNone : public ReadCommand
{
public:
    ReadNone(CredentialIssuerCommands * credsIssuerConfig) : ReadCommand("read-none", credsIssuerConfig)
    {
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered read. Defaults to true.");
        AddArgument("data-versions", 0, UINT32_MAX, &mDataVersions,
                    "Comma-separated list of data versions for the clusters being read.");
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReadCommand::AddArguments(true /* skipEndpoints */);
    }

    ~ReadNone() {}

    void OnDone(chip::app::ReadClient * aReadClient) override
    {
        InteractionModelReports::CleanupReadClient(aReadClient);
        SetCommandExitStatus(mError);
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReadCommand::ReadNone(device);
    }
};

class ReadAll : public ReadCommand
{
public:
    ReadAll(CredentialIssuerCommands * credsIssuerConfig) :
        ReadCommand("read-all", credsIssuerConfig, "Read attributes and events for the given paths (which may include wildcards).")
    {
        AddArgument("cluster-ids", 0, UINT32_MAX, &mClusterIds,
                    "Comma-separated list of cluster ids to read from (e.g. \"6\" or \"8,0x201\").\n  Allowed to be 0xFFFFFFFF to "
                    "indicate a wildcard cluster.");
        AddArgument("attribute-ids", 0, UINT32_MAX, &mAttributeIds,
                    "Comma-separated list of attribute ids to read (e.g. \"0\" or \"1,0xFFFC,0xFFFD\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard attribute.");
        AddArgument("event-ids", 0, UINT32_MAX, &mEventIds,
                    "Comma-separated list of event ids to read (e.g. \"0\" or \"1,2,3\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard event.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered read. Defaults to true.");
        AddArgument("data-versions", 0, UINT32_MAX, &mDataVersions,
                    "Comma-separated list of data versions for the clusters being read.");
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ReadCommand::AddArguments();
    }

    ~ReadAll() {}

    void OnDone(chip::app::ReadClient * aReadClient) override
    {
        InteractionModelReports::CleanupReadClient(aReadClient);
        SetCommandExitStatus(mError);
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ReadCommand::ReadAll(device, endpointIds, mClusterIds, mAttributeIds, mEventIds);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
    std::vector<chip::EventId> mEventIds;
};

class SubscribeNone : public SubscribeCommand
{
public:
    SubscribeNone(CredentialIssuerCommands * credsIssuerConfig) : SubscribeCommand("subscribe-none", credsIssuerConfig)
    {
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval,
                    "The requested minimum interval between reports. Sets MinIntervalFloor in the Subscribe Request.");
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval,
                    "The requested maximum interval between reports. Sets MaxIntervalCeiling in the Subscribe Request.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered read. Defaults to true.");
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions,
                    "false - Terminate existing subscriptions from initiator.\n  true - Leave existing subscriptions in place.");
        SubscribeCommand::AddArguments(true /* skipEndpoints */);
    }

    ~SubscribeNone() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return SubscribeCommand::SubscribeNone(device);
    }
};

class SubscribeAll : public SubscribeCommand
{
public:
    SubscribeAll(CredentialIssuerCommands * credsIssuerConfig) :
        SubscribeCommand("subscribe-all", credsIssuerConfig,
                         "Subscribe to attributes and events for the given paths (which may include wildcards).")
    {
        AddArgument("cluster-ids", 0, UINT32_MAX, &mClusterIds,
                    "Comma-separated list of cluster ids to read from (e.g. \"6\" or \"8,0x201\").\n  Allowed to be 0xFFFFFFFF to "
                    "indicate a wildcard cluster.");
        AddArgument("attribute-ids", 0, UINT32_MAX, &mAttributeIds,
                    "Comma-separated list of attribute ids to read (e.g. \"0\" or \"1,0xFFFC,0xFFFD\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard attribute.");
        AddArgument("event-ids", 0, UINT32_MAX, &mEventIds,
                    "Comma-separated list of event ids to read (e.g. \"0\" or \"1,2,3\").\n  Allowed to be "
                    "0xFFFFFFFF to indicate a wildcard event.");
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval,
                    "The requested minimum interval between reports. Sets MinIntervalFloor in the Subscribe Request.");
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval,
                    "The requested maximum interval between reports. Sets MaxIntervalCeiling in the Subscribe Request.");
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered,
                    "Boolean indicating whether to do a fabric-filtered read. Defaults to true.");
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions,
                    "false - Terminate existing subscriptions from initiator.\n  true - Leave existing subscriptions in place.");
        SubscribeCommand::AddArguments();
    }

    ~SubscribeAll() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        SubscribeCommand::SetPeerLIT(IsPeerLIT());
        return SubscribeCommand::SubscribeAll(device, endpointIds, mClusterIds, mAttributeIds, mEventIds);
    }

private:
    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;
    std::vector<chip::EventId> mEventIds;
};
