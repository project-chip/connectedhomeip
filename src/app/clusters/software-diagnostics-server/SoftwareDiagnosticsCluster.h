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

#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsLogic.h>
#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
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
class SoftwareDiagnosticsServerCluster : public DefaultServerCluster, public SoftwareDiagnostics::SoftwareFaultListener
{
public:
    SoftwareDiagnosticsServerCluster(const SoftwareDiagnosticsLogic::OptionalAttributeSet & optionalAttributeSet) :
        DefaultServerCluster({ kRootEndpointId, SoftwareDiagnostics::Id }), mLogic(optionalAttributeSet)
    {}

    // software fault listener
    void OnSoftwareFaultDetect(const SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    void Shutdown(ClusterShutdownType shutdownType) override;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return mLogic.AcceptedCommands(builder);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        return mLogic.Attributes(builder);
    }

private:
    // Encodes the `value` in `encoder`, while handling a potential `readError` that occurred
    // when the input `value` was read:
    //   - CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE results in a 0 being encoded
    //   - any other read error is just forwarded
    CHIP_ERROR EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder);

    SoftwareDiagnosticsLogic mLogic;
};

} // namespace Clusters
} // namespace app
} // namespace chip
