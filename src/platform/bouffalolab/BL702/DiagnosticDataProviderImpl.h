/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <memory>

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
    CHIP_ERROR GetThreadMetrics(ThreadMetrics ** threadMetricsOut) override;
    void ReleaseThreadMetrics(ThreadMetrics * threadMetrics) override;
    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults) override;
    CHIP_ERROR GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults) override;
    CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults) override;
    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;
};

} // namespace DeviceLayer
} // namespace chip
