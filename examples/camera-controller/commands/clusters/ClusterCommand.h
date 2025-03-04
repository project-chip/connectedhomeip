/*
 *   Copyright (c) 2025 Project CHIP Authors
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

            ChipLogError(NotSpecified, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
            return;
        }

        if (data != nullptr)
        {
            LogErrorOnFailure(RemoteDataModelLogger::LogCommandAsJSON(path, data));
        }
    }

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override
    {
        LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(error));

        ChipLogProgress(NotSpecified, "Error: %s", chip::ErrorStr(error));
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

    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mPayload;
};
