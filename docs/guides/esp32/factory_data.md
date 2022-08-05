## Using ESP32 Factory Data Provider

By default applications uses test-mode or default commissionable data provider,
device attestation credentials provider, and device instance info provider.

To use different values for them you can use the ESP32 Factory Data Provider.

### Configuration Options

Enable config option `CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER` to use ESP32
specific implementation of CommissionableDataProvider and
DeviceAttestationCredentialsProvider.

[Component config → CHIP Device Layer → Commissioning options → Use ESP32
Factory Data Provider]

Enable config option `CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER` to use ESP32
specific implementation of DeviceInstanceInfoProvider.

[Component config → CHIP Device Layer → Commissioning options → Use ESP32 Device
Instance Info Provider]

ESP32 implementation reads factory data from nvs partition, chip-factory data
must be flashed into the configured nvs partition. Factory partition can be
configured using `CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL` option, default
is "nvs".

### Generate NVS Binary Image

`scripts/tools/generate_esp32_chip_factory_bin.py` script generates the
chip-factory NVS binary image `partition.bin`.

#### Supported data

Script supports adding following data to the NVS binary image:

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

Please check help for available options,
`scripts/tools/generate_esp32_chip_factory_bin.py`

#### Dependency

Script has dependency on
[spake2p](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/spake2p)
for generating spake2p parameters.

Build spake2p:

```
cd path/to/connectedhomeip
gn gen out/host
ninja -C out/host
cd -
```

Add spake2p to \$PATH environment variable

```
export PATH=$PATH:path/to/connectedhomeip/out/host
```

#### Usage

Below mentioned command generates the nvs image with test DAC with VID:0xFFF2
and PID:8000 and some examples values for other fields.

```
./generate_esp32_chip_factory_bin.py -d 3434 -p 99663300 \
                                    --product-name ESP-lighting-app --product-id 0x8000 \
                                    --vendor-name Test-vendor --vendor-id 0xFFF2 \
                                    --hw-ver 1 --hw-ver-str DevKit \
                                    --dac-cert path/to/connectedhomeip/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Cert.der \
                                    --dac-key path/to/connectedhomeip/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Key.der \
                                    --pai-cert path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.der \
                                    --cd path/to/connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der \
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

Please check [Flash and NVS encryption guide](flash_nvs_encryption.md)
