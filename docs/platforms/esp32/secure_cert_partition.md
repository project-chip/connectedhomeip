# Using esp_secure_cert partition

**WARNING:** The following steps outline the development workflow for building a
matter device.

Please take a look at
[security considerations](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/security.html)
and review the security guidelines outlined in
[security workflow](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/host-based-security-workflows.html)
for production workflows.

Ensure to select the appropriate SoC from the menu on the left-hand side, as the
provided references are specific to ESP32.

---

## 1.1 ESP Secure Cert Partition

-   When a device is pre-provisioned, PKI credentials are generated for the
    device and stored in esp_secure_cert partition.
-   In the Matter Pre-Provisioning service, the Matter attestation information
    is pre-flashed into the esp_secure_cert partition.
-   The ESP32SecureCertDACProvider reads the attestation information from the
    esp_secure_cert partition.
-   The DAC and PAI are read from the esp_secure_cert partition, while the
    certification declaration is read from the factory partition.
-   The usage of the esp_secure_cert partition is demonstrated in the
    lighting-app.

-   During the development phase, the esp_secure_cert partition can be generated
    on the host with the help of the configure_esp_secure_cert.py utility.
-   The steps below demonstrate how to generate certificates and the respective
    partitions to be used during the development phase.

## 1.2 Prerequisites:

To generate the esp_secure_cert_partition and the factory_data_partition, we
need the DER encoded DAC, PAI certificate, DAC private key, and certification
declaration.

### 1.2.1 Build chip-cert:

Run the commands below:

```
cd path/to/connectedhomeip
source scripts/activate.sh
gn gen out/host
ninja -C out/host chip-cert
cd out/host
```

At /path/to/connectedhomeip/out/host run the below commands.

### 1.2.2 Generating Certification Declaration

```
./chip-cert gen-cd -K ../../credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem \
    -C ../../credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem \
    -O esp_dac_fff1_8000.der -f 1 \
    -V 0xfff1 -p 0x8000 -d 0x0016 -c "CSA00000SWC00000-01" -l 0 -i 0 -n 1 -t 0
```

### 1.2.3 Generating PAI

```
./chip-cert gen-att-cert -t i -c "ESP TEST PAI" -V 0xfff1 -P 0x8000 \
    -C ../../credentials/development/attestation/Chip-Development-PAA-Cert.pem \
    -K ../../credentials/development/attestation/Chip-Development-PAA-Key.pem \
    -o Esp-Development-PAI-Cert.pem -O Esp-Development-PAI-Key.pem -l 4294967295
```

### 1.2.4 Generating DAC

```
./chip-cert gen-att-cert -t d -c "ESP TEST DAC 01" -V 0xfff1 -P 0x8000 \
    -C Esp-Development-PAI-Cert.pem -K Esp-Development-PAI-Key.pem \
    -o Esp-Development-DAC-01.pem -O Esp-Development-DAC-Key-01.pem -l 4294967295
```

### 1.2.5 Change format for the certificates and key (.pem to .der format)

-   Convert DAC key from .pem to .der format.

```
openssl ec -in Esp-Development-DAC-Key-01.pem -out Esp-Development-DAC-Key-01.der -inform pem -outform der
```

-   Convert DAC and PAI cert from .pem to .der format

```
openssl x509 -in Esp-Development-DAC-01.pem -out Esp-Development-DAC-01.der -inform pem -outform der
openssl x509 -in Esp-Development-PAI-Cert.pem -out Esp-Development-PAI-Cert.der -inform pem -outform der
```

The certificates in the steps 1.2 will be generated at
/path/to/connectedhomeip/out/host.For steps 1.3 and 1.4 go to
connectedhomeip/scripts/tools, and set IDF_PATH.

## 1.3 Generating esp_secure_cert_partition

To generate the esp_secure_cert_partition install esp-secure-cert-tool using
below command. Please use the tool with version >= 1.0.1

```
pip install esp-secure-cert-tool
```

Please use esp-secure-cert-tool with version >= esp-secure-cert-too

Espressif have SoCs with and without ECDSA peripheral, so there is a bit
different flow for both. Currently only ESP32H2 has the ECDSA Peripheral.

### 1.3.2 For SoCs without ECDSA Peripheral (Except ESP32H2)

The following command generates the secure cert partition and flashes it to the
connected device. Additionally, it preserves the generated partition on the
host, allowing it to be flashed later if the entire flash is erased.

```
configure_esp_secure_cert.py --private-key Esp-Development-DAC-Key-01.der \
    --device-cert Esp-Development-DAC-01.der \
    --ca-cert Esp-Development-PAI-Cert.der \
    --target_chip esp32c3 \
    --keep_ds_data_on_host \
    --port /dev/ttyUSB0
```

