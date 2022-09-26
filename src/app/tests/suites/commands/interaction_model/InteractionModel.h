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
    InteractionModelReports(chip::app::ReadClient::Callback * callback) : mBufferedReadAdapter(*callback) {}

protected:
    CHIP_ERROR ReadAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                             std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                             const chip::Optional<bool> & fabricFiltered                         = chip::Optional<bool>(true),
                             const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional)
    {
        return ReportAttribute(device, endpointIds, clusterIds, attributeIds, chip::app::ReadClient::InteractionType::Read, 0, 0,
                               fabricFiltered, dataVersions, chip::NullOptional, chip::NullOptional);
    }

    CHIP_ERROR SubscribeAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                                  std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                                  uint16_t minInterval, uint16_t maxInterval, const chip::Optional<bool> & fabricFiltered,
                                  const chip::Optional<std::vector<chip::DataVersion>> & dataVersions,
                                  const chip::Optional<bool> & keepSubscriptions, const chip::Optional<bool> & autoResubscribe)
    {
        return ReportAttribute(device, endpointIds, clusterIds, attributeIds, chip::app::ReadClient::InteractionType::Subscribe,
                               minInterval, maxInterval, fabricFiltered, dataVersions, keepSubscriptions, autoResubscribe);
    }

    CHIP_ERROR ReportAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                               std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                               chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval, uint16_t maxInterval,
                               const chip::Optional<bool> & fabricFiltered,
                               const chip::Optional<std::vector<chip::DataVersion>> & dataVersions,
                               const chip::Optional<bool> & keepSubscriptions, const chip::Optional<bool> & autoResubscribe);

    CHIP_ERROR ReadEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                         std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds,
                         const chip::Optional<bool> & fabricFiltered           = chip::Optional<bool>(true),
                         const chip::Optional<chip::EventNumber> & eventNumber = chip::NullOptional)
    {
        return ReportEvent(device, endpointIds, clusterIds, eventIds, chip::app::ReadClient::InteractionType::Read, 0, 0,
                           fabricFiltered, eventNumber, chip::NullOptional, chip::NullOptional, chip::NullOptional);
    }

    CHIP_ERROR SubscribeEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds, uint16_t minInterval,
                              uint16_t maxInterval, const chip::Optional<bool> & fabricFiltered,
                              const chip::Optional<chip::EventNumber> & eventNumber, const chip::Optional<bool> & keepSubscriptions,
                              const chip::Optional<std::vector<bool>> & isUrgents, const chip::Optional<bool> & autoResubscribe)
    {
        return ReportEvent(device, endpointIds, clusterIds, eventIds, chip::app::ReadClient::InteractionType::Subscribe,
                           minInterval, maxInterval, fabricFiltered, eventNumber, keepSubscriptions, isUrgents, autoResubscribe);
    }

    CHIP_ERROR ReportEvent(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                           std::vector<chip::ClusterId> clusterIds, std::vector<chip::EventId> eventIds,
                           chip::app::ReadClient::InteractionType interactionType, uint16_t minInterval, uint16_t maxInterval,
                           const chip::Optional<bool> & fabricFiltered, const chip::Optional<chip::EventNumber> & eventNumber,
                           const chip::Optional<bool> & keepSubscriptions, const chip::Optional<std::vector<bool>> & isUrgents,
                           const chip::Optional<bool> & autoResubscribe);

    CHIP_ERROR ReadAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                       std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                       std::vector<chip::EventId> eventIds,
                       const chip::Optional<bool> & fabricFiltered                         = chip::Optional<bool>(true),
                       const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional,
                       const chip::Optional<chip::EventNumber> & eventNumber               = chip::NullOptional)
    {
        return ReportAll(device, endpointIds, clusterIds, attributeIds, eventIds, chip::app::ReadClient::InteractionType::Read, 0,
                         0, fabricFiltered, dataVersions, eventNumber);
    }

    CHIP_ERROR SubscribeAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                            std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                            std::vector<chip::EventId> eventIds, uint16_t minInterval = 0, uint16_t maxInterval = 0,
                            const chip::Optional<bool> & fabricFiltered           = chip::Optional<bool>(true),
                            const chip::Optional<chip::EventNumber> & eventNumber = chip::NullOptional,
                            const chip::Optional<bool> & keepSubscriptions        = chip::NullOptional)
    {
        return ReportAll(device, endpointIds, clusterIds, attributeIds, eventIds, chip::app::ReadClient::InteractionType::Subscribe,
                         minInterval, maxInterval, fabricFiltered, chip::NullOptional, eventNumber, keepSubscriptions);
    }

    CHIP_ERROR ReportAll(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                         std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                         std::vector<chip::EventId> eventIds, chip::app::ReadClient::InteractionType interactionType,
                         uint16_t minInterval = 0, uint16_t maxInterval = 0,
                         const chip::Optional<bool> & fabricFiltered                         = chip::Optional<bool>(true),
                         const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional,
                         const chip::Optional<chip::EventNumber> & eventNumber               = chip::NullOptional,
                         const chip::Optional<bool> & keepSubscriptions                      = chip::NullOptional);

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams);

    void Shutdown() { mReadClients.clear(); }

    void CleanupReadClient(chip::app::ReadClient * aReadClient);

    std::vector<std::unique_ptr<chip::app::ReadClient>> mReadClients;
    chip::app::BufferedReadCallback mBufferedReadAdapter;
};

