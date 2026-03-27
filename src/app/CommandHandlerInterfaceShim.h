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

#include "CommandHandlerInterface.h"

#include <clusters/MetadataQuery.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SplitLambda.h>

namespace chip {
namespace app {

/// class CommandHandlerInterfaceShim
/// @brief Use this as a quick shim, but actual usage is NOT recommended.
///        This class provides a convenience conversion for updating into the new CHI interface, its a drop-in replacement for the
///        old interface.
//         However we DON'T expect people to use this very long term
///
/// @tparam  TClusterIds A list of the IDs the shim will search the metadata of, leave empty to search for the metadata in all
///          clusters
template <ClusterId... TClusterIds>
class CommandHandlerInterfaceShim : public CommandHandlerInterface
{

    using CommandHandlerInterface::CommandHandlerInterface;

private:
    std::optional<DataModel::AcceptedCommandEntry> GetEntry(const ConcreteClusterPath & cluster, CommandId command)
    {
        return DataModel::detail::AcceptedCommandEntryFor<TClusterIds...>(cluster.mClusterId, command);
    }

public:
    CHIP_ERROR RetrieveAcceptedCommands(const ConcreteClusterPath & cluster,
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
            auto opt_entry = GetEntry(cluster, commandId);
            if (!opt_entry)
            {
                err = CHIP_ERROR_NOT_FOUND;
            }
            else
            {
                err = builder.Append(*opt_entry);
            }
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

    CHIP_ERROR RetrieveGeneratedCommands(const ConcreteClusterPath & cluster, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        size_t commandCount = 0;
        CHIP_ERROR err      = CHIP_NO_ERROR;

        auto counter = SplitLambda([&](CommandId commandId) {
            commandCount++;
            return Loop::Continue;
        });

        ReturnErrorOnFailure(EnumerateGeneratedCommands(cluster, counter.Caller(), counter.Context()));
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

        auto appender = SplitLambda([&](CommandId commandId) {
            err = builder.Append(commandId);
            return err == CHIP_NO_ERROR ? Loop::Continue : Loop::Break;
        });

        ReturnErrorOnFailure(EnumerateGeneratedCommands(cluster, appender.Caller(), appender.Context()));
        ReturnErrorOnFailure(err);
        // the two invocations MUST return the same sizes
        VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
        return CHIP_NO_ERROR;
    }
};
} // namespace app
} // namespace chip
