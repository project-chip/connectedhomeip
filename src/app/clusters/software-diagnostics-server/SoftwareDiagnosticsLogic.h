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

#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/SoftwareDiagnostics/Attributes.h>
#include <clusters/SoftwareDiagnostics/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

/// Type-safe implementation for callbacks for the SoftwareDiagnostics server
class SoftwareDiagnosticsLogic
{
public:
    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
        SoftwareDiagnostics::Attributes::ThreadMetrics::Id, SoftwareDiagnostics::Attributes::CurrentHeapFree::Id,
        SoftwareDiagnostics::Attributes::CurrentHeapUsed::Id, SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id>;

    SoftwareDiagnosticsLogic(const OptionalAttributeSet & optionalAttributeSet) : mOptionalAttributeSet(optionalAttributeSet) {}
    virtual ~SoftwareDiagnosticsLogic() = default;

    CHIP_ERROR GetCurrentHeapFree(uint64_t & out) const { return DeviceLayer::GetDiagnosticDataProvider().GetCurrentHeapFree(out); }
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & out) const { return DeviceLayer::GetDiagnosticDataProvider().GetCurrentHeapUsed(out); }
    CHIP_ERROR GetCurrentHighWatermark(uint64_t & out) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetCurrentHeapHighWatermark(out);
    }

    // Encodes the thread metrics list, using the provided encoder.
    CHIP_ERROR ReadThreadMetrics(AttributeValueEncoder & encoder) const;

    /// Determines the feature map based on the DiagnosticsProvider support.
    BitFlags<SoftwareDiagnostics::Feature> GetFeatureMap() const
    {
        return BitFlags<SoftwareDiagnostics::Feature>().Set(
            SoftwareDiagnostics::Feature::kWatermarks,
            mOptionalAttributeSet.IsSet(SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Id) &&
                DeviceLayer::GetDiagnosticDataProvider().SupportsWatermarks());
    }

    CHIP_ERROR ResetWatermarks() { return DeviceLayer::GetDiagnosticDataProvider().ResetWatermarks(); }

    /// Returns acceptable attributes for the given Diagnostics data provider:
    ///   - ALWAYS includes global attributes
    ///   - adds heap/watermark depending on feature flags and if the interface supports it.
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    /// Determines what commands are supported
    CHIP_ERROR AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder);

private:
    const OptionalAttributeSet mOptionalAttributeSet;
};

} // namespace Clusters
} // namespace app
} // namespace chip
