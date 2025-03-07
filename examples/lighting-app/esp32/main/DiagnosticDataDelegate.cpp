#include "DiagnosticDataDelegate.h"
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostic_trace/Diagnostics.h>
#include <unordered_map>

#ifdef CONFIG_ENABLE_ESP_INSIGHTS
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
    static DiagnosticDataDelegateImpl & GetInstance(Tracing::Diagnostics::DiagnosticStorageInterface * storageInstance)
    {
        static DiagnosticDataDelegateImpl * mInstance = nullptr;
        if (mInstance == nullptr)
        {
            mInstance = new DiagnosticDataDelegateImpl(storageInstance);
        }
        return *mInstance;
    }

    DiagnosticDataDelegateImpl(Tracing::Diagnostics::DiagnosticStorageInterface * storageInstance) :
        mStorageInstance(storageInstance)
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

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    CHIP_ERROR EnableDiagnostics() override
    {
        VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE);
        if (IsDiagnosticBufferEmpty())
        {
            return CHIP_ERROR_NOT_FOUND;
        }

        // Get the TLV data
        MutableByteSpan encodedSpan(mRetrievalBuffer, CONFIG_RETRIEVAL_BUFFER_SIZE);
        uint32_t readEntries = 0;

        // Retrieve encoded data
        CHIP_ERROR err = mStorageInstance->Retrieve(encodedSpan, readEntries);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE("Diagnostics", "Failed to retrieve data");
            return err;
        }

        chip::TLV::TLVReader mReader;
        char label[kMaxStringValueSize];
        char value[kMaxStringValueSize];
        mReader.Init(encodedSpan.data(), encodedSpan.size());
        while ((err = mReader.Next()) == CHIP_NO_ERROR)
        {
            if (mReader.GetType() == chip::TLV::kTLVType_Structure && mReader.GetTag() == chip::TLV::AnonymousTag())
            {
                chip::TLV::TLVReader tempReader(mReader);
                Diagnostic<char *> trace(label, value, 0);
                err = trace.Decode(tempReader);
                if (err == CHIP_NO_ERROR)
                {
                    LogTraceData(trace.GetLabel(), trace.GetValue(), trace.GetTimestamp());
                    continue;
                }

                chip::TLV::TLVReader tempReader2(mReader);

                Diagnostic<int32_t> metricInt32(label, 0, 0);
                err = metricInt32.Decode(tempReader2);
                if (err == CHIP_NO_ERROR)
                {
                    LogMetricData(metricInt32.GetLabel(), ValueType::kInt32, metricInt32.GetValue());
                    continue;
                }

                chip::TLV::TLVReader tempReader3(mReader);

                Diagnostic<uint32_t> metricUint32(label, 0, 0);
                err = metricUint32.Decode(tempReader3);
                if (err == CHIP_NO_ERROR)
                {
                    LogMetricData(metricUint32.GetLabel(), ValueType::kUInt32, metricUint32.GetValue());
                    continue;
                }

                ESP_LOGE("Diagnostics", "Failed to decode diagnostic");
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
#else
    CHIP_ERROR EnableDiagnostics() override { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE

private:
    Tracing::Diagnostics::DiagnosticStorageInterface * mStorageInstance = nullptr;
#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    uint8_t mRetrievalBuffer[CONFIG_RETRIEVAL_BUFFER_SIZE];
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    System::Clock::Timeout mTimeout = System::Clock::kZero;
    std::unordered_map<const char *, ValueType> mRegisteredMetrics;

    bool IsDiagnosticBufferEmpty() override { return (mStorageInstance == nullptr) ? true : mStorageInstance->IsBufferEmpty(); }

    void LogTraceData(const char * label, const char * group, uint32_t timestamp)
    {
#ifdef CONFIG_ENABLE_ESP_INSIGHTS
        ESP_LOGI("Diagnostics", "Log Trace Data");
        const char * tag    = "MTR_TRC";
        const char * format = "EV (%" PRIu32 ") %s: %s";
        esp_diag_log_event(tag, format, timestamp, label, group);
#endif // CONFIG_ENABLE_ESP_INSIGHTS
    }

#ifdef CONFIG_ENABLE_ESP_INSIGHTS
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
        case ValueType::kUInt32:
            esp_diag_metrics_register("SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                                      "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_UINT /* data_type */);
            break;

        case ValueType::kInt32:
            esp_diag_metrics_register("SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                                      "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_INT /* data_type */);
            break;

        case ValueType::kChipErrorCode:
            esp_diag_metrics_register("SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                                      "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_UINT /* data_type */);
            break;

        case ValueType::kUndefined:
            ESP_LOGE("mtr", "failed to register %s as its value is undefined", key);
            break;
        }

        mRegisteredMetrics[key] = type;
    }
#endif // CONFIG_ENABLE_ESP_INSIGHTS
    void LogMetricData(const char * key, ValueType type, uint32_t value)
    {
#ifdef CONFIG_ENABLE_ESP_INSIGHTS
        ESP_LOGI("Diagnostics", "Log Metric Data");
        if (key == nullptr)
        {
            ESP_LOGE("mtr", "Invalid null pointer for metric key");
            return;
        }

        if (mRegisteredMetrics.find(key) == mRegisteredMetrics.end())
        {
            RegisterMetric(key, type);
        }

        // Use a fixed-size buffer for log messages
        static constexpr size_t MAX_LOG_LENGTH = 128;
        char logBuffer[MAX_LOG_LENGTH];

        switch (type)
        {
        case ValueType::kInt32:
            snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is %ld", key, static_cast<int32_t>(value));
            ESP_LOGI("mtr", "%s", logBuffer);
            esp_diag_metrics_add_int(key, static_cast<int32_t>(value));
            break;

        case ValueType::kUInt32:
            snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is %lu", key, value);
            ESP_LOGI("mtr", "%s", logBuffer);
            esp_diag_metrics_add_uint(key, value);
            break;

        case ValueType::kChipErrorCode:
            snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is error with code %lu", key, value);
            ESP_LOGI("mtr", "%s", logBuffer);
            esp_diag_metrics_add_uint(key, value);
            break;

        case ValueType::kUndefined:
            ESP_LOGI("mtr", "The value of %s is undefined", key);
            break;

        default:
            ESP_LOGI("mtr", "The value of %s is of an UNKNOWN TYPE", key);
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
            // Retrieve and send diagnostics
            instance->EnableDiagnostics();
        }

        // Schedule next sampling
        DeviceLayer::SystemLayer().StartTimer(instance->mTimeout, DiagnosticSamplingHandler, instance);
        ESP_LOGI("Diagnostics", "Free heap Memory: %ld\n", esp_get_free_heap_size());
    }
};

DiagnosticDataDelegate & DiagnosticDataDelegate::GetInstance(Tracing::Diagnostics::DiagnosticStorageInterface * storageInstance)
{
    return DiagnosticDataDelegateImpl::GetInstance(storageInstance);
}

} // namespace Diagnostics
} // namespace chip
