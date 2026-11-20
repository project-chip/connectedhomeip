# Manufacturing Partition Generator Utility

This tool is designed to generate factory partitions for mass production.

## Dependencies

Please make sure you have had the following tools before using the generator
tool.

-   [CHIP Certificate Tool](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/chip-cert)

-   [SPAKE2P Parameters Tool](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/spake2p)

-   [chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### [Build Matter tools](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#build-for-the-host-os-linux-or-macos)

1.  Using the following commands to generate chip-tool, spake2p and chip-cert at
    `path/to/connectedhomeip/build/out/host`.

    ```shell
    cd path/to/connectedhomeip
    source scripts/activate.sh
    gn gen build/out/host
    ninja -C build/out/host
    ```

2.  Add the tools path to \$PATH

    ```shell
    export PATH="$PATH:path/to/connectedhomeip/build/out/host"
    ```

### Install python dependencies

```shell
cd path/to/connectedhomeip/scripts/tools/telink/
python3 -m pip install -r requirements.build.txt
python3 -m pip install -r requirements.telink.txt
```

## Usage

The following commands generate factory partitions using the default testing PAI
keys, certificates, and CD in Matter project. You can make it using yours
instead in real production.

### Generate a factory partition

```shell
python3 mfg_tool.py -v 0xFFF2 -p 0x8001 \
--serial-num AABBCCDDEEFF11223344556677889900 \
--vendor-name "Telink Semiconductor" \
--product-name "not-specified" \
--mfg-date 2022-12-12 \
--hw-ver 1 \
--hw-ver-str "prerelase" \
--pai \
--key /path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Key.pem \
--cert /path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.pem \
-cd /path/to/connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der  \
--spake2-path /path/to/spake2p \
--chip-tool-path /path/to/chip-tool \
--chip-cert-path /path/to/chip-cert
```

### Generate 5 factory partitions [Optional argument : --count]

```shell
python3 mfg_tool.py --count 5 -v 0xFFF2 -p 0x8001 \
--serial-num AABBCCDDEEFF11223344556677889900 \
--vendor-name "Telink Semiconductor" \
--product-name "not-specified" \
--mfg-date 2022-02-02 \
--hw-ver 1 \
--hw-ver-str "prerelase" \
--pai \
--key /path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Key.pem \
--cert /path/to/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.pem \
-cd /path/to/connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der  \
--spake2-path /path/to/spake2p \
--chip-tool-path /path/to/chip-tool \
--chip-cert-path /path/to/chip-cert
```

## Output files and directory structure

```
out
├── device_sn.csv
└── fff2_8001
    ├── aabbccddeeff11223344556677889900
    │   ├── factory_data.bin
    │   ├── factory_data.hex
    │   ├── internal
    │   │   ├── DAC_cert.der
    │   │   ├── DAC_cert.pem
    │   │   ├── DAC_key.pem
    │   │   ├── DAC_private_key.bin
    │   │   ├── DAC_public_key.bin
    │   │   └── pai_cert.der
    │   ├── onb_codes.csv
    │   ├── pin_disc.csv
    │   ├── qrcode.png
    │   └── summary.json
    ├── aabbccddeeff11223344556677889901
    │   ├── factory_data.bin
    │   ├── factory_data.hex
    │   ├── internal
    │   │   ├── DAC_cert.der
    │   │   ├── DAC_cert.pem
    │   │   ├── DAC_key.pem
    │   │   ├── DAC_private_key.bin
    │   │   ├── DAC_public_key.bin
    │   │   └── pai_cert.der
    │   ├── onb_codes.csv
    │   ├── pin_disc.csv
    │   ├── qrcode.png
    │   └── summary.json
    ├── aabbccddeeff11223344556677889902
    │   ├── factory_data.bin
    │   ├── factory_data.hex
    │   ├── internal
    │   │   ├── DAC_cert.der
    │   │   ├── DAC_cert.pem
    │   │   ├── DAC_key.pem
    │   │   ├── DAC_private_key.bin
    │   │   ├── DAC_public_key.bin
    │   │   └── pai_cert.der
    │   ├── onb_codes.csv
    │   ├── pin_disc.csv
    │   ├── qrcode.png
    │   └── summary.json
    └── aabbccddeeff11223344556677889903
        ├── factory_data.bin
        ├── factory_data.hex
        ├── internal
        │   ├── DAC_cert.der
        │   ├── DAC_cert.pem
        │   ├── DAC_key.pem
        │   ├── DAC_private_key.bin
        │   ├── DAC_public_key.bin
        │   └── pai_cert.der
        ├── onb_codes.csv
        ├── pin_disc.csv
        ├── qrcode.png
        └── summary.json
```

Tool generates following output files:

-   Partition Binary : `factory_data.bin` and `factory_data.hex`
-   Partition JSON : `summary.json`
-   Onboarding codes : `onb_codes.csv`
-   QR Code image : `qrcode.png`

Other intermediate files are stored in `internal/` directory:

-   PAI Certificate : `pai_cert.der`
-   DAC Certificates : `DAC_cert.der` and `DAC_cert.pem`
-   DAC Private Key : `DAC_private_key.bin`
-   DAC Public Key : `DAC_public_key.bin`

Above files are stored at `out/<vid_pid>/<SN>`. Each device is identified with
an unique SN.

## Flashing the factory partition FW into Matter App

You can try one of these factory partition FW on developing stage.

1. Prepare a Matter App FW with empty factory data partition.

    For example, `lighting-app`. Please generate the FW as below:

    ```shell
    cd path/to/connectedhomeip/example/ligting-app/telink/
    west build -- -DCONFIG_CHIP_FACTORY_DATA=y
    ```

    The output FW is stored at `./build/zephyr/zephyr.bin`.

2. Then flash Matter App FW onto B91 board.

3. Then flash the `factory_data.bin` generated from the generator tool at
   specific address:

    > Note: The offset for Matter
    > [v1.0-branch](https://github.com/telink-semi/tl_zephyr/blob/telink_matter_v1.0-branch/boards/riscv/tlsr9518adk80d/tlsr9518adk80d.dts)
    > is `0xF4000` and for
    > [master branch](https://github.com/telink-semi/tl_zephyr/blob/telink_matter/boards/riscv/tlsr9518adk80d/tlsr9518adk80d.dts)
    > is `0x107000`. You can check the `factory_partition` reg at
    > `tlsr9518adk80d.dts` for details.

    For example, the `factory_data_bin` with serial number
    `aabbccddeeff11223344556677889900`. Here is the expected output in logging:

    ```shell
    ...
    I: 947 [DL]Device Configuration:
    I: 951 [DL]  Serial Number: aabbccddeeff11223344556677889900
    I: 957 [DL]  Vendor Id: 65522 (0xFFF2)
    I: 961 [DL]  Product Id: 32769 (0x8001)
    I: 965 [DL]  Hardware Version: 1
    I: 969 [DL]  Setup Pin Code (0 for UNKNOWN/ERROR): 93320241
    I: 975 [DL]  Setup Discriminator (0xFFFF for UNKNOWN/ERROR): 3008 (0xBC0)
    I: 983 [DL]  Manufacturing Date: 2022-02-02
    I: 988 [DL]  Device Type: 65535 (0xFFFF)
    I: 993 [SVR]SetupQRCode: [MT:634J042C00O-KB7Z-10]
    I: 999 [SVR]Copy/paste the below URL in a browser to see the QR Code:
    I: 1006 [SVR]https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A634J042C00O-KB7Z-10
    I: 1017 [SVR]Manual pairing code: [26251356956]
    ...
    ```

## Secure programming (per-device provisioning)

Secure programming uses a different factory-data layout and stores the DAC
certificate and private key in a separate flash partition. Build the firmware
with the following options:

-   `CONFIG_CHIP_FACTORY_DATA=y`
-   `CONFIG_SECURE_PROGRAMMING=y`
-   `CONFIG_CHIP_FACTORY_DATA_BUILD=n`

Do not use the build-time factory-data generator or merge its regular
`factory_data.bin` into the secure firmware image. The secure factory-data
format has an additional six-byte prefix, which the firmware skips before
parsing the CBOR data.

For **each device**, run `mfg_tool.py` using the usual manufacturing options (as
in the example above) and also supply:

```shell
--secure-programming-verification \
--chip-id 00112233445566770000000000000000
```

The chip ID above is an **example only**. Replace it with the target device's
actual 8-byte IEEE address followed by eight zero bytes, encoded as 32 hex
characters. Use a unique device-specific value and credentials for each device;
secure generation supports only one device per invocation (`--count 1`).

Program the two generated binary files into the corresponding flash partitions:

| Generated file         | Flash partition         |
| ---------------------- | ----------------------- |
| `factory_data_ext.bin` | `factory_partition`     |
| `dac_cert_key.bin`     | `dac_keypair_partition` |

Determine the partition offsets and sizes from the target board's devicetree. Do
not program the regular `factory_data.bin` (or `factory_data.hex`) for secure
programming, and do not include device-specific DAC keys in a shared firmware
build artifact.
