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

#pragma once

#include <app/BufferedReadCallback.h>
#include <app/ChunkedWriteCallback.h>
#include <app/CommandSender.h>
#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>

constexpr uint8_t kMaxAllowedPaths = 10;

namespace chip {
namespace test_utils {
void BusyWaitMillis(uint16_t busyWaitForMs);
} // namespace test_utils
} // namespace chip

class InteractionModelConfig
{
public:
    struct AttributePathsConfig
    {
        size_t count = 0;
        std::unique_ptr<chip::app::AttributePathParams[]> attributePathParams;
        std::unique_ptr<chip::app::DataVersionFilter[]> dataVersionFilter;
    };

    static CHIP_ERROR GetAttributePaths(std::vector<chip::EndpointId> endpointIds, std::vector<chip::ClusterId> clusterIds,
                                        std::vector<chip::AttributeId> attributeIds,
                                        const chip::Optional<std::vector<chip::DataVersion>> & dataVersions,
                                        AttributePathsConfig & pathsConfig);
};

class InteractionModelReports
{
public:
    InteractionModelReports(chip::app::ReadClient::Callback * callback) : mBufferedReadAdapter(*callback) { ResetOptions(); }

protected:
    CHIP_ERROR ReadAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                             std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds)
    {
        return ReportAttribute(device, endpointIds, clusterIds, attributeIds, chip::app::ReadClient::InteractionType::Read);
    }

    CHIP_ERROR SubscribeAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                                  std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds)
    {
        return ReportAttribute(device, endpointIds, clusterIds, attributeIds, chip::app::ReadClient::InteractionType::Subscribe);
    }

    CHIP_ERROR ReportAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                               std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                               chip::app::ReadClient::InteractionType interactionType);

    CHIP_ERROR ReadEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                         std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds)
    {
        return ReportEvent(device, endpointIds, clusterIds, eventIds, chip::app::ReadClient::InteractionType::Read);
    }

    CHIP_ERROR SubscribeEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds)
    {
        return ReportEvent(device, endpointIds, clusterIds, eventIds, chip::app::ReadClient::InteractionType::Subscribe);
    }

    CHIP_ERROR ReportEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                           std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds,
                           chip::app::ReadClient::InteractionType interactionType);

    CHIP_ERROR ReadAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                       std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                       std::vector<chip::EventId> eventIds)
    {
        return ReportAll(device, endpointIds, clusterIds, attributeIds, eventIds, chip::app::ReadClient::InteractionType::Read);
    }

    CHIP_ERROR SubscribeAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                            std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                            std::vector<chip::EventId> eventIds)
    {
        return ReportAll(device, endpointIds, clusterIds, attributeIds, eventIds,
                         chip::app::ReadClient::InteractionType::Subscribe);
    }

    CHIP_ERROR ReportAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                         std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                         std::vector<chip::EventId> eventIds, chip::app::ReadClient::InteractionType interactionType);

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams);

    void Shutdown() { mReadClients.clear(); }

    void CleanupReadClient(chip::app::ReadClient * aReadClient);

    std::vector<std::unique_ptr<chip::app::ReadClient>> mReadClients;
    chip::app::BufferedReadCallback mBufferedReadAdapter;

    InteractionModelReports & SetDataVersions(const std::vector<chip::DataVersion> & dataVersions)
    {
        mDataVersions.SetValue(dataVersions);
        return *this;
    }

    InteractionModelReports & SetDataVersions(const chip::Optional<std::vector<chip::DataVersion>> & dataVersions)
    {
        mDataVersions = dataVersions;
        return *this;
    }

    InteractionModelReports & SetIsUrgents(const std::vector<bool> isUrgents)
    {
        mIsUrgents.SetValue(isUrgents);
        return *this;
    }

    InteractionModelReports & SetIsUrgents(const chip::Optional<std::vector<bool>> & isUrgents)
    {
        mIsUrgents = isUrgents;
        return *this;
    }

    InteractionModelReports & SetEventNumber(const chip::Optional<chip::EventNumber> & eventNumber)
    {
        mEventNumber = eventNumber;
        return *this;
    }

    InteractionModelReports & SetEventNumber(chip::EventNumber eventNumber)
    {
        mEventNumber.SetValue(eventNumber);
        return *this;
    }

    InteractionModelReports & SetFabricFiltered(bool fabricFiltered)
    {
        mFabricFiltered.SetValue(fabricFiltered);
        return *this;
    }

    InteractionModelReports & SetKeepSubscriptions(bool keepSubscriptions)
    {
        mKeepSubscriptions.SetValue(keepSubscriptions);
        return *this;
    }

    InteractionModelReports & SetKeepSubscriptions(const chip::Optional<bool> & keepSubscriptions)
    {
        mKeepSubscriptions = keepSubscriptions;
        return *this;
    }

    InteractionModelReports & SetAutoResubscribe(bool autoResubscribe)
    {
        mAutoResubscribe.SetValue(autoResubscribe);
        return *this;
    }

    InteractionModelReports & SetAutoResubscribe(const chip::Optional<bool> & autoResubscribe)
    {
        mAutoResubscribe = autoResubscribe;
        return *this;
    }

    InteractionModelReports & SetMinInterval(uint16_t minInterval)
    {
        mMinInterval = minInterval;
        return *this;
    }

    InteractionModelReports & SetMaxInterval(uint16_t maxInterval)
    {
        mMaxInterval = maxInterval;
        return *this;
    }

    void ResetOptions()
    {
        mDataVersions      = chip::NullOptional;
        mIsUrgents         = chip::NullOptional;
        mEventNumber       = chip::NullOptional;
        mFabricFiltered    = chip::Optional<bool>(true);
        mKeepSubscriptions = chip::NullOptional;
        mAutoResubscribe   = chip::NullOptional;
        mMinInterval       = 0;
        mMaxInterval       = 0;
    }

    chip::Optional<std::vector<chip::DataVersion>> mDataVersions;
    chip::Optional<std::vector<bool>> mIsUrgents;
    chip::Optional<chip::EventNumber> mEventNumber;
    chip::Optional<bool> mFabricFiltered;
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<bool> mAutoResubscribe;
    uint16_t mMinInterval;
    uint16_t mMaxInterval;
};

