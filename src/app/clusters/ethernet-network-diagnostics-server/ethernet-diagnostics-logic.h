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
#include <clusters/EthernetNetworkDiagnostics/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

struct EthernetDiagnosticsEnabledAttributes
{
    bool enableCarrierDetect : 1;
    bool enableCollisionCount : 1;
    bool enableFullDuplex : 1;
    bool enableOverrunCount : 1;
    bool enablePacketRxCount : 1;
    bool enablePacketTxCount : 1;
    bool enablePHYRate : 1;
    bool enableTimeSinceReset : 1;
    bool enableTxErrCount : 1;
};

/// Type-safe implementation for callbacks for the EthernetDiagnostics server
class EthernetDiagnosticsLogic
{
public:
    EthernetDiagnosticsLogic(const EthernetDiagnosticsEnabledAttributes enabledAttributes) : mEnabledAttributes(enabledAttributes)
    {}
    virtual ~EthernetDiagnosticsLogic() = default;

    CHIP_ERROR GetTimeSinceReset(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthTimeSinceReset(out); }
    CHIP_ERROR GetPacketRxCount(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthPacketRxCount(out); }
    CHIP_ERROR GetPacketTxCount(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthPacketTxCount(out); }
    CHIP_ERROR GetTxErrCount(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthTxErrCount(out); }
    CHIP_ERROR GetCollisionCount(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthCollisionCount(out); }
    CHIP_ERROR GetOverrunCount(uint64_t & out) const { return GetDiagnosticDataProvider().GetEthOverrunCount(out); }

    CHIP_ERROR ReadPHYRate(AttributeValueEncoder & encoder) const;
    CHIP_ERROR ReadFullDuplex(AttributeValueEncoder & encoder) const;
    CHIP_ERROR ReadCarrierDetect(AttributeValueEncoder & encoder) const;

    /// Determines the feature map based on the DiagnosticsProvider support.
    BitFlags<EthernetNetworkDiagnostics::Feature> GetFeatureMap() const;

    CHIP_ERROR ResetCounts() { return GetDiagnosticDataProvider().ResetEthNetworkDiagnosticsCounts(); }

    /// Returns acceptable attributes for the given Diagnostics data provider:
    ///   - ALWAYS includes global attributes
    ///   - adds heap/watermark depending on feature flags and if the interface supports it.
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    /// Determines what commands are supported
    CHIP_ERROR AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder);

protected:
    /// Ethernet Diagnostics provider logic generally forwards all calls to a diagnostic data provider
    ///
    /// To allow unit testing, we have this as a protected member so we can both
    /// use `DeviceLayer::GetDiagnosticDataProvider` and not incur RAM cost and also
    /// have a RAM-based implementation for unit testing.
    [[nodiscard]] virtual DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() const = 0;

private:
    const EthernetDiagnosticsEnabledAttributes mEnabledAttributes;
};

/// Minimal class that uses DeviceLayer (singleton) diagnostics provider
class DeviceLayerEthernetDiagnosticsLogic : public EthernetDiagnosticsLogic
{
public:
    DeviceLayerEthernetDiagnosticsLogic(const EthernetDiagnosticsEnabledAttributes enabledAttributes) :
        EthernetDiagnosticsLogic(enabledAttributes)
    {}

protected:
    [[nodiscard]] DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() const override
    {
        return DeviceLayer::GetDiagnosticDataProvider();
    }
};

/// Minimal class that uses an injected diagnostics provider (i.e. uses RAM but is unit testable)
class InjectedDiagnosticsEthernetDiagnosticsLogic : public EthernetDiagnosticsLogic
{
public:
    InjectedDiagnosticsEthernetDiagnosticsLogic(DeviceLayer::DiagnosticDataProvider & provider,
                                                const EthernetDiagnosticsEnabledAttributes enabledAttributes) :
        EthernetDiagnosticsLogic(enabledAttributes),
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
