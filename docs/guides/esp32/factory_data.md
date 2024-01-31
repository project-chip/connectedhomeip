## Using ESP32 Factory Data Provider

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

By default applications uses test-mode or default commissionable data provider,
device attestation credentials provider, device instance info provider, and
device info provider.

To use different values for them you can use the ESP32 Factory Data Provider.

### Supported data

Following data can be added to the manufacturing partition using
`scripts/tools/generate_esp32_chip_factory_bin.py` script.

-   Commissionable data

    -   Discriminator
    -   Spake2p verifier, salt, and iteration count

-   Device attestation data

    -   Device attestation key and certificate
    -   Product attestation intermediate certificate
    -   Certificate declaration

-   Device instance information

    -   Vendor id and name
    -   Product id and name
    -   Hardware version and version string
    -   Serial Number
    -   Unique identifier

-   Device information
    -   Fixed Labels
    -   Supported locales
    -   Supported calendar types
    -   Supported modes
        -   Note: As per spec at max size of label should be 64 and `\0` will be
            added at the end.

### Configuration Options

Enable config option `CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER` to use ESP32
specific implementation of `CommissionableDataProvider` and
`DeviceAttestationCredentialsProvider`.

[Component config → CHIP Device Layer → Commissioning options → Use ESP32
Factory Data Provider]

By default, the factory data provider implementation reads the Certification
Declaration (CD) from the 'chip-factory' NVS namespace. Enable
`CONFIG_ENABLE_SET_CERT_DECLARATION_API` option to enable an API which lets you
set the CD from the application and the configured CD will be used for
subsequent CD reads.

[Component config -> CHIP Device Layer -> Commissioning options -> Enable Set CD
API]

Enable config option `CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER` to use
ESP32 specific implementation of `DeviceInstanceInfoProvider`.

[Component config → CHIP Device Layer → Commissioning options → Use ESP32 Device
Instance Info Provider]

Enable config option `CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER` to use ESP32
specific implementation of `DeviceInfoProvider`.

[Component config → CHIP Device Layer → Commissioning options → Use ESP32 Device
Info Provider]

ESP32 implementation reads factory data from nvs partition, chip-factory data
must be flashed into the configured nvs partition. Factory partition can be
configured using `CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL` option, default
is "nvs".

[Component config -> CHIP Device Layer -> Matter Manufacturing Options ->
chip-factory namespace partition label]

### Generate NVS Binary Image

`scripts/tools/generate_esp32_chip_factory_bin.py` script generates the
chip-factory NVS binary image `partition.bin`.

Please check help for available options,
`scripts/tools/generate_esp32_chip_factory_bin.py`

#### Usage

Below mentioned command generates the nvs image with test DAC with VID:0xFFF2
and PID:8000 and some examples values for other fields.

For more help please check `./generate_esp32_chip_factory_bin.py -h`

```
./generate_esp32_chip_factory_bin.py -d 3434 -p 99663300 \
                                    --product-name ESP-lighting-app --product-id 0x8000 \
                                    --vendor-name Test-vendor --vendor-id 0xFFF2 \
                                    --hw-ver 1 --hw-ver-str DevKit \
                                    --dac-cert path/to/connectedhomeip/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Cert.der \
                                    --dac-key path/to/connectedhomeip/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Key.der \
                                    --pai-cert path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.der \
                                    --cd path/to/connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der
```

#### Flashing the generated NVS image

Use the following command to flash the NVS image. `0x9000` is default address
for `nvs` partition.

```
esptool.py -p <port> write_flash 0x9000 factory_partition.bin
```

NOTE: Above command uses the default NVS address, please flash the NVS image at
appropriate address.

### Securing NVS binary image with NVS Encryption

WARNING: NVS binary image may contain the sensitive information and it must be
secured using NVS encryption. For more details please check
[Flash and NVS encryption guide](flash_nvs_encryption.md)
