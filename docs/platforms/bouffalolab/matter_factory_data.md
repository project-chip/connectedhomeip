# Introduction to Matter factory data

Each Matter device should have it own unique factory data manufactured.

This guide demonstrates what `Bouffalo Lab` provides to support factory data:

-   credential factory data protected by hardware security engine
-   reference tool to generate factory data
-   tool/method to program factory data

# Matter factory data

## How to enable

One dedicate flash region allocates for factory data as below which is read-only
for firmware.

```toml
name = "MFD"
address0 = 0x3FE000
size0 = 0x1000
```

To enable matter factory data feature, please append `-mfd` option at end of
target name. Take BL602 Wi-Fi Matter Light as example.

```
./scripts/build/build_examples.py --target bouffalolab-bl602dk-light-littlefs-mfd build
```

## Factory data

This flash region is divided to two parts:

-   One is plain text data, such as Vendor ID, Product ID, Serial number and so
    on.

    > For development/test purpose, all data can put in plain text data.

-   Other is cipher text data, such as private key for device attestation data.

    `Bouffalo Lab` provides hardware security engine to decrypt this part data
    with **only hardware access** efuse key.

Current supported data

-   DAC certificate and private key
-   PAI certificate
-   Certificate declaration

-   Discriminator ID
-   Pass Code
-   Spake2p iteration count, salt and verifier
-   Vendor ID and name
-   Product ID and name
-   Product part number and product label
-   Manufacturing date
-   Hardware version and version string
-   Serial Number
-   Unique identifier

> Note, it is available to add customer/product own information in factory data,
> please reference to `bl_mfd.h`/`bl_mfd.c` in SDK and reference generation
> script
> [generate_factory_data.py](../../../scripts/tools/bouffalolab/generate_factory_data.py)

# Generate Matter factory data

Script tool
[generate_factory_data.py](../../../scripts/tools/bouffalolab/generate_factory_data.py)
call `chip-cert` to generate test certificates and verify certificates.

Please run below command to compile `chip-cert` tool under `connnectedhomeip`
repo for Linux platform.

```shell
./scripts/build/build_examples.py --target linux-x64-chip-cert build
```

## Command options

-   `--cd`, certificate declare

    If not specified, `Chip-Test-CD-Signing-Cert.pem` and
    `Chip-Test-CD-Signing-Key.pem` will sign a test certificate declare for
    development and test purpose

-   `--pai_cert` and `--pai-key`, PAI certificate and PAI private key

    If not specified, `Chip-Test-PAI-FFF1-8000-Cert.pem` and
    `Chip-Test-PAI-FFF1-8000-Key.pem` will be used for development and test
    purpose.

-   `--dac_cert` and `--dac_key`, DAC certificate and DAC private key.

    If not specified, script will use PAI certificate and key specified
    by`--pai_cert` and `--pai-key` to generate DAC certificate and private key
    for development and test purpose.

-   `--discriminator`, discriminator ID

    If not specified, script will generate for user.

-   `--passcode`, passcode

    If not specified, script will generate for user.

-   `--spake2p_it` and `--spake2p_salt`

    If not specified, script will generate and calculate verifier for user.

Please reference to `--help` for more detail.

## Generate with default configuration

-   Default setting uses the following parameters

    -   PAI certification:
        [test PAI certification](../../../credentials/test/attestation/Chip-Test-PAI-FFF1-8000-Key.pem)

    -   Vendor ID for DAC: 0xFFF1; Vendor ID for CD: 0x130d

    -   Product ID for DAC: 0x8000, Product ID for CD: 0x1001

*   Run following command to generate all plain text factory data

    Please create output folder first. Here takes `out/test-cert` as example.

    ```shell
    ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert
    ```

    -   Check DAC certificate. Here takes `out_130d_1001_106_dac_cert.pem` as
        generated test certificate.

        ```shell
        openssl x509 -noout -text -in out/test-cert/out_130d_1001_106_dac_cert.pem
        ```

    -   Check Certification Declare. Here takes `out_130d_1001_cd.der` as
        generated test certificate.

        ```shell
        ./out/linux-x64-chip-cert/chip-cert print-cd out/test-cert/out_130d_1001_cd.der
        ```

*   Run following command to generate factory data which encrypt private of
    device attestation data

    ```shell
    ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert --key <hex string of 16 bytes>
    ```

    > An example of hex string of 16 bytes: 12345678123456781234567812345678.

