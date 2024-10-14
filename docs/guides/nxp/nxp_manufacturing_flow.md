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
--dac_cert         -> path to the DAC (der format) location
--dac_key          -> path to the DAC key (der format) location
--pai_cert         -> path to the PAI (der format) location
--spake2p_path     -> path to the spake2p tool
--out              -> name of the binary that will be used for storing all the generated data
```

Here is the interpretation of the **optional** parameters:

```shell
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

Use `chip_with_factory_data=1` when compiling to enable factory data usage.

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
`chip_with_factory_data=1` is used can be found in
`./scripts/tools/nxp/demo_generated_certs`.

## 6. Increased security for DAC private key

### 6.1 SSS-based platforms

Supported platforms:

-   `k32w1`
-   `mcxw71`

For platforms that have a secure subsystem (`SSS`), the DAC private key can be
converted to an encrypted blob. This blob will overwrite the DAC private key in
factory data and will be imported in the `SSS` at initialization, by the factory
data provider instance.

The application will check at initialization whether the DAC private key has
been converted or not and convert it if needed. However, the conversion process
should be done at manufacturing time for security reasons.

Reference factory data generation command:

```shell
python3 ./scripts/tools/nxp/factory_data_generator/generate.py -i 10000 -s UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA= -p 14014 -d 1000 --vid "0x1037" --pid "0xA221" --vendor_name "NXP Semiconductors" --product_name "Lighting app" --serial_num "12345678" --date "2023-01-01" --hw_version 1 --hw_version_str "1.0" --cert_declaration ./Chip-Test-CD-1037-A221.der --dac_cert ./Chip-DAC-NXP-1037-A221-Cert.der --dac_key ./Chip-DAC-NXP-1037-A221-Key.der --pai_cert ./Chip-PAI-NXP-1037-A221-Cert.der --spake2p_path ./out/spake2p --unique_id "00112233445566778899aabbccddeeff" --hw_params --out ./factory_data.bin
```

There is no need for an extra binary.

-   Write factory data binary.
-   Build the application with `chip_with_factory_data=1` set.
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
adding the following gn argument `chip_use_plain_dac_key=true`.

### 6.2 RW61X

Supported platforms:

-   RW61X

there are three implementations for factory data protection

-   whole factory data protection with AES encryption ( chip_with_factory_data=1
    chip_enable_secure_whole_factory_data=true )
    `examples/platform/nxp/rt/rw61x/factory_data/source/AppFactoryDataExample.cpp`\
    `src/platform/nxp/rt/rw61x/FactoryDataProviderEncImpl.cpp`

-   only dac private key protection ( chip_with_factory_data=1
    chip_enable_secure_dac_private_key_storage=true )  
    `examples/platform/nxp/rt/rw61x/factory_data/source/AppFactoryDataExample.cpp`
    \
    `src/platform/nxp/rt/rw61x/FactoryDataProviderImpl.cpp`

-   whole factory data protection with hard-coded AES key (
    chip_with_factory_data=1 )
    `examples/platform/nxp/common/factory_data/source/AppFactoryDataDefaultImpl.cpp`
    \
    `src/platform/nxp/common/factory_data/FactoryDataProviderFwkImpl.cpp`

for the first one, the whole factory data is encrypted by an AES-256 key, the
AES key can be passed through serial link when in factory production mode, and
will be provisioned into Edge Lock, and the returned AES Key blob (wrapped key)
can be stored in the end of factory data region in TLV format. for the
decryption process, the blob is retrieved and provisioned into Edge Lock and the
whole factory data can be decrypted using the returned key index in Edge Lock.
Compared with only dac private key protection solution, this solution can avoid
tampering with the original factory data.

the factory data should be encrypted by an AES-256 key using "--aes256_key"
option in "generate.py" script file.

it will check whether there is AES key blob in factory data region when in each
initialization, if not, the default AES key is converted and the result is
stored into flash, it run only once.

for the second one, it only protect the dac private key inside the factory data,
the dac private key is retrieved and provisioned into Edge Lock, the returned
key blob replace the previous dac private key, and also update the overall size
and hash, and re-write the factory data. when device is doing matter
commissioning, the blob is retrieved and provisioned into Edge Lock and the
signing can be done using the returned key index in Edge Lock.

the factory data should be plain text for the first programming. it will check
whether there is dac private key blob (base on the size of blob, should be 48)
in factory data when in each initialization, if not, the dac private key is
converted and the result is stored into flash, it run only once.

for the third one, it is a little similar to the first one, the whole factory
data is encrypted by an AES key, but there are two differences:

-   the AES key is hard-coded and not provisioned into Edge Lock
-   the factory data should be encrypted by AES-128 key using "--aes128_key"
    option in "generate.py" script file.
