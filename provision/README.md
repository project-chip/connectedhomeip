# Matter Provisioning


The provisioning script is used to load mandatory information into Matter devices, including commissioning and authentication
data. To facilitate the transition between development and production, this tool uses the same inputs as Silicon Labs's
[Custom Part Manufacturing Service](https://www.silabs.com/services/custom-part-manufacturing-service). Most of the information
is stored once during the manufacturing process, and do not change during the lifetime of the device.

Matter defines three interfaces to access the provisioned data during runtime:
* [DeviceInstanceInfoProvider](../src/include/platform/DeviceInstanceInfoProvider.h)
* [CommissionableDataProvider](../src/include/platform/CommissionableDataProvider.h)
* [DeviceAttestationCredentialsProvider](../src/credentials/DeviceAttestationCredentialsProvider.h)
In Silicon Labs devices, all three interfaces are implemented by the [ProvisionStorage](../examples/platform/silabs/./ProvisionStorage.h).

The provisioning script on this folder now supercedes the following tools:
* [Credentials Example](https://github.com/SiliconLabs/matter/tree/release_1.1.0-1.1/silabs_examples/credentials)
* [Factory Data Provider](../scripts/tools/silabs/README.md)

## Provisioned Data

The _Commissionable Data_ includes _Serial Number_, _Vendor Id_, _Product Id_, and the _Setup Payload_ (typicallty displayed in the QR code),
while the _Attestation Credentials_ include the _Certificate Declaration_ (CD), the _Product Attestation Intermediate certificate_ (PAI),
and the _Device Attestation Certificate_ (DAC).

During commissioning, Matter devices perform a Password Authenticated Key Exchange using the SPAKE2+ protocol.
Calculating the SPAKE2+ verifier is computationally costly, for large iteration counts it may take several minutes to
compute on the target device. For this reason, the SPAKE2+ verifier is calculated on the host side by the script itself.

The `passcode` is used to derive a QR code, typically printed on the label, or displayed by the device itself.
The QR code contains the pre-computed setup payload, which allows the commissioner to establish a session with the device.
The parameters required to generate and validate the session keys are static and stored in NVM3.

To protect the attestation private-key (used to generate the DAC), the asymmetric key-pair may be generated on-device, using PSA,
and the most secure storage location available to the specific part. However, the private-key may be generated externally,
and imported using the `--dac_key` option.

The DAC is generated and signed by a _Certification Authority_ (CA), which may operate from a separate host. The `modules/signing_server.py`
script simulates the role of the CA, and uses OpenSSL to to generate and sign the DAC. In a real factory environment,
this script is replaced by an actual CA.

## Generator Firmware

The _Generator Firmware_ (GFW) is a FreeRTOS application that runs on the targeted device, and assists with the provisioning of the device.
The GFW performs the following tasks:
* When key-generation is used:
  - Generates the key-pair on device in the most secure location available.
  - Generates and returns a _Certificate Signing Request_ (CSR). The CSR contains the device public-key, Vendor Id, Product Id, and Serial Number.
* When key-import is used:
  - Imports the key into the most secure location available.
* Calculates the _Setup Payload_.
* Stores the Commissionable Data into NVM3 (including the _Setup Payload_).
* Stores the Attestation Data on the main flash (CD, PAI, DAC)
* Stores the size and offsets used to store the Attestation Data, along with the KeyId used to store the private-key.

The main source code of the GFW is located under `./generator`, while the board support is located under `./support`.
Pre-compiled images for the supported chips can be found in `./images`.
Backwards-compatibility script files are stored under `./modules/vX_Y` where X.Y matches the targeted version.

The directory structure is as follows:
- provision
    - generator
    - images
    - modules
        - v1_0
        - v2_0
    - support
        - efr32mg12
        - efr32mg24


## Provision Script

The `provision.py` file is the main script used to load all the required data on the Matter device. This script requires:
* [Simplicity Commander](https://community.silabs.com/s/article/simplicity-commander?language=en_US)
* [SEGGER J-Link](https://www.segger.com/downloads/jlink/)
* [PyLink](https://pylink.readthedocs.io/en/latest/index.html)
* [ECDSA](https://pypi.org/project/ecdsa/)
* [Bleak](https://pypi.org/project/bleak/)

The Provisioner Script executes the following steps:
1. Gathers the parameter definitions from the internal `./modules/parameters.yaml` file, local `parameters.yaml`, and the file indicated by the `--params` option.
2. Parses the inputs from the local `default.json` file, the file indicated by the `--inputs` option, and command-line arguments.
3. Generates test certificates (if the `--generate` option is used). This step requires an external [`chip-cert`](../src/tools/chip-cert/README.md) tool binary.
4. If a PKCS#12 file is provided, extracts the PAI, DAC, and DAC key files in DER format.
4. Generates default values for the SPAKE2+ arguments, if necessary.
5. Saves the input parameters as a JSON file (`latest.json` in the local folder, or the file indicated by `--output`).
5. Flashes the Generator Firmware (GFW) onto the device, if required.
6. Sends the provisioned data to the targed device using the selected channel and protocol.
7. Flashes the Production Firmware (PFW), if provided the inputs.

## Provision Protocol

The Provision Tool exchange information with the Provision firmware (either GFW or the PFW itself) using a proprietary protocol.
The original protocol, known as version 1.x, serves the basic purposes of CPMS and other factory environments, but is limited in scope.
It can only transmit a pre-defined structures, and it doesn't support fragmentation. To overcome these limitations, a new protocol (version 2.x) has been developed. For backwards compatibility, users familiar to 1.x should not notice the difference between the protocols. The difference
only becomes evident when new functionality is required, which is only supported in version 2.x.

### Version 1.x
Verion 1 defines the following commands:
* Init: Used for initialization. Sends the flash size and location to the firmware.
* CSR: Used for on-device key and CSR generation.
* Import: Used to transmit files to the device (DAC, DAC key, PAI, and CD). Each file is sent whole in a single exchange.
* Setup: Used to transmit all other factory device to the device. The transmitted data is sent in a single package,
in precise order, with fixed data types and sizes.
Users of version 1.0 do not execute any of these commands manually. The script simply execute these commands sequentially as needed.
There are no extension capabilities on this protocol, nor any provision to read-back the provisioned data.

### Version 2.x
Version 2 of the Provision Protocol enforces a maximum package size. Data larger than the limit is fragmented in
as many packages are needed. This is done in both directions. Commands defined in this protocol are:
* Init: Used for initialization. Sends the flash size and location to the firmware.
* Finish: Signal that provisioning is complete.
* CSR: Used for on-device key and CSR generation.
* Write: Send any number of arguments to the target device.
* Read: Returns any number of arguments from the target device.
With _Write_ and _Read_, any number of arguments, of any length, may be sent in any order.
However, by default, the script uses the _Write_ and _Read_ commands to send the same data as version 1,
thus preserving the user experience of the tool.
Instead of fixed-positions, this protocol uses IDs to identify the arguments in both ends.

## Channels

The Provision Tool can transfer the arguments to the device in two ways:
* J-Link RTT: When the device is physically connected to the host, the GFW can communicate through the serial port using J-Link RTT.
This method can be used both in development and factory environments. This method works with the legacy Protocol version 1.x or
the new protocol version 2.x.
* Bluetooth: The provision script can transmit the data directly to applications running in provision-mode. While in this mode,
Silicon Labs' example applications use the bluetooth communication to receive provisionning data. The Bluetooh channel requires
Provision Protocol v2.x.

### Parameters

Parameter files define the name, type, and restrictions of the arguments recognized by the target device. The `modules/parameters.yaml`
file defines the well-known (default) parameters used by the automatic provisioning action.

### Well-known Parameters

| Parameters                | Conformance          | Type               | Description                                                                             |
| ------------------------- | -------------------- | ------------------ | --------------------------------------------------------------------------------------- |
| -v,  --version            | optional             | string             | Provisioning version.   |
| -p,  --params             | optional             | path               | YAML parameters file.   |
| -i,  --inputs             | optional             | path               | JSON inputs file. Formerly --config.       |
| -o,  --output             | optional             | path               | JSON output file.       |
| -t,  --temp               | optional             | path               | Temporary folder.       |
| -d,  --device             | optional             | string             | Target Device.          |
| -c,  --channel            | optional<sup>1</sup> | dec/string         | Connection string.      |
| -s,  --stop               | optional             | flag               | Stop mode: When true, only generate the JSON configuration, and exit.                    |
| -g,  --generate           | optional             | flag               | Auto-generate test certificates            |
| -r,  --csr                | optional             | flag               | CSR mode: When true, instructs the GFW to generate the private key, and issue a CSR.                    |
| -gf, --gen_fw             | optional             | dec/hex            | Path to the Generator Firmware image.                                                   |
| -pf, --prod_fw            | optional             | dec/hex            | Path to the Production Firmware image.                                                   |
| -ct, --cert_tool          | optional             | string             | Path to the chip-cert tool. Defaults to `../out/tools/chip-cert`          |
| -jl, --pylink_lib         | optional             | string             | Path to the PyLink library.  |
| -sn,  --serial_number     | optional             | string             | Serial Number.          |
| -vi,  --vendor_id         | optional             | dec/hex            | Vendor ID. e.g: 65521 or 0xFFF1 (Max 2 bytes).                              |
| -vn,  --vendor_name       | optional             | string             | Vendor name (Max 32 char).                                                  |
| -pi,  --product_id        | optional             | dec/hex            | Product ID. e.g: 32773 or 0x8005 (Max 2 bytes).                             |
| -pn,  --product_name      | optional             | string             | Product name (Max 32 char).                                                 |
| -pl, --product_label      | optional             | string             | Product label.                |
| -pu, --product_url        | optional             | string             | Product URL.                |
| -pm, --part_number        | optional             | dec/hex            | Device Part Number (Max 32 char).                                               |
| -hv, --hw_version         | optional             | dec/hex            | The hardware version value (Max 2 bytes).                                       |
| -hs, --hw_version_str     | optional             | string             | The hardware version string (Max 64 char).                                      |
| -md, --manufacturing_date | optional             | string             | Manufacturing date.                |
| -ui, --unique_id         | optional<sup>5</sup> | hex string         | A 128 bits hex string unique id (without 0x).                                           |
| -sd,  --discriminator     | optional<sup>2</sup> | dec/hex            | BLE pairing discriminator. e.g: 3840 or 0xF00. (12-bit)                                 |
| -sp, --spake2p_passcode   | required             | dec/hex            | Session passcode used to generate the SPAKE2+ verifier.        |
| -si, --spake2p_iterations | required             | dec/hex            | Iteration count used to generate the SPAKE2+ verifier.                  |
| -ss, --spake2p_salt       | required             | string<sup>6</sup> | Salt used to generate the SPAKE2+ verifier.                             |
| -sv, --spake2p_verifier   | optional             | string<sup>6</sup> | Pre-generated SPAKE2+ verifier.                                          |
| -sy, --setup_payload      | optional             | string<sup>6</sup> | Setup Payload.                             |
| -sf, --commissioning_flow | optional             | dec/hex            | Commissioning Flow 0=Standard, 1=User Action, 2=Custom.                         |
| -sr, --rendezvous_flags   | optional             | dec/hex            | Rendez-vous flag: 1=SoftAP, 2=BLE 4=OnNetwork (Can be combined).                |
| -fi, --firmware_info      | optional             | string             | Firmware Information            |
| -cd, --certification      | required             | string             | Path to the Certification Declaration (CD) file.                |
| -cc, --cd_cert            | optional             | string             | Certification Declaration Signing Cert            |
| -ck, --cd_key             | optional             | string             | Certification Declaration Signing Key            |
| -ac, --paa_cert           | required             | string             | Path to the PAA certificate.                |
| -ak, --paa_key            | optional<sup>3</sup> | string             | Path to the PAA private-key.                |
| -ic, --pai_cert           | required             | string             | Path to the PAI certificate.                |
| -ik, --pai_key            | required             | string             | Path to the PAI private-key.                |
| -dc, --dac_cert           | optional<sup>3</sup> | string             | Path to the DAC certificate.                |
| -dk, --dac_key            | optional<sup>3</sup> | dec/hex            | Path to the DAC private-key.                |
| -di, --key_id             | required             | dec/hex            | Attestation Key ID.                |
| -dp, --key_pass           | optional<sup>3</sup> | string             | Password for the key file.                |
| -dx, --pkcs12             | optional<sup>3</sup> | string             | Path to the PKCS#12 attestation certificates file. Formerly --att_certs.        |
| -dn, --common_name        | optional<sup>4</sup> | string             | Common Name to use in the Device Certificate (DAC) .                |
| -ok, --ota_key            | optional             | string             | Over The Air (OTA) update key.                |

<sup>1</sup> Use xxxxxxxxx for serial, xxx.xxx.xxx.xxx[:yyyy] for TCP, or bt:XXXXXXXXXXXXXXXX for bluetooth
<sup>2</sup> If not provided (or zero), the `discriminator` is generated at random
<sup>3</sup> If the DAC is provided, its corresponding private-key must also be provided
<sup>4</sup> Required if the DAC is not provided
<sup>5</sup> If not provided, the `unique_id` is randomly generated
<sup>6</sup> Salt and verifier must be provided as base64 string

WARNING:
    With the release of version 2.0, many shortcuts have been modified. Single-characters are now reserved for tool options.
    Most long versions have been preserved, with the exception of `--config` and `--att_certs`.

### Custom Parameters

Custom parameters may be defined in a YAML file, with the following format as an example:
```
custom:
  - id: 1
    name: "example1"
    short: "x1"
    type: "int8u"
    min: 1
    max: 100
    desc: "Example Parameter 1"

  - id: 2
    name: "example2"
    short: "x2"
    type: "string"
    max: 8
    desc: "Example Parameter 2"
```
Where `custom` is an arbitrary section name. There is no limit to the number of sections, but only 256 parameters may be defined.
Names and shortcuts must be selected to not conflict with existing well-defined parameters.
Custom IDs are ranged 0x00 to 0xff (255). To avoid conflict, well-defined (default) parameters IDs are ranged 0x100 (256) to 0x1ff (511).
If a parameters file named `parameters.yaml` exists in the local directory, the parameters from that file are loaded automatically.
Otherwise, the path to the parameters file may be provided with the `--params` option.

Supported types are int8u, int16u, int32u, string, binary, and path. The `path` parameters must point to an existing file in the filesystem.
If such file exists, its contents are read and sent to the firmware as a binary value.

Given the previous configuration, the actual arguments may be provided as command-line:
```
python3 provision.py -x1 99 --example2 "ABC123"
```
Or, as part of an input file:
```
{
    "version": "2.0",
    "options": {
        "generate": true,
    },
    "matter": {
        "instance_info": {
            "vendor_id": 4169,
            "product_id": 32773
        }
    },
    "custom": {
        "example1": 99,
        "example2": "ABC123"
    }
}
```

On the firmware side, custom parameters are sent to the [Custom Provision Storage](examples/platform/silabs/provision/ProvisionStorageCustom.cpp).
A sample implementation is provided, but it must be replaced with a class that matches with parameters defined in the YAML descriptor.


## Actions

When using version 1.x, there is only action available, which performs device provisioning as automatically as possible.
In contrast, version 2.x defines the following actions:
* `auto`: This is the default action, and emulates the automatic provisioning performed in version 1.
   This action sends: `version`, `serial_number`, `vendor_id`, `vendor_name`, `product_id`, `product_name`, `product_label`, `product_url`, `part_number`, `hw_version`, `hw_version_str`, `manufacturing_date`, `unique_id`, `discriminator`, `spake2p_passcode`, `spake2p_iterations`, `spake2p_salt`, `spake2p_verifier`, `setup_payload`, `commissioning_flow`, `rendezvous_flags`, `firmware_info`, `dac_cert`, `pai_cert`, `certification`, `dac_key`
    And reads-back: `unique_id`, `discriminator`, `spake2p_passcode`, `setup_payload`
* `write`: Only arguments defined either in input files or command-line are sent, for instance:
```
python3 provision.py write --vendor_name "Silicon Labs" --product_name "Silabs Example"
```
* `read`: Only arguments defined in a comma-separated list are read from the device. Arguments of type `path` are read in binary form, and
stored in the given path, for instance:
```
python3 provision.py read "vendor_name, product_name, version, dac_cert" --dac_cert cert.der
```

## Use

### Zero Command-line Arguments

The simplest use of the Provision Tool is using all-defaults, for instance:
```
cd ./provision/
python3 ./provision.py
```
This command works because the `./provision` folder contains a `defaults.json` file including `vendor_id`, `product_id`, and the `--generate` option.
Any or all of these arguments may be overwritten though the command line.

### Argument Files

The -i/--inputs argument reads all the required arguments from a provided JSON file. The same validation rules apply
both for command line or configuration file, but JSON does not support hexadecimal numbers. Command line arguments
take precedence over file arguments. For instance, with the configuration `example.json`:
```
{
    "version": "2.0",
    "options": {
        "generate": true
    },
    "matter": {
        "instance_info": {
            "vendor_id": 65521,
            "product_id": 32773
        },
        "commissionable_data": {
            "discriminator": 3840,
            "passcode": 20202021,
            "iterations": 15000,
            "salt": "U1BBS0UyUCBLZXkgU2FsdA==",
            "commissioning_flow": 0,
            "rendezvous_flags": 2
        }
    }
}
```
You may run:
```
python3 ./provision.py --inputs example.json
```
Which will set the connected device with discriminator **3840**, product ID **32773**, and use **15000** SPAKE2+ iterations. However, if you run instead:
```
python3 ./provision.py --inputs example.json --discriminator 2748 --product_id 0x8006 --spake2p_iterations 10000
```
The connected device will be set with discriminator **2748** (instead of 3840), product ID **32774** (instead of 32773),
and use **10000** SPAKE2+ iterations (instead of 15000).

For each run, `provision.py` will generate a local file `latest.json`, containing the arguments compiled from the different sources.
Example input files may be found under `./inputs/`:
```
python3 ./provision.py --inputs inputs/develop.json
```
NOTE:
    In earlier versions, the JSON files where found under `./config`. Version 2.x uses two kinds of files: parameters (YAML) and inputs (JSON).

### Default Arguments

To ease development and testing, either the Provision Tool or the firmware provide defaults for all arguments. The only
arguments that are truly mandatory are `vendor_id`, and `product_id`, and these are included in `defaults.json`.
Test certificates may be auto-generated using the `--generate` flag (provided the `chip-cert` can be found, either
in the default location, or through the `--cert-tool` argument).
For instance, you may run:
```
python3 ./provision.py --vendor_id 0x1049 --product_id 0x8005 --generate
```
Which will generate the test certificates using `chip-cert`, and provide the device with the following parameters:
```
{
    "version": "2.0",
    "matter": {
        "instance_info": {
            "vendor_id": 4169,
            "product_id": 32773
        },
        "commissionable_data": {
            "discriminator": 3840,
            "passcode": 20202021,
            "iterations": 1500
        }
    }
}
```

## Attestation Files

The `--generate` option instructs the `provider.py` script to generate test attestation files with the given _Vendor ID_, and _Product ID_.
These files are generated using [the chip-cert tool](../src/tools/chip-cert/README.md),
and stored under the `./temp` folder (or the folder selected with `--temp` option).

To generate the certificates manually (check chip-cert help for details):
```
chip-cert gen-cd -f 1 -V 0xfff1 -p 0x8005 -d 0x0016 -c ZIG20142ZB330003-24 -l 0 -i 0 -n 257 -t 0 -o 0xfff1 -r 0x8005 -C credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem -K credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem -O ./temp/cd.der

chip-cert gen-att-cert -t a -l 3660 -c "Matter PAA" -V 0xfff1 -o ./temp/paa_cert.pem -O ./temp/paa_key.pem

chip-cert gen-att-cert -t i -l 3660 -c "Matter PAI" -V 0xfff1 -P 0x8005 -C ./temp/paa_cert.pem -K ./temp/paa_key.pem -o ./temp/pai_cert.pem -O ./temp/pai_key.pem

chip-cert gen-att-cert -t d -l 3660 -c "Matter DAC" -V 0xfff1 -P 0x8005 -C ./temp/pai_cert.pem -K ./temp/pai_key.pem -o ./temp/dac_cert.pem -O ./temp/dac_key.pem
```

By default, `provision.py` uses the Matter Test PAA [Chip-Test-PAA-NoVID-Cert.der](../credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.der) and
its key [Chip-Test-PAA-NoVID-Key.der](../credentials/test/attestation/Chip-Test-PAA-NoVID-Key.der), which are recognized by
[chip-tool](../examples/chip-tool). So when using `chip-tool`, no `--paa-trust-store-path` argument is required.

### Example

From the root of the Silicon Labs Matter repo, build an sample application. For instance:
```
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/lighting-app/ BRD4187C
```

Set up the device with key generation:
```
python3 ./provision.py --vendor_id 0x1049 --product_id 0x8005 \
    --csr --common_name  "Silabs Device" --certification ./samples/light/1/cd.bin --pai_cert ./samples/light/1/pai_cert.der \
    --dac_cert ./samples/light/1/dac_cert.der -dk ./samples/light/1/dac_key.der \
    --spake2p_passcode 62034001 --spake2p_salt 95834coRGvFhCB69IdmJyr5qYIzFgSirw6Ja7g5ySYA= --spake2p_iterations 15000 \
    --discriminator 0xf01 --prod_fw ../out/lighting-app/BRD4187C/matter-silabs-lighting-example.s37
```

Or, set up the device with imported key:
```
python3 ./provision.py --vendor_id 0x1049 --product_id 0x8005 \
    --certification ./samples/light/1/cd.bin --pai_cert ./samples/light/1/pai_cert.der --dac_cert ./samples/light/1/dac_cert.der -dk ./samples/light/1/dac_key.der \
    --spake2p_passcode 62034001 --spake2p_salt 95834coRGvFhCB69IdmJyr5qYIzFgSirw6Ja7g5ySYA= --spake2p_iterations 15000 \
    --discriminator 0xf01 --prod_fw ../out/lighting-app/BRD4187C/matter-silabs-lighting-example.s37
```

## Self-Provisioning

Silicon Labs' Matter examples include the same provisioning engine used by the GFW. This allows applications to be flashed once
but provisioned multiple times. There are two ways to put the application in provisioning mode:
* Factory-reset by pressing both BTN0 and BTN1 for six seconds
* Write 1 to the NVM3 key 0x87228. This is useful in boards with less than two buttons, and can be accomplished using Simplicity Commander:
```
commander nvm3 read -o ./temp/nvm3.s37
commander nvm3 set ./temp/nvm3.s37 --object 0x87228:01 --outfile ./temp/nvm3+.s37
commander flash ./temp/nvm3+.s37
```
Once in provisioning mode, the example firmware can respond to Provision Protocol 2.0 commands using bluetooth. To use the
bluetooth channel, use the `--channel` option with the string `bt:` followed by the bluetooth address of the device, for instance:
```
python3 provision.py --inputs inputs/develop.json --channel bt:84:FD:27:EC:5D:FA
```
The bluetooth channel closes upon the reception of the Finish command, or reset.
To obtain the address of the device, the `bluet.py` tool may be used with the scan feature:
```
python3 bluet.py scan
    Scanning....
    00:0D:6F:5C:FB:E5  -40  "SiLabs-Light"
    62:3A:46:09:CA:15  -48  "62-3A-46-09-CA-15"
```

## Validation

If the certificate injection is successful, the commissioning process should
complete normally. In order to verify that the new certificates are actually
being used, first check the last page of the flash using Commander. The content
of the flash must then be compared with the credentials received by the
commissioner, which can be done using a debugger.

### Flash Dump

On EFR32MG12, the last page starts at address 0x000FF800. On EFR32MG24,
the last page is located at 0x0817E000. These addresses can be found in
the memory map of the board's datasheet. For instance, for a MG24 board:

```shell
commander readmem --range 0x0817E000:+1536 --serialno 440266330
```

The output should look something like:
```
commander readmem --range 0x0817E000:+1536 --serialno 440266330
Reading 1536 bytes from 0x0817e000...
{address:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F}
000ff800: 30 82 01 D8 30 82 01 7F A0 03 02 01 02 02 04 07
000ff810: 5B CD 15 30 0A 06 08 2A 86 48 CE 3D 04 03 02 30
...
000ff9c0: 2B BA 15 32 2F 4C 69 F2 38 48 D2 BC 62 2A 47 FB
000ff9d0: 3F F7 28 8A 7C 90 75 72 58 84 96 E7 00 00 00 00
000ff9e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ff9f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ffa00: 30 82 01 C8 30 82 01 6E A0 03 02 01 02 02 08 79
000ffa10: 6E 32 5A FA 5B D1 F8 30 0A 06 08 2A 86 48 CE 3D
...
000ffbb0: FD 92 D1 EB 59 95 D8 38 DE 5D 80 E3 05 65 24 4A
000ffbc0: 62 FD 9F E9 D8 00 FA CD 0F 32 7C C9 00 00 00 00
000ffbd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ffbe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ffbf0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ffc00: 30 81 EF 06 09 2A 86 48 86 F7 0D 01 07 02 A0 81
000ffc10: E1 30 81 DE 02 01 03 31 0D 30 0B 06 09 60 86 48
...
000ffce0: 28 41 FD B8 28 CD 19 F2 BB DB A0 0F 33 B2 21 D3
000ffcf0: 33 CE 00 00 00 00 00 00 00 00 00 00 00 00 00 00
000ffd00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00


On this example, the DAC is located at address 0817e000 (offset 0), and
has 476 octets:
0817e000: 30 82 01 D9 30 82 01 7F A0 03 02 01 02 02 04 07
0817e010: 5B CD 15 30 0A 06 08 2A 86 48 CE 3D 04 03 02 30
...
0817e1c0: 2E 4F 10 20 38 BA A6 B5 F6 A4 77 7A 19 91 23 79
0817e1d0: 2F A0 FF AF F5 5C A1 59 98 08 C7 BC 5F 00 00 00
```
This should match the contents of the DER-formatted DAC certificate, which is
stored by the setup script as ./temp/dac.der :

```shell
$ xxd ./temp/dac.der

00000000: 3082 01d8 3082 017f a003 0201 0202 0407  0...0...........
00000010: 5bcd 1530 0a06 082a 8648 ce3d 0403 0230  [..0...*.H.=...0
...
000001c0: 2bba 1532 2f4c 69f2 3848 d2bc 622a 47fb  +..2/Li.8H..b*G.
000001d0: 3ff7 288a 7c90 7572 5884 96e7            ?.(.|.urX...
```

The PAI certificate is located at address 0x0817e200 (offset 512), and
has 460 octets:
```
0817e200: 30 82 01 C8 30 82 01 6E A0 03 02 01 02 02 08 79
0817e210: 6E 32 5A FA 5B D1 F8 30 0A 06 08 2A 86 48 CE 3D
...
0817e3b0: FD 92 D1 EB 59 95 D8 38 DE 5D 80 E3 05 65 24 4A
0817e3c0: 62 FD 9F E9 D8 00 FA CD 0F 32 7C C9 00 00 00 00
```

This should match the contents of the DER-formatted PAI certificate, which is
stored by the setup script as ./temp/pai_cert.der :

```shell
$ xxd ./temp/pai_cert.der

00000000: 3082 01c8 3082 016e a003 0201 0202 0879  0...0..n.......y
00000010: 6e32 5afa 5bd1 f830 0a06 082a 8648 ce3d  n2Z.[..0...*.H.=
...
000001b0: fd92 d1eb 5995 d838 de5d 80e3 0565 244a  ....Y..8.]...e$J
000001c0: 62fd 9fe9 d800 facd 0f32 7cc9            b........2|.
```

Finally, on this example the CD is located at address 0817e400
(offset 1024), and contains 541 octets:
```
0817e400: 30 81 EF 06 09 2A 86 48 86 F7 0D 01 07 02 A0 81
0817e410: E1 30 81 DE 02 01 03 31 0D 30 0B 06 09 60 86 48
...
0817e4d0: 02 20 38 B9 9C 73 B2 30 92 D7 A2 92 47 30 14 F7
0817e4e0: 28 41 FD B8 28 CD 19 F2 BB DB A0 0F 33 B2 21 D3
0817e4f0: 33 CE 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

The CD is a binary file, and is neither modified, nor validated by the setup
script. It is simply stored in flash after the DAC:

```shell
$ xxd cd.der

00000000: 3081 ef06 092a 8648 86f7 0d01 0702 a081  0....*.H........
00000010: e130 81de 0201 0331 0d30 0b06 0960 8648  .0.....1.0...`.H
...
000000e0: 2841 fdb8 28cd 19f2 bbdb a00f 33b2 21d3  (A..(.......3.!.
000000f0: 33ce                                     3.
```

The 0xff octets between the files and at the end of the flash are unmodified
sections of the flash storage.

### Device Terminal

Logs have beed added to the SilabsDeviceAttestationCreds, to help verify if the attestation
files are loaded correctly. The size and first eight bytes of CD, PAI, and DAC are printed and
must match the contents of `cd.der`, `pai_cert.der`, and `dac.der`, respectively:
```
...
[00:00:05.109][info  ][ZCL] OpCreds: Certificate Chain request received for PAI
[00:00:05.109][info  ][DL] GetProductAttestationIntermediateCert, addr:0xffa00, size:460
[00:00:05.110][detail][ZCL] 0x30, 0x82, 0x01, 0xc8, 0x30, 0x82, 0x01, 0x6e,
...
[00:00:05.401][info  ][ZCL] OpCreds: Certificate Chain request received for DAC
[00:00:05.402][info  ][DL] GetDeviceAttestationCert, addr:0xff800, size:477
[00:00:05.402][detail][ZCL] 0x30, 0x82, 0x01, 0xd8, 0x30, 0x82, 0x01, 0x7f,
...

[00:00:05.694][info  ][ZCL] OpCreds: Received an AttestationRequest command
[00:00:05.695][info  ][DL] GetCertificationDeclaration, addr:0xffc00, size:242
[00:00:05.695][detail][ZCL] 0x30, 0x81, 0xef, 0x06, 0x09, 0x2a, 0x86, 0x48,
...
```

## Board Support

Pre-compiled images of the Generator Firmware can be found under ./images. The source
code of these images is found under ./support. A single image is provided for all EFR32MG12
parts, and another one for the EFR32MG24 family. To cope with the different flash sizes, the
`provision.py` script reads the device information using `commander`, and send it to the GFW,
which configures the NVM3 during the initialization step.
