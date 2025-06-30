# Using ESP32 Diagnostic Logs Provider

This document outlines how to integrate the ESP32 Diagnostic Logs Provider into
your Matter application.

## Configuration Options

Enable the following configuration options to use the ESP32 Diagnostic Logs
Provider:

```
CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE=y
```

This option enables the diagnostic logs storage functionality.

## Implementation Reference

The ESP32 diagnostic logs provider delegate implementation is available at:

```
examples/platform/esp32/diagnostics/diagnostic-logs-provider-delegate-impl.h
examples/platform/esp32/diagnostics/diagnostic-logs-provider-delegate-impl.cpp
```

These files contain the implementation of the `LogProvider` class which
implements the `DiagnosticLogsProviderDelegate` interface.

## Integration Steps

### 1. Include the Header File

Add the diagnostic logs provider delegate header to your application:

```cpp
#include <diagnostic-logs-provider-delegate-impl.h>
```

### 2. Define Buffer Storage

Define buffers to store and retrieve diagnostic data:

```cpp
#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
static uint8_t retrievalBuffer[CONFIG_RETRIEVAL_BUFFER_SIZE]; // Buffer for retrieving diagnostics
static uint8_t endUserBuffer[CONFIG_END_USER_BUFFER_SIZE];    // Buffer for storing diagnostics

using namespace chip::app::Clusters::DiagnosticLogs;
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
```

The buffer sizes can be configured through Kconfig options:

-   `CONFIG_RETRIEVAL_BUFFER_SIZE`: Size of the buffer used for retrieving
    diagnostic data inside the diagnostic-logs-provider-delegate.
-   `CONFIG_END_USER_BUFFER_SIZE`: Size of the buffer used to store diagnostic
    data in esp32_diagnostic_trace backend.

### Kconfig.projbuild Configuration

Add the following configuration menu to your application's `main/Kconfig.projbuild` file:

```kconfig
menu "Platform Diagnostics"

    config END_USER_BUFFER_SIZE
        int "Set buffer size for end user diagnostic data"
        depends on ENABLE_ESP_DIAGNOSTICS_TRACE
        default 4096
        help
            Defines the buffer size (in bytes) for storing diagnostic data related to end user activity.
            This buffer will hold logs and traces relevant to user interactions with the Matter protocol.

    config RETRIEVAL_BUFFER_SIZE
        int "Set buffer size for retrieval of diagnostics from diagnostic storage"
        depends on ENABLE_ESP_DIAGNOSTICS_TRACE
        default 4096
        help
            Defines the buffer size (in bytes) for retrieval of diagnostic data.
endmenu
```

**Note**: The `examples/temperature-measurement-app/esp32/main/Kconfig.projbuild` demonstrates this complete configuration.

### 3. Initialize the Log Provider

Implement the diagnostic logs cluster initialization callback:

```cpp
#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint)
{
    auto & logProvider = LogProvider::GetInstance();
    logProvider.Init(endUserBuffer, CONFIG_END_USER_BUFFER_SIZE, retrievalBuffer, CONFIG_RETRIEVAL_BUFFER_SIZE);
    DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
```

This callback initializes the log provider with the configured buffers and sets
it as the delegate for the DiagnosticLogs cluster.

### 4. Include Diagnostic Logs Server

Make sure to include the diagnostic logs server in your application:

```cpp
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
```

## Additional Configuration

### Crash Logs Support

To enable crash logs retrieval, configure the ESP32 core dump functionality:

```
CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH=y
CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF=y
```

## Example Integration

The `examples/temperature-measurement-app/esp32/` demonstrates proper
integration: For more information related to the retrieval of diagnostics
through diagnosticlogs cluster refer readme.md in app folder.

# Using ESP32 Insights Integration

This section describes how to integrate ESP Insights functionality for automatic
diagnostic data collection and cloud reporting.

## Configuration Options

Enable the following configuration options to use ESP Insights:

```
CONFIG_ESP_INSIGHTS_ENABLED=y
CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE=y
```

Both options are required:
- `CONFIG_ESP_INSIGHTS_ENABLED`: Enables ESP Insights cloud integration
- `CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE`: Enables diagnostic data collection

## Implementation Reference

The ESP32 Insights delegate implementation is available at:

```
examples/platform/esp32/diagnostics/insights/insights-delegate.h
examples/platform/esp32/diagnostics/insights/insights-delegate.cpp
```

These files contain the implementation of the `InsightsDelegate` class which
handles diagnostic data collection and transmission to ESP Insights.

## Integration Steps

### 1. Include the Header File

Add the insights delegate header to your application:

```cpp
#if CONFIG_ESP_INSIGHTS_ENABLED && CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
#include <insights-delegate.h>
#endif
```

### 2. Define Buffer Storage and Auth Key

Define buffers and authentication key for insights:

