# Using esp_secure_cert partition

## 1.1 ESP Secure Cert Partition

-   When a device is pre-provisioned, the PKI credentials are generated for the
    device and stored in a partition named esp_secure_cert.
-   In the Matter Pre-Provisioning service, the Matter DAC certificate is
    pre-flashed in esp_secure_cert partition.
-   The ESP32SecureCertDACProvider reads the PKI credentials from
    esp_secure_cert_partition.
-   The DAC,PAI and private key are read from the esp_secure_cert_partition, but
    the certificate declaration is read from the factory data partition.
    Therefore, we need to also generate a factory partition besides
    esp_secure_cert_partition.
-   The esp_secure_cert partition can be generated on host with help of
    configure_esp_secure_cert.py utility.
-   The use of esp_secure_cert_partition is demonstrated in lighting-app.

## 1.2 Prerequisites:

To generate the esp_secure_cert_partition and the factory_data_partition, we
need the DAC and PAI certificate as well as the private key(DAC key) in .der
format. The factory_data_provider in addition requires the certificate
declaration in .der format. The generation of the required certificates and keys
is mentioned in the steps given below.

### 1.2.1 Build certification generation tool:

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
./chip-cert gen-cd -K ../../credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem -C ../../credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem -O esp_dac_fff1_8000.der -f 1 -V 0xfff1 -p 0x8000 -d 0x0016 -c "CSA00000SWC00000-01" -l 0 -i 0 -n 1 -t 0
```

### 1.2.3 Generating PAI

```
./chip-cert gen-att-cert -t i -c "ESP TEST PAI" -V 0xfff1 -P 0x8000 -C ../../credentials/development/attestation/Chip-Development-PAA-Cert.pem -K ../../credentials/development/attestation/Chip-Development-PAA-Key.pem -o Esp-Development-PAI-Cert.pem -O Esp-Development-PAI-Key.pem -l 4294967295
```

### 1.2.4 Generating DAC

```
./chip-cert gen-att-cert -t d -c "ESP TEST DAC 01" -V 0xfff1 -P 0x8000 -C Esp-Development-PAI-Cert.pem -K Esp-Development-PAI-Key.pem -o Esp-Development-DAC-01.pem -O Esp-Development-DAC-Key-01.pem -l 4294967295
```

### 1.2.5 Change format for the certificates and key (.pem to .der format)

-   Convert DAC key from .pem to .der format.

```
openssl ec -in Esp-Development-DAC-Key-01.pem -out Esp-Development-DAC-Key-01.der -inform pem -outform der
```

-   Convert DAC and PAI cert from .pem to .der format

```
openssl x509 -in Esp-Development-DAC-01.pem -out Esp-Development-DAC-01.der-inform pem -outform der
openssl x509 -in Esp-Development-PAI-Cert.pem -out Esp-Development-PAI-Cert.der -inform pem -outform der
```

The certificates in the steps 1.2 will be generated at
/path/to/connectedhomeip/out/host.For steps 1.3 and 1.4 go to
connectedhomeip/scripts/tools , set IDF_PATH.

## 1.3 Generating esp_secure_cert_partition

To generate the esp_secure_cert_partition install esp-secure-cert-tool using

```
pip install esp-secure-cert-tool
```

Example command to generate a esp_secure_cert_partition

```
configure_esp_secure_cert.py --private-key path/to/dac-key \
--device-cert path/to/dac-cert \
--ca-cert path/to/pai-cert \
--target_chip esp32c3 \
--port /dev/ttyUSB0 -- skip_flash
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
                                    --dac-cert path/to/dac-cert \
                                    --dac-key path/to/dac-key \
                                    --pai-cert path/to/pai-cert \
                                    --cd path/to/certificate-declaration
```

Refer
https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/esp32/factory_data.md#generate-nvs-binary-image
to generate a factory_data_partition.

## 1.5 Build the firmware with below configuration options

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

### Monitor

```
idf.py monitor
```

Please flash the above mentioned partitions by looking into the addresses in
partitions.csv.The above commands are for example purpose.

## 1.6 Test commissioning using chip-tool

Run the following command from host to commission the device.

```
./chip-tool pairing ble-wifi 1234 my_SSID my_PASSPHRASE my_PASSCODE my_DISCRIMINATOR --paa-trust-store-path /path/to/PAA-Certificates/
```

For example:

```
./chip-tool pairing ble-wifi 0x7283 my_SSID my_PASSPHRASE 99663300 3434 --paa-trust-store-path /path/to/connectedhomeip/credentials/development/attestation/
```
