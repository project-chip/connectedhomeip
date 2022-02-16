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
        ModelCommand::AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand("write-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        ModelCommand::AddArguments();
    }

    WriteAttribute(const char * attributeName, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand("write", credsIssuerConfig)
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId) override
    {
        return WriteAttribute::SendCommand(device, endpointId, mClusterId, mAttributeId, mAttributeValue);
    }

    /////////// WriteClient Callback Interface /////////
    void OnResponse(const chip::app::WriteClient * client, const chip::app::ConcreteDataAttributePath & path,
                    chip::app::StatusIB status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            SetCommandExitStatus(error);
            return;
        }
    }

    void OnError(const chip::app::WriteClient * client, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        SetCommandExitStatus(error);
    }

    void OnDone(chip::app::WriteClient * client) override
    {
        mWriteClient.reset();
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    template <class T>
    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::AttributeId attributeId, const T & value)
    {
        ChipLogProgress(chipTool, "Sending WriteAttribute to cluster " ChipLogFormatMEI " on endpoint %" PRIu16,
                        ChipLogValueMEI(clusterId), endpointId);
        chip::app::AttributePathParams attributePathParams;
        if (!device->GetSecureSession().Value()->IsGroupSession())
        {
            attributePathParams.mEndpointId = endpointId;
        }
        attributePathParams.mClusterId   = clusterId;
        attributePathParams.mAttributeId = attributeId;

        mWriteClient = std::make_unique<chip::app::WriteClient>(device->GetExchangeManager(), this, mTimedInteractionTimeoutMs);

        ReturnErrorOnFailure(mWriteClient->EncodeAttribute(attributePathParams, value, mDataVersion));

        return mWriteClient->SendWriteRequest(device->GetSecureSession().Value());
    }

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<chip::DataVersion> mDataVersion = chip::NullOptional;
    CustomArgument mAttributeValue;
    std::unique_ptr<chip::app::WriteClient> mWriteClient;
};