class InteractionModelCommands
{
public:
    InteractionModelCommands(chip::app::CommandSender::Callback * callback) : mCallback(callback) { ResetOptions(); }

protected:
    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value)
    {
        uint16_t repeat = mRepeatCount.ValueOr(1);
        while (repeat--)
        {

            chip::app::CommandPathParams commandPath = { endpointId, clusterId, commandId,
                                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
            auto commandSender = std::make_unique<chip::app::CommandSender>(mCallback, device->GetExchangeManager(),
                                                                            mTimedInteractionTimeoutMs.HasValue());
            VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(commandSender->AddRequestDataNoTimedCheck(commandPath, value, mTimedInteractionTimeoutMs,
                                                                           mSuppressResponse.ValueOr(false)));
            ReturnErrorOnFailure(commandSender->SendCommandRequest(device->GetSecureSession().Value()));
            mCommandSender.push_back(std::move(commandSender));

            if (mBusyWaitForMs.HasValue())
            {
                chip::test_utils::BusyWaitMillis(mBusyWaitForMs.Value());
            }

            if (mRepeatDelayInMs.HasValue())
            {
                chip::test_utils::SleepMillis(mRepeatDelayInMs.Value());
            }
        }
        return CHIP_NO_ERROR;
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                chip::CommandId commandId, const T & value)
    {
        chip::app::CommandPathParams commandPath = { groupId, clusterId, commandId, (chip::app::CommandPathFlags::kGroupIdValid) };

        chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();
        VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::CommandSender commandSender(mCallback, exchangeManager);
        ReturnErrorOnFailure(commandSender.AddRequestDataNoTimedCheck(commandPath, value, chip::NullOptional));

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        return commandSender.SendGroupCommandRequest(chip::SessionHandle(session));
    }

    void Shutdown()
    {
        for (auto & commandSender : mCommandSender)
        {
            commandSender.reset();
        }
        mCommandSender.clear();
    }

    std::vector<std::unique_ptr<chip::app::CommandSender>> mCommandSender;
    chip::app::CommandSender::Callback * mCallback;

    InteractionModelCommands & SetTimedInteractionTimeoutMs(uint16_t timedInteractionTimeoutMs)
    {
        mTimedInteractionTimeoutMs.SetValue(timedInteractionTimeoutMs);
        return *this;
    }

    InteractionModelCommands & SetTimedInteractionTimeoutMs(const chip::Optional<uint16_t> & timedInteractionTimeoutMs)
    {
        mTimedInteractionTimeoutMs = timedInteractionTimeoutMs;
        return *this;
    }

    InteractionModelCommands & SetSuppressResponse(bool suppressResponse)
    {
        mSuppressResponse.SetValue(suppressResponse);
        return *this;
    }

    InteractionModelCommands & SetSuppressResponse(const chip::Optional<bool> & suppressResponse)
    {
        mSuppressResponse = suppressResponse;
        return *this;
    }

    InteractionModelCommands & SetRepeatCount(uint16_t repeatCount)
    {
        mRepeatCount.SetValue(repeatCount);
        return *this;
    }

    InteractionModelCommands & SetRepeatCount(const chip::Optional<uint16_t> & repeatCount)
    {
        mRepeatCount = repeatCount;
        return *this;
    }

    InteractionModelCommands & SetRepeatDelayInMs(uint16_t repeatDelayInMs)
    {
        mRepeatDelayInMs.SetValue(repeatDelayInMs);
        return *this;
    }

    InteractionModelCommands & SetRepeatDelayInMs(const chip::Optional<uint16_t> & repeatDelayInMs)
    {
        mRepeatDelayInMs = repeatDelayInMs;
        return *this;
    }

    InteractionModelCommands & SetBusyWaitForMs(uint16_t busyWaitForMs)
    {
        mBusyWaitForMs.SetValue(busyWaitForMs);
        return *this;
    }

    InteractionModelCommands & SetBusyWaitForMs(const chip::Optional<uint16_t> & busyWaitForMs)
    {
        mBusyWaitForMs = busyWaitForMs;
        return *this;
    }

    void ResetOptions()
    {
        mTimedInteractionTimeoutMs = chip::NullOptional;
        mSuppressResponse          = chip::NullOptional;
        mRepeatCount               = chip::NullOptional;
        mRepeatDelayInMs           = chip::NullOptional;
        mBusyWaitForMs             = chip::NullOptional;
    }

    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<bool> mSuppressResponse;
    chip::Optional<uint16_t> mRepeatCount;
    chip::Optional<uint16_t> mRepeatDelayInMs;
    chip::Optional<uint16_t> mBusyWaitForMs;
};

