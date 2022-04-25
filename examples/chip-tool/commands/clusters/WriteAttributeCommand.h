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

#include <app/WriteClient.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

class WriteAttribute : public ModelCommand, public chip::app::WriteClient::Callback
{
public:
    WriteAttribute(CredentialIssuerCommands * credsIssuerConfig) : ModelCommand("write-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
        ModelCommand::AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand("write-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
        ModelCommand::AddArguments();
    }

    WriteAttribute(const char * attributeName, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand("write", credsIssuerConfig)
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return WriteAttribute::SendCommand(device, endpointIds.at(0), mClusterId, mAttributeId, mAttributeValue);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) override
    {
        return WriteAttribute::SendGroupCommand(groupId, fabricIndex, mClusterId, mAttributeId, mAttributeValue);
    }

    /////////// WriteClient Callback Interface /////////
    void OnResponse(const chip::app::WriteClient * client, const chip::app::ConcreteDataAttributePath & path,
                    chip::app::StatusIB status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
        }
    }

    void OnError(const chip::app::WriteClient * client, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    void OnDone(chip::app::WriteClient * client) override
    {
        mWriteClient.reset();
        SetCommandExitStatus(mError);
    }

    template <class T>
    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::AttributeId attributeId, const T & value)
    {
        ChipLogProgress(chipTool, "Sending WriteAttribute to cluster " ChipLogFormatMEI " on endpoint %u",
                        ChipLogValueMEI(clusterId), endpointId);
        chip::app::AttributePathParams attributePathParams;
        if (!device->GetSecureSession().Value()->IsGroupSession())
        {
            attributePathParams.mEndpointId = endpointId;
        }
        attributePathParams.mClusterId   = clusterId;
        attributePathParams.mAttributeId = attributeId;

        mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), this, mTimedInteractionTimeoutMs,
                                                                mSuppressResponse.ValueOr(false));

        ReturnErrorOnFailure(mWriteClient->EncodeAttribute(attributePathParams, value, mDataVersion));

        return mWriteClient->SendWriteRequest(device->GetSecureSession().Value());
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                chip::AttributeId attributeId, const T & value)
    {

        chip::app::AttributePathParams attributePathParams;
        attributePathParams.mClusterId   = clusterId;
        attributePathParams.mAttributeId = attributeId;

        chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();

        ChipLogDetail(chipTool, "Sending Write Attribute to Group %u, on Fabric %x, for cluster %u with attributeId %u", groupId,
                      fabricIndex, clusterId, attributeId);

        auto writeClient = chip::Platform::MakeUnique<chip::app::WriteClient>(exchangeManager, this, mTimedInteractionTimeoutMs);
        VerifyOrReturnError(writeClient != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(writeClient->EncodeAttribute(attributePathParams, value, mDataVersion));

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        ReturnErrorOnFailure(writeClient->SendWriteRequest(chip::SessionHandle(session)));
        writeClient.release();

        return CHIP_NO_ERROR;
    }

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<chip::DataVersion> mDataVersion = chip::NullOptional;
    chip::Optional<bool> mSuppressResponse;
    CustomArgument mAttributeValue;
    std::unique_ptr<chip::app::WriteClient> mWriteClient;
};
