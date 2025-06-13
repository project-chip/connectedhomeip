#include "DiagnosticDataDelegate.h"
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>
#include <map>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

#if CONFIG_ESP_INSIGHTS_ENABLED
#include <esp_diagnostics_metrics.h>
#endif // CONFIG_ENABLE_ESP_INSIGHTS

#define kMaxStringValueSize 128

using namespace chip::Tracing::Diagnostics;

namespace chip {
namespace Diagnostics {

// Make implementation class public so it can be used in static_cast
class DiagnosticDataDelegateImpl : public DiagnosticDataDelegate
{
public:
    static DiagnosticDataDelegateImpl & GetInstance(Tracing::Diagnostics::CircularDiagnosticBuffer * storageInstance)
    {
        static DiagnosticDataDelegateImpl * mInstance = nullptr;
        if (mInstance == nullptr)
        {
            mInstance = new DiagnosticDataDelegateImpl(storageInstance);
        }
        return *mInstance;
    }

    DiagnosticDataDelegateImpl(Tracing::Diagnostics::CircularDiagnosticBuffer * storageInstance) : mStorageInstance(storageInstance)
    {}

    CHIP_ERROR StartPeriodicDiagnostics(chip::System::Clock::Timeout aTimeout) override
    {
        if (aTimeout == System::Clock::kZero)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mTimeout = aTimeout;
        return DeviceLayer::SystemLayer().StartTimer(mTimeout, DiagnosticSamplingHandler, this);
    }

    CHIP_ERROR StopPeriodicDiagnostics() override
    {
        DeviceLayer::SystemLayer().CancelTimer(DiagnosticSamplingHandler, this);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSamplingInterval(chip::System::Clock::Timeout aTimeout) override
    {
        mTimeout = aTimeout;

        if (mTimeout == System::Clock::kZero)
        {
            return StopPeriodicDiagnostics();
        }

        // Cancel existing timer and start new one
        DeviceLayer::SystemLayer().CancelTimer(DiagnosticSamplingHandler, this);
        return DeviceLayer::SystemLayer().StartTimer(mTimeout, DiagnosticSamplingHandler, this);
    }

    CHIP_ERROR EnableDiagnostics() override
    {
        uint32_t bufferSize = mStorageInstance->GetDataSize();
        uint8_t retrievalBuffer[bufferSize];
        MutableByteSpan encodedSpan(retrievalBuffer, bufferSize);
        uint32_t readEntries = 0;

        // Retrieve encoded data
        CHIP_ERROR err = mStorageInstance->Retrieve(encodedSpan, readEntries);
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_BUFFER_TOO_SMALL && err != CHIP_ERROR_END_OF_TLV)
        {
            ESP_LOGE("Diagnostics", "Failed to retrieve data");
            return err;
        }

        chip::TLV::TLVReader mReader;
        mReader.Init(encodedSpan.data(), encodedSpan.size());
        while ((err = mReader.Next()) == CHIP_NO_ERROR)
        {
            if (mReader.GetType() == chip::TLV::kTLVType_Structure && mReader.GetTag() == chip::TLV::AnonymousTag())
            {
                DiagnosticEntry entry;
                err = Decode(mReader, entry);
                if (err == CHIP_NO_ERROR)
                {
                    if (entry.type == ValueType::kCharString)
                    {
                        LogTraceData(entry);
                    }
                    else if (entry.type == ValueType::kSignedInteger || entry.type == ValueType::kUnsignedInteger)
                    {
                        LogMetricData(entry);
                    }
                    else
                    {
                        ESP_LOGE("Diagnostics", "Unsupported data type");
                    }
                }
                else
                {
                    ESP_LOGE("Diagnostics", "Failed to decode diagnostic");
                }
            }
            else
            {
                ESP_LOGW("Diagnostics", "Skipping unexpected TLV element.");
            }
        }
        // Clear buffer after successful processing
        mStorageInstance->ClearBuffer(readEntries);
        return CHIP_NO_ERROR;
    }

private:
    Tracing::Diagnostics::CircularDiagnosticBuffer * mStorageInstance = nullptr;
    System::Clock::Timeout mTimeout                                   = System::Clock::kZero;
    struct StringCompare
    {
        bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; }
    };
    std::map<const char *, ValueType, StringCompare> mRegisteredMetrics;

    bool IsDiagnosticBufferEmpty() override { return (mStorageInstance == nullptr) ? true : mStorageInstance->IsBufferEmpty(); }

