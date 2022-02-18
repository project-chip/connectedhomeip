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
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <lib/support/CodeUtils.h>

class InteractionModel : public chip::app::ReadClient::Callback,
                         public chip::app::WriteClient::Callback,
                         public chip::app::CommandSender::Callback
{
public:
    InteractionModel() : mBufferedReadAdapter(*this), mChunkedWriteCallback(this){};
    virtual ~InteractionModel(){};

    virtual void OnResponse(const chip::app::StatusIB & status, chip::TLV::TLVReader * data) = 0;
    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err)                              = 0;
    virtual chip::DeviceProxy * GetDevice(const char * identity)                             = 0;

    CHIP_ERROR ReadAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                             chip::AttributeId attributeId, bool fabricFiltered = true);

    CHIP_ERROR SubscribeAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                  chip::AttributeId attributeId, uint16_t minInterval, uint16_t maxInterval,
                                  bool fabricFiltered = true);

    CHIP_ERROR ReadEvent(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::EventId eventId,
                         bool fabricFiltered = true);

    CHIP_ERROR SubscribeEvent(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::EventId eventId,
                              uint16_t minInterval, uint16_t maxInterval, bool fabricFiltered = true);

    CHIP_ERROR WaitForReport() { return CHIP_NO_ERROR; }

    template <class T>
    CHIP_ERROR WriteAttribute(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId,
                              chip::AttributeId attributeId, const T & value,
                              chip::Optional<uint16_t> timedInteractionTimeoutMs = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::AttributePathParams attributePathParams;
        if (endpointId != chip::kInvalidEndpointId)
        {
            attributePathParams.mEndpointId = endpointId;
        }

        if (clusterId != chip::kInvalidClusterId)
        {
            attributePathParams.mClusterId = clusterId;
        }

        if (attributeId != chip::kInvalidAttributeId)
        {
            attributePathParams.mAttributeId = attributeId;
        }

        mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), &mChunkedWriteCallback,
                                                                timedInteractionTimeoutMs);
        VerifyOrReturnError(mWriteClient != nullptr, CHIP_ERROR_NO_MEMORY);

        // TODO: Add data version supports
        chip::Optional<chip::DataVersion> dataVersion = chip::NullOptional;
        ReturnErrorOnFailure(mWriteClient->EncodeAttribute(attributePathParams, value, dataVersion));
        return mWriteClient->SendWriteRequest(device->GetSecureSession().Value());
    }

    template <class T>
    CHIP_ERROR WriteGroupAttribute(const char * identity, chip::GroupId groupId, chip::ClusterId clusterId,
                                   chip::AttributeId attributeId, const T & value,
                                   chip::Optional<uint16_t> timedInteractionTimeoutMs = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::AttributePathParams attributePathParams;

        if (clusterId != chip::kInvalidClusterId)
        {
            attributePathParams.mClusterId = clusterId;
        }

        if (attributeId != chip::kInvalidAttributeId)
        {
            attributePathParams.mAttributeId = attributeId;
        }

        mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), &mChunkedWriteCallback,
                                                                timedInteractionTimeoutMs);
        VerifyOrReturnError(mWriteClient != nullptr, CHIP_ERROR_NO_MEMORY);

        // TODO: Add data version supports
        chip::Optional<chip::DataVersion> dataVersion = chip::NullOptional;
        ReturnErrorOnFailure(mWriteClient->EncodeAttribute(attributePathParams, value, dataVersion));

        chip::FabricIndex fabricIndex = device->GetSecureSession().Value()->GetFabricIndex();
        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        return mWriteClient->SendWriteRequest(chip::SessionHandle(session));
    }

    template <class T>
    CHIP_ERROR SendCommand(const char * identity, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                           const T & value, chip::Optional<uint16_t> timedInteractionTimeoutMs = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::CommandPathParams commandPath = { endpointId, 0 /* groupId */, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kEndpointIdValid) };
        mCommandSender =
            std::make_unique<chip::app::CommandSender>(this, device->GetExchangeManager(), timedInteractionTimeoutMs.HasValue());
        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(mCommandSender->AddRequestDataNoTimedCheck(commandPath, value, timedInteractionTimeoutMs));
        return mCommandSender->SendCommandRequest(device->GetSecureSession().Value());
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(const char * identity, chip::GroupId groupId, chip::ClusterId clusterId, chip::CommandId commandId,
                                const T & value, chip::Optional<uint16_t> timedInteractionTimeoutMs = chip::NullOptional)
    {
        chip::DeviceProxy * device = GetDevice(identity);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);

        chip::app::CommandPathParams commandPath = { 0 /* endpoint */, groupId, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kGroupIdValid) };

        mCommandSender =
            std::make_unique<chip::app::CommandSender>(this, device->GetExchangeManager(), timedInteractionTimeoutMs.HasValue());
        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::FabricIndex fabricIndex = device->GetSecureSession().Value()->GetFabricIndex();
        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        ReturnErrorOnFailure(mCommandSender->AddRequestDataNoTimedCheck(commandPath, value, timedInteractionTimeoutMs));
        return mCommandSender->SendGroupCommandRequest(chip::SessionHandle(session));
    }

    void Shutdown();

    /////////// ReadClient Callback Interface /////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
    void OnEventData(const chip::app::EventHeader & eventHeader, chip::TLV::TLVReader * data,
                     const chip::app::StatusIB * status) override;
    void OnError(CHIP_ERROR error) override;
    void OnDone() override;
    void OnSubscriptionEstablished(uint64_t subscriptionId) override;

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

protected:
    std::unique_ptr<chip::app::ReadClient> mReadClient;
    // TODO: Add multiple subscriptions at the same time supports
    std::unique_ptr<chip::app::ReadClient> mSubscribeClient;
    std::unique_ptr<chip::app::WriteClient> mWriteClient;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;

    chip::app::BufferedReadCallback mBufferedReadAdapter;
    chip::app::ChunkedWriteCallback mChunkedWriteCallback;
};
