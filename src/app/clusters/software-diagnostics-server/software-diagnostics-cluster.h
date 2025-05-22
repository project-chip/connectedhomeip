/*
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

#include <app/clusters/software-diagnostics-server/software-diagnostics-logic.h>

#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/SoftwareDiagnostics/ClusterId.h>
#include <clusters/SoftwareDiagnostics/Commands.h>
#include <clusters/SoftwareDiagnostics/Events.h>
#include <clusters/SoftwareDiagnostics/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>
#include <sys/types.h>

namespace chip {
namespace app {
namespace Clusters {

/// Integration of Software diagnostics logic within the Matter data model
///
/// Translates between matter calls and Software Diagnostics logic
///
/// This cluster is expected to only ever exist on endpoint 0 as it is a singleton cluster.
template <typename LOGIC>
class SoftwareDiagnosticsServerCluster : public DefaultServerCluster,
                                         public SoftwareDiagnostics::SoftwareFaultListener,
                                         private LOGIC
{
public:
    template <typename... Args>
    SoftwareDiagnosticsServerCluster(Args &&... args) :
        DefaultServerCluster({ kRootEndpointId, SoftwareDiagnostics::Id }), LOGIC(std::forward<Args>(args)...)
    {}

    // software fault listener
    void OnSoftwareFaultDetect(const SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault) override
    {
        VerifyOrReturn(mContext != nullptr);
        (void) mContext->interactionContext->eventsGenerator->GenerateEvent(softwareFault, kRootEndpointId);
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

        if (SoftwareDiagnostics::SoftwareFaultListener::GetGlobalListener() == nullptr)
        {
            SoftwareDiagnostics::SoftwareFaultListener::SetGlobalListener(this);
        }

        return CHIP_NO_ERROR;
    }

    void Shutdown() override
    {
        if (SoftwareDiagnostics::SoftwareFaultListener::GetGlobalListener() == this)
        {
            SoftwareDiagnostics::SoftwareFaultListener::SetGlobalListener(nullptr);
        }
        DefaultServerCluster::Shutdown();
    }

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case SoftwareDiagnostics::Attributes::CurrentHeapFree::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetCurrentHeapFree(value);
            return EncodeValue(value, err, encoder);
        }
        case SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetCurrentHeapUsed(value);
            return EncodeValue(value, err, encoder);
        }
        case SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id: {
            uint64_t value;
            CHIP_ERROR err = LOGIC::GetCurrentHighWatermark(value);
            return EncodeValue(value, err, encoder);
        }
        case SoftwareDiagnostics::Attributes::ThreadMetrics::Id:
            return LOGIC::ReadThreadMetrics(encoder);
        case Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(LOGIC::GetFeatureMap());
        case Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(SoftwareDiagnostics::kRevision);
        default:
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        switch (request.path.mCommandId)
        {
        case SoftwareDiagnostics::Commands::ResetWatermarks::Id:
            return LOGIC::ResetWatermarks();
        default:
            return Protocols::InteractionModel::Status::UnsupportedCommand;
        }
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return LOGIC::AcceptedCommands(builder);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        return LOGIC::Attributes(builder);
    }

private:
    // Encodes the `value` in `encoder`, while handling a potential `readError` that occured
    // when the input `value` was read:
    //   - CHIP_ERROR_NOT_IMPLEMENTED results in a 0 being encoded
    //   - any other read error is just forwarded
    CHIP_ERROR EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
    {
        if (readError == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            value = 0;
        }
        else if (readError != CHIP_NO_ERROR)
        {
            return readError;
        }
        return encoder.Encode(value);
    }
};

} // namespace Clusters
} // namespace app
} // namespace chip
