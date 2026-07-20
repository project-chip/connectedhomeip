# All-Devices Providers

This directory contains `all-devices-app` providers that implement or wrap core
Matter SDK provider interfaces.

They provide clean separation between the application data model, POSIX test
simulator overrides (such as dynamic CLI options), and platform hardware
delegates.

---

## Provider Overview

| Provider Class                                                                                                                                                                                                           | Base SDK Interface                                                                                                                                                                | Description                                                                                                                                                                                                                                                      |
| :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [`AllDevicesExampleDACProvider`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/examples/all-devices-app/all-devices-common/providers/AllDevicesExampleDACProvider.h)                                       | [`chip::Credentials::DeviceAttestationCredentialsProvider`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/src/credentials/DeviceAttestationCredsProvider.h#L34-L70) | Handles Device Attestation Credentials (DAC, PAI, CD, and DAC private key signing). In POSIX test/simulation environments, supports loading test credentials from a JSON vector file (`--dac_provider <path>`), falling back to default SDK example credentials. |
| [`AllDevicesExampleDeviceInfoProviderImpl`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/examples/all-devices-app/all-devices-common/providers/AllDevicesExampleDeviceInfoProviderImpl.h)                 | [`chip::DeviceLayer::DeviceInfoProvider`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/src/platform/DeviceInfoProvider.h)                                          | Provides fixed labels, user labels, supported locales, and calendar types across dynamic endpoints in `all-devices-app`.                                                                                                                                         |
| [`AllDevicesExampleDeviceInstanceInfoProviderImpl`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/examples/all-devices-app/all-devices-common/providers/AllDevicesExampleDeviceInstanceInfoProviderImpl.h) | [`chip::DeviceLayer::DeviceInstanceInfoProvider`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/src/platform/DeviceInstanceInfoProvider.h)                          | Wraps the underlying platform `DeviceInstanceInfoProvider` delegate and overrides Vendor ID and Product ID when custom `--vendor-id` and `--product-id` CLI options are passed.                                                                                  |

---

## Production vs. Simulation Usage

### 1. Device Attestation Credentials (DAC)

In real commercial products, DAC private keys must never be stored in plaintext
JSON files or memory accessible to user space.

-   **Simulation / Test Harnesses (POSIX)**: The POSIX entrypoint
    ([`posix/main.cpp`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/examples/all-devices-app/posix/main.cpp#L347-L352))
    initializes `AllDevicesExampleDACProvider` with
    `AppOptions::GetConfig().dacProvider` to allow test runners (such as WOCA
    and Python certification scripts) to supply test vectors dynamically.
-   **Embedded MCUs (ESP32, Silicon Labs, Nordic)**: Hardware platforms
    initialize a `FactoryDataProvider` (which reads from secure flash or NVM)
    and register it at boot.
-   **Commercial Linux Gateways**: Real products replace
    `AllDevicesExampleDACProvider` with a provider that delegates signing
    directly to a Hardware Secure Element (e.g., `ATECC608`, NXP `SE050`), TPM, or
    platform Secure Enclave / TrustZone
    ([`TrustyDACProvider`](file:///usr/local/google/home/sergiosoares/connectedhomeip2/src/platform/Linux/DeviceAttestationCredsTrusty.h#L26-L45)).

### 2. Device Instance Info

`AllDevicesExampleDeviceInstanceInfoProviderImpl` demonstrates the decorator
pattern: it intercepts basic info queries to allow CLI customization during
development while forwarding hardware-level queries (such as rotating device ID,
serial numbers, and manufacturing dates) to the underlying platform delegate.
