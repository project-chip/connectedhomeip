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
#include <app/clusters/software-diagnostics-server/software-diagnostics-logic.h>

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/support/CodeUtils.h>

#include <clusters/SoftwareDiagnostics/Commands.h>
#include <clusters/SoftwareDiagnostics/Ids.h>
#include <clusters/SoftwareDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {
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

using namespace chip::app::Clusters::SoftwareDiagnostics;

CHIP_ERROR SoftwareDiagnosticsLogic::ReadThreadMetrics(AttributeValueEncoder & encoder) const
{
    AutoFreeThreadMetrics metrics(DeviceLayer::GetDiagnosticDataProvider());

    if (metrics.ReadThreadMetrics() != CHIP_NO_ERROR)
    {
        // TODO: silently dropping error is what we historically did. We may want to at least log this...
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

CHIP_ERROR SoftwareDiagnosticsLogic::AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mOptionalAttributeSet.IsSet(Attributes::CurrentHeapHighWatermark::Id) &&
        DeviceLayer::GetDiagnosticDataProvider().SupportsWatermarks())
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::ResetWatermarks::kMetadataEntry };
        return builder.ReferenceExisting(kAcceptedCommands);
    }

    // no commands supported
    return CHIP_NO_ERROR;
}

CHIP_ERROR SoftwareDiagnosticsLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    static constexpr DataModel::AttributeEntry optionalEntries[] = {
        //
        Attributes::ThreadMetrics::kMetadataEntry,            //
        Attributes::CurrentHeapFree::kMetadataEntry,          //
        Attributes::CurrentHeapUsed::kMetadataEntry,          //
        Attributes::CurrentHeapHighWatermark::kMetadataEntry, //
    };

    return listBuilder.Append(Span(SoftwareDiagnostics::Attributes::kMandatoryMetadata), Span(optionalEntries),
                              mOptionalAttributeSet);
}

} // namespace Clusters
} // namespace app
} // namespace chip