class InteractionModelWriter
{
public:
    InteractionModelWriter(chip::app::WriteClient::Callback * callback) : mChunkedWriteCallback(callback) {}

protected:
    template <class T>
    CHIP_ERROR WriteAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                              const std::vector<T> & values)
    {
        InteractionModelConfig::AttributePathsConfig pathsConfig;
        ReturnErrorOnFailure(
            InteractionModelConfig::GetAttributePaths(endpointIds, clusterIds, attributeIds, mDataVersions, pathsConfig));

        VerifyOrReturnError(pathsConfig.count == values.size() || values.size() == 1, CHIP_ERROR_INVALID_ARGUMENT);

        uint16_t repeat = mRepeatCount.ValueOr(1);
        while (repeat--)
        {

            mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), &mChunkedWriteCallback,
                                                                    mTimedInteractionTimeoutMs, mSuppressResponse.ValueOr(false));
            VerifyOrReturnError(mWriteClient != nullptr, CHIP_ERROR_NO_MEMORY);

            for (uint8_t i = 0; i < pathsConfig.count; i++)
            {
                auto & path        = pathsConfig.attributePathParams[i];
                auto & dataVersion = pathsConfig.dataVersionFilter[i].mDataVersion;
                const T & value    = i >= values.size() ? values.at(0) : values.at(i);
                ReturnErrorOnFailure(EncodeAttribute<T>(path, dataVersion, value));
            }

            ReturnErrorOnFailure(mWriteClient->SendWriteRequest(device->GetSecureSession().Value()));

            if (mBusyWaitForMs.HasValue())
            {
                chip::test_utils::BusyWaitMillis(mBusyWaitForMs.Value());
            }

            if (mRepeatDelayInMs.HasValue())
            {
                chip::test_utils::SleepMillis(mRepeatDelayInMs.Value());
            }
        }

        return CHIP_NO_ERROR;
    }

    template <class T>
    CHIP_ERROR WriteAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds, const T & value)
    {
        std::vector<T> values = { value };
        return WriteAttribute(device, endpointIds, clusterIds, attributeIds, values);
    }

    template <class T>
    CHIP_ERROR WriteGroupAttribute(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                   chip::AttributeId attributeId, const std::vector<T> & value,
                                   const chip::Optional<chip::DataVersion> & dataVersion = chip::NullOptional)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    template <class T>
    CHIP_ERROR WriteGroupAttribute(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                   chip::AttributeId attributeId, const T & value,
                                   const chip::Optional<chip::DataVersion> & dataVersion = chip::NullOptional)
    {
        chip::app::AttributePathParams attributePathParams;

        if (clusterId != chip::kInvalidClusterId)
        {
            attributePathParams.mClusterId = clusterId;
        }

        if (attributeId != chip::kInvalidAttributeId)
        {
            attributePathParams.mAttributeId = attributeId;
        }

        chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();
        VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::WriteClient writeClient(exchangeManager, &mChunkedWriteCallback, chip::NullOptional);
        ReturnErrorOnFailure(writeClient.EncodeAttribute(attributePathParams, value, dataVersion));

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        return writeClient.SendWriteRequest(chip::SessionHandle(session));
    }

    void Shutdown() { mWriteClient.reset(); }

    std::unique_ptr<chip::app::WriteClient> mWriteClient;
    chip::app::ChunkedWriteCallback mChunkedWriteCallback;

    InteractionModelWriter & SetTimedInteractionTimeoutMs(uint16_t timedInteractionTimeoutMs)
    {
        mTimedInteractionTimeoutMs.SetValue(timedInteractionTimeoutMs);
        return *this;
    }

    InteractionModelWriter & SetTimedInteractionTimeoutMs(const chip::Optional<uint16_t> & timedInteractionTimeoutMs)
    {
        mTimedInteractionTimeoutMs = timedInteractionTimeoutMs;
        return *this;
    }

    InteractionModelWriter & SetSuppressResponse(bool suppressResponse)
    {
        mSuppressResponse.SetValue(suppressResponse);
        return *this;
    }

    InteractionModelWriter & SetSuppressResponse(const chip::Optional<bool> & suppressResponse)
    {
        mSuppressResponse = suppressResponse;
        return *this;
    }

    InteractionModelWriter & SetDataVersions(const std::vector<chip::DataVersion> & dataVersions)
    {
        mDataVersions.SetValue(dataVersions);
        return *this;
    }

    InteractionModelWriter & SetDataVersions(const chip::Optional<std::vector<chip::DataVersion>> & dataVersions)
    {
        mDataVersions = dataVersions;
        return *this;
    }

    InteractionModelWriter & SetRepeatCount(uint16_t repeatCount)
    {
        mRepeatCount.SetValue(repeatCount);
        return *this;
    }

    InteractionModelWriter & SetRepeatCount(const chip::Optional<uint16_t> & repeatCount)
    {
        mRepeatCount = repeatCount;
        return *this;
    }

    InteractionModelWriter & SetRepeatDelayInMs(uint16_t repeatDelayInMs)
    {
        mRepeatDelayInMs.SetValue(repeatDelayInMs);
        return *this;
    }

    InteractionModelWriter & SetRepeatDelayInMs(const chip::Optional<uint16_t> & repeatDelayInMs)
    {
        mRepeatDelayInMs = repeatDelayInMs;
        return *this;
    }

    InteractionModelWriter & SetBusyWaitForMs(uint16_t busyWaitForMs)
    {
        mBusyWaitForMs.SetValue(busyWaitForMs);
        return *this;
    }

    InteractionModelWriter & SetBusyWaitForMs(const chip::Optional<uint16_t> & busyWaitForMs)
    {
        mBusyWaitForMs = busyWaitForMs;
        return *this;
    }

    void ResetOptions()
    {
        mTimedInteractionTimeoutMs = chip::NullOptional;
        mSuppressResponse          = chip::NullOptional;
        mDataVersions              = chip::NullOptional;
        mRepeatCount               = chip::NullOptional;
        mRepeatDelayInMs           = chip::NullOptional;
        mBusyWaitForMs             = chip::NullOptional;
    }

    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<std::vector<chip::DataVersion>> mDataVersions;
    chip::Optional<bool> mSuppressResponse;
    chip::Optional<uint16_t> mRepeatCount;
    chip::Optional<uint16_t> mRepeatDelayInMs;
    chip::Optional<uint16_t> mBusyWaitForMs;

