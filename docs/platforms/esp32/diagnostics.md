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

## Important Notes

-   The diagnostic logs provider **must** be explicitly enabled through the
    `CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE` option
-   Buffer sizes should be adjusted based on your application's needs
-   The provider supports end-user support logs and crash logs (when configured)
