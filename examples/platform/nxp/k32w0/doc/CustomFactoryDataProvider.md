---
orphan: true
---

## Usage of custom Factory Data Provider

`FactoryDataProviderImpl` is the default factory data provider. It is an
extension of `FactoryDataProvider` K32W common class, which implements the
following interfaces:

-   CommissionableDataProvider
-   DeviceAttestationCredentialsProvider
-   DeviceInstanceInfoProvider

A user can use `FactoryDataProvider::SearchForId` API to extract information
about custom factory data ids.

What **shall** be done to enable usage of a custom factory provider in the
reference app:

-   Set `use_custom_factory_provider=1`. This option is only available when
    factory data is used (`chip_with_factory_data=1`). An assert will be raised
    if the condition is not met.
-   Modify the `BUILD.gn` of the application to take into account the new files
    where the custom factory provider is implemented:

    ```
    if (chip_with_factory_data == 1 && use_custom_factory_provider == 1)
    {
      sources += [
        "${k32w0_platform_dir}/common/CustomFactoryDataProvider.h",
        "${k32w0_platform_dir}/common/CustomFactoryDataProvider.cpp",
      ]

      defines = [
        "CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER=1",
      ]
    }
    ```

Note that new flags were introduced:

-   `CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER`: add custom factory data related
    code in reference app.

Please refer to `CustomFactoryDataProvider` for a minimal example of leveraging
the standard factory data provider to parse custom factory data information.
Real applications could use the public API of `FactoryDataProvider` directly.
