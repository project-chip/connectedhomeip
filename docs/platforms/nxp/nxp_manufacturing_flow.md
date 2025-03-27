# NXP Manufacturing data

By default, the example application is configured to use generic test
certificates and provisioning data embedded with the application code. It is
possible for a final stage application to generate its own manufacturing data
using the procedure described below.

## 1. Prerequisites

Generate build files from Matter root folder by running:

```
gn gen out
```

Build `chip-cert` tool:

```
ninja -C out chip-cert
```

Build `spake2p` tool:

```
ninja -C out spake2p
```

## 2. Generate

### a. Certificates

To generate the different certificates, NXP provides a Python script
`scripts/tools/nxp/generate_certs.py`. This script will always generate the PAI
and DAC certificates/keys. It can also generate the Certification Declaration
and the PAA certificate/key depending on the parameters.

| Parameter          | Description                                                                                       | Type                   | Required |
| ------------------ | ------------------------------------------------------------------------------------------------- | ---------------------- | -------- |
| `--chip_cert_path` | Path to chip-cert executable                                                                      | string                 | Yes      |
| `--output`         | Output path to store certificates                                                                 | string                 | Yes      |
| `--vendor_id`      | Vendor Identification Number                                                                      | integer or hex integer | Yes      |
| `--product_id`     | Product Identification Number                                                                     | integer or hex integer | Yes      |
| `--vendor_name`    | Human-readable vendor name                                                                        | string                 | Yes      |
| `--product_name`   | Human-readable product name                                                                       | string                 | Yes      |
| `--gen_cd`         | Use this option to enable Certificate Declaration generation                                      | boolean                | No       |
| `--cd_type`        | Type of generated Certification Declaration: `0` - development, `1` - provisional, `2` - official | integer                | No       |
| `--device_type`    | The primary device type implemented by the node                                                   | int                    | No       |
| `--paa_cert`       | Path to the Product Attestation Authority (PAA) certificate. Will be generated if not provided.   | string                 | No       |
| `--paa_key`        | Path to the Product Attestation Authority (PAA) key. Will be generated if not provided.           | string                 | No       |
| `--valid_from`     | The start date for the certificate's validity period.                                             | string                 | No       |
| `--lifetime`       | The lifetime for the certificates, in whole days.                                                 | string                 | No       |

You can also run the following command to get more details on the parameters and
their default value (if applicable):

```shell
python scripts/tools/nxp/generate_certs.py --help
```

Example of a command that will generate CD, PAA, PAI and DAC certificates and
keys in both .pem and .der formats:

```shell
python scripts/tools/nxp/generate_certs.py --gen_cd --cd_type 1 --chip_cert_path ./out/chip-cert --vendor_id 0x1037 --product_id 0xA220 --vendor_name "NXP Semiconductors" --product_name all-clusters-app --device_type 65535 --output .
```

> **Note**: the commands provided in this guide are just for the example and
> shall be adapted to your use case accordingly

### c. Provisioning data

Generate new provisioning data and convert all the data to a binary (unencrypted
data):

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid 0x1037 --pid 0xA220 --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "12345678" --date "2023-01-01" --hw_version 1 --hw_version_str "1.0" --cert_declaration ./Chip-Test-CD-1037-A220.der --dac_cert ./Chip-DAC-NXP-1037-A220-Cert.der --dac_key ./Chip-DAC-NXP-1037-A220-Key.der --pai_cert ./Chip-PAI-NXP-1037-A220-Cert.der --spake2p_path ./out/spake2p --unique_id "00112233445566778899aabbccddeeff" --out ./factory_data.bin
```

Same example as above, but with an already generated verifier passed as input:

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid "0x1037" --pid "0xA220" --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "12345678" --date "2023-01-01" --hw_version 1 --hw_version_str "1.0" --cert_declaration ./Chip-Test-CD-1037-A220.der --dac_cert ./Chip-DAC-NXP-1037-A220-Cert.der --dac_key ./Chip-DAC-NXP-1037-A220-Key.der --pai_cert ./Chip-PAI-NXP-1037-A220-Cert.der --spake2p_path ./out/spake2p --spake2p_verifier ivD5n3L2t5+zeFt6SjW7BhHRF30gFXWZVvvXgDxgCNcE+BGuTA5AUaVm3qDZBcMMKn1a6CakI4SxyPUnJr0CpJ4pwpr0DvpTlkQKqaRvkOQfAQ1XDyf55DuavM5KVGdDrg== --unique_id "00112233445566778899aabbccddeeff" --out ./factory_data.bin
```

Generate new provisioning data and convert all the data to a binary (encrypted
data with the AES key). Add the following option to one of the above examples:

