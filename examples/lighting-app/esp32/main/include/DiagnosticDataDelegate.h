#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>
#include <system/SystemStats.h>
#include <tracing/esp32_diagnostic_trace/Diagnostics.h>

namespace chip {
namespace Diagnostics {

/**
 * @brief Delegate interface for handling diagnostic data operations
 */
class DiagnosticDataDelegate
{
public:
    virtual ~DiagnosticDataDelegate() = default;

    /**
     * @brief Retrieve diagnostic data from storage in TLV format, parse diagnostics and send to insights
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise
     */
    virtual CHIP_ERROR EnableDiagnostics() = 0;

    /**
     * @brief Clear diagnostic data from storage
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise
     */
    virtual CHIP_ERROR ClearSentDiagnosticsData() = 0;

    /**
     * @brief Check if diagnostic buffer is empty
     *
     * @return bool true if empty, false otherwise
     */
    virtual bool IsDiagnosticBufferEmpty() = 0;

    virtual CHIP_ERROR StartPeriodicDiagnostics(chip::System::Clock::Timeout aTimeout) = 0;
    virtual CHIP_ERROR StopPeriodicDiagnostics()                                       = 0;
    virtual CHIP_ERROR SetSamplingInterval(chip::System::Clock::Timeout aTimeout)      = 0;

    static DiagnosticDataDelegate & GetInstance(Tracing::Diagnostics::DiagnosticStorageInterface * storageInstance);
};

// Forward declaration of implementation class
class DiagnosticDataDelegateImpl;

} // namespace Diagnostics
} // namespace chip