private:
    template <typename T>
    CHIP_ERROR EncodeAttribute(const chip::app::AttributePathParams & path, const chip::Optional<chip::DataVersion> & dataVersion,
                               T value, typename std::enable_if<!std::is_pointer<T>::value>::type * = 0)
    {
        return mWriteClient->EncodeAttribute(path, value, dataVersion);
    }

    template <typename T>
    CHIP_ERROR EncodeAttribute(const chip::app::AttributePathParams & path, const chip::Optional<chip::DataVersion> & dataVersion,
                               T value, typename std::enable_if<std::is_pointer<T>::value>::type * = 0)
    {
        return mWriteClient->EncodeAttribute(path, *value, dataVersion);
    }
};

class InteractionModel : public InteractionModelReports,
                         public InteractionModelCommands,
                         public InteractionModelWriter,
                         public chip::app::ReadClient::Callback,
                         public chip::app::WriteClient::Callback,
                         public chip::app::CommandSender::Callback
{
public:
    InteractionModel() : InteractionModelReports(this), InteractionModelCommands(this), InteractionModelWriter(this){};
    virtual ~InteractionModel(){};

    virtual void OnResponse(const chip::app::StatusIB & status, chip::TLV::TLVReader * data) = 0;
    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err)                              = 0;
    virtual chip::DeviceProxy * GetDevice(const char * identity)                             = 0;

    CHIP_ERROR ReadAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                             chip::AttributeId attributeId, bool fabricFiltered = true,
                             const chip::Optional<chip::DataVersion> & dataVersion = chip::NullOptional);

    CHIP_ERROR SubscribeAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                  chip::AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered,
                                  const chip::Optional<chip::DataVersion> & dataVersion,
                                  const chip::Optional<bool> & keepSubscriptions, const chip::Optional<bool> & autoResubscribe);

    CHIP_ERROR ReadEvent(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::EventId eventId,
                         bool fabricFiltered = true, const chip::Optional<chip::EventNumber> & eventNumber = chip::NullOptional);

    CHIP_ERROR SubscribeEvent(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::EventId eventId,
                              uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered,
                              const chip::Optional<chip::EventNumber> & eventNumber, const chip::Optional<bool> & keepSubscriptions,
                              const chip::Optional<bool> & autoResubscribe);

    CHIP_ERROR WaitForReport() { return CHIP_NO_ERROR; }

    template <class T>
    CHIP_ERROR WriteAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                              chip::AttributeId attributeId, const T & value,
                              const chip::Optional<uint16_t> & timedInteractionTimeoutMs = chip::NullOptional,
                              const chip::Optional<bool> & suppressResponse              = chip::NullOptional,
                              const chip::Optional<chip::DataVersion> & dataVersion      = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        std::vector<chip::EndpointId> endpointIds   = { endpointId };
        std::vector<chip::ClusterId> clusterIds     = { clusterId };
        std::vector<chip::AttributeId> attributeIds = { attributeId };

        chip::Optional<std::vector<chip::DataVersion>> optionalDataVersions;
        if (dataVersion.HasValue())
        {
            std::vector<chip::DataVersion> dataVersions = { dataVersion.Value() };
            optionalDataVersions.SetValue(dataVersions);
        }

        InteractionModelWriter::ResetOptions();
        InteractionModelWriter::SetTimedInteractionTimeoutMs(timedInteractionTimeoutMs);
        InteractionModelWriter::SetSuppressResponse(suppressResponse);
        InteractionModelWriter::SetDataVersions(optionalDataVersions);

        return InteractionModelWriter::WriteAttribute(device, endpointIds, clusterIds, attributeIds, value);
    }

    template <class T>
    CHIP_ERROR WriteGroupAttribute(const char * identity, chip::GroupId groupId, chip::ClusterId clusterId,
                                   chip::AttributeId attributeId, const T & value,
                                   const chip::Optional<chip::DataVersion> & dataVersion = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::FabricIndex fabricIndex = device->GetSecureSession().Value()->GetFabricIndex();
        return InteractionModelWriter::WriteGroupAttribute(groupId, fabricIndex, clusterId, attributeId, value, dataVersion);
    }

    template <class T>
    CHIP_ERROR SendCommand(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                           const T & value, chip::Optional<uint16_t> timedInteractionTimeoutMs = chip::NullOptional,
                           const chip::Optional<bool> & suppressResponse = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        InteractionModelCommands::ResetOptions();
        InteractionModelCommands::SetTimedInteractionTimeoutMs(timedInteractionTimeoutMs);
        InteractionModelCommands::SetSuppressResponse(suppressResponse);

        return InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value);
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(const char * identity, chip::GroupId groupId, chip::ClusterId clusterId, chip::CommandId commandId,
                                const T & value)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::FabricIndex fabricIndex = device->GetSecureSession().Value()->GetFabricIndex();
        return InteractionModelCommands::SendGroupCommand(groupId, fabricIndex, clusterId, commandId, value);
    }

    void Shutdown();

    /////////// ReadClient Callback Interface /////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
    void OnEventData(const chip::app::EventHeader & eventHeader, chip::TLV::TLVReader * data,
                     const chip::app::StatusIB * status) override;
    void OnError(CHIP_ERROR error) override;
    void OnDone(chip::app::ReadClient * aReadClient) override;
    void OnSubscriptionEstablished(chip::SubscriptionId subscriptionId) override;
    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override;
    /////////// WriteClient Callback Interface /////////
    void OnResponse(const chip::app::WriteClient * client, const chip::app::ConcreteDataAttributePath & path,
                    chip::app::StatusIB status) override;
    void OnError(const chip::app::WriteClient * client, CHIP_ERROR error) override;
    void OnDone(chip::app::WriteClient * client) override;

    /////////// CommandSender Callback Interface /////////
    void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                    const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override;
    void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override;
    void OnDone(chip::app::CommandSender * client) override;
};
