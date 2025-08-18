#include "insights-delegate.h"

#include <esp_diagnostics_metrics.h>
#include <esp_heap_caps.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>
#include <map>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticStorage.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticTracing.h>

#define kMaxStringValueSize 128

using namespace chip;
using namespace chip::Tracing;
using namespace chip::Tracing::Diagnostics;
using ValueType = chip::Tracing::Diagnostics::ValueType;

constexpr const char * TAG = "Insights";

namespace chip {
namespace Insights {

CHIP_ERROR InsightsDelegate::Init(InsightsInitParams & initParams)
{
    esp_insights_config_t config = { .log_type      = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_WARNING | ESP_DIAG_LOG_TYPE_EVENT,
                                     .node_id       = NULL,
                                     .auth_key      = initParams.authKey,
                                     .alloc_ext_ram = false };
    esp_err_t ret                = esp_insights_init(&config);
    VerifyOrReturnError(ret == ESP_OK, CHIP_ERROR_INTERNAL, ESP_LOGE(TAG, "Failed to initialize ESP Insights"));

    mStorageInstance = new CircularDiagnosticBuffer(initParams.diagnosticBuffer, initParams.diagnosticBufferSize);
    VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_NO_MEMORY, ESP_LOGE(TAG, "Failed to create diagnostic buffer"));
    static ESP32Diagnostics diagnosticBackend(mStorageInstance);
    Register(diagnosticBackend);

    return CHIP_NO_ERROR;
}

CHIP_ERROR InsightsDelegate::StartPeriodicInsights(chip::System::Clock::Timeout aTimeout)
{
    VerifyOrReturnError(aTimeout != System::Clock::kZero, CHIP_ERROR_INVALID_ARGUMENT);
    mTimeout = aTimeout;
    return DeviceLayer::SystemLayer().StartTimer(mTimeout, InsightsHandler, this);
}

CHIP_ERROR InsightsDelegate::StopPeriodicInsights()
{
    DeviceLayer::SystemLayer().CancelTimer(InsightsHandler, this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR InsightsDelegate::SetSamplingInterval(chip::System::Clock::Timeout aTimeout)
{
    mTimeout = aTimeout;

    if (mTimeout == System::Clock::kZero)
    {
        StopPeriodicInsights();
        return CHIP_NO_ERROR;
    }

    // Cancel existing timer and start new one
    DeviceLayer::SystemLayer().CancelTimer(InsightsHandler, this);
    return DeviceLayer::SystemLayer().StartTimer(mTimeout, InsightsHandler, this);
}

CHIP_ERROR InsightsDelegate::SendInsightsData()
{
    uint32_t bufferSize = mStorageInstance->GetDataSize();
    uint8_t retrievalBuffer[bufferSize];
    MutableByteSpan encodedSpan(retrievalBuffer, bufferSize);
    uint32_t readEntries = 0;

    // Retrieve encoded data
    CHIP_ERROR err = mStorageInstance->Retrieve(encodedSpan, readEntries);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

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
                    ESP_LOGE(TAG, "Unsupported data type");
                }
            }
            else
            {
                ESP_LOGE(TAG, "Failed to decode diagnostic");
            }
        }
        else
        {
            ESP_LOGW(TAG, "Skipping unexpected TLV element.");
        }
    }
    // Clear buffer after successful processing
    mStorageInstance->ClearBuffer(readEntries);
    return CHIP_NO_ERROR;
}

void InsightsDelegate::LogTraceData(const DiagnosticEntry & entry)
{
    const char * tag    = "MTR_TRC";
    const char * format = "EV (%" PRIu32 ") %s: %s";
    esp_err_t err       = esp_diag_log_event(tag, format, entry.timestamps_ms_since_boot, entry.label, entry.stringValue);
    if (err == ESP_OK)
    {
        ESP_LOGD(TAG, "Event %s logged successfully", entry.label);
    }
}

void InsightsDelegate::RegisterMetric(const char * key, ValueType type)
{
    // Check for the same key will not have two different types.
    if (mRegisteredMetrics.find(key) != mRegisteredMetrics.end())
    {
        if (mRegisteredMetrics[key] != type)
        {
            ESP_LOGE(TAG, "Type mismatch for metric key %s", key);
            return;
        }
    }

    switch (type)
    {
    case ValueType::kUnsignedInteger: {
        esp_err_t err =
            esp_diag_metrics_register("SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                                      "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_UINT /* data_type */);
        if (err == ESP_OK)
        {
            ESP_LOGD(TAG, "Metric %s registered successfully", key);
        }
        break;
    }

    case ValueType::kSignedInteger: {
        esp_err_t err =
            esp_diag_metrics_register("SYS_MTR" /*Tag of metrics */, key /* Unique key 8 */, key /* label displayed on dashboard */,
                                      "insights.mtr" /* hierarchical path */, ESP_DIAG_DATA_TYPE_INT /* data_type */);
        if (err == ESP_OK)
        {
            ESP_LOGD(TAG, "Metric %s registered successfully", key);
        }
        break;
    }

    default:
        ESP_LOGE(TAG, "failed to register %s as its value is undefined", key);
        break;
    }

    mRegisteredMetrics[key] = type;
}

void InsightsDelegate::LogMetricData(const DiagnosticEntry & entry)
{
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
            ESP_LOGD(TAG, "Metric %s added successfully", entry.label);
        }
        break;
    }

    case ValueType::kUnsignedInteger: {
        snprintf(logBuffer, MAX_LOG_LENGTH, "The value of %s is %lu", entry.label, entry.uintValue);
        esp_err_t err = esp_diag_metrics_add_uint(entry.label, entry.uintValue);
        if (err == ESP_OK)
        {
            ESP_LOGD(TAG, "Metric %s added successfully", entry.label);
        }
        break;
    }

    default:
        ESP_LOGD(TAG, "The value of %s is of an UNKNOWN TYPE", entry.label);
        break;
    }
}

void InsightsDelegate::InsightsHandler(System::Layer * systemLayer, void * context)
{
    auto * instance = static_cast<InsightsDelegate *>(context);
    VerifyOrReturn(instance != nullptr);
    VerifyOrReturn(instance->mStorageInstance != nullptr);
    while (!instance->mStorageInstance->IsBufferEmpty())
    {
        instance->SendInsightsData();
    }

    // Schedule next sampling
    DeviceLayer::SystemLayer().StartTimer(instance->mTimeout, InsightsHandler, instance);
    ESP_LOGD(TAG, "Free heap Memory: %ld\n", esp_get_free_heap_size());
}
} // namespace Insights
} // namespace chip
