## Providers Implemented for ESP32 Platform

The ESP32 platform has implemented several providers that can be used with data
stored in the factory or by setting fixed data.

Below are the providers that have been implemented:

-   [Commissionable Data Provider](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/ESP32FactoryDataProvider.h#L47)
    This provider reads the discriminator and setup pincode related parameters
    from the factory partition.
-   [Device Attestation Credentials Provider](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/ESP32FactoryDataProvider.h#L56)
    This provider manages the attestation data.
-   [Device Instance Info Provider](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/ESP32FactoryDataProvider.h#L86)
    This provider reads basic device information from the factory partition.
-   [Device Info Provider](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/ESP32DeviceInfoProvider.h#L31)
    This provider provides fixed labels, supported calendar types, and supported
    locales from the factory partition.
-   [Supported Modes](https://github.com/project-chip/connectedhomeip/blob/master/examples/platform/esp32/mode-support/static-supported-modes-manager.h#L28)
    This provider offers the supported modes for the mode-select cluster.

More information can be found in the [factory data guide](factory_data.md).

### Device Info Provider

Currently, there are two implementations for this provider:

1. [Reads data stored in the factory partition](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/ESP32FactoryDataProvider.h#L56)
   _(This will be deprecated in the future)_
2. [Provides APIs to set fixed data that gets read later](https://github.com/project-chip/connectedhomeip/blob/master/src/platform/ESP32/StaticESP32DeviceInfoProvider.h)

-   New products should use the `StaticESP32DeviceInfoProvider`. Utilize the
    `Set...()` APIs to set the fixed data.
-   Existing products using the first implementation can continue to use it if
    they do not wish to change the data.
-   For products using the first implementation and wanting to change the fixed
    data via OTA, they should switch to the second implementation in the OTA
    image and use the `Set...()` APIs to set the fixed data.

#### Example:

```cpp
#include <platform/ESP32/StaticESP32FactoryDataProvider.h>

DeviceLayer::StaticESP32DeviceInfoProvider deviceInfoProvider;

// Define array for Supported Calendar Types
using namespace chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum;
CalendarTypeEnum supportedCalendarTypes[] = {
    CalendarTypeEnum::kGregorian, CalendarTypeEnum::kCoptic,
    CalendarTypeEnum::kEthiopian, CalendarTypeEnum::kChinese,
};

// Define array for Supported Locales
const char* supportedLocales[] = {
    "en-US",
    "en-EU",
};

// Define array for Fixed labels { EndpointId, Label, Value }
struct StaticESP32DeviceInfoProvider::FixedLabelEntry fixedLabels[] = {
    { 0, "Room", "Bedroom 2" },
    { 0, "Orientation", "North" },
    { 0, "Direction", "Up" },
};

Span<CalendarTypeEnum> sSupportedCalendarTypes(supportedCalendarTypes);
Span<const char*> sSupportedLocales(supportedLocales);
Span<StaticESP32DeviceInfoProvider::FixedLabelEntry> sFixedLabels(fixedLabels);

{
    deviceInfoProvider.SetSupportedLocales(sSupportedLocales);
    deviceInfoProvider.SetSupportedCalendarTypes(sSupportedCalendarTypes);
    deviceInfoProvider.SetFixedLabels(sFixedLabels);
    DeviceLayer::SetDeviceInfoProvider(&deviceInfoProvider);
}
```
