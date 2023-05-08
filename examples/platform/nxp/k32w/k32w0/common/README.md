---
orphan: true
---

## Usage of custom Factory Data Provider

`K32W0FactoryDataProvider` is the default factory data provider. It implements
three interfaces from common Matter stack:

-   CommissionableDataProvider
-   DeviceAttestationCredentialsProvider
-   DeviceInstanceInfoProvider

A user can inherit `K32W0FactoryDataProvider` to provide additional
functionality (such as parsing custom factory data fields) that is customer
specific. Additionally, each method from the above three interfaces could be
overwritten in the custom provider, but the current platform provider
implementation should be enough for most use cases.

What **MUST** be done to enable usage of a custom factory provider:

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
        "CHIP_DEVICE_CONFIG_CUSTOM_PROVIDER_NUMBER_IDS=3"
      ]
    }
    ```

    Note that new flags were introduced:

    -   `CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER`: select between using
        `K32W0FactoryDataProvider` or `CustomFactoryDataProvider` when
        registering the provider.
    -   `CHIP_DEVICE_CONFIG_CUSTOM_PROVIDER_NUMBER_IDS`: set the number of
        custom IDs (which are different than the default IDs). If this macro is
        not correctly set, compilation should fail.

-   The default IDs are validated directly in `SearchForId`, but custom IDs
    should set their maximum lengths through `SetCustomIds` method.
-   `K32W0FactoryDataProvider::kNumberOfIds` is computed at compile time and
    should represent the number of IDs (custom + default). Custom IDs should
    start from `FactoryDataId::kMaxId`, which is the next valid ID. IDs
    `[1, FactoryDataId::kMaxId - 1]` are reserved for default factory data IDs.

Please refer to `CustomFactoryDataProvider` for a minimal example of a custom
factory data provider and note that its implementation is just an example of how
to integrate a custom provider into the application, while still using the
default implementation provided by `K32W0FactoryDataProvider`.
