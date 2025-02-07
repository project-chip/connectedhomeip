#include "DiagnosticDataDelegate.h"
#include <esp_diagnostics_metrics.h>
#include <esp_log.h>
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostic_trace/Diagnostics.h>
#include <unordered_map>

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
        MutableByteSpan span(mRetrievalBuffer, CONFIG_RETRIEVAL_BUFFER_SIZE);
        CHIP_ERROR err = mStorageInstance->Retrieve(span, mReadDiagnosticEntries);
        ParseAndSendDiagnostics(span.data(), span.size());
        return err;
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
    uint32_t mReadDiagnosticEntries = 0;
    System::Clock::Timeout mTimeout = System::Clock::kZero;
    std::unordered_map<const char *, ValueType> mRegisteredMetrics;

    CHIP_ERROR ClearSentDiagnosticsData() override
    {
        VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR err = CHIP_NO_ERROR;
        if (mReadDiagnosticEntries > 0)
        {
            err                    = mStorageInstance->ClearBuffer(mReadDiagnosticEntries);
            mReadDiagnosticEntries = 0;
        }
        return err;
    }

    bool IsDiagnosticBufferEmpty() override { return (mStorageInstance == nullptr) ? true : mStorageInstance->IsBufferEmpty(); }

    CHIP_ERROR ParseAndSendDiagnostics(const uint8_t * inBuffer, uint32_t inBufferSize)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        printf("Parsing and sending diagnostics\n");

        // Create TLV reader for input buffer
        TLV::TLVReader reader;
        reader.Init(inBuffer, inBufferSize);

        while (CHIP_NO_ERROR == (err = reader.Next()))
        {
            if (reader.GetType() == TLV::kTLVType_Structure)
            {
                TLV::TLVType containerType;
                ReturnErrorOnFailure(reader.EnterContainer(containerType));

                uint32_t timestamp = 0;
                char * label       = nullptr;
                uint32_t labelLen  = 0;
                uint32_t valueLen  = 0;

                // Read timestamp
                err = reader.Next(TLV::ContextTag(Tracing::Diagnostics::DiagTag::TIMESTAMP));
                if (err == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(reader.Get(timestamp));
                }

                // Read label
                err = reader.Next(TLV::ContextTag(Tracing::Diagnostics::DiagTag::LABEL));
                if (err == CHIP_NO_ERROR)
                {
                    labelLen = reader.GetLength();
                    label    = (char *) chip::Platform::MemoryAlloc(labelLen + 1);
                    ReturnErrorOnFailure(reader.GetString(label, labelLen + 1));
                }

                // Read value
                err = reader.Next(TLV::ContextTag(Tracing::Diagnostics::DiagTag::VALUE));
                if (err == CHIP_NO_ERROR)
                {
                    switch (reader.GetType())
                    {
                    case TLV::kTLVType_UTF8String: {
                        valueLen     = reader.GetLength();
                        char * value = (char *) chip::Platform::MemoryAlloc(valueLen + 1);
                        ReturnErrorOnFailure(reader.GetString(value, valueLen + 1));

                        LogTraceData(label, value);
                        chip::Platform::MemoryFree((void *) value);
                        break;
                    }
                    case TLV::kTLVType_UnsignedInteger: {
                        uint32_t intValue;
                        ReturnErrorOnFailure(reader.Get(intValue));

                        LogMetricData(label, ValueType::kUInt32, intValue);
                        break;
                    }
                    case TLV::kTLVType_SignedInteger: {
                        int32_t intValue;
                        ReturnErrorOnFailure(reader.Get(intValue));

                        LogMetricData(label, ValueType::kInt32, intValue);
                        break;
                    }
                    default: {
                        LogTraceData(label, "<unsupported type>");
                        break;
                    }
                    }
                }
                chip::Platform::MemoryFree((void *) label);
                ReturnErrorOnFailure(reader.ExitContainer(containerType));
            }
        }

        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        return err;
    }

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
        auto * instance = static_cast<DiagnosticDataDelegateImpl *>(context);

        // Retrieve and send diagnostics
        instance->EnableDiagnostics();

        // Clear processed diagnostics
        instance->ClearSentDiagnosticsData();

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
