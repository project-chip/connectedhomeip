#include "DiagnosticDataDelegate.h"
#include <esp_diagnostics_metrics.h>
#include <esp_log.h>
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostic_trace/Diagnostics.h>
#include <unordered_map>

using namespace chip::Tracing::Diagnostics;

namespace chip {
namespace Diagnostics {

enum class ValueType : uint8_t
{
    kUndefined,    // Value is not valid
    kInt32,        // int32_t
    kUInt32,       // uint32_t
    kChipErrorCode // chip::ChipError
};

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
        printf("Starting periodic diagnostic timer\n");
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
        printf("inside enable diagnostics\n");
        VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE);
        if (IsDiagnosticBufferEmpty())
        {
            printf("diagnostic buffer is empty\n");
            return CHIP_ERROR_NOT_FOUND;
        }

        // First try as trace data
        Diagnostic<const char*> trace("", "", 0);
        CHIP_ERROR traceErr = mStorageInstance->RetrieveDecodedEntry(trace);
        if (traceErr == CHIP_NO_ERROR)
        {
            printf("Retrieved trace data - Label: %s, Value: %s\n", 
                   trace.GetLabel(), trace.GetValue());
            LogTraceData(trace.GetLabel(), trace.GetValue());
        }
        else
        {
            printf("Failed to retrieve trace data: %s\n", chip::ErrorStr(traceErr));
            
            // If trace retrieval failed, try as uint32
            Diagnostic<uint32_t> metricUint("", 0, 0);
            CHIP_ERROR metricUintErr = mStorageInstance->RetrieveDecodedEntry(metricUint);
            if (metricUintErr == CHIP_NO_ERROR)
            {
                printf("Retrieved uint32 metric - Label: %s, Value: %lu\n", 
                       metricUint.GetLabel(), metricUint.GetValue());
                LogMetricData(metricUint.GetLabel(), ValueType::kUInt32, metricUint.GetValue());
            }
            else
            {
                printf("Failed to retrieve uint32 metric: %s\n", chip::ErrorStr(metricUintErr));
                
                // Try as int32
                Diagnostic<int32_t> metricInt("", 0, 0);
                CHIP_ERROR metricIntErr = mStorageInstance->RetrieveDecodedEntry(metricInt);
                if (metricIntErr == CHIP_NO_ERROR)
                {
                    printf("Retrieved int32 metric - Label: %s, Value: %ld\n", 
                           metricInt.GetLabel(), metricInt.GetValue());
                    LogMetricData(metricInt.GetLabel(), ValueType::kInt32, 
                                static_cast<uint32_t>(metricInt.GetValue()));
                }
                else
                {
                    printf("Failed to retrieve int32 metric: %s\n", chip::ErrorStr(metricIntErr));
                    return metricIntErr;
                }
            }
        }

        // Clear buffer after successful processing
        mStorageInstance->ClearBuffer(1);
        return CHIP_NO_ERROR;
    }
#else
    CHIP_ERROR EnableDiagnostics() override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE

private:
    Tracing::Diagnostics::DiagnosticStorageInterface * mStorageInstance = nullptr;
#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    uint8_t mRetrievalBuffer[CONFIG_RETRIEVAL_BUFFER_SIZE];
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    System::Clock::Timeout mTimeout = System::Clock::kZero;
    std::unordered_map<const char *, ValueType> mRegisteredMetrics;

    bool IsDiagnosticBufferEmpty() override { return (mStorageInstance == nullptr) ? true : mStorageInstance->IsBufferEmpty(); }

    void LogTraceData(const char * label, const char * group)
    {
        printf("Writing trace data to ESP_DIAG\n");
        const char * tag    = "MTR_TRC";
        const char * format = "EV (%" PRIu32 ") %s: %s";
        esp_err_t err       = ESP_OK;
        err                 = esp_diag_log_event(tag, format, esp_log_timestamp(), label, group);
        if (err == ESP_OK)
        {
            printf("Send data to insights successfully\n");
        }
    }

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

    void LogMetricData(const char * key, ValueType type, uint32_t value)
    {
        printf("Writing metric data to ESP_DIAG\n");
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
    }

    static void DiagnosticSamplingHandler(System::Layer * systemLayer, void * context)
    {
        printf("DiagnosticSamplingHandler\n");
        auto * instance = static_cast<DiagnosticDataDelegateImpl *>(context);
        VerifyOrReturn(instance != nullptr);
        VerifyOrReturn(instance->mStorageInstance != nullptr);
        
        while(!instance->mStorageInstance->IsBufferEmpty()) {
            // Retrieve and send diagnostics
            instance->EnableDiagnostics();   
        }

        // Schedule next sampling
        DeviceLayer::SystemLayer().StartTimer(instance->mTimeout, DiagnosticSamplingHandler, instance);
    }
};

DiagnosticDataDelegate & DiagnosticDataDelegate::GetInstance(Tracing::Diagnostics::DiagnosticStorageInterface * storageInstance)
{
    return DiagnosticDataDelegateImpl::GetInstance(storageInstance);
}

} // namespace Diagnostics
} // namespace chip
