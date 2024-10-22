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

#include "DataModelLogger.h"
#include "ModelCommand.h"
#include <app/tests/suites/commands/interaction_model/InteractionModel.h>
#include <lib/core/ClusterEnums.h>

class ClusterCommand : public InteractionModelCommands, public ModelCommand, public chip::app::CommandSender::Callback
{
public:
    ClusterCommand(CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand("command-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddByIdArguments();
        AddArguments();
    }

    ClusterCommand(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand("command-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddByIdArguments();
        AddArguments();
    }

    ~ClusterCommand() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return InteractionModelCommands::SendCommand(device, endpointIds.at(0), mClusterId, mCommandId, mPayload);
    }

    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value)
    {
        return InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value);
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId,
                           const chip::app::Clusters::IcdManagement::Commands::UnregisterClient::Type & value)
    {
        ReturnErrorOnFailure(InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value));
        mPeerNodeId = chip::ScopedNodeId(device->GetDeviceId(), device->GetSecureSession().Value()->GetFabricIndex());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId,
                           const chip::app::Clusters::IcdManagement::Commands::RegisterClient::Type & value)
    {
        ReturnErrorOnFailure(InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value));
        mPeerNodeId       = chip::ScopedNodeId(device->GetDeviceId(), device->GetSecureSession().Value()->GetFabricIndex());
        mCheckInNodeId    = chip::ScopedNodeId(value.checkInNodeID, device->GetSecureSession().Value()->GetFabricIndex());
        mMonitoredSubject = value.monitoredSubject;
        mClientType       = value.clientType;
        memcpy(mICDSymmetricKey, value.key.data(), value.key.size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId,
                           const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsRequest::Type & value)
    {
        ReturnErrorOnFailure(InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value));

        if (value.transferFileDesignator.HasValue() &&
            value.requestedProtocol == chip::app::Clusters::DiagnosticLogs::TransferProtocolEnum::kBdx)
        {
            auto sender         = mCommandSender.back().get();
            auto fileDesignator = value.transferFileDesignator.Value();
            BDXDiagnosticLogsServerDelegate::GetInstance().AddFileDesignator(sender, fileDesignator);
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) override
    {
        return InteractionModelCommands::SendGroupCommand(groupId, fabricIndex, mClusterId, mCommandId, mPayload);
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                chip::CommandId commandId, const T & value)
    {
        return InteractionModelCommands::SendGroupCommand(groupId, fabricIndex, clusterId, commandId, value);
    }

    /////////// CommandSender Callback Interface /////////
    virtual void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                            const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(path, status));

            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
            return;
        }

        if (data != nullptr)
        {
            {
                // log a snapshot to not advance the data reader.
                chip::TLV::TLVReader logTlvReader;
                logTlvReader.Init(*data);
                LogErrorOnFailure(RemoteDataModelLogger::LogCommandAsJSON(path, &logTlvReader));
                error = DataModelLogger::LogCommand(path, &logTlvReader);
            }
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }
            if ((path.mEndpointId == chip::kRootEndpointId) && (path.mClusterId == chip::app::Clusters::IcdManagement::Id) &&
                (path.mCommandId == chip::app::Clusters::IcdManagement::Commands::RegisterClientResponse::Id))
            {
                // log a snapshot to not advance the data reader.
                chip::TLV::TLVReader counterTlvReader;
                counterTlvReader.Init(*data);
                chip::app::Clusters::IcdManagement::Commands::RegisterClientResponse::DecodableType value;
                CHIP_ERROR err = chip::app::DataModel::Decode(counterTlvReader, value);
                if (CHIP_NO_ERROR != err)
                {
                    ChipLogError(chipTool, "Failed to decode ICD counter: %" CHIP_ERROR_FORMAT, err.Format());
                    return;
                }
                chip::app::ICDClientInfo clientInfo;

                clientInfo.peer_node         = mPeerNodeId;
                clientInfo.check_in_node     = mCheckInNodeId;
                clientInfo.monitored_subject = mMonitoredSubject;
                clientInfo.start_icd_counter = value.ICDCounter;
                clientInfo.client_type       = mClientType;

                StoreICDEntryWithKey(clientInfo, chip::ByteSpan(mICDSymmetricKey));
            }
        }

        if ((path.mEndpointId == chip::kRootEndpointId) && (path.mClusterId == chip::app::Clusters::IcdManagement::Id) &&
            (path.mCommandId == chip::app::Clusters::IcdManagement::Commands::UnregisterClient::Id))
        {
            ClearICDEntry(mPeerNodeId);
        }
    }

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override
    {
        LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(error));

        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    virtual void OnDone(chip::app::CommandSender * client) override
    {
        if (mCommandSender.size())
        {
            mCommandSender.front().reset();
            mCommandSender.erase(mCommandSender.begin());
        }

        // If the command is repeated N times, wait for all the responses to comes in
        // before exiting.
        bool shouldStop = true;
        if (mRepeatCount.HasValue())
        {
            mRepeatCount.SetValue(static_cast<uint16_t>(mRepeatCount.Value() - 1));
            shouldStop = mRepeatCount.Value() == 0;
        }

        BDXDiagnosticLogsServerDelegate::GetInstance().RemoveFileDesignator(client);

        if (shouldStop)
        {
            SetCommandExitStatus(mError);
        }
    }

    void Shutdown() override
    {
        mError = CHIP_NO_ERROR;
        ModelCommand::Shutdown();
    }

