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
#include <app/server-cluster/AttributeListBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace SoftwareDiagnostics;

void SoftwareDiagnosticsServerCluster::OnSoftwareFaultDetect(const Events::SoftwareFault::Type & softwareFault)
{
    VerifyOrReturn(mContext != nullptr);
    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(softwareFault, kRootEndpointId);
}

CHIP_ERROR SoftwareDiagnosticsServerCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (SoftwareFaultListener::GetGlobalListener() == nullptr)
    {
        SoftwareFaultListener::SetGlobalListener(this);
    }

    return CHIP_NO_ERROR;
}

void SoftwareDiagnosticsServerCluster::Shutdown(ClusterShutdownType shutdownType)
{
    if (SoftwareFaultListener::GetGlobalListener() == this)
    {
        SoftwareFaultListener::SetGlobalListener(nullptr);
    }
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus SoftwareDiagnosticsServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::CurrentHeapFree::Id: {
        uint64_t value;
        CHIP_ERROR err = mDiagnosticDataProvider.GetCurrentHeapFree(value);
        return EncodeValue(value, err, encoder);
    }
    case Attributes::CurrentHeapUsed::Id: {
        uint64_t value;
        CHIP_ERROR err = mDiagnosticDataProvider.GetCurrentHeapUsed(value);
        return EncodeValue(value, err, encoder);
    }
    case Attributes::CurrentHeapHighWatermark::Id: {
        uint64_t value;
        CHIP_ERROR err = mDiagnosticDataProvider.GetCurrentHeapHighWatermark(value);
        return EncodeValue(value, err, encoder);
    }
    case Attributes::ThreadMetrics::Id:
        return ReadThreadMetrics(encoder);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(GetFeatureMap());
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
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
    case Commands::ResetWatermarks::Id:
        return ResetWatermarks();
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

namespace {

/// Wrapper around `DeviceLayer::GetDiagnosticDataProvider::GetThreadMetrics` that ensures
/// that `ReleaseThreadMetrics` is always called on the underlying value.
class AutoFreeThreadMetrics
{
public:
    AutoFreeThreadMetrics(DeviceLayer::DiagnosticDataProvider & provider) : mProvider(provider) {}
    ~AutoFreeThreadMetrics()
    {
        if (mMetrics != nullptr)
        {
            mProvider.ReleaseThreadMetrics(mMetrics);
        }
    }

    CHIP_ERROR ReadThreadMetrics()
    {
        VerifyOrReturnError(mMetrics == nullptr, CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR err = mProvider.GetThreadMetrics(&mMetrics);
        if (err != CHIP_NO_ERROR)
        {
            mMetrics = nullptr; // do not assume it is valid, so we do not try to free later
        }
        return err;
    }

    const DeviceLayer::ThreadMetrics * ThreadMetrics() const { return mMetrics; }

private:
    DeviceLayer::ThreadMetrics * mMetrics = nullptr;
    DeviceLayer::DiagnosticDataProvider & mProvider;
};

} // namespace

CHIP_ERROR SoftwareDiagnosticsServerCluster::ReadThreadMetrics(AttributeValueEncoder & encoder)
{
    AutoFreeThreadMetrics metrics(mDiagnosticDataProvider);

    CHIP_ERROR err = metrics.ReadThreadMetrics();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "SoftwareDiagnostics: Failed to read thread metrics: %" CHIP_ERROR_FORMAT, err.Format());
        return encoder.EncodeEmptyList();
    }

    return encoder.EncodeList([&metrics](const auto & itemEncoder) -> CHIP_ERROR {
        for (const DeviceLayer::ThreadMetrics * thread = metrics.ThreadMetrics(); thread != nullptr; thread = thread->Next)
        {
            ReturnErrorOnFailure(itemEncoder.Encode(*thread));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR SoftwareDiagnosticsServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    (void) path;

    if (mOptionalAttributeSet.IsSet(Attributes::CurrentHeapHighWatermark::Id) && mDiagnosticDataProvider.SupportsWatermarks())
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::ResetWatermarks::kMetadataEntry };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    // no commands supported
    return CHIP_NO_ERROR;
}

CHIP_ERROR SoftwareDiagnosticsServerCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    (void) path;

    AttributeListBuilder listBuilder(builder);

    static constexpr DataModel::AttributeEntry optionalEntries[] = {
        //
        Attributes::ThreadMetrics::kMetadataEntry,            //
        Attributes::CurrentHeapFree::kMetadataEntry,          //
        Attributes::CurrentHeapUsed::kMetadataEntry,          //
        Attributes::CurrentHeapHighWatermark::kMetadataEntry, //
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalEntries), mOptionalAttributeSet);
}

} // namespace Clusters
} // namespace app
} // namespace chip
