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

class ReportCommand : public ModelCommand, public chip::app::ReadClient::Callback
{
public:
    ReportCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand(commandName, credsIssuerConfig), mBufferedReadAdapter(*this)
    {}

    virtual void OnAttributeSubscription(){};
    virtual void OnEventSubscription(){};

    /////////// ReadClient Callback Interface /////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::DataVersion aVersion, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            SetCommandExitStatus(error);
            return;
        }

        if (data == nullptr)
        {
            ChipLogError(chipTool, "Response Failure: No Data");
            SetCommandExitStatus(CHIP_ERROR_INTERNAL);
            return;
        }

        error = DataModelLogger::LogAttribute(path, data);
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: Can not decode Data");
            SetCommandExitStatus(error);
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
                SetCommandExitStatus(error);
                return;
            }
        }

        if (data == nullptr)
        {
            ChipLogError(chipTool, "Response Failure: No Data");
            SetCommandExitStatus(CHIP_ERROR_INTERNAL);
            return;
        }

        CHIP_ERROR error = DataModelLogger::LogEvent(eventHeader, data);
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: Can not decode Data");
            SetCommandExitStatus(error);
            return;
        }
    }

    void OnError(CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        SetCommandExitStatus(error);
    }

    void OnDone() override
    {
        mReadClient.reset();
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    void OnSubscriptionEstablished(uint64_t subscriptionId) override { OnAttributeSubscription(); }

protected:
    CHIP_ERROR ReportAttribute(ChipDevice * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                               chip::AttributeId attributeId, chip::app::ReadClient::InteractionType interactionType,
                               uint16_t minInterval = 0, uint16_t maxInterval = 0)
    {
        chip::app::AttributePathParams attributePathParams[1];
        attributePathParams[0].mEndpointId  = endpointId;
        attributePathParams[0].mClusterId   = clusterId;
        attributePathParams[0].mAttributeId = attributeId;

        chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
        params.mpEventPathParamsList        = nullptr;
        params.mEventPathParamsListSize     = 0;
        params.mpAttributePathParamsList    = attributePathParams;
        params.mAttributePathParamsListSize = 1;

        if (interactionType == chip::app::ReadClient::InteractionType::Subscribe)
        {
            params.mMinIntervalFloorSeconds   = minInterval;
            params.mMaxIntervalCeilingSeconds = maxInterval;
        }

        mReadClient = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
                                                              device->GetExchangeManager(), mBufferedReadAdapter, interactionType);
        return mReadClient->SendRequest(params);
    }

    CHIP_ERROR ReportEvent(ChipDevice * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::EventId eventId,
                           chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval = 0,
                           uint16_t maxInterval = 0)
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = endpointId;
        eventPathParams[0].mClusterId  = clusterId;
        eventPathParams[0].mEventId    = eventId;

        chip::app::ReadPrepareParams params(device->GetSecureSession().Value());
        params.mpEventPathParamsList        = eventPathParams;
        params.mEventPathParamsListSize     = 1;
        params.mpAttributePathParamsList    = nullptr;
        params.mAttributePathParamsListSize = 0;

        if (interactionType == chip::app::ReadClient::InteractionType::Subscribe)
        {
            params.mMinIntervalFloorSeconds   = minInterval;
            params.mMaxIntervalCeilingSeconds = maxInterval;
        }

        mReadClient = std::make_unique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
                                                              device->GetExchangeManager(), mBufferedReadAdapter, interactionType);
        return mReadClient->SendRequest(params);
    }

    std::unique_ptr<chip::app::ReadClient> mReadClient;
    chip::app::BufferedReadCallback mBufferedReadAdapter;
};

class ReadAttribute : public ReportCommand
{
public:
    ReadAttribute(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        ReportCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                  CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read", credsIssuerConfig),
        mClusterId(clusterId), mAttributeId(attributeId)
    {
        AddArgument("attr-name", attributeName);
        ReportCommand::AddArguments();
    }

    ~ReadAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId) override
    {
        ChipLogProgress(chipTool, "Sending ReadAttribute to cluster " ChipLogFormatMEI " on endpoint %" PRIu16,
                        ChipLogValueMEI(mClusterId), endpointId);
        return ReportCommand::ReportAttribute(device, endpointId, mClusterId, mAttributeId,
                                              chip::app::ReadClient::InteractionType::Read);
    }

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
};

class SubscribeAttribute : public ReportCommand
{
public:
    SubscribeAttribute(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("subscribe-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                       CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe", credsIssuerConfig),
        mClusterId(clusterId), mAttributeId(attributeId)
    {
        AddArgument("attr-name", attributeName);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId) override
    {
        ChipLogProgress(chipTool, "Sending SubscribeAttribute to cluster " ChipLogFormatMEI " on endpoint %" PRIu16,
                        ChipLogValueMEI(mClusterId), endpointId);
        return ReportCommand::ReportAttribute(device, endpointId, mClusterId, mAttributeId,
                                              chip::app::ReadClient::InteractionType::Subscribe, mMinInterval, mMaxInterval);
    }

    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mWait ? UINT16_MAX : 10);
    }

    void OnAttributeSubscription() override
    {
        if (!mWait)
        {
            SetCommandExitStatus(CHIP_NO_ERROR);
        }
    }

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;

    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    bool mWait;
};

class ReadEvent : public ReportCommand
{
public:
    ReadEvent(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("read-event-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        ReportCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
              CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("read-event", credsIssuerConfig),
        mClusterId(clusterId), mEventId(eventId)
    {
        AddArgument("event-name", eventName);
        ReportCommand::AddArguments();
    }

    ~ReadEvent() {}

    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId) override
    {
        ChipLogProgress(chipTool, "Sending ReadEvent to cluster " ChipLogFormatMEI " on endpoint %" PRIu16,
                        ChipLogValueMEI(mClusterId), endpointId);
        return ReportCommand::ReportEvent(device, endpointId, mClusterId, mEventId, chip::app::ReadClient::InteractionType::Read);
    }

private:
    chip::ClusterId mClusterId;
    chip::EventId mEventId;
};

class SubscribeEvent : public ReportCommand
{
public:
    SubscribeEvent(CredentialIssuerCommands * credsIssuerConfig) : ReportCommand("subscribe-event-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, const char * eventName, chip::EventId eventId,
                   CredentialIssuerCommands * credsIssuerConfig) :
        ReportCommand("subscribe-event", credsIssuerConfig),
        mClusterId(clusterId), mEventId(eventId)
    {
        AddArgument("attr-name", eventName);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("wait", 0, 1, &mWait);
        ReportCommand::AddArguments();
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId) override
    {
        ChipLogProgress(chipTool, "Sending SubscribeEvent to cluster " ChipLogFormatMEI " on endpoint %" PRIu16,
                        ChipLogValueMEI(mClusterId), endpointId);
        return ReportCommand::ReportEvent(device, endpointId, mClusterId, mEventId,
                                          chip::app::ReadClient::InteractionType::Subscribe, mMinInterval, mMaxInterval);
    }

    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mWait ? UINT16_MAX : 10);
    }

    void OnEventSubscription() override
    {
        if (!mWait)
        {
            SetCommandExitStatus(CHIP_NO_ERROR);
        }
    }

private:
    chip::ClusterId mClusterId;
    chip::EventId mEventId;

    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    bool mWait;
};