    void LogTraceData(const DiagnosticEntry & entry)
    {
#if CONFIG_ESP_INSIGHTS_ENABLED
        const char * tag    = "MTR_TRC";
        const char * format = "EV (%" PRIu32 ") %s: %s";
        esp_err_t err       = esp_diag_log_event(tag, format, entry.timestamps_ms_since_boot, entry.label, entry.stringValue);
        if (err == ESP_OK)
        {
            ESP_LOGD("MTR_TRC", "Event %s logged successfully", entry.label);
        }
#endif // CONFIG_ENABLE_ESP_INSIGHTS
    }

#if CONFIG_ESP_INSIGHTS_ENABLED
    void RegisterMetric(const char * key, ValueType type)
    {
        // Check for the same key will not have two different types.
        if (mRegisteredMetrics.find(key) != mRegisteredMetrics.end())
        {
            if (mRegisteredMetrics[key] != type)
            {
                ESP_LOGE("SYS.MTR", "Type mismatch for metric key %s", key);
                return;
            }
        }

        switch (type)
        {
        case ValueType::kUnsignedInteger: {
            esp_err_t err = esp_diag_metrics_register(
                "SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_UINT /* data_type */);
            if (err == ESP_OK)
            {
                ESP_LOGD("SYS.MTR", "Metric %s registered successfully", key);
            }
            break;
        }

        case ValueType::kSignedInteger: {
            esp_err_t err = esp_diag_metrics_register(
                "SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_INT /* data_type */);
            if (err == ESP_OK)
            {
                ESP_LOGD("SYS.MTR", "Metric %s registered successfully", key);
            }
            break;
        }

        default:
            ESP_LOGE("mtr", "failed to register %s as its value is undefined", key);
            break;
        }

        mRegisteredMetrics[key] = type;
    }
#endif // CONFIG_ENABLE_ESP_INSIGHTS
    void LogMetricData(const DiagnosticEntry & entry)
    {
#if CONFIG_ESP_INSIGHTS_ENABLED
        if (mRegisteredMetrics.find(entry.label) == mRegisteredMetrics.end())
        {
            RegisterMetric(entry.label, entry.type);
        }

        // Use a fixed-size buffer for log messages
        static constexpr size_t MAX_LOG_LENGTH = 128;
        char logBuffer[MAX_LOG_LENGTH];

        switch (entry.type)
        {
        case ValueType::kSignedInteger: {
            snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is %ld", entry.label, static_cast<int32_t>(entry.intValue));
            esp_err_t err = esp_diag_metrics_add_int(entry.label, static_cast<int32_t>(entry.intValue));
            if (err == ESP_OK)
            {
                ESP_LOGD("SYS.MTR", "Metric %s added successfully", entry.label);
            }
            break;
        }

        case ValueType::kUnsignedInteger: {
            snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is %lu", entry.label, entry.uintValue);
            esp_err_t err = esp_diag_metrics_add_uint(entry.label, entry.uintValue);
            if (err == ESP_OK)
            {
                ESP_LOGD("SYS.MTR", "Metric %s added successfully", entry.label);
            }
            break;
        }

        default:
            ESP_LOGD("mtr", "The value of %s is of an UNKNOWN TYPE", entry.label);
            break;
        }
#endif // CONFIG_ENABLE_ESP_INSIGHTS
    }

    static void DiagnosticSamplingHandler(System::Layer * systemLayer, void * context)
    {
        auto * instance = static_cast<DiagnosticDataDelegateImpl *>(context);
        VerifyOrReturn(instance != nullptr);
        VerifyOrReturn(instance->mStorageInstance != nullptr);

        while (!instance->mStorageInstance->IsBufferEmpty())
        {
            instance->EnableDiagnostics();
        }

        // Schedule next sampling
        DeviceLayer::SystemLayer().StartTimer(instance->mTimeout, DiagnosticSamplingHandler, instance);
        ESP_LOGD("Diagnostics", "Free heap Memory: %ld\n", esp_get_free_heap_size());
    }
};

DiagnosticDataDelegate & DiagnosticDataDelegate::GetInstance(Tracing::Diagnostics::CircularDiagnosticBuffer * storageInstance)
{
    return DiagnosticDataDelegateImpl::GetInstance(storageInstance);
}

} // namespace Diagnostics
} // namespace chip