```shell
--aes128_key 2B7E151628AED2A6ABF7158809CF4F3C
```

Here is the interpretation of the **required** parameters:

```shell
-i                 -> SPAKE2+ iteration
-s                 -> SPAKE2+ salt (passed as base64 encoded string)
-p                 -> SPAKE2+ passcode
-d                 -> discriminator
--vid              -> Vendor ID
--pid              -> Product ID
--vendor_name      -> Vendor Name
--product_name     -> Product Name
--hw_version       -> Hardware Version as number
--hw_version_str   -> Hardware Version as string
--cert_declaration -> path to the Certification Declaration (der format) location
--pai_cert         -> path to the PAI (der format) location
--spake2p_path     -> path to the spake2p tool
--out              -> name of the binary that will be used for storing all the generated data
```

Here is the interpretation of the **optional** parameters:

```shell
--dac_cert              -> path to the DAC certificate (der format) location
--dac_key               -> path to the DAC key (der format) location
--EL2GO_bin             -> path to the EdgeLock 2Go binary (bin format) location
--EL2GO_DAC_KEY_ID      -> DAC key ID configured into EdgeLock 2Go as hex value
--EL2GO_DAC_CERT_ID     -> DAC certificate ID configured into EdgeLock 2Go as hex value
--dac_key_password      -> Password to decode DAC key
--dac_key_use_sss_blob  -> Used when --dac_key contains a path to an encrypted blob, instead of the
                           actual DAC private key. The blob metadata size is 24, so the total length
                           of the resulting value is private key length (32) + 24 = 56. False by default.
--spake2p_verifier      -> SPAKE2+ verifier (passed as base64 encoded string). If this option is set,
                           all SPAKE2+ inputs will be encoded in the final binary. The spake2p tool
                           will not be used to generate a new verifier on the fly.
--aes128_key            -> 128 bits AES key used to encrypt the whole dataset. Please make sure
                           that the target application/board supports this feature: it has access to
                           the private key and implements a mechanism which can be used to decrypt
                           the factory data information.
--date                  -> Manufacturing Date (YYYY-MM-DD format)
--part_number           -> Part number as string
--product_url           -> Product URL as string
--product_label         -> Product label as string
--serial_num            -> Serial Number
--unique_id             -> Unique id used for rotating device id generation
--product_finish        -> Visible finish of the product
--product_primary_color -> Representative color of the visible parts of the product
--hw_params             -> Use application factory data from Hardware Parameters component
```

## 3. Write provisioning data

| platform  | tool                             | command                                                                                    | details                                                                                            |
| --------- | -------------------------------- | ------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------- |
| `k32w0x1` | `DK6Programmer.exe` or `dk6prog` | `DK6Programmer.exe -Y -V2 -s <COM_PORT> -P 1000000 -Y -p FLASH@0x9D600="factory_data.bin"` | NA                                                                                                 |
| `k32w1`   | `JLink`                          | `loadfile factory_data.bin 0xFE080`                                                        | NA                                                                                                 |
| `mcxw71`  | `JLink`                          | `loadfile factory_data.bin 0xFE080`                                                        | NA                                                                                                 |
| `rw61x`   | `JLink`                          | `loadfile factory_data.bin 0xBFFF000`                                                      | Here, `0xBFFF000` is the value of symbol `__FACTORY_DATA_START` from the corresponding `.map` file |
| `rt1060`  | `MCUXpresso Flash Tool GUI`      | NA                                                                                         | The address is given by the `__FACTORY_DATA_START` symbol in the `.map` file                       |
| `rt1170`  | `MCUXpresso Flash Tool GUI`      | NA                                                                                         | The address is given by the `__FACTORY_DATA_START` symbol in the `.map` file                       |

## 4. Build app and usage

Use `nxp_use_factory_data=true` when compiling to enable factory data usage.

Run chip-tool with a new PAA:

```shell
./chip-tool pairing ble-thread 2 hex: $hex_value 14014 1000 --paa-trust-store-path /home/ubuntu/certs/paa
```

Here is the interpretation of the parameters:

```shell
--paa-trust-store-path -> path to the generated PAA (der format)
```

`paa-trust-store-path` must contain only the PAA certificate. Avoid placing
other certificates in the same location as this may confuse `chip-tool`.

**PAA** certificate can be copied to the chip-tool machine using **SCP** for
example.

This is needed for testing self-generated **DACs**, but likely not required for
"true production" with production **PAI** issued **DACs**.

## 5. Useful information/Known issues

Implementation of manufacturing data provisioning has been validated using test
certificates generated by `OpenSSL 1.1.1l`.