After command executes successfully, the output folder will has files as below:

-   Test certificate declare file which file name ends with `cd.der`

    If user wants to reuse CD generated before, please specify CD with option
    `--cd` as below.

    ```shell
    ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert --cd <cd file>
    ```

-   Test DAC certificate and DAC certificate key which file names ends with
    `dac_cert.pem` and `dac_key.pem` separately.

-   QR code picture which file name ends with `onboard.png`
-   On board information which file name ends with `onboard.txt`
-   Matter factory data which file name ends with `mfd.bin`.

## Generate with self-defined PAA/PAI certificates

Self-defined PAA/PAI certificates may use in development and test scenario. But,
user should know it has limit to work with real ecosystem.

-   Export environment variables in terminal for certificates generation

    ```
    export TEST_CERT_VENDOR_ID=130D   # Vendor ID hex string
    export TEST_CERT_PRODUCT_ID=1001  # Product ID hex string
    export TEST_CERT_CN=BFLB          # Common Name
    ```

-   Generate PAA certificate and key to `out/cert` folder.

    ```shell
    ./out/linux-x64-chip-cert/chip-cert gen-att-cert --type a --subject-cn "${TEST_CERT_CN} PAA 01" --valid-from "2020-10-15 14:23:43" --lifetime 7305 --out-key out/test-cert/Chip-PAA-Key-${TEST_CERT_VENDOR_ID}.pem --out out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem --subject-vid ${TEST_CERT_VENDOR_ID}
    ```

    -   Check PAA certificate

        ```shell
        openssl x509 -noout -text -in out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem
        ```

-   Convert PAA PEM format file to PAA DER format file

    ```shell
    ./out/linux-x64-chip-cert/chip-cert convert-cert -d out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.der
    ```

    > Please save this PAA DER format file which will be used by `chip-tool`
    > during commissioning.

-   Generate PAI certificate and key:

    ```shell
    ./out/linux-x64-chip-cert/chip-cert gen-att-cert --type i --subject-cn "${TEST_CERT_CN} PAI 01" --subject-vid ${TEST_CERT_VENDOR_ID} --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key out/test-cert/Chip-PAA-Key-${TEST_CERT_VENDOR_ID}.pem --ca-cert out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem --out-key out/test-cert/Chip-PAI-Key-${TEST_CERT_VENDOR_ID}.pem --out out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem
    ```

    -   Check PAI certificate

        ```shell
        openssl x509 -noout -text -in out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem
        ```

-   Generate `MFD` in plain text data with same VID/PID in DAC and CD

    -   Use same environment variables `TEST_CERT_VENDOR_ID` and
        `TEST_CERT_PRODUCT_ID` for CD.

        ```shell
        ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert --paa_cert out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem --paa_key out/test-cert/Chip-PAA-Key-${TEST_CERT_VENDOR_ID}.pem --pai_cert out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --pai_key out/test-cert/Chip-PAI-Key-${TEST_CERT_VENDOR_ID}.pem --dac_pid 0x${TEST_CERT_PRODUCT_ID} --vendor_id 0x${TEST_CERT_VENDOR_ID} --product_id 0x${TEST_CERT_PRODUCT_ID}
        ```

        > Appending `--key <hex string of 16 bytes>` option to enable encrypt
        > private key of attestation device data.

    -   Check DAC certificate. Here takes `out_130d_1001_1349_dac_cert.pem` as
        generated test certification.

        ```shell
        openssl x509 -noout -text -in out/test-cert/out_130d_1001_1349_dac_cert.pem
        ```

    -   Check PAA/PAI/DAC certificate chain.

        ```shell
        ./out/linux-x64-chip-cert/chip-cert validate-att-cert --dac out/test-cert/out_130d_1001_1349_dac_cert.pem --pai out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --paa out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem
        ```

    -   Check Certification Declare. Here takes `out_130d_1001_cd.der` as
        generated test certification.

        ```shell
        ./out/linux-x64-chip-cert/chip-cert print-cd out/test-cert/out_130d_1001_cd.der
        ```