class InteractionModelCommands
{
public:
    InteractionModelCommands(chip::app::CommandSender::Callback * callback) : mCallback(callback) {}

protected:
    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value,
                           const chip::Optional<uint16_t> & timedInteractionTimeoutMs = chip::NullOptional,
                           const chip::Optional<bool> & suppressResponse              = chip::NullOptional,
                           const chip::Optional<uint16_t> & repeatCount               = chip::NullOptional,
                           const chip::Optional<uint16_t> & repeatDelayInMs           = chip::NullOptional)
    {
        uint16_t repeat = repeatCount.ValueOr(1);
        while (repeat--)
        {

            chip::app::CommandPathParams commandPath = { endpointId, clusterId, commandId,
                                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
            auto commandSender = std::make_unique<chip::app::CommandSender>(mCallback, device->GetExchangeManager(),
                                                                            timedInteractionTimeoutMs.HasValue());
            VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(commandSender->AddRequestDataNoTimedCheck(commandPath, value, timedInteractionTimeoutMs,
                                                                           suppressResponse.ValueOr(false)));
            ReturnErrorOnFailure(commandSender->SendCommandRequest(device->GetSecureSession().Value()));
            mCommandSender.push_back(std::move(commandSender));

            if (repeatDelayInMs.HasValue())
            {
                chip::test_utils::SleepMillis(repeatDelayInMs.Value());
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
};

class InteractionModelWriter
{
public:
    InteractionModelWriter(chip::app::WriteClient::Callback * callback) : mChunkedWriteCallback(callback) {}

protected:
    template <class T>
    CHIP_ERROR WriteAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds,
                              const std::vector<T> & values,
                              const chip::Optional<uint16_t> & timedInteractionTimeoutMs          = chip::NullOptional,
                              const chip::Optional<bool> & suppressResponse                       = chip::NullOptional,
                              const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional,
                              const chip::Optional<uint16_t> & repeatCount                        = chip::NullOptional,
                              const chip::Optional<uint16_t> & repeatDelayInMs                    = chip::NullOptional)
    {
        InteractionModelConfig::AttributePathsConfig pathsConfig;
        ReturnErrorOnFailure(
            InteractionModelConfig::GetAttributePaths(endpointIds, clusterIds, attributeIds, dataVersions, pathsConfig));

        VerifyOrReturnError(pathsConfig.count == values.size() || values.size() == 1, CHIP_ERROR_INVALID_ARGUMENT);

        uint16_t repeat = repeatCount.ValueOr(1);
        while (repeat--)
        {

            mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), &mChunkedWriteCallback,
                                                                    timedInteractionTimeoutMs, suppressResponse.ValueOr(false));
            VerifyOrReturnError(mWriteClient != nullptr, CHIP_ERROR_NO_MEMORY);

            for (uint8_t i = 0; i < pathsConfig.count; i++)
            {
                auto & path        = pathsConfig.attributePathParams[i];
                auto & dataVersion = pathsConfig.dataVersionFilter[i].mDataVersion;
                const T & value    = i >= values.size() ? values.at(0) : values.at(i);
                ReturnErrorOnFailure(EncodeAttribute<T>(path, dataVersion, value));
            }

            ReturnErrorOnFailure(mWriteClient->SendWriteRequest(device->GetSecureSession().Value()));

            if (repeatDelayInMs.HasValue())
            {
                chip::test_utils::SleepMillis(repeatDelayInMs.Value());
            }
        }

        return CHIP_NO_ERROR;
    }

    template <class T>
    CHIP_ERROR WriteAttribute(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                              std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds, const T & value,
                              const chip::Optional<uint16_t> & timedInteractionTimeoutMs          = chip::NullOptional,
                              const chip::Optional<bool> & suppressResponse                       = chip::NullOptional,
                              const chip::Optional<std::vector<chip::DataVersion>> & dataVersions = chip::NullOptional,
                              const chip::Optional<uint16_t> & repeatCount                        = chip::NullOptional,
                              const chip::Optional<uint16_t> & repeatDelayInMs                    = chip::NullOptional)
    {
        std::vector<T> values = { value };
        return WriteAttribute(device, endpointIds, clusterIds, attributeIds, values, timedInteractionTimeoutMs, suppressResponse,
                              dataVersions, repeatCount, repeatDelayInMs);
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

        return InteractionModelWriter::WriteAttribute(device, endpointIds, clusterIds, attributeIds, value,
                                                      timedInteractionTimeoutMs, suppressResponse, optionalDataVersions);
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

        return InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value, timedInteractionTimeoutMs,
                                                     suppressResponse);
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
