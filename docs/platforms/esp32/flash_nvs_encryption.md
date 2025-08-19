# Flash and NVS encryption

Below is the quick start guide for encrypting the application and factory
partition but before proceeding further please READ THE DOCS FIRST.
Documentation References:

-   [Flash Encryption](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/flash-encryption.html)
-   [NVS Encryption](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#nvs-encryption)

## Enable flash and NVS encryption some factory settings using `idf.py menuconfig`

-   Enable the Flash encryption [Security features → Enable flash encryption on
    boot]
-   The NVS Encryption is enabled by default when Flash Encryption is enabled,
    [Component config → NVS → Enable NVS encryption]
-   Use `partitions_encrypted.csv` partition table [Partition Table → Custom
    partition CSV file]

Please enable the below options if you want to use ESP32 Factory Data Provider

-   Enable ESP32 Factory Data Provider [Component config → CHIP Device Layer →
    Commissioning options → Use ESP32 Factory Data Provider]
-   Enable ESP32 Device Instance Info Provider [Component config → CHIP Device
    Layer → Commissioning options → Use ESP32 Device Instance Info Provider]

## Generate the factory partition using `generate_esp32_chip_factory_bin.py` script

-   Please check [generating factory data guide](factory_data.md) for various
    available factory data options
-   Provide `-e` option along with other options to generate the encrypted
    factory partition
-   Two partition binaries will be generated `factory_partition.bin` and
    `keys/nvs_key_partition.bin`

## Flashing the application, factory partition, and nvs keys

-   Flash the application using `idf.py flash`.

    NOTE: If not flashing for the first time you will have to use
    `idf.py encrypted-flash`

-   Flash the factory partition, this SHALL be non encrypted write as NVS
    encryption works differently

```
esptool.py -p (PORT) write_flash 0x9000 path/to/factory_partition.bin
```

-   Encrypted flash the nvs keys partition

```
esptool.py -p (PORT) write_flash --encrypt 0x317000 path/to/nvs_key_partition.bin
```

NOTE: Above command uses the default addressed printed in the boot logs