-   Generate `MFD` in plain text data with different VID/PID in DAC and CD

    -   Export vendor ID and product ID for CD

        ```shell
        export TEST_CD_VENDOR_ID=730D   # Vendor ID hex string
        export TEST_CD_PRODUCT_ID=7001  # Product ID hex string
        ```

    -   Run script to generate DAC/CD and `MFD`.

        ```shell
        ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert --paa_cert out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem --paa_key out/test-cert/Chip-PAA-Key-${TEST_CERT_VENDOR_ID}.pem --pai_cert out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --pai_key out/test-cert/Chip-PAI-Key-${TEST_CERT_VENDOR_ID}.pem --dac_pid 0x${TEST_CERT_PRODUCT_ID} --vendor_id 0x${TEST_CD_VENDOR_ID} --product_id 0x${TEST_CD_PRODUCT_ID}
        ```

        > Appending `--key <hex string of 16 bytes>` option to enable encrypt
        > private key of attestation device data.
        >
        > Please use --`vendor_name` and `--product_name` to change vendor name
        > and product name.

    -   Check DAC certificate. Here takes `out_130d_1001_1349_dac_cert.pem` as
        generated test certification.

        ```shell
        openssl x509 -noout -text -in out/test-cert/out_130d_1001_1349_dac_cert.pem
        ```

    -   Check PAA/PAI/DAC certificate chain.

        ```shell
        ./out/linux-x64-chip-cert/chip-cert validate-att-cert --dac out/test-cert/out_130d_1001_1349_dac_cert.pem --pai out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --paa out/test-cert/Chip-PAA-Cert-${TEST_CERT_VENDOR_ID}.pem
        ```

    -   Check Certification Declare. Here takes `out_730D_7001_cd.der` as
        generated test certification.

        ```shell
        ./out/linux-x64-chip-cert/chip-cert print-cd out/test-cert/out_130d_1001_cd.der
        ```

## Generate with self-defined DAC certificate and key

Self-defined DAC certificates may use in development and test scenario. But,
user should know it has limit to work with real ecosystem.

-   Export environment variables in terminal for easy operations

    ```
    export TEST_CERT_VENDOR_ID=130D  # Vendor ID hex string
    export TEST_CERT_PRODUCT_ID=1001 # Product ID hex string
    export TEST_CERT_CN=BFLB         # Common Name
    ```

-   Generate DAC certificate and key

    ```shell
    out/linux-x64-chip-cert/chip-cert gen-att-cert --type d --subject-cn "${TEST_CERT_CN} PAI 01" --subject-vid ${TEST_CERT_VENDOR_ID} --subject-pid ${TEST_CERT_PRODUCT_ID} --valid-from "2020-10-16 14:23:43" --lifetime 5946 --ca-key out/test-cert/Chip-PAI-Key-${TEST_CERT_VENDOR_ID}.pem --ca-cert out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --out-key out/test-cert/Chip-DAC-Key-${TEST_CERT_VENDOR_ID}-${TEST_CERT_PRODUCT_ID}.pem --out out/test-cert/Chip-DAC-Cert-${TEST_CERT_VENDOR_ID}-${TEST_CERT_PRODUCT_ID}.pem
    ```

    > **Note**, `--valid-from` and `--lifetime` should be in `--valid-from` and
    > `--lifetime` of PAI certificate.

-   Generate `MFD` in plain text data

    ```shell
    ./scripts/tools/bouffalolab/generate_factory_data.py --output out/test-cert --pai_cert out/test-cert/Chip-PAI-Cert-${TEST_CERT_VENDOR_ID}.pem --dac_cert out/test-cert/Chip-DAC-Cert-${TEST_CERT_VENDOR_ID}-${TEST_CERT_PRODUCT_ID}.pem --dac_key out/test-cert/Chip-DAC-Key-${TEST_CERT_VENDOR_ID}-${TEST_CERT_PRODUCT_ID}.pem
    ```

    > Appending `--key <hex string of 16 bytes>` option to enable encrypt
    > private key of attestation device data.

# Program factory data

After each target built successfully, a flash programming python script will be
generated under out folder.

Take BL616 Wi-Fi Matter Light as example, `chip-bl616-lighting-example.flash.py`
is using to program firmware, and also for factory data and factory decryption
key.

```shell
/out/bouffalolab-bl616dk-light-wifi-mfd/chip-bl616-lighting-example.flash.py --port <serial port>  --mfd out/test-cert/<mfd bin file>
```

> If `MFD` file has cipher text data, please append
> `--key <hex string of 16 bytes>` option to program to this key to efuse.

Please free contact to `Bouffalo Lab` for DAC provider service and higher
security solution, such as SoC inside certificate requesting.