Also, demo **DAC**, **PAI** and **PAA** certificates needed in case
`nxp_use_factory_data=true` is used can be found in
`./scripts/tools/nxp/demo_generated_certs`.

## 6. Increased security for DAC private key

### 6.1 SSS-based with EdgeLock2go support

EdgeLock2go services could be used to securely provisioned DAC key/cert during
manufacturing.

Prior to the generation of the factory data binary. `EL2GO` data needs to be
generated following `EL2GO` process.

For the factory data generation following option need to be added:

`--EL2GO_bin ~/secure_objects.bin` containing `EL2GO` information including
encrypted DAC private key and certificate. `--EL2GO_DAC_KEY_ID 1234` containing
corresponding to the ID of the DAC key chosen during `EL2GO` key generation.
`--EL2GO_DAC_CERT_ID 4321` containing corresponding to the ID of the DAC
certification chosen during `EL2GO` key generation.

Reference factory data generation command:

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p ${passcode} -d ${discriminator} --vid "0x$VID" --pid "0x$PID" --vendor_name "NXP Semiconductors" --product_name "Thermostat" --serial_num "12345678" --date "$DATE" --hw_version 1 --hw_version_str "1.0" --cert_declaration $FACTORY_DATA_DEST/Chip-Test-CD-$VID-$PID.der --EL2GO_bin ~/secure_objects.bin --EL2GO_DAC_KEY_ID 1234 --EL2GO_DAC_CERT_ID 4321 --pai_cert $FACTORY_DATA_DEST/Chip-PAI-NXP-$VID-$PID-Cert.der --spake2p_path ./out/spake2p --unique_id "00112233445566778899aabbccddeeff" --out $FACTORY_DATA_DEST/factory_data.bin
```

Supported platforms:

-   `rw61x`

In addition to the GN flag `nxp_use_factory_data=true`, a Matter application
needs to be built with `nxp_enable_secure_EL2GO_factory_data=true` to allow
loading of EdgeLock2go data to the secure element.

In this mode EdgeLock2go keys will always remain encrypted and only usable by
the `SSS`. In this case, all operations that requires DAC private access will be
transferred to the `SSS`.

### 6.2 SSS-based without EdgeLock2go support for DAC private key secure storage

Supported platforms:

-   `k32w1`
-   `mcxw71`
-   `rw61x`

For platforms that have a secure subsystem (`SSS`), the DAC private key can be
converted to an encrypted blob. This blob will overwrite the DAC private key in
factory data and will be imported in the `SSS` by the factory data provider
instance.

In this architecture, outside of the manufacturing flow, the DAC private will
always remain usable only by the `SSS`. In this case, all operations that
requires DAC private access will be transferred to the `SSS`.

The application will check at initialization whether the DAC private key has
been converted or not and convert it if needed. However, the conversion process
should be done at manufacturing time for security reasons.

Reference factory data generation command:

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid "0x1037" --pid "0xA221" --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "12345678" --date "2023-01-01" --hw_version 1 --hw_version_str "1.0" --cert_declaration ./Chip-Test-CD-1037-A221.der --dac_cert ./Chip-DAC-NXP-1037-A221-Cert.der --dac_key ./Chip-DAC-NXP-1037-A221-Key.der --pai_cert ./Chip-PAI-NXP-1037-A221-Cert.der --spake2p_path ./out/spake2p --unique_id "00112233445566778899aabbccddeeff" --hw_params --out ./factory_data.bin
```

There is no need for an extra binary.

-   Write factory data binary.
-   Build the application with `nxp_use_factory_data=true` set.
-   Write the application to the board and use it as usual.

Factory data should now contain a corresponding encrypted blob instead of the
DAC private key.

If an encrypted blob of the DAC private key is already available (e.g. obtained
previously, using other methods), then the conversion process shall be skipped.
Instead, option `--dac_key_use_sss_blob` can be used in the factory data
generation command:

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid "0x1037" --pid "0xA221" --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "12345678" --date "2023-01-01" --hw_version 1 --hw_version_str "1.0" --cert_declaration ./Chip-Test-CD-1037-A221.der --dac_cert ./Chip-DAC-NXP-1037-A221-Cert.der --dac_key ./Chip-DAC-NXP-1037-A221-Key-encrypted-blob.bin --pai_cert ./Chip-PAI-NXP-1037-A221-Cert.der --spake2p_path ./out/spake2p --unique_id "00112233445566778899aabbccddeeff" --dac_key_use_sss_blob --out ./factory_data_with_blob.bin
```

Please note that `--dac_key` now points to a binary file that contains the
encrypted blob.

The user can use the DAC private in plain text instead of using the `SSS` by
adding the following gn argument `chip_use_plain_dac_key=true` (not supported on
rw61x).
