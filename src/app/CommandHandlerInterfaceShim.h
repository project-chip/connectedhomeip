/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <CommandHandlerInterface.h>
#include <clusters/AllMetadataBridge.h>

template <ClusterId... TClusterIds>
class CommandHandlerInterfaceShim : public CommandHandlerInterface
{

    using CommandHandlerInterface::CommandHandlerInterface;
    DataModel::AcceptedCommandEntry GetEntry(const ConcreteClusterPath & cluster, CommandId command)
    {
        if constexpr (sizeof...(TClusterIds) == 0)
        {
            return DataModel::AcceptedCommandEntryFor(cluster.mClusterId, command);
        }
        else
        {
            return DataModel::AcceptedCommandEntryFor<TClusterIds...>(cluster.mClusterId, command);
        }
    }

    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        size_t commandCount = 0;
        CHIP_ERROR err      = CHIP_NO_ERROR;

        auto counter = SplitLambda([&](CommandId commandId) {
            commandCount++;
            return Loop::Continue;
        });

        ReturnErrorOnFailure(EnumerateAcceptedCommands(cluster, counter.Caller(), counter.Context()));
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

        auto appender = SplitLambda([&](CommandId commandId) {
            err = builder.Append(GetEntry(cluster, commandId));
            return err == CHIP_NO_ERROR ? Loop::Continue : Loop::Break;
        });

        ReturnErrorOnFailure(EnumerateAcceptedCommands(cluster, appender.Caller(), appender.Context()));
        ReturnErrorOnFailure(err);
        // the two invocations MUST return the same sizes
        VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
        return CHIP_NO_ERROR;
    }

    // [[deprecated("This interface is only provided to make the transition simpler,"
    //              "and it might be removed on any subsequent releases")]]
    virtual CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // [[deprecated("This interface is only provided to make the transition simpler,"
    //              "and it might be removed on any subsequent releases")]]
    virtual CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        size_t commandCount = 0;
        CHIP_ERROR err      = CHIP_NO_ERROR;

        auto counter = SplitLambda([&](CommandId commandId) {
            commandCount++;
            return Loop::Continue;
        });

        ReturnErrorOnFailure(this->EnumerateGeneratedCommands(cluster, counter.Caller(), counter.Context()));
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

        auto appender = SplitLambda([&](CommandId commandId) {
            err = builder.Append(commandId);
            return err == CHIP_NO_ERROR ? Loop::Continue : Loop::Break;
        });

        ReturnErrorOnFailure(this->EnumerateGeneratedCommands(cluster, appender.Caller(), appender.Context()));
        ReturnErrorOnFailure(err);
        // the two invocations MUST return the same sizes
        VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
        return CHIP_NO_ERROR;
    }
};
