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
#include <clusters/SoftwareDiagnostics/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

struct SoftwareDiagnosticsEnabledAttributes
{
    bool enableThreadMetrics : 1;
    bool enableCurrentHeapFree : 1;
    bool enableCurrentHeapUsed : 1;
    bool enableCurrentWatermarks : 1;
};

/// Type-safe implementation for callbacks for the SoftwareDiagnostics server
class SoftwareDiagnosticsLogic
{
public:
    SoftwareDiagnosticsLogic(const SoftwareDiagnosticsEnabledAttributes enabledAttributes) : mEnabledAttributes(enabledAttributes)
    {}
    virtual ~SoftwareDiagnosticsLogic() = default;

    CHIP_ERROR GetCurrentHeapFree(uint64_t & out) const { return GetDiagnosticDataProvider().GetCurrentHeapFree(out); }
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & out) const { return GetDiagnosticDataProvider().GetCurrentHeapUsed(out); }
    CHIP_ERROR GetCurrentHighWatermark(uint64_t & out) const
    {
        return GetDiagnosticDataProvider().GetCurrentHeapHighWatermark(out);
    }

    // Encodes the thread metrics list, using the provided encoder.
    CHIP_ERROR ReadThreadMetrics(AttributeValueEncoder & encoder) const;

    /// Determines the feature map based on the DiagnosticsProvider support.
    BitFlags<SoftwareDiagnostics::Feature> GetFeatureMap() const
    {
        return BitFlags<SoftwareDiagnostics::Feature>().Set(SoftwareDiagnostics::Feature::kWatermarks,
                                                            mEnabledAttributes.enableCurrentWatermarks &&
                                                                GetDiagnosticDataProvider().SupportsWatermarks());
    }

    CHIP_ERROR ResetWatermarks() { return GetDiagnosticDataProvider().ResetWatermarks(); }

    /// Returns acceptable attributes for the given Diagnostics data provider:
    ///   - ALWAYS includes global attributes
    ///   - adds heap/watermark depending on feature flags and if the interface supports it.
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    /// Determines what commands are supported
    CHIP_ERROR AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder);

protected:
    /// Software Diagnostics provider logic generally forwards all calls to a diagnostic data provider
    ///
    /// To allow unit testing, we have this as a protected member so we can both
    /// use `DeviceLayer::GetDiagnosticDataProvider` and not incur RAM cost and also
    /// have a RAM-based implementation for unit testing.
    [[nodiscard]] virtual DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() const = 0;

private:
    const SoftwareDiagnosticsEnabledAttributes mEnabledAttributes;
};

/// Minimal class that uses DeviceLayer (singleton) diagnostics provider
class DeviceLayerSoftwareDiagnosticsLogic : public SoftwareDiagnosticsLogic
{
public:
    DeviceLayerSoftwareDiagnosticsLogic(const SoftwareDiagnosticsEnabledAttributes enabledAttributes) :
        SoftwareDiagnosticsLogic(enabledAttributes)
    {}

protected:
    [[nodiscard]] DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() const override
    {
        return DeviceLayer::GetDiagnosticDataProvider();
    }
};

/// Minimal class that uses an injected diagnostics provider (i.e. uses RAM but is unit testable)
class InjectedDiagnosticsSoftwareDiagnosticsLogic : public SoftwareDiagnosticsLogic
{
public:
    InjectedDiagnosticsSoftwareDiagnosticsLogic(DeviceLayer::DiagnosticDataProvider & provider,
                                                const SoftwareDiagnosticsEnabledAttributes enabledAttributes) :
        SoftwareDiagnosticsLogic(enabledAttributes),
        mProvider(provider)
    {}

protected:
    [[nodiscard]] DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() const override { return mProvider; }

private:
    DeviceLayer::DiagnosticDataProvider & mProvider;
};

} // namespace Clusters
} // namespace app
} // namespace chip
