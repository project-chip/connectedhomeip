# Configuring factory data for the nRF Connect examples

Factory data is a set of device parameters written to the non-volatile memory
during the manufacturing process. This guide describes the process of creating
and programming factory data using Matter and the nRF Connect platform from
Nordic Semiconductor.

The factory data parameter set includes different types of information, for
example about device certificates, cryptographic keys, device identifiers, and
hardware. All those parameters are vendor-specific and must be inserted into a
device's persistent storage during the manufacturing process. The factory data
parameters are read at the boot time of a device. Then, they can be used in the
Matter stack and user application (for example during commissioning).

All of the factory data parameters are protected against modifications by the
software, and the firmware data parameter set must be kept unchanged during the
lifetime of the device. When implementing your firmware, you must make sure that
the factory data parameters are not re-written or overwritten during the Device
Firmware Update (DFU) or factory resets, except in some vendor-defined cases.

For the nRF Connect platform, the factory data is stored by default in a
separate partition of the internal flash memory. This helps to keep the factory
data secure by applying hardware write protection.

> **Note:** Due to hardware limitations, in the nRF Connect platform, protection
> against writing can be applied only to the internal memory partition. The
> [Fprotect](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/libraries/security/bootloader/fprotect.html)
> is the hardware flash protection driver, and we used it to ensure write
> protection of the factory data partition in internal flash memory.

<hr>