protected:
    ClusterCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand(commandName, credsIssuerConfig)
    {
        // Subclasses are responsible for calling AddArguments.
    }

    void AddByIdArguments()
    {
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload,
                    "The command payload.  This should be a JSON-encoded object, with string representations of field ids as keys. "
                    " The values for the keys are represented as follows, depending on the type:\n"
                    "  * struct: a JSON-encoded object, with field ids as keys.\n"
                    "  * list: a JSON-encoded array of values.\n"
                    "  * null: A literal null.\n"
                    "  * boolean: A literal true or false.\n"
                    "  * unsigned integer: One of:\n"
                    "      a) The number directly, as decimal.\n"
                    "      b) A string starting with \"u:\" followed by decimal digits\n"
                    "  * signed integer: One of:\n"
                    "      a) The number directly, if it's negative.\n"
                    "      b) A string starting with \"s:\" followed by decimal digits\n"
                    "  * single-precision float: A string starting with \"f:\" followed by the number.\n"
                    "  * double-precision float: One of:\n"
                    "      a) The number directly, if it's not an integer.\n"
                    "      b) A string starting with \"d:\" followed by the number.\n"
                    "  * octet string: A string starting with \"hex:\" followed by the hex encoding of the bytes.\n"
                    "  * string: A string with the characters.\n"
                    "\n"
                    "  An example payload may look like this: '{ \"0x0\": { \"0\": null, \"1\": false }, \"1\": [17, \"u:17\"], "
                    "\"0x2\": [ -17, \"s:17\", \"s:-17\" ], \"0x3\": \"f:2\", \"0x4\": [ \"d:3\", 4.5 ], \"0x5\": \"hex:ab12\", "
                    "\"0x6\": \"ab12\" }' and represents:\n"
                    "    Field 0: a struct with two fields, one with value null and one with value false.\n"
                    "    Field 1: A list of unsigned integers.\n"
                    "    Field 2: A list of signed integers.\n"
                    "    Field 3: A single-precision float.\n"
                    "    Field 4: A list of double-precision floats.\n"
                    "    Field 5: A 2-byte octet string.\n"
                    "    Field 6: A 4-char character string.");
    }

    void AddArguments()
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs,
                    "If provided, do a timed invoke with the given timed interaction timeout. See \"7.6.10. Timed Interaction\" in "
                    "the Matter specification.");
        AddArgument("busyWaitForMs", 0, UINT16_MAX, &mBusyWaitForMs,
                    "If provided, block the main thread processing for the given time right after sending a command.");
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
        ModelCommand::AddArguments();
    }

private:
    chip::ClusterId mClusterId;
    chip::CommandId mCommandId;
    // The scoped node ID to which RegisterClient and UnregisterClient command will be sent.  Not set for other commands.
    chip::ScopedNodeId mPeerNodeId;
    // The scoped node ID to which a Check-In message will be sent.  Only set for the RegisterClient command.
    chip::ScopedNodeId mCheckInNodeId;

    // Used to determine if a particular client has an active subscription for the given entry.
    // The MonitoredSubject, when it is a NodeID, MAY be the same as the CheckInNodeID.
    // The MonitoredSubject gives the registering client the flexibility of having a different
    // CheckInNodeID from the MonitoredSubject.
    uint64_t mMonitoredSubject = static_cast<uint64_t>(0);

    // Client type of the client registering
    chip::app::Clusters::IcdManagement::ClientTypeEnum mClientType = chip::app::Clusters::IcdManagement::ClientTypeEnum::kPermanent;

    // Shared secret between the client and the ICD to encrypt the Check-In message.
    uint8_t mICDSymmetricKey[chip::Crypto::kAES_CCM128_Key_Length];

    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mPayload;
};
