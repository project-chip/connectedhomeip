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
 *          Defines the public interface for the Device Layer DiagnosticDataProvider object.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <platform/CHIPDeviceBuildConfig.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace DeviceLayer {

// Maximum length of vendor defined name or prefix of the software thread that is
// static for the duration of the thread.
static constexpr size_t kMaxThreadNameLength = 32;

struct ThreadMetrics : public app::Clusters::SoftwareDiagnostics::Structs::ThreadMetrics::Type
{
    char NameBuf[kMaxThreadNameLength + 1];
    ThreadMetrics * Next; /* Pointer to the next structure.  */
};

/**
 * Defines the delegate class of Platform Manager to notify platform updates.
 */
class DiagnosticsDelegate
{
public:
    virtual ~DiagnosticsDelegate() {}

    /**
     * @brief
     *   Called after the current device is rebooted
     */
    virtual void OnDeviceRebooted() {}
};

/**
 * Provides access to runtime and build-time configuration information for a chip device.
 */
class DiagnosticDataProvider
{
public:
    void SetDelegate(DiagnosticsDelegate * delegate) { mDelegate = delegate; }
    DiagnosticsDelegate * GetDelegate() const { return mDelegate; }

    /**
     * General Diagnostics methods.
     */
    virtual CHIP_ERROR GetRebootCount(uint16_t & rebootCount);
    virtual CHIP_ERROR GetUpTime(uint64_t & upTime);
    virtual CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours);
    virtual CHIP_ERROR GetBootReason(uint8_t & bootReason);
    virtual CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults);
    virtual CHIP_ERROR GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults);
    virtual CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults);

    /**
     * Software Diagnostics methods.
     */
    virtual CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree);
    virtual CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed);
    virtual CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark);

    /*
     * Get the linked list of thread metrics of the current plaform. After usage, each caller of GetThreadMetrics
     * needs to release the thread metrics list it gets via ReleaseThreadMetrics.
     *
     */
    virtual CHIP_ERROR GetThreadMetrics(ThreadMetrics ** threadMetricsOut);
    virtual void ReleaseThreadMetrics(ThreadMetrics * threadMetrics);

protected:
    // Construction/destruction limited to subclasses.
    DiagnosticDataProvider()          = default;
    virtual ~DiagnosticDataProvider() = default;

private:
    DiagnosticsDelegate * mDelegate = nullptr;

    // No copy, move or assignment.
    DiagnosticDataProvider(const DiagnosticDataProvider &)  = delete;
    DiagnosticDataProvider(const DiagnosticDataProvider &&) = delete;
    DiagnosticDataProvider & operator=(const DiagnosticDataProvider &) = delete;
};

/**
 * Returns a reference to a DiagnosticDataProvider object.
 *
 * Applications should use this to access the features of the DiagnosticDataProvider.
 */
DiagnosticDataProvider & GetDiagnosticDataProvider();

/**
 * Sets a reference to a DiagnosticDataProvider object.
 *
 * This must be called before any calls to GetDiagnosticDataProvider. If a nullptr is passed in,
 * no changes will be made.
 */
void SetDiagnosticDataProvider(DiagnosticDataProvider * diagnosticDataProvider);

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline void DiagnosticDataProvider::ReleaseThreadMetrics(ThreadMetrics * threadMetrics) {}

inline CHIP_ERROR DiagnosticDataProvider::GetRebootCount(uint16_t & rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetUpTime(uint64_t & upTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetBootReason(uint8_t & bootReason)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace DeviceLayer
} // namespace chip