-   [Configuring factory data for the nRF Connect examples](#configuring-factory-data-for-the-nrf-connect-examples)
    -   [Overview](#overview)
        -   [Factory data component table](#factory-data-component-table)
        -   [Factory data format](#factory-data-format)
            -   [Appearance field description](#appearance-field-description)
    -   [Enabling factory data support](#enabling-factory-data-support)
    -   [Generating factory data](#generating-factory-data)
        -   [Creating the factory data JSON and HEX files with the first script](#creating-the-factory-data-json-and-hex-files-with-the-first-script)
        -   [How to set user data](#how-to-set-user-data)
            -   [How to handle user data](#how-to-handle-user-data)
        -   [Verifying using the JSON Schema tool](#verifying-using-the-json-schema-tool)
            -   [Option 1: Using the php-json-schema tool](#option-1-using-the-php-json-schema-tool)
            -   [Option 2: Using a website validator](#option-2-using-a-website-validator)
            -   [Option 3: Using the nRF Connect Python script](#option-3-using-the-nrf-connect-python-script)
        -   [Generating onboarding codes](#generating-onboarding-codes)
            -   [Enabling onboarding codes generation within the build system](#enabling-onboarding-codes-generation-within-the-build-system)
        -   [Preparing factory data partition on a device](#preparing-factory-data-partition-on-a-device)
        -   [Creating a factory data partition with the second script](#creating-a-factory-data-partition-with-the-second-script)
    -   [Building an example with factory data](#building-an-example-with-factory-data)
        -   [Providing factory data parameters as a build argument list](#providing-factory-data-parameters-as-a-build-argument-list)
        -   [Setting factory data parameters using interactive Kconfig interfaces](#setting-factory-data-parameters-using-interactive-kconfig-interfaces)
        -   [Default Kconfig values and developing aspects](#default-kconfig-values-and-developing-aspects)
    -   [Programming factory data](#programming-factory-data)
    -   [Using own factory data implementation](#using-own-factory-data-implementation)

<hr>

## Overview

You can implement the factory data set described in the
[factory data component table](#factory-data-component-table) in various ways,
as long as the final HEX file contains all mandatory components defined in the
table. In this guide, the [generating factory data](#generating-factory-data)
and the
[building an example with factory data](#building-an-example-with-factory-data)
sections describe one of the implementations of the factory data set created by
the nRF Connect platform's maintainers. At the end of the process, you get a HEX
file that contains the factory data partition in the CBOR format.

The factory data accessor is a component that reads and decodes factory data
parameters from the device's persistent storage and creates an interface to
provide all of them to the Matter stack and to the user application.

The default implementation of the factory data accessor assumes that the factory
data stored in the device's flash memory is provided in the CBOR format.
However, it is possible to generate the factory data set without using the nRF
Connect scripts and implement another parser and a factory data accessor. This
is possible if the newly provided implementation is consistent with the
[Factory Data Provider](../../src/platform/nrfconnect/FactoryDataProvider.h).
For more information about preparing a factory data accessor, see the section
about
[using own factory data implementation](#using-own-factory-data-implementation).

> **Note:** Encryption and security of the factory data partition is not
> provided yet for this feature.

### Factory data component table

The following table lists the parameters of a factory data set:

|       Key name       |              Full name               |        Length        |    Format    | Conformance |                                                                                                                                                                                                                                               Description                                                                                                                                                                                                                                               |
| :------------------: | :----------------------------------: | :------------------: | :----------: | :---------: | :-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
|      `version`       |         factory data version         |         2 B          |    uint16    |  mandatory  |                                                                                                                                                                   A version of the current factory data set. It cannot be changed by a user and it must be coherent with current version of the Factory Data Provider on device side.                                                                                                                                                                   |
|         `sn`         |            serial number             |      <1, 32> B       | ASCII string |  mandatory  |                                                                                                                                                                                  A serial number parameter defines an unique number of manufactured device. The maximum length of the serial number is 32 characters.                                                                                                                                                                                   |
|     `vendor_id`      |              vendor ID               |         2 B          |    uint16    |  mandatory  |                                                                                                                                                                                                              A CSA-assigned ID for the organization responsible for producing the device.                                                                                                                                                                                                               |
|     `product_id`     |              product ID              |         2 B          |    uint16    |  mandatory  |                                                                                                                                                                          A unique ID assigned by the device vendor to identify the product. It defaults to a CSA-assigned ID that designates a non-production or test product.                                                                                                                                                                          |
|    `vendor_name`     |             vendor name              |      <1, 32> B       | ASCII string |  mandatory  |                                                                                                                                                                         A human-readable vendor name that provides a simple string containing identification of device's vendor for the application and Matter stack purposes.                                                                                                                                                                          |
|    `product_name`    |             product name             |      <1, 32> B       | ASCII string |  mandatory  |                                                                                                                                                                         A human-readable product name that provides a simple string containing identification of the product for the application and the Matter stack purposes.                                                                                                                                                                         |
|        `date`        |          manufacturing date          |         10 B         |   ISO 8601   |  mandatory  |                                                                                                                                                                                  A manufacturing date specifies the date that the device was manufactured. The date format used is ISO 8601, for example `YYYY-MM-DD`.                                                                                                                                                                                  |
|       `hw_ver`       |           hardware version           |         2 B          |    uint16    |  mandatory  |                                                                                                                                                                    A hardware version number that specifies the version number of the hardware of the device. The value meaning and the versioning scheme is defined by the vendor.                                                                                                                                                                     |
|     `hw_ver_str`     |       hardware version string        |      <1, 64> B       |    uint16    |  mandatory  |                                                                                                                       A hardware version string parameter that specifies the version of the hardware of the device as a more user-friendly value than that presented by the hardware version integer value. The value meaning and the versioning scheme is defined by the vendor.                                                                                                                       |
|       `rd_uid`       |     rotating device ID unique ID     |      <16, 32> B      | byte string  |  mandatory  | The unique ID for rotating device ID, which consists of a randomly-generated 128-bit (or longer) octet string. This parameter should be protected against reading or writing over-the-air after initial introduction into the device, and stay fixed during the lifetime of the device. When building an application with the Factory Data support, the `CONFIG_CHIP_FACTORY_DATA_ROTATING_DEVICE_UID_MAX_LEN` must be set with the length of the actual `rd_uid` stored in the Factory Data partition. |
|      `dac_cert`      | (DAC) Device Attestation Certificate |      <1, 602> B      | byte string  |  mandatory  |                                                                                                       The Device Attestation Certificate (DAC) and the corresponding private key are unique to each Matter device. The DAC is used for the Device Attestation process and to perform commissioning into a fabric. The DAC is a DER-encoded X.509v3-compliant certificate, as defined in RFC 5280.                                                                                                       |
|      `dac_key`       |           DAC private key            |         68 B         | byte string  |  mandatory  |                                                                                                                                                   The private key associated with the Device Attestation Certificate (DAC). This key should be encrypted and maximum security should be guaranteed while generating and providing it to factory data.                                                                                                                                                   |
|      `pai_cert`      |   Product Attestation Intermediate   |      <1, 602> B      | byte string  |  mandatory  |                                                                                                        An intermediate certificate is an X.509 certificate, which has been signed by the root certificate. The last intermediate certificate in a chain is used to sign the leaf (the Matter device) certificate. The PAI is a DER-encoded X.509v3-compliant certificate as defined in RFC 5280.                                                                                                        |
|     `spake2_it`      |      SPAKE2+ iteration counter       |         4 B          |    uint32    |  mandatory  |                                                                                                                                                                    A SPAKE2+ iteration counter is the amount of PBKDF2 (a key derivation function) interactions in a cryptographic process used during SPAKE2+ Verifier generation.                                                                                                                                                                     |
|    `spake2_salt`     |             SPAKE2+ salt             |      <32, 64> B      | byte string  |  mandatory  |                                                                                                                                    The SPAKE2+ salt is a random piece of data, at least 32 byte long. It is used as an additional input to a one-way function that performs the cryptographic operations. A new salt should be randomly generated for each password.                                                                                                                                    |
|  `spake2_verifier`   |           SPAKE2+ verifier           |         97 B         | byte string  |  mandatory  |                                                                                                                                                                                                           The SPAKE2+ verifier generated using SPAKE2+ salt, iteration counter, and passcode.                                                                                                                                                                                                           |
|   `discriminator`    |            Discriminator             |         2 B          |    uint16    |  mandatory  |                                                                                                                                                                                      A 12-bit value matching the field of the same name in the setup code. The discriminator is used during the discovery process.                                                                                                                                                                                      |
|      `passcode`      |            SPAKE passcode            |         4 B          |    uint32    |  optional   |                                   A pairing passcode is a 27-bit unsigned integer which serves as a proof of possession during the commissioning. Its value must be restricted to the values from `0x0000001` to `0x5F5E0FE` (`00000001` to `99999998` in decimal), excluding the following invalid passcode values: `00000000`, `11111111`, `22222222`, `33333333`, `44444444`, `55555555`, `66666666`, `77777777`, `88888888`, `99999999`, `12345678`, `87654321`.                                    |
| `product_appearance` |      Product visible appearance      |         2 B          |   CBOR map   |  optional   |                                                                                             The appearance field is a structure that describes the visible appearance of the product. This field is provided in a CBOR map and consists of two attributes: `finish` (1 B), `primary_color` (1 B). See the [Appearance field description](#appearance-field-description) to learn how to set all attributes.                                                                                             |
|        `user`        |              User data               | variable, max 1024 B |   CBOR map   |  optional   |                                                            The user data is provided in the JSON format. This parameter is optional and depends on the device manufacturer's purpose. It is provided as a CBOR map type from persistent storage and should be parsed in the user application. This data is not used by the Matter stack. To learn how to work with user data, see the [How to set user data](#how-to-set-user-data) section.                                                            |

### Factory data format

The factory data set must be saved into a HEX file that can be written to the
flash memory of the Matter device.

In the nRF Connect example, the factory data set is represented in the CBOR
format and is stored in a HEX file. The file is then programmed to a device. The
JSON format is used as an intermediate, human-readable representation of the
data. The format is regulated by the
[JSON Schema](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/nrfconnect/nrfconnect_factory_data.schema)
file.

All parameters of the factory data set are either mandatory or optional:

-   Mandatory parameters must always be provided, as they are required for
    example to perform commissioning to the Matter network.
-   Optional parameters can be used for development and testing purposes. For
    example, the `user` data parameter consists of all data that is needed by a
    specific manufacturer and that is not included in the mandatory parameters.

In the factory data set, the following formats are used:

-   uint8, uint16, and uint32 -- These are the numeric formats representing,
    respectively, one-byte length unsigned integer, two-bytes length unsigned
    integer, and four-bytes length unsigned integer. This value is stored in a
    HEX file in the big-endian order.
-   Byte string - This parameter represents the sequence of integers between `0`
    and `255`(inclusive), without any encoding. Because the JSON format does not
    allow to use of byte strings, the `hex:` prefix is added to a parameter, and
    its representation is converted to a HEX string. For example, an ASCII
    string _`abba`_ is represented as _`hex:61626261`_ in the JSON file and then
    stored in the HEX file as `0x61626261`. The HEX string length in the JSON
    file is two times greater than the byte string plus the size of the prefix.
-   ASCII string is a string representation in ASCII encoding without
    null-terminating.
-   ISO 8601 format is a
    [date format](https://www.iso.org/iso-8601-date-and-time-format.html) that
    represents a date provided in the `YYYY-MM-DD` or `YYYYMMDD` format.
-   All certificates stored in factory data are provided in the
    [X.509](https://www.itu.int/rec/T-REC-X.509-201910-I/en) format.

#### Appearance field description

The `appearance` field in the factory data set describes the device's visible
appearance.

-   `finish` - A string name that indicates the visible exterior finish of the
    product. It refers to the `ProductFinishEnum` enum, and currently, you can
    choose one of the following names:

|    Name    | Enum value |
| :--------: | :--------: |
|  `matte`   |     0      |
|  `satin`   |     1      |
| `polished` |     2      |
|  `rugged`  |     3      |
|  `fabric`  |     4      |
|  `other`   |    255     |

-   `primary_color` - A string name that represents the RGB color space of the
    device's case color, which is the most representative. It refers to the
    `ColorEnum` enum, and currently, you can choose one of the following names:

(Enum value) color name (`RGB value`)

| (0) $$\color{black} \color{black}{black}$$ (`#000000`) | (1) $$\color{#000080}{navy}$$ (`#000080`)    | (2) $$\color{#008000}{green}$$ (`#008000`)    | (3) $$\color{#008080}{teal}$$ (`#008080`)    | (4) $$\color{#800080}{maroon}$$ (`#800080`)             |
| ------------------------------------------------------ | -------------------------------------------- | --------------------------------------------- | -------------------------------------------- | ------------------------------------------------------- |
| (5) $$\color{#800080}{purple}$$ (`#800080`)            | (6) $$\color{#808000}{olive}$$ (`#800080`)   | (7) $$\color{#808080}{gray}$$ (`#800080`)     | (8) $$\color{blue}{blue}$$ (`#0000FF`)       | (9) $$\color{lime}{lime}$$ (`#00FF00`)                  |
| (10) $$\color{aqua}{aqua}$$ (`#00FFFF`)                | (11) $$\color{red}{red}$$ (`#FF0000`)        | (12) $$\color{fuchsia}{fuchsia}$$ (`#FF00FF`) | (13) $$\color{yellow}{yellow}$$ (`#FFFF00`)  | (14) $$\color{white} \color{white}{white}$$ (`#800080`) |
| (15) $$\color{#727472}{nickel}$$ (`#727472`)           | (16) $$\color{#a8a9ad}{chrome}$$ (`#a8a9ad`) | (17) $$\color{#E1C16E}{brass}$$ (`#E1C16E`)   | (18) $$\color{#B87333}{copper}$$ (`#B87333`) | (19) $$\color{#C0C0C0}{silver}$$ (`#C0C0C0`)            |
| (20) $$\color{gold}{gold}$$ (`#FFD700`)                |

<hr>

## Enabling factory data support

By default, the factory data support is disabled in all nRF Connect examples and
the nRF Connect device uses predefined parameters from the Matter core, which
you should not change. To start using factory data stored in the flash memory
and the **Factory Data Provider** from the nRF Connect platform, build an
example with the following option (replace _<build_target>_ with your board
name, for example, `nrf52840dk_nrf52840`):

```
$ west build -b <build_target> -- -DCONFIG_CHIP_FACTORY_DATA=y
```

## Generating factory data

This section describes generating factory data using the following nRF Connect
Python scripts:

-   The first script creates a JSON file that contains a user-friendly
    representation of the factory data.
-   The second script uses the JSON file to create a factory data partition and
    save it to a HEX file.

After these operations, you will program a HEX file containing factory data
partition into the device's flash memory.

You can use the second script without invoking the first one by providing a JSON
file written in another way. To make sure that the JSON file is correct and the
device is able to read out parameters,
[verify the file using the JSON schema tool](#verifying-using-the-json-schema-tool).

You can also use only the first script to generate both JSON and HEX files, by
providing optional `offset` and `size` arguments, which results in invoking the
script internally. Such option is the recommended one, but invoking two scripts
one by one is also supported to provide backward compatibility.

### Creating the factory data JSON and HEX files with the first script

A Matter device needs a proper factory data partition stored in the flash memory
to read out all required parameters during startup. To simplify the factory data
generation, you can use the
[generate_nrfconnect_chip_factory_data.py](../../scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py)
Python script to provide all required parameters and generate a human-readable
JSON file and save it to a HEX file.

To use this script, complete the following steps:

1. Navigate to the `connectedhomeip` root directory.

2. Run the script with `-h` option to see all possible options:

    ```
    $ python scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py -h
    ```

3. Prepare a list of arguments:

    a. Fill up all mandatory arguments:

    ```
    --sn --vendor_id, --product_id, --vendor_name, --product_name, --date, --hw_ver, --hw_ver_str, --spake2_it, --spake2_salt, --discriminator
    ```

    b. Add output path to store .json file, e.g. my_dir/output:

    ```
    -o <path_to_output_file>
    ```

    c. Generate SPAKE2 verifier using one of the following methods:

    - Automatic:

        ```
        --passcode <pass_code>
        ```

    - Manual:

        ```
        --spake2_verifier <verifier>
        ```

    d. Add paths to `.der` files that contain PAI and DAC certificates and the
    DAC private key (replace the respective variables with the file names) using
    one of the following methods:

    - Automatic:

        ```
        --chip_cert_path <path to chip-cert executable>
        --gen_certs
        ```

    > **Note:** To generate new certificates, you need the `chip-cert`
    > executable. See the note at the end of this section to learn how to get
    > it.

    - Manual:

        ```
        --dac_cert <path to DAC certificate>.der --dac_key <path to DAC key>.der --pai_cert <path to PAI certificate>.der
        ```

    e. (optional) Add the new unique ID for rotating device ID using one of the
    following options:

    - Provide an existing ID:

        ```
        --rd_uid <rotating device ID unique ID>
        ```

    - (optional) Generate a new ID and provide it:

        ```
        --generate_rd_uid
        --rd_uid <rotating device ID unique ID>
        ```

        You can find a newly generated unique ID in the console output.

    f. (optional) Add the JSON schema to verify the JSON file (replace the
    respective variable with the file path):

    ```
    --schema <path to JSON Schema file>
    ```

    g. (optional) Add a request to include a pairing passcode in the JSON file:

    ```
    --include_passcode
    ```

    h. (optional) Add the request to overwrite existing the JSON file:

    ```
    --overwrite
    ```

    i. (optional) Add the appearance of the product:

    ```
    --product_finish <finish>
    --product_color <color>
    ```

    j. (optional) Generate Certification Declaration for testing purposes

    ```
    --chip_cert_path <path to chip-cert executable>
    --gen_cd
    ```

    > **Note:** To generate new Certification Declaration, you need the
    > `chip-cert` executable. See the note at the end of this section to learn
    > how to get it.

    k. (optional) Partition offset that is an address in device's NVM memory,
    where factory data will be stored.

    ```
    --offset <offset>
    ```

    > **Note:** To generate a HEX file with factory data, you need to provide
    > both `offset` and `size` optional arguments. As a result,
    > `factory_data.hex` and `factory_data.bin` files are created in the
    > `output` directory. The first file contains the required memory offset.
    > For this reason, it can be programmed directly to the device using a
    > programmer (for example, `nrfjprog`).

    l. (optional) The maximum partition size in device's NVM memory, where
    factory data will be stored.

    ```
    --size <size>
    ```

    > **Note:** To generate a HEX file with factory data, you need to provide
    > both `offset` and `size` optional arguments. As a result,
    > `factory_data.hex` and `factory_data.bin` files are created in the
    > `output` directory. The first file contains the required memory offset.
    > For this reason, it can be programmed directly to the device using a
    > programmer (for example, `nrfjprog`).

4. Run the script using the prepared list of arguments:

    ```
    $ python generate_nrfconnect_chip_factory_data.py <arguments>
    ```

For example, a final invocation of the Python script can look similar to the
following one:

```
$ python scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py \
--sn "11223344556677889900" \
--vendor_id 65521 \
--product_id 32774 \
--vendor_name "Nordic Semiconductor ASA" \
--product_name "not-specified" \
--date "2022-02-02" \
--hw_ver 1 \
--hw_ver_str "prerelase" \
--dac_cert "credentials/development/attestation/Matter-Development-DAC-FFF1-8006-Cert.der" \
--dac_key "credentials/development/attestation/Matter-Development-DAC-FFF1-8006-Key.der" \
--pai_cert "credentials/development/attestation/Matter-Development-PAI-FFF1-noPID-Cert.der" \
--spake2_it 1000 \
--spake2_salt "U1BBS0UyUCBLZXkgU2FsdA==" \
--discriminator 0xF00 \
--generate_rd_uid \
--passcode 20202021 \
--product_finish "matte" \
--product_color "black" \
--out "build" \
--schema "scripts/tools/nrfconnect/nrfconnect_factory_data.schema" \
--offset 0xf7000 \
--size 0x1000
```

As the result of the above example, a unique ID for the rotating device ID is
created, SPAKE2+ verifier is generated using the `spake2p` executable, and the
JSON file is verified using the prepared JSON Schema.

If the script finishes successfully, go to the location you provided with the
`-o` argument. Use the JSON file you find there when
[generating factory data](#generating-factory-data).

> **Note:** Generating new certificates is optional if default vendor and
> product IDs are used and requires providing a path to the `chip-cert`
> executable. To get it, complete the following steps:
>
> 1.  Navigate to the `connectedhomeip` root directory.
> 2.  In a terminal, run the command:
>     `cd src/tools/chip-cert && gn gen out && ninja -C out chip-cert` to build
>     the executable.
> 3.  Add the `connectedhomeip/src/tools/chip-cert/out/chip-cert` path as an
>     argument of `--chip_cert_path` for the Python script.

> **Note:** By default, overwriting the existing JSON file is disabled. This
> means that you cannot create a new JSON file with the same name in the exact
> location as an existing file. To allow overwriting, add the `--overwrite`
> option to the argument list of the Python script.

### How to set user data

The user data is an optional field provided in the factory data JSON file and
depends on the manufacturer's purpose. The `user` field in a JSON factory data
file is represented by a flat JSON map and it can consist of `string` or `int32`
data types only. On the device side, the `user` data will be available as a CBOR
map containing all defined `string` and `int32` fields.

To add user data as an argument to the
[generate_nrfconnect_chip_factory_data.py](../../scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py)
script, add the following line to the argument list:

```
--user-data {user data JSON}
```

As `user data JSON`, provide a flat JSON map with a value file that consists of
`string` or `int32` types. For example, you can use a JSON file that looks like
follows:

```
{
    "name": "product_name",
    "version": 123,
    "revision": "0x123"
}
```

When added to the argument line, the final result would look like follows:

```
--user-data '{"name": "product_name", "version": 123, "revision": "0x123"}'
```

#### How to handle user data

The user data is not handled anywhere in the Matter stack, so you must handle it
in your application. To do this, you can use the
[Factory Data Provider](../../src/platform/nrfconnect/FactoryDataProvider.h) and
apply one of the following methods:

-   `GetUserData` method to obtain raw data in the CBOR format as a
    `MutableByteSpan`.

-   `GetUserKey` method that lets you search along the user data list using a
    specific key, and if the key exists in the user data, the method returns its
    value.

If you opt for `GetUserKey`, complete the following steps to set up the search:

1. Add the `GetUserKey` method to your code.

2. Given that all integer fields of the `user` Factory Data field are `int32`,
   provide a buffer that has a size of at least `4B` or an `int32_t` variable to
   `GetUserKey`. To read a string field from user data, the buffer should have a
   size of at least the length of the expected string.

3. Set it up to read all user data fields.

Only after this setup is complete, can you use all variables in your code and
cast the result to your own purpose.

The code example of how to read all fields from the JSON example one by one can
look like follows:

```
chip::DeviceLayer::FactoryDataProvider factoryDataProvider;

factoryDataProvider.Init();

uint8_t user_name[12];
size_t name_len = sizeof(user_name);
factoryDataProvider.GetUserKey("name", user_name, name_len);

int32_t version;
size_t version_len = sizeof(version);
factoryDataProvider.GetUserKey("version", &version, version_len);

uint8_t revision[5];
size_t revision_len = sizeof(revision);
factoryDataProvider.GetUserKey("revision", revision, revision_len);
```

### Verifying using the JSON Schema tool

The JSON file that contains factory data can be verified using the
[JSON Schema file](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/nrfconnect/nrfconnect_factory_data.schema).
You can use one of three options to validate the structure and contents of the
JSON data.

#### Option 1: Using the php-json-schema tool

To check the JSON file using a JSON Schema verification tool manually on a Linux
machine, complete the following steps:

1. Install the `php-json-schema` package:

    ```
    $ sudo apt install php-json-schema
    ```

2. Run the following command, with _<path_to_JSON_file>_ and
   _<path_to_schema_file>_ replaced with the paths to the JSON file and the
   Schema file, respectively:

    ```
    $ validate-json <path_to_JSON_file> <path_to_schema_file>
    ```

The tool returns empty output in case of success.

#### Option 2: Using a website validator

You can also use external websites instead of the `php-json-schema` tool to
verify a factory data JSON file. For example, go to the
[JSON Schema Validator website](https://www.jsonschemavalidator.net/),
copy-paste the content of the
[JSON Schema file](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/nrfconnect/nrfconnect_factory_data.schema)
to the first window and a JSON file to the second one. A message under the
window indicates the validation status.

#### Option 3: Using the nRF Connect Python script

You can have the JSON file checked automatically by the Python script during the
file generation. For this to happen, you must install the `jsonschema` Python
module in your Python environment and provide the path to the JSON schema file
as an additional argument. To do this, complete the following steps:

1. Install the `jsonschema` Python module by invoking one of the following
   commands from the Matter root directory:

    - Install only the `jsonschema` module:

        ```
        $ python -m pip install jsonschema
        ```

    - Install the `jsonschema` module together with all dependencies for Matter:

        ```
        $ python -m pip install -r ./scripts/setup/requirements.nrfconnect.txt
        ```

2. Run the following command (remember to replace the _<path_to_schema>_
   variable):

    ```
    $ python generate_nrfconnect_chip_factory_data.py --schema <path_to_schema>
    ```

> **Note:** To learn more about the JSON schema, visit
> [this unofficial JSON Schema tool usage website](https://json-schema.org/understanding-json-schema/).

### Generating onboarding codes

The
[generate_nrfconnect_chip_factory_data.py](../../scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py)
script lets you generating a manual code and a QR code from the given factory
data parameters. You can use these codes to perform commissioning to the Matter
network over Bluetooth LE since they include all the pairing data required by
the Matter controller. You can place these codes on the device packaging or on
the device itself during production.

To generate a manual pairing code and a QR code, complete the following steps:

1. Install all required Python dependencies for Matter:

    ```
    $ python -m pip install -r ./scripts/setup/requirements.nrfconnect.txt
    ```

2. Complete steps 1, 2, and 3 from the
   [Creating the factory data JSON and HEX files with the first script](#creating-the-factory-data-json-and-hex-files-with-the-first-script)
   section to prepare the final invocation of the Python script.

3. Add the `--generate_onboarding` argument to the Python script final
   invocation.

4. Run the script.

5. Navigate to the output directory provided as the `-o` argument.

The output directory contains the following files you need:

-   JSON file containing the latest factory data set.
-   Test file containing the generated manual code and the text version of the
    QR Code.
-   PNG file containing the generated QR Code as an image.

#### Enabling onboarding codes generation within the build system

You can generate onboarding codes using the nRF Connect platform build system
described in
[Building an example with factory data](#building-an-example-with-factory-data),
and build an example with the following additional option:
`-DCONFIG_CHIP_FACTORY_DATA_GENERATE_ONBOARDING_CODES=y`.

For example, the build command for the nRF52840 DK could look like this:

```
$ west build -b nrf52840dk_nrf52840 -- \
-DCONFIG_CHIP_FACTORY_DATA=y \
-DSB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y \
-DCONFIG_CHIP_FACTORY_DATA_GENERATE_ONBOARDING_CODES=y
```

### Preparing factory data partition on a device

The factory data partition is an area in the device's persistent storage where a
factory data set is stored. This area is configured using the
[Partition Manager](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/scripts/partition_manager/partition_manager.html),
within which all partitions are declared in the `pm_static.yml` file.

To prepare an example that supports factory data, add a partition called
`factory_data` to the `pm_static.yml` file. The partition size should be a
multiple of one flash page (for nRF52 and nRF53 SoCs, a single page size equals
4 kB).

See the following code snippet for an example of a factory data partition in the
`pm_static.yml` file. The snippet is based on the `pm_static.yml` file from the
[Lock application example](../../examples/lock-app/nrfconnect/configuration/nrf52840dk_nrf52840/pm_static_dfu.yml)
and uses the nRF52840 DK:

```
...
mcuboot_primary_app:
    orig_span: &id002
        - app
    span: *id002
    address: 0x7200
    size: 0xf3e00

factory_data:
    address: 0xfb00
    size: 0x1000
    region: flash_primary

settings_storage:
    address: 0xfc000
    size: 0x4000
    region: flash_primary
...
```

In this example, a `factory_data` partition has been placed between the
application partition (`mcuboot_primary_app`) and the settings storage. Its size
has been set to one flash page (4 kB).

Use Partition Manager's report tool to ensure you created a factory data
partition correctly. To do that, navigate to the example directory and run the
following command:

```
$ west build -t partition_manager_report
```

The output will look similar to the following one:

```

  external_flash (0x800000 - 8192kB):
+---------------------------------------------+
| 0x0: mcuboot_secondary (0xf4000 - 976kB)    |
| 0xf4000: external_flash (0x70c000 - 7216kB) |
+---------------------------------------------+

  flash_primary (0x100000 - 1024kB):
+-------------------------------------------------+
| 0x0: mcuboot (0x7000 - 28kB)                    |
+---0x7000: mcuboot_primary (0xf4000 - 976kB)-----+
| 0x7000: mcuboot_pad (0x200 - 512B)              |
+---0x7200: mcuboot_primary_app (0xf3e00 - 975kB)-+
| 0x7200: app (0xf3e00 - 975kB)                   |
+-------------------------------------------------+
| 0xfb000: factory_data (0x1000 - 4kB)            |
| 0xfc000: settings_storage (0x4000 - 16kB)       |
+-------------------------------------------------+

  sram_primary (0x40000 - 256kB):
+--------------------------------------------+
| 0x20000000: sram_primary (0x40000 - 256kB) |
+--------------------------------------------+

```

### Creating a factory data partition with the second script

To store the factory data set in the device's persistent storage, convert the
data from the JSON file to its binary representation in the CBOR format. This is
done by the
[generate_nrfconnect_chip_factory_data.py](../../scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py),
if you provide optional `offset` and `size` arguments. If you provided these
arguments, skip the following steps of this section.

You can skip these optional arguments and do this, using the
[nrfconnect_generate_partition.py](../../scripts/tools/nrfconnect/nrfconnect_generate_partition.py)
script, but this is obsolete solution kept only for backward compatibility:

1. Navigate to the _connectedhomeip_ root directory
2. Run the following command pattern:

    ```
    $ python scripts/tools/nrfconnect/nrfconnect_generate_partition.py -i <path_to_JSON_file> -o <path_to_output> --offset <partition_address_in_memory> --size <partition_size>
    ```

    In this command:

    - _<path_to_JSON_file>_ is a path to the JSON file containing appropriate
      factory data.
    - _<path_to_output>_ is a path to an output file without any prefix. For
      example, providing `/build/output` as an argument will result in creating
      `/build/output.hex` and `/build/output.bin`.
    - _<partition_address_in_memory>_ is an address in the device's persistent
      storage area where a partition data set is to be stored.
    - _<partition_size>_ is a size of partition in the device's persistent
      storage area. New data is checked according to this value of the JSON data
      to see if it fits the size.

To see the optional arguments for the script, use the following command:

```
$ python scripts/tools/nrfconnect/nrfconnect_generate_partition.py -h
```

**Example of the command for the nRF52840 DK:**

```
$ python scripts/tools/nrfconnect/nrfconnect_generate_partition.py -i build/light_bulb/zephyr/factory_data.json -o build/light_bulb/zephyr/factory_data --offset 0xfb000 --size 0x1000
```

As a result, `factory_data.hex` and `factory_data.bin` files are created in the
`/build/light_bulb/zephyr/` directory. The first file contains the memory
offset. For this reason, it can be programmed directly to the device using a
programmer (for example, `nrfjprog`).

<hr>

## Building an example with factory data

You can manually generate the factory data set using the instructions described
in the [Generating factory data](#generating-factory-data) section. Another way
is to use the nRF Connect platform build system that creates factory data
content automatically using Kconfig options and includes the content in the
final firmware binary.

To enable generating the factory data set automatically, go to the example's
directory and build the example with the following option (replace
`nrf52840dk_nrf52840` with your board name):

```
$ west build -b nrf52840dk_nrf52840 -- -DCONFIG_CHIP_FACTORY_DATA=y -DSB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y
```

Alternatively, you can also add `SB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y`
Kconfig setting to the example's `sysbuild.conf` file.

Each factory data parameter has a default value. These are described in the
[Kconfig file](../../config/nrfconnect/chip-module/Kconfig). Setting a new value
for the factory data parameter can be done either by providing it as a build
argument list or by using interactive Kconfig interfaces.

### Providing factory data parameters as a build argument list

This way for providing factory data can be used with third-party build script,
as it uses only one command. All parameters can be edited manually by providing
them as an additional option for the west command. For example (replace
`nrf52840dk_nrf52840` with own board name):

```
$ west build -b nrf52840dk_nrf52840 -- -DCONFIG_CHIP_FACTORY_DATA=y --DSB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y --DCONFIG_CHIP_DEVICE_DISCRIMINATOR=0xF11
```

Alternatively, you can add the relevant Kconfig option lines to the example's
`prj.conf` file.

### Setting factory data parameters using interactive Kconfig interfaces

You can edit all configuration options using the interactive Kconfig interface.

See the
[Configuring nRF Connect examples](../guides/nrfconnect_examples_configuration.md)
page for information about how to configure Kconfig options.

In the configuration window, expand the items
`Modules -> connectedhomeip (/home/arbl/matter/connectedhomeip/config/nrfconnect/chip-module) -> Connected Home over IP protocol stack`.
You will see all factory data configuration options, as in the following
snippet:

```
(65521) Device vendor ID
(32774) Device product ID
[*] Enable Factory Data build
[*]     Enable merging generated factory data with the build tar
[*]     Use default certificates located in Matter repository
[ ]     Enable SPAKE2 verifier generation
[*]     Enable generation of a new Rotating device id unique id
(11223344556677889900) Serial number of device
(Nordic Semiconductor ASA) Human-readable vendor name
(not-specified) Human-readable product name
(2022-01-01) Manufacturing date in ISO 8601
(0) Integer representation of hardware version
(prerelease) user-friendly string representation of hardware ver
(0xF00) Device pairing discriminator
(20202021) SPAKE2+ passcode
(1000) SPAKE2+ iteration count
(U1BBS0UyUCBLZXkgU2FsdA==) SPAKE2+ salt in string format
(uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ
(91a9c12a7c80700a31ddcfa7fce63e44) A rotating device id unique i
```

> **Note:** To get more information about how to use the interactive Kconfig
> interfaces, read the
> [Kconfig documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/kconfig/menuconfig.html).

### Default Kconfig values and developing aspects

Each factory data parameter has its default value reflected in the Kconfig. The
list below shows some Kconfig settings that are configured in the nRF Connect
build system and have an impact on the application. You can modify them to
achieve the desired behavior of your application.

-   The device uses the test certificates located in the
    `credentials/development/attestation/` directory, which are generated using
    all default values. If you want to change the default `vendor_id`,
    `product_id`, `vendor_name`, or `device_name` and generate new test
    certificates, add the `CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED=y`
    Kconfig option. Remember to build the `chip-cert` application and add it to
    the system PATH.

    For developing a production-ready product, you need to write the
    certificates obtained during the certification process. To do this, add the
    `CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER=y` Kconfig option and set the
    appropriate paths for the following Kconfig options:

    -   `CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_CERT`
    -   `CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_KEY`
    -   `CONFIG_CHIP_FACTORY_DATA_USER_CERTS_PAI_CERT`

-   By default, the SPAKE2+ verifier is generated during each example's build.
    This means that this value will change automatically if you change any of
    the following parameters:

    -   `CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE`
    -   `CONFIG_CHIP_DEVICE_SPAKE2_SALT`
    -   `CONFIG_CHIP_DEVICE_SPAKE2_IT`

    You can disable the generation of the SPAKE2+ verifier by setting the
    `CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER=n` Kconfig option. Then,
    you will need to provide the externally-generated SPAKE2+ verifier using the
    `CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER` Kconfig value.

-   Generating the rotating device ID unique ID is disabled by default, but you
    can enable it by setting the `CONFIG_CHIP_ROTATING_DEVICE_ID=y` and
    `CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID=y` Kconfig values.
    Moreover, if you set the `CONFIG_CHIP_ROTATING_DEVICE_ID` Kconfig option to
    `y` and disable the `CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID`
    Kconfig option, you will need to provide it manually using the
    `CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID` Kconfig value.

-   You can generate the test Certification Declaration by using the
    `CONFIG_CHIP_FACTORY_DATA_GENERATE_CD=y` Kconfig option. Remember to build
    the `chip-cert` application and add it to the system PATH.

<hr>

## Programming factory data

The HEX file containing factory data can be programmed into the device's flash
memory using `nrfjprog` and the J-Link programmer. To do this, use the following
command:

```
$ nrfjprog --program factory_data.hex
```

In this command, you can add the `--family` argument and provide the name of the
DK: `NRF52` for the nRF52840 DK or `NRF53` for the nRF5340 DK. For example:

```
$ nrfjprog --family NRF52 --program factory_data.hex
```

> Note: For more information about how to use the `nrfjprog` utility, visit
> [Programming SoCs with nrfjprog](https://docs.nordicsemi.com/bundle/ug_nrf_cltools/page/UG/cltools/nrf_nrfjprogexe.html)

Another way to program the factory data to a device is to use the nRF Connect
platform build system described in
[Building an example with factory data](#building-an-example-with-factory-data),
and build an example with the additional option
`-DSB_CONFIG_MATTER_FACTORY_DATA_MERGE_WITH_FIRMWARE=y`:

```
$ west build -b nrf52840dk_nrf52840 -- \
-DCONFIG_CHIP_FACTORY_DATA=y \
-DSB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y \
-DSB_CONFIG_MATTER_FACTORY_DATA_MERGE_WITH_FIRMWARE=y
```

You can also build an example with auto-generation of new CD, DAC and PAI
certificates. The newly generated certificates will be added to factory data set
automatically. To generate new certificates disable using default certificates
by building an example with the additional option
`-DCONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS=n`:

```
$ west build -b nrf52840dk_nrf52840 -- \
-DCONFIG_CHIP_FACTORY_DATA=y \
-DSB_CONFIG_MATTER_FACTORY_DATA_GENERATE=y \
-DSB_CONFIG_MATTER_FACTORY_DATA_MERGE_WITH_FIRMWARE=y \
-DCONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS=n
```

> Note: To generate new certificates using the nRF Connect platform build
> system, you need the `chip-cert` executable in your system variable PATH. To
> learn how to get `chip-cert`, go to the note at the end of
> [creating the factory data partition with the second script](#creating-a-factory-data-partition-with-the-second-script)
> section, and then add the newly built executable to the system variable PATH.
> The Cmake build system will find this executable automatically.

After that, use the following command from the example's directory to write
firmware and newly generated factory data at the same time:

```
$ west flash
```

<hr>

## Using own factory data implementation

The [factory data generation process](#generating-factory-data) described above
is only an example valid for the nRF Connect platform. You can well create a HEX
file containing all [factory data components](#factory-data-component-table) in
any format and then implement a parser to read out all parameters and pass them
to a provider. Each manufacturer can implement a factory data set on its own by
implementing a parser and a factory data accessor inside the Matter stack. Use
the [nRF Connect Provider](../../src/platform/nrfconnect/FactoryDataProvider.h)
and [FactoryDataParser](../../src/platform/nrfconnect/FactoryDataParser.h) as
examples.

You can read the factory data set from the device's flash memory in different
ways, depending on the purpose and the format. In the nRF Connect example, the
factory data is stored in the CBOR format. The device uses the
[Factory Data Parser](../../src/platform/nrfconnect/FactoryDataParser.h) to read
out raw data, decode it, and store it in the `FactoryData` structure. The
[Factor Data Provider](../../src/platform/nrfconnect/FactoryDataProvider.cpp)
implementation uses this parser to get all needed factory data parameters and
provide them to the Matter core.

In the nRF Connect example, the `FactoryDataProvider` is a template class that
inherits from `DeviceAttestationCredentialsProvider`,
`CommissionableDataProvider`, and `DeviceInstanceInfoProvider` classes. Your
custom implementation must also inherit from these classes and implement their
functions to get all factory data parameters from the device's flash memory.
These classes are virtual and need to be overridden by the derived class. To
override the inherited classes, complete the following steps:

1. Override the following methods:

    ```
    // ===== Members functions that implement the DeviceAttestationCredentialsProvider
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override;

    // ===== Members functions that implement the DeviceInstanceInfoProvider
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
    ```

2. Move the newly created parser and provider files to your project directory.
3. Add the files to the `CMakeList.txt` file.
4. Disable building both the default and the nRF Connect implementations of
   factory data providers to start using your own implementation of factory data
   parser and provider. This can be done in one of the following ways:

    - Add `CONFIG_FACTORY_DATA_CUSTOM_BACKEND=y` Kconfig setting to `prj.conf`
      file.
    - Build an example with following option (replace _<build_target>_ with your
      board name, for example `nrf52840dk_nrf52840`):

        ```
        $ west build -b <build_target> -- -DCONFIG_FACTORY_DATA_CUSTOM_BACKEND=y
        ```