### 1.3.1 For SoCs with ECDSA Peripheral (ESP32H2)

The following command generates the secure cert partition, flashes it onto the
connected device, burns the efuse block with the private key, and preserves the
generated partition on the host for future use in case of a complete flash
erase.

```
configure_esp_secure_cert.py --private-key Esp-Development-DAC-Key-01.der \
    --priv_key_algo ECDSA 256 --efuse_key_id 2 --configure_ds \
    --device-cert Esp-Development-DAC-01.der \
    --ca-cert Esp-Development-PAI-Cert.der \
    --target_chip esp32h2 \
    --keep_ds_data_on_host \
    --port /dev/ttyUSB0
```

Refer
https://github.com/espressif/esp_secure_cert_mgr/tree/main/tools#generate-esp_secure_cert-partition
for more help.

## 1.4 Generating the factory_data_partition

Example command to generate a factory_data_partition

```
./generate_esp32_chip_factory_bin.py -d 3434 -p 99663300 \
    --product-name ESP-lighting-app --product-id 0x8000 \
    --vendor-name Test-vendor --vendor-id 0xFFF1 \
    --hw-ver 1 --hw-ver-str DevKit \
    --cd esp_dac_fff1_8000.der
```

Refer
https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/esp32/factory_data.md#generate-nvs-binary-image
to generate a factory_data_partition.

## 1.5 Build the firmware with below configuration options

-   For SoCs without ECDSA Peripheral (Except ESP32H2)

```
# Disable the DS Peripheral support
CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL=n
# Use DAC Provider implementation which reads attestation data from secure cert partition
CONFIG_SEC_CERT_DAC_PROVIDER=y
# Enable some options which reads CD and other basic info from the factory partition
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y
CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER=y
CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"
```

-   For SoCs with ECDSA Peripheral (ESP32H2)

```
# Enable the DS Peripheral support
CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL=y
# Use DAC Provider implementation which reads attestation data from secure cert partition
CONFIG_SEC_CERT_DAC_PROVIDER=y
# Enable some options which reads CD and other basic info from the factory partition
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y
CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER=y
CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"
```

By default, the secure cert DAC provider implementation reads the Certification
Declaration (CD) from the 'chip-factory' NVS namespace. Enable
`CONFIG_ENABLE_SET_CERT_DECLARATION_API` option to enable an API which lets you
set the CD from the application and the configured CD will be used for
subsequent CD reads.

[Component config -> CHIP Device Layer -> Commissioning options -> Enable Set CD
API]

In order to use the esp_secure_cert_partition, in addition to enabling the above
config options, you should also have the esp_secure_cert_partition and factory
partition in your app. For reference, refer to partitions.csv file of
lighting-app.

## 1.6 Flashing firmware, secure cert and factory partition

### Build and flash

```
idf.py -p (PORT) build flash
```

### Flash `esp_secure_cert` and factory partition binaries

The `esp_secure_cert` partition binary contains device attestation information
and the factory partition binary contains Matter manufacturing specific data.

-   Flash `esp_secure_cert` partition binary

```
esptool.py -p (PORT) write_flash 0xd000 path/to/secure_cert_partition.bin
```

-   Flash factory partition binary

```
esptool.py -p (PORT) write_flash 0x3E0000 path/to/factory_partition.bin
```

## 1.7 Protecting the DAC private key with ESP-TEE

On SoCs that support ESP-TEE (Trusted Execution Environment), the Device
Attestation Certificate (DAC) signing can be confined to the secure world so
that the DAC private key is never present in the application (REE) and never
stored in plaintext in flash. The application calls into the TEE only to obtain
a signature; the key material stays inside the secure world.

Two mechanisms are used depending on the SoC's peripherals:

| SoC                                  | Mechanism                                                                           | DAC private key                                                             |
| ------------------------------------ | ----------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| Has HMAC peripheral (ESP32-C6/H2/C5) | Derived at runtime inside the TEE using PBKDF2-HMAC-SHA256 and signed in the TEE    | Never stored — derived on demand from the eFuse HMAC key                    |
| No HMAC peripheral (e.g. ESP32-C61)  | Provisioned into the ECDSA peripheral, with peripheral access restricted to the TEE | Held in the ECDSA peripheral; signing happens in hardware, gated by the TEE |

In both cases the DAC and PAI **certificates** are public and continue to be
read from the `esp_secure_cert` partition; only the private-key handling
differs.

### 1.7.1 HMAC + PBKDF2 derived DAC key (HMAC-capable SoCs)