```cpp
#if CONFIG_ESP_INSIGHTS_ENABLED && CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
#define START_TIMEOUT_MS 10000
static uint8_t endUserBuffer[CONFIG_END_USER_BUFFER_SIZE]; // Global static buffer used to store diagnostics
extern const char insights_auth_key_start[] asm("_binary_insights_auth_key_txt_start");
extern const char insights_auth_key_end[] asm("_binary_insights_auth_key_txt_end");
#endif
```

### 3. Create Auth Key File

Create an `insights_auth_key.txt` file in your application's main directory containing your ESP Insights authentication key.

### 4. Initialize Insights

Implement the insights initialization function:

```cpp
static void InitInsights()
{
#if CONFIG_ESP_INSIGHTS_ENABLED && CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
    chip::Insights::InsightsInitParams initParams = {
        .diagnosticBuffer     = endUserBuffer,
        .diagnosticBufferSize = CONFIG_END_USER_BUFFER_SIZE,
        .authKey              = insights_auth_key_start
    };

    chip::Insights::InsightsDelegate & insightsDelegate = chip::Insights::InsightsDelegate::GetInstance();
    CHIP_ERROR error = insightsDelegate.Init(initParams);
    VerifyOrReturn(error == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to initialize ESP Insights"));

    error = insightsDelegate.StartPeriodicInsights(chip::System::Clock::Timeout(START_TIMEOUT_MS));
    VerifyOrReturn(error == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to start periodic insights"));

    error = insightsDelegate.SetSamplingInterval(chip::System::Clock::Timeout(10000));
    VerifyOrReturn(error == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to set sampling interval"));
#endif
}
```

### 5. CMakeLists.txt Configuration

Add the following to your application's CMakeLists.txt:

```cmake
if (CONFIG_ESP_INSIGHTS_ENABLED AND CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE)
    list(APPEND PRIV_INCLUDE_DIRS_LIST "${CHIP_ROOT}/examples/platform/esp32/diagnostics/insights/")
    list(APPEND SRC_DIRS_LIST "${CHIP_ROOT}/examples/platform/esp32/diagnostics/insights/")
    target_add_binary_data(${COMPONENT_TARGET} "insights_auth_key.txt" TEXT)
endif()
```

### Kconfig.projbuild Configuration for Insights

Add the following configuration menu to your application's `main/Kconfig.projbuild` file:

```kconfig
menu "Platform Diagnostics"

    config END_USER_BUFFER_SIZE
        int "Set buffer size for end user diagnostic data"
        depends on ENABLE_ESP_DIAGNOSTICS_TRACE
        default 4096
        help
            Defines the buffer size (in bytes) for storing diagnostic data related to end user activity.
            This buffer will hold logs and traces relevant to user interactions with the Matter protocol.
endmenu
```

**Note**: For ESP Insights, only the `END_USER_BUFFER_SIZE` configuration is required. Reference implementations can be found in:
- `examples/lighting-app/esp32/main/Kconfig.projbuild`
- `examples/energy-management-app/esp32/main/Kconfig.projbuild`

## Features

The ESP Insights integration provides:

- **Automatic Data Collection**: Collects diagnostic traces and metrics
- **Cloud Integration**: Automatically sends data to ESP Insights dashboard
- **Periodic Reporting**: Configurable sampling intervals for data transmission
- **Trace Data**: Logs string-based diagnostic events
- **Metric Data**: Reports signed and unsigned integer metrics
- **Memory Monitoring**: Tracks heap usage and other system metrics

## API Reference

### InsightsDelegate Methods

- `Init(InsightsInitParams & initParams)`: Initialize insights with buffer and auth key
- `StartPeriodicInsights(timeout)`: Start periodic data transmission
- `StopPeriodicInsights()`: Stop periodic data transmission
- `SetSamplingInterval(timeout)`: Configure data sampling interval

### Configuration Parameters

- `diagnosticBuffer`: Buffer for storing diagnostic data
- `diagnosticBufferSize`: Size of the diagnostic buffer
- `authKey`: ESP Insights authentication key



## Insights Example Integration

The following examples demonstrate proper ESP Insights integration:

### Working Examples

- **`examples/lighting-app/esp32/`** - Complete implementation showing:
  - Conditional compilation guards
  - Buffer initialization and auth key setup
  - Periodic insights reporting with configurable intervals
  - Integration with diagnostic trace collection

- **`examples/energy-management-app/esp32/`** - Alternative implementation demonstrating:

For more detailed information about insights integration refer to the README.md file in each application folder.

## Important Notes

-   The diagnostic logs provider **must** be explicitly enabled through the
    `CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE` option
-   ESP Insights requires both `CONFIG_ESP_INSIGHTS_ENABLED` and `CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE`
-   Buffer sizes should be adjusted based on your application's needs
-   The provider supports end-user support logs and crash logs (when configured)
-   Authentication key must be obtained from ESP Insights dashboard and stored in `insights_auth_key.txt`
