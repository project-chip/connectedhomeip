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
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsCluster.h>

namespace chip {
namespace app {
namespace Clusters {
void SoftwareDiagnosticsServerCluster::OnSoftwareFaultDetect(const SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault)
{
    VerifyOrReturn(mContext != nullptr);
    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(softwareFault, kRootEndpointId);
}

CHIP_ERROR SoftwareDiagnosticsServerCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (SoftwareDiagnostics::SoftwareFaultListener::GetGlobalListener() == nullptr)
    {
        SoftwareDiagnostics::SoftwareFaultListener::SetGlobalListener(this);
    }

    return CHIP_NO_ERROR;
}

void SoftwareDiagnosticsServerCluster::Shutdown(ClusterShutdownType shutdownType)
{
    if (SoftwareDiagnostics::SoftwareFaultListener::GetGlobalListener() == this)
    {
        SoftwareDiagnostics::SoftwareFaultListener::SetGlobalListener(nullptr);
    }
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus SoftwareDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case SoftwareDiagnostics::Attributes::CurrentHeapFree::Id: {
        uint64_t value;
        CHIP_ERROR err = mLogic.GetCurrentHeapFree(value);
        return EncodeValue(value, err, encoder);
    }
    case SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id: {
        uint64_t value;
        CHIP_ERROR err = mLogic.GetCurrentHeapUsed(value);
        return EncodeValue(value, err, encoder);
    }
    case SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id: {
        uint64_t value;
        CHIP_ERROR err = mLogic.GetCurrentHighWatermark(value);
        return EncodeValue(value, err, encoder);
    }
    case SoftwareDiagnostics::Attributes::ThreadMetrics::Id:
        return mLogic.ReadThreadMetrics(encoder);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mLogic.GetFeatureMap());
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(SoftwareDiagnostics::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
SoftwareDiagnosticsServerCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case SoftwareDiagnostics::Commands::ResetWatermarks::Id:
        return mLogic.ResetWatermarks();
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR SoftwareDiagnosticsServerCluster::EncodeValue(uint64_t value, CHIP_ERROR readError, AttributeValueEncoder & encoder)
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
} // namespace Clusters
} // namespace app
} // namespace chip