The DAC private key is derived inside the TEE with PBKDF2-HMAC-SHA256, using the
HMAC peripheral together with a per-device HMAC key burned into an eFuse block
(purpose `HMAC_UP`). The PBKDF2 salt is the device's Spake2p salt, read from the
factory partition. The `ESP32SecureCertDACProvider` performs attestation signing
by calling `esp_tee_sec_storage_ecdsa_sign_pbkdf2()`, which derives the key and
signs entirely inside the TEE and returns only the signature.

Enable it in the lighting-app with the bundled TEE configuration:

```
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32c6_tee" set-target esp32c6 build
```

The relevant options (see
[`sdkconfig.defaults.esp32c6_tee`](../../../examples/lighting-app/esp32/sdkconfig.defaults.esp32c6_tee)):

```
CONFIG_SECURE_ENABLE_TEE=y
# eFuse block holding the HMAC key the TEE uses for PBKDF2 derivation.
# Must differ from the secure-storage XTS key block (which is 0 in dev mode),
# so use a non-zero block such as 1 (BLOCK_KEY1).
CONFIG_SECURE_TEE_PBKDF2_EFUSE_HMAC_KEY_ID=1
CONFIG_SEC_CERT_DAC_PROVIDER=y
CONFIG_USE_ESP32_TEE_DAC_KEY_PBKDF2=y
CONFIG_USE_ESP32_ECDSA_PERIPHERAL=n
```

Burn the same HMAC key used during provisioning into the configured eFuse block
with purpose `HMAC_UP`:

```
idf.py -p (PORT) efuse-burn-key BLOCK_KEY1 hmac_key.bin HMAC_UP
```

> **NOTE:** The manufacturing tool must derive the DAC public key with the same
> HMAC key and the same Spake2p salt so that the issued DAC certificate matches
> the key the device derives at runtime.

> **WARNING:** eFuse burning is irreversible. Keep the HMAC key secret — anyone
> with it can re-derive the DAC private key.

### 1.7.2 ECDSA peripheral key restricted to the TEE (non-HMAC SoCs)

On SoCs without an HMAC peripheral, the DAC key is provisioned into the ECDSA
peripheral and signing is done in hardware. ESP-TEE restricts access to the
ECDSA peripheral to the secure world (via the Access Permission Management / APM
configuration of the TEE), so the application cannot drive the peripheral
directly. From the Matter side this uses the existing
`CONFIG_USE_ESP32_ECDSA_PERIPHERAL` signing path; the TEE-only restriction is a
property of the TEE build, not of the Matter code.

### 1.7.3 Trade-offs

| Approach                   | Key in app (REE) RAM | Signing location               | Requires                                     | Notes                                                                                                        |
| -------------------------- | -------------------- | ------------------------------ | -------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| Default format key         | Yes                  | Software (mbedTLS)             | Flash encryption to protect the key in flash | Simplest; key is materialized in RAM.                                                                        |
| ECDSA peripheral (no TEE)  | Never                | On-chip ECDSA peripheral       | SoC with ECDSA peripheral                    | Key never enters software, but the app can drive the peripheral.                                             |
| **TEE + HMAC/PBKDF2**      | Never                | Inside the TEE                 | HMAC peripheral + ESP-TEE                    | No key in flash; derived on demand; signing isolated in the secure world. Recommended for HMAC-capable SoCs. |
| **TEE + ECDSA peripheral** | Never                | ECDSA peripheral, gated by TEE | ECDSA peripheral + ESP-TEE                   | Hardware signing with peripheral access locked to the secure world. For non-HMAC SoCs.                       |

Notes on the design decision:

-   An earlier iteration **stored** the DAC private key in TEE secure storage
    and signed it through a TEE-bound mbedTLS context. That has been dropped in
    favor of deriving the key (HMAC/PBKDF2) so that no key — not even an
    encrypted blob — needs to be stored on the device. The `secure_storage`
    partition is therefore not used to hold the DAC key.
-   The PBKDF2 path keeps the DAC key out of REE RAM entirely (signing happens
    in the TEE), which is stronger than the plain `esp_secure_cert` software
    paths where the key is loaded into application memory at sign time.
-   For SoCs with neither an HMAC nor an ECDSA peripheral, fall back to the
    default-format key protected by flash encryption.

## 1.7 Protecting the DAC private key

The DAC private key is the most sensitive secret on the device: anything that
can read it can impersonate the device during attestation. ESP32 SoCs offer a
few ways to protect it, and which one to use depends on the peripherals the SoC
provides. The `esp_secure_cert` partition records the key type, and the
`ESP32SecureCertDACProvider` selects the matching signing path at runtime based
on `esp_secure_cert_get_priv_key_type()`.

The supported key types (see `esp_secure_cert_key_type_t` in
`esp_secure_cert_read.h`) are:

