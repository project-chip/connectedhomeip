# All-Devices Providers

This directory contains `all-devices-app` providers that implement or wrap core
Matter SDK provider interfaces.

They provide clean separation between the application data model, POSIX test
simulator overrides (such as dynamic CLI options), and platform hardware
delegates.

---

## Provider Overview

| Provider Class | Base SDK Interface | Description |
| :--- | :--- | :--- |
| [`AllDevicesExampleDACProvider`](AllDevicesExampleDACProvider.h) | [`chip::Credentials::DeviceAttestationCredentialsProvider`](../../../../src/credentials/DeviceAttestationCredsProvider.h) | Handles Device Attestation Credentials (DAC, PAI, CD, and DAC private key signing). In POSIX test/simulation environments, supports loading test credentials from a JSON vector file (`--dac_provider <path>`), falling back to default SDK example credentials. |
| [`AllDevicesExampleDeviceInfoProviderImpl`](AllDevicesExampleDeviceInfoProviderImpl.h) | [`chip::DeviceLayer::DeviceInfoProvider`](../../../../src/platform/DeviceInfoProvider.h) | Provides fixed labels, user labels, supported locales, and calendar types across dynamic endpoints in `all-devices-app`. |
| [`AllDevicesExampleDeviceInstanceInfoProviderImpl`](AllDevicesExampleDeviceInstanceInfoProviderImpl.h) | [`chip::DeviceLayer::DeviceInstanceInfoProvider`](../../../../src/platform/DeviceInstanceInfoProvider.h) | Wraps the underlying platform `DeviceInstanceInfoProvider` delegate and overrides Vendor ID and Product ID when custom `--vendor-id` and `--product-id` CLI options are passed. |

---

## DAC Test Vector JSON Schema

When supplying a custom test vector file via `--dac_provider <path>`, the JSON file is parsed by `TestHarnessDACProvider` (`src/app/tests/suites/credentials/TestHarnessDACProvider.cpp`).

The supported JSON keys are:

| JSON Key | Type | Description |
| :--- | :--- | :--- |
| `dac_cert` | Hex String | DER-encoded Device Attestation Certificate (DAC). |
| `dac_private_key` | Hex String | 32-byte SECP256r1 private key scalar used to sign the device attestation challenge. |
| `dac_public_key` | Hex String *(Optional)* | 65-byte uncompressed SECP256r1 public key corresponding to `dac_private_key`. |
| `pai_cert` | Hex String | DER-encoded Product Attestation Intermediate (PAI) certificate. |
| `certification_declaration` | Hex String | CMS-signed DER-encoded Certification Declaration (CD) payload. |
| `firmware_information` | Hex String *(Optional)* | Optional firmware information payload. |
| `basic_info_pid` | Integer *(Optional)* | Expected Product ID matching the test vector. |
| `description` | String *(Optional)* | Human-readable description of the test scenario. |
| `is_success_case` | Boolean *(Optional)* | Indicates whether the test vector is expected to pass or fail attestation. |

### Pre-Existing Test Vectors

Pre-existing test vector JSON files are available in the Matter SDK repository under:
[`credentials/development/commissioner_dut/`](../../../../credentials/development/commissioner_dut/)

Examples:
- **Valid CMS v3 CD Vector**: `credentials/development/commissioner_dut/struct_cd_cms_v3/test_case_vector.json`
- **Official CD Vector**: `credentials/development/commissioner_dut/struct_cd_official_cd/test_case_vector.json`
- **Attestation Fallback / PID Mismatch Vector**: `credentials/development/commissioner_dut/struct_dac_subject_pid_mismatch/test_case_vector.json`

Example invocation:
```bash
./out/linux-x64-all-devices-clang/all-devices-app \
  --dac_provider credentials/development/commissioner_dut/struct_cd_cms_v3/test_case_vector.json \
  --product-id 0x8000
```

---

## Production vs. Simulation Usage

### 1. Device Attestation Credentials (DAC)

In real commercial products, DAC private keys must never be stored in plaintext
JSON files or memory accessible to user space.

-   **Simulation / Test Harnesses (POSIX)**: The POSIX entrypoint
    ([`posix/main.cpp`](../../posix/main.cpp))
    initializes `AllDevicesExampleDACProvider` with
    `AppOptions::GetConfig().dacProvider` to allow test runners (such as WOCA
    and Python certification scripts) to supply test vectors dynamically.
-   **Embedded MCUs (ESP32, Silicon Labs, Nordic)**: Hardware platforms
    initialize a `FactoryDataProvider` (which reads from secure flash or NVM)
    and register it at boot.
-   **Commercial Linux Gateways**: Real products replace
    `AllDevicesExampleDACProvider` with a provider that delegates signing
    directly to a Hardware Secure Element (e.g., `ATECC608`, NXP `SE050`), TPM,
    or platform Secure Enclave / TrustZone
    ([`TrustyDACProvider`](../../../../src/platform/Linux/DeviceAttestationCredsTrusty.h)).

### 2. Device Instance Info

`AllDevicesExampleDeviceInstanceInfoProviderImpl` demonstrates the decorator
pattern: it intercepts basic info queries to allow CLI customization during
development while forwarding hardware-level queries (such as rotating device ID,
serial numbers, and manufacturing dates) to the underlying platform delegate.
