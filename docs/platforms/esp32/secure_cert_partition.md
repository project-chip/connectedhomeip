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