-   `ESP_SECURE_CERT_DEFAULT_FORMAT_KEY` — the DAC private key is stored in the
    `esp_secure_cert` partition in plaintext DER. It is loaded into software and
    used with mbedTLS. Protect the partition with flash encryption.
-   `ESP_SECURE_CERT_ECDSA_PERIPHERAL_KEY` — the private key lives in an eFuse
    block and signing is done entirely inside the on-chip ECDSA peripheral; the
    key is never materialized in software. Available on SoCs with an ECDSA
    peripheral (e.g. ESP32-H2). Enabled with
    `CONFIG_USE_ESP32_ECDSA_PERIPHERAL`.
-   `ESP_SECURE_CERT_HMAC_DERIVED_ECDSA_KEY` — the DAC private key is
    **derived** at runtime from a per-device HMAC key (burned into an eFuse
    block with purpose `HMAC_UP`) using the HMAC peripheral together with
    PBKDF2. The derived key is reconstructed by `esp_secure_cert_mgr` on demand
    and never stored in plaintext in flash. This is the recommended option for
    SoCs that have an HMAC peripheral but no ECDSA peripheral (e.g.
    ESP32-C6/H2/C5).

### 1.7.1 HMAC-derived ECDSA DAC key (HMAC-capable SoCs)

For SoCs with an HMAC peripheral, prefer the HMAC-derived ECDSA key. The root
secret is the eFuse HMAC key; the DAC private key is derived from it at runtime,
so no usable private key sits in flash. No ESP-TEE app or secure-storage
partition is required — the standard `esp_secure_cert` + factory partition
layout is used.

Provision it with the manufacturing tool by selecting the HMAC-derived ECDSA key
type and supplying the HMAC key (the exact flags are documented with
[esp_secure_cert_mgr](https://github.com/espressif/esp_secure_cert_mgr) and the
[esp-matter-mfg-tool](https://github.com/espressif/esp-matter-tools/tree/main/mfg_tool)).
The same HMAC key must be burned into the device eFuse with purpose `HMAC_UP`:

```
idf.py -p (PORT) efuse-burn-key BLOCK_KEY1 hmac_key.bin HMAC_UP
```

Build configuration — the DAC provider's software signing path handles the
derived key; no peripheral-specific option is needed:

```
# Read attestation data from the esp_secure_cert partition
CONFIG_SEC_CERT_DAC_PROVIDER=y
# Not an ECDSA-peripheral key
CONFIG_USE_ESP32_ECDSA_PERIPHERAL=n
# DS peripheral is not used for this key type
CONFIG_ESP_SECURE_CERT_DS_PERIPHERAL=n
# Read CD and basic info from the factory partition
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y
CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER=y
CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"
```

> **WARNING:** eFuse burning is irreversible. Keep `hmac_key.bin` secret —
> anyone with it can re-derive the DAC private key.

### 1.7.2 Trade-offs

| Approach               | Key at rest                                                   | Signing happens in                   | Requires                            | Notes                                                                                                |
| ---------------------- | ------------------------------------------------------------- | ------------------------------------ | ----------------------------------- | ---------------------------------------------------------------------------------------------------- |
| Default format key     | Plaintext DER in `esp_secure_cert` (rely on flash encryption) | Software (mbedTLS)                   | Flash encryption                    | Simplest; key is materialized in RAM.                                                                |
| ECDSA peripheral key   | eFuse block, never readable                                   | On-chip ECDSA peripheral             | SoC with ECDSA peripheral (e.g. H2) | Strongest isolation; key never enters software.                                                      |
| HMAC-derived ECDSA key | Not stored — derived from eFuse HMAC key at runtime           | Software (mbedTLS), after derivation | SoC with HMAC peripheral (C6/H2/C5) | No plaintext key in flash; no ECDSA peripheral or TEE needed. **Recommended for HMAC-capable SoCs.** |

Notes on the design decision:

-   **Why not store the DAC directly in TEE secure storage?** An earlier
    iteration signed inside ESP-TEE with the DAC key held in TEE secure storage.
    That pulls in the full TEE app, a `secure_storage` partition and TEE OTA
    data, and is only available on a subset of SoCs/IDF versions. For
    HMAC-capable SoCs the HMAC-derived key gives equivalent "no usable key in
    flash" protection without the extra partitions and TEE runtime, so the
    direct TEE secure-storage path is not used here.
-   For SoCs that lack both an HMAC and an ECDSA peripheral, fall back to the
    default-format key protected by flash encryption.
-   In all cases the DAC and PAI **certificates** are public and continue to be
    read from the `esp_secure_cert` partition; only the private-key handling
    differs.
