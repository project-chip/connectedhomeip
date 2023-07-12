/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;
    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;


    /// @Neethu Maybe we need to implement ethernet methods as well.
    /**
     * Ethernet network diagnostics methods
     */
    // CHIP_ERROR GetEthPHYRate(app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate);
    // CHIP_ERROR GetEthFullDuplex(bool & fullDuplex);
    // CHIP_ERROR GetEthCarrierDetect(bool & carrierDetect);
    // CHIP_ERROR GetEthTimeSinceReset(uint64_t & timeSinceReset);
    // CHIP_ERROR GetEthPacketRxCount(uint64_t & packetRxCount);
    // CHIP_ERROR GetEthPacketTxCount(uint64_t & packetTxCount);
    // CHIP_ERROR GetEthTxErrCount(uint64_t & txErrCount);
    // CHIP_ERROR GetEthCollisionCount(uint64_t & collisionCount);
    // CHIP_ERROR GetEthOverrunCount(uint64_t & overrunCount);
    // CHIP_ERROR ResetEthNetworkDiagnosticsCounts();
};

/**
 * Returns the platform-specific implementation of the DiagnosticDataProvider singleton object.
 *
 * Applications can use this to gain access to features of the DiagnosticDataProvider
 * that are specific to the selected platform.
 */
DiagnosticDataProvider & GetDiagnosticDataProviderImpl();

} // namespace DeviceLayer
} // namespace chip
