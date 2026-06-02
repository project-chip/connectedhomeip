# NXP SE05x Secure Element Integration in Matter

-   [Introduction](#introduction)
-   [Supported Platforms](#supported_platforms)
-   [SE05x Crypto Configurations](#se05x_crypto_configurations)
-   [SE05x Type Configurations](#se05x_type_configurations)
-   [Device Attestation](#device_attestation)
-   [SCP03 Authentication](#scp03)
-   [Using Trust Provisioned Verifiers of SE051H for SPAKE2+](#trust_prov_verifiers_se051h)
-   [SE051H NFC / Unpowered Commissioning](#se051h_nfc_unpowered_commissioning)
-   [GPIO Notification on NFC Commissioning Complete](#gpio_noti_on_nfc_comm_complete)

## GPIO Notification on NFC Commissioning Complete

<hr>

<a name="introduction"></a>

## Introduction

The integration of SE05x Secure Element in the Matter stack enables offloading
of cryptographic operations to the SE05x hardware security module.

### Supported Crypto Operations

The following cryptographic operations can be offloaded to SE05x:

-   Random number generation
-   EC Key generation
-   ECDSA signing
-   ECDSA verification
-   HKDF (HMAC-based Key Derivation Function)
-   HMAC (Hash-based Message Authentication Code)
-   SPAKE2+ (Password Authenticated Key Exchange)

The EC key generation / ECDSA Sign for the operational key is by default
offloaded to SE05x on enabling secure element in the build. To offload other
crypto operations to SE05x, application / matter stack changes will be required.

<a name="supported_platforms"></a>

## Supported Platforms

The SE05x secure element integration is supported on the following NXP
platforms:

### NXP i.MX 8M Mini EVK and FRDM-i.MX93

Integration of SE05x with i.MX 8M Mini EVK and FRDM-i.MX93 is demonstrated using
the thermostat example.

**Prerequisites:** Refer to [i.MX 8M Mini EVK](nxp_imx8m_linux_examples.md) to
set up the build environment.

#### Hardware Connections

##### i.MX 8M Mini EVK with OM-SE051ARD Board

| Signal  | i.MX 8M Pin | OM-SE051ARD Pin |
| ------- | ----------- | --------------- |
| I2C SDA | J801_SDA    | J2_9            |
| I2C SCL | J801_SCL    | J2_10           |
| 3V3     | J801_VCC    | J8_4            |
| GND     | J801_GND    | J8_7            |

##### FRDM-i.MX93 with OM-SE051ARD Board

| Signal            | FRDM i.MX 93 Pin        | OM-SE051ARD Pin |
| ----------------- | ----------------------- | --------------- |
| I2C SDA           | Pin 9 on P12 Connector  | J2_9            |
| I2C SCL           | Pin 7 on P12 Connector  | J2_10           |
| 3V3               | Pin 1 on P12 Connector  | J8_4            |
| GND               | Pin 6 on P12 Connector  | J8_7            |
| Enable (optional) | Pin 29 on P11 Connector | J1_6            |

> **Note:** To control the secure element using the enable pin, ensure jumper
> J14 on the OM-SE051ARD board is connected to pins 3-4. Also build the example
> with `--args="chip_se05x_host_gpio=frdm_imx93"` to allow the FRDM-i.MX93 GPIO
> to control the enable pin.

#### Building the Example

```bash
cd connectedhomeip
```

```bash
export IMX_SDK_ROOT=<path-to-i.MX-Yocto-SDK>
```

```bash
./scripts/examples/imxlinux_example.sh -s examples/thermostat/nxp/linux-se05x/ -o out/thermostat -d
```

Refer to [SE05x Crypto Configurations](#se05x_crypto_configurations) to control
which crypto operations are offloaded to SE05x.

Refer to [SE05x Type Configurations](#se05x_type_configurations) to select the
correct secure element variant.

### RW61x

Integration of SE05x with RW61x is demonstrated using the thermostat and
all-clusters-app examples.

**Prerequisites:** Refer to [RW61x](nxp_rw61x_guide.md) to set up the build
environment.

Refer to [RW61x and SE05x](./nxp_rw61x_guide.md#se05x_secure_element_with_rw61x)
for detailed hardware connections and build instructions.

Refer to [SE05x Crypto Configurations](#se05x_crypto_configurations) to control
which crypto operations are offloaded to SE05x.

Refer to [SE05x Type Configurations](#se05x_type_configurations) to select the
correct secure element variant.

#### Building the Example

**With Pre-Provisioned WiFi Credentials:**

```bash
west build -d <out_dir> -b frdmrw612 examples/thermostat/nxp/ -DCONF_FILE_NAME=prj_wifi.conf -DCONFIG_CHIP_SE05X=y
```

**Without Pre-Provisioned WiFi Credentials:**

```bash
west build -d <out_dir> -b frdmrw612 examples/thermostat/nxp/ -DCONF_FILE_NAME=prj_wifi_onnetwork.conf -DCONFIG_CHIP_SE05X=y -DCONFIG_CHIP_APP_WIFI_SSID=\"<wifi_ssid>\" -DCONFIG_CHIP_APP_WIFI_PASSWORD=\"<password>\"
```

### RT1060 EVKB

Integration of SE05x with RT1060 EVKB board is demonstrated using the thermostat
and all-clusters-app examples.

**Prerequisites:** Refer to [RT1060](nxp_rt1060_guide.md) to set up the build
environment.

Refer to
[RT1060 and SE05x](./nxp_rt1060_guide.md#se05x_secure_element_with_rt1060) for
detailed hardware connections and build instructions.

Refer to [SE05x Crypto Configurations](#se05x_crypto_configurations) to control
which crypto operations are offloaded to SE05x.

Refer to [SE05x Type Configurations](#se05x_type_configurations) to select the
correct secure element variant.

### MCXW72

Integration of SE05x with MCXW72 board is demonstrated using the lighting app
example.

**Prerequisites:** Refer to [MCXW72](nxp_mcxw72_guide.md) to set up the build
environment.

Refer to
[MCXW72 and SE05x](./nxp_mcxw72_guide.md#se05x_secure_element_with_mcxw72) for
detailed hardware connections and build instructions.

Refer to [SE05x Crypto Configurations](#se05x_crypto_configurations) to control
which crypto operations are offloaded to SE05x.

Refer to [SE05x Type Configurations](#se05x_type_configurations) to select the
correct secure element variant.

**Note:** -

1. It is not recommended to enable random number generation from SE05x when
   using with W72.
2. Offloading Device attestation and Spake2+ to SE05x is not yet supported on
   MCXW72 platform.

<a name="se05x_crypto_configurations"></a>

## SE05x Crypto Configurations

The following build options control which cryptographic operations are offloaded
to SE05x:

### GN Build Options

| GN Option                                 | Description                            | Type    | Default | SE05x objects needed                                  |
| ----------------------------------------- | -------------------------------------- | ------- | ------- | ----------------------------------------------------- |
| `chip_se05x_spake_verifier`               | SPAKE2+ verifier on SE                 | Boolean | false   | 0x7D200001(W0) and 0x7D200002(L) are created runtime. The default passcode, salt and iteration count from matter stack are used. |
| `chip_se05x_spake_prover`                 | SPAKE2+ prover on SE                   | Boolean | false   | 0x7D200003(w0) and 0x7D200004(w1) are created runtime. The default passcode, salt and iteration count from matter stack are used. |
| `chip_se05x_rnd_gen`                      | Random number generation               | Boolean | false   | Nothing needed                                        |
| `chip_se05x_device_attestation`           | Device attestation                     | Boolean | false   | 0x7FFF3007(DA Key) and 0x7FFF3003 (DA Cert)           |
| `chip_se05x_spake_verifier_use_tp_values` | SPAKE with trust provisioned verifiers | Boolean | false   | Uses Trust provisioned verifiers. Refer section below |
| `chip_se05x_spake_verifier_tp_set_no`     | Trust provisioned passcode set number  | Integer | 1       | -                                                     |
| `chip_se05x_spake_verifier_tp_iter_cnt`   | SPAKE iteration count                  | Integer | 1000    | -                                                     |


**Example:**

```bash
gn gen out --args="chip_se05x_device_attestation=true"
```

#### Objects Required for Using `chip_se05x_spake_verifier_use_tp_values` option,

1. Passcode Object id - `0x7FFF2000`. Contains 3 set of passcode and salt values. Refer `Using Trust Provisioned Verifiers of SE051H for SPAKE2+` section below for more details.
2. PBKDF Param id - `0x7FFF3002`. Required to specific which passcode set and Iteration to be used. Refer `third_party/simw-top-mini\demos\se051h_nfc_comm_prov\common\se051h_nfc_comm_prov.h` for more details.
3. Based on passcode set and Iteration count below objects need to be provisioned / used from TP values.

| Passcode set number and iteration count  | W0 and L Ids           |
| ---------------------------------------- | -----------------------|
| passcode Set no 1 and Iteration 1000     | 0x7FFF2011, 0x7FFF2021 |
| passcode Set no 1 and Iteration 5000     | 0x7FFF2012, 0x7FFF2022 |
| passcode Set no 1 and Iteration 10000    | 0x7FFF2013, 0x7FFF2023 |
| passcode Set no 1 and Iteration 50000    | 0x7FFF2014, 0x7FFF2024 |
| passcode Set no 1 and Iteration 100000   | 0x7FFF2015, 0x7FFF2025 |
|                                          |                        |
| passcode Set no 2 and Iteration 1000     | 0x7FFF2016, 0x7FFF2026 |
| passcode Set no 2 and Iteration 5000     | 0x7FFF2017, 0x7FFF2027 |
| passcode Set no 2 and Iteration 10000    | 0x7FFF2018, 0x7FFF2028 |
| passcode Set no 2 and Iteration 50000    | 0x7FFF2019, 0x7FFF2029 |
| passcode Set no 2 and Iteration 100000   | 0x7FFF201A, 0x7FFF202A |
|                                          |                        |
| passcode Set no 3 and Iteration 1000     | 0x7FFF201B, 0x7FFF202B |
| passcode Set no 3 and Iteration 5000     | 0x7FFF201C, 0x7FFF202C |
| passcode Set no 3 and Iteration 10000    | 0x7FFF201D, 0x7FFF202D |
| passcode Set no 3 and Iteration 50000    | 0x7FFF201E, 0x7FFF202E |
| passcode Set no 3 and Iteration 100000   | 0x7FFF201F, 0x7FFF202F |

**Note:** - The Pass code, PBKDF, Verifier objects (only the first set of verifiers by default) can be updated / created using SE051H Provision example. Refer [SE051H Provision Example](https://github.com/NXP/plug-and-trust/blob/matter_1_6_se05x/demos/se051h_nfc_comm_prov/readme.md). The other set of verifiers can also be created / updated by modifying the example.

### CMake/Kconfig Options

| Kconfig Option                                   | Description                            | Default | SE05x objects needed                                  |
| ------------------------------------------------ | -------------------------------------- | ------- | ----------------------------------------------------- |
| `CONFIG_CHIP_SE05X_SPAKE_VERIFIER`               | SPAKE2+ verifier on SE                 | n       | 0x7D200001(w0) and 0x7D200002(L) are created runtime. The default passcode, salt and iteration count from matter stack are used. |
| `CONFIG_CHIP_SE05X_SPAKE_PROVER`                 | SPAKE2+ prover on SE                   | n       | 0x7D200003(w0) and 0x7D200004(w1) are created runtime. The default passcode, salt and iteration count from matter stack are used. |
| `CONFIG_CHIP_SE05X_RND_GEN`                      | Random number generation               | n       | Nothing needed                                        |
| `CONFIG_CHIP_SE05X_DEVICE_ATTESTATION`           | Device attestation                     | n       | 0x7FFF3007(DA Key) and 0x7FFF3003 (DA Cert)           |
| `CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES` | SPAKE with trust provisioned verifiers | n       | Uses Trust provisioned verifiers. Refer section above |
| `CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO`     | Trust provisioned passcode set number  | 1       | -                                                     |
| `CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITER_CNT`   | SPAKE iteration count                  | 1000    | -                                                     |

**Example:**

```bash
west build -d <out_dir> -b <board> <example_path> -DCONFIG_CHIP_SE05X_DEVICE_ATTESTATION=y
```

<a name="se05x_type_configurations"></a>

## SE05x Type Configuration

Configure the SE05x product variant by updating the feature file at
`third_party/simw-top-mini/repo/fsl_sss_ftr.h`.

**Default Configuration:** SE051H is enabled by default.

```c
/** Compiling without any Applet Support */
#define SSS_HAVE_APPLET_NONE 0

/** SE050 Type A (ECC) */
#define SSS_HAVE_APPLET_SE05X_A 0

/** SE050 Type B (RSA) */
#define SSS_HAVE_APPLET_SE05X_B 0

/** SE050 (Super set of A + B), SE051, SE052 */
#define SSS_HAVE_APPLET_SE05X_C 0

/** SE051 with SPAKE Support */
#define SSS_HAVE_APPLET_SE051_H 1

/** AUTH */
#define SSS_HAVE_APPLET_AUTH 0

/** SE050E */
#define SSS_HAVE_APPLET_SE050_E 0
```

Set the appropriate variant to `1` and others to `0` based on your hardware.

<a name="device_attestation"></a>

## Device Attestation

To use SE05x for device attestation:

### Step 1: Enable Device Attestation in Build

**For GN Build:**

```bash
gn gen out --args="chip_se05x_device_attestation=true"
```

**For CMake Build:**

```bash
west build -d <out_dir> -b <board> <example_path> -DCONFIG_CHIP_SE05X_DEVICE_ATTESTATION=y
```

### Step 2: Provision Device Attestation Keys (One-Time)

Run the provisioning example to store the device attestation key at ID
`0x7FFF3007` and certificate at ID `0x7FFF3003`:

```bash
cd third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/linux
```

```bash
gn gen out
```

```bash
ninja -C out se05x_dev_attest_key_prov
```

```bash
./out/se05x_dev_attest_key_prov
```

> **Important:** This example is currently supported for i.MX 8M Mini EVK,
> FRDM-i.MX93, and RW61x. Adapt the commands to match your platform's build
> system.

**Reference:**
[SE05x Device Attestation Example](https://github.com/NXP/plug-and-trust/blob/int/matter_1_6_se05x/demos/se05x_dev_attest_key_prov/readme.md)

<a name="scp03"></a>

## SCP03 Authentication

To enable SCP03 (Secure Channel Protocol 03) authentication with SE05x:

### GN Build

```bash
gn gen out --args="chip_se05x_auth=\"scp03\""
```

### CMake Build

```bash
west build -d <out_dir> -b <board> <example_path> -DCONFIG_SE05X_SCP03=y
```

> **Important:** Ensure CMAC (`MBEDTLS_CMAC_C`) is enabled in your mbedTLS
> configuration file.

<a name="trust_prov_verifiers_se051h"></a>

## Using Trust Provisioned Verifiers of SE051H for SPAKE2+

SE051H contains a binary file (ID: `0x7FFF2000`) with 3 sets of passcode and
salt values:

```
[4 byte passcode #1] [16 byte salt #1]
[4 byte passcode #2] [16 byte salt #2]
[4 byte passcode #3] [16 byte salt #3]
```

Pre-calculated SPAKE2+ verifiers (w0 and L) for these passcodes, salts, and 5
different iteration counts (1000, 5000, 10000, 50000, 100000) are
trust-provisioned in SE051H.

### Provision SE051H T4T Applet with QR Code

Use SIMW tools to retrieve the trust-provisioned passcode and provision the
corresponding QR code to the T4T applet.

**Build the tools:**

-   [SE051H Get Passcode Example](https://github.com/NXP/plug-and-trust/blob/int/matter_1_6_se05x/demos/se05x_get_passcode/readme.md)
-   [SE051H Provision Example](https://github.com/NXP/plug-and-trust/blob/int/matter_1_6_se05x/demos/se051h_nfc_comm_prov/readme.md)

**Execute the following commands:**

```bash
./se05x_get_passcode --tp_passcode_set_no 1
```

```bash
./chip_tool payload generate-qrcode --setup-pin-code <PASSCODE_FROM_PREVIOUS_STEP>
```

```bash
./se051h_nfc_comm_prov --only_t4t_provision --qrcode <QR_CODE_FROM_PREVIOUS_STEP>
```

### Build Example with Trust Provisioned Verifiers

> **Note:** Refer to `platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_config.h`
> for additional code changes required for this integration.

**GN Build:**

```bash
gn gen out --args="chip_se05x_spake_verifier=true chip_se05x_spake_verifier_use_tp_values=true chip_se05x_spake_verifier_tp_set_no=1 chip_se05x_spake_verifier_tp_iter_cnt=1000"
```

**CMake Build:**

**Note:** Factory data must be provisioned first to use Trust provisioned
values. Refer to [NXP Manufacturing Flow](nxp_manufacturing_flow.md).

```bash
west build -d <out_dir> -b <board> <example_path> -DCONF_FILE_NAME=prj_wifi_fdata.conf -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER=y -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES=y -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO=1 -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITER_CNT=1000
```

**Running the Example:**

When running the example, pass the iteration count via command line:

```bash
./thermostat-app --spake2p-iterations 1000
```

<a name="se051h_nfc_unpowered_commissioning"></a>

## SE051H NFC / Unpowered Commissioning

SE051H secure element supports NFC-based commissioning for Matter devices
without requiring device power during the commissioning process.

### Step 1: SE051H Provisioning (One-Time)

Provision the SE051H with necessary cryptographic objects using the provisioning
example.

**Reference:**
[SE051H Provision Example](https://github.com/NXP/plug-and-trust/blob/int/matter_1_6_se05x/demos/se051h_nfc_comm_prov/readme.md)

### Step 2: Read Passcode from SE051H

The SE051H contains a binary file with the passcode and salt required for NFC
commissioning. Retrieve the passcode using the get-passcode example.

**Reference:**
[SE051H Get Passcode Example](https://github.com/NXP/plug-and-trust/blob/int/matter_1_6_se05x/demos/se05x_get_passcode/readme.md)

### Step 3: Build chip-tool with NFC Support

Build the chip-tool on an Ubuntu machine with NFC commissioning support:

```bash
./scripts/build/build_examples.py --target linux-x64-chip-tool-nfc-commission build
```

### Step 4: Perform NFC Commissioning

1. Connect the PCSC reader to your Ubuntu machine
2. Connect jumper J6 on the OM-SE051ARD board to pins 2-3 to use the onboard
   antenna
3. Run the commissioning command:

```bash
./out/linux-x64-chip-tool-nfc-commission/chip-tool pairing nfc-wifi 1 <ssid> <password> <passcode> 3840
```

Replace `<ssid>`, `<password>`, and `<passcode>` (from Step 2) with your values.

### Step 5: Build and Run Thermostat Example

Build the thermostat example with SE05x support enabled. Refer to
[Supported Platforms](#supported_platforms) for platform-specific build
instructions.

#### Integration details

During the boot up of the device, required content is read from the SE05x and stored in
the KVS file system for CASE (Certificate Authenticated Session Establishment)
operations.

**Content Read from SE05x (assuming Fabric ID = 1):**

| Content                            | Key ID                                  | Key Name   |
| ---------------------------------- | --------------------------------------- | ---------- |
| Node Operational Certificate       | Part of operational credential cluster¹ | f/1/n      |
| Root Certificate                   | Part of operational credential cluster² | f/1/r      |
| Intermediate Certificate Authority | Part of operational credential cluster³ | f/1/i      |
| IPK Epoch Key                      | 0x7FFF3601                              | f/1/k/0    |
| Node Operational Key Pair          | 0x7FFF3101⁴                             | f/1/o      |
| ACL (Access Control List)          | 0x7FFF3501                              | f/1/ac/0/0 |
| Fabric Group Info                  | -                                       | f/1/g      |
| Metadata                           | Vendor ID from 0x7FFE0028               | f/1/m      |
| Fabric Index Info                  | -                                       | g/fidx     |
| WiFi SSID                          | 0x7FFF3401⁵                             | wifi-ssid  |
| WiFi Password                      | 0x7FFF3401⁵                             | wifi-pass  |

**Notes:**

1. Binary file ID containing the node operational certificate is embedded in the
   operational credential cluster TLV
2. Binary file ID containing the root certificate is embedded in the operational
   credential cluster TLV
3. Binary file ID containing the intermediate certificate authority is embedded
   in the operational credential cluster TLV
4. The node operational key pair cannot be fully read from SE05x. Only the
   public key is extracted. Private key bytes contain the node operational key
   ID (reference key) used for ECDSA signing operations
5. It is recommended to protect this key ID using an authenticated session

### Step 6: Test with chip-tool

Establish a CASE session using the node operational key created during NFC
commissioning:

```bash
./out/linux-x64-chip-tool-nfc-commission/chip-tool thermostat read local-temperature 1 1
```

<a name="gpio_noti_on_nfc_comm_complete"></a>

## GPIO Notification on NFC Commissioning Complete

SE051H sends a notification on the IO2 pin when NFC commissioning completes.
This can trigger a GPIO interrupt on the host MCU to initiate the remaining
commissioning flow.

### Notification Configuration

SE051H can send one or two notifications:

1. During Add NOC (Node Operational Credentials) command
2. During Connect Network command

**Default:** Notification is sent on Connect Network command only.

To change this behavior, update the configuration in:
`matter/third_party/simw-top-mini/repo/demos/se051h_nfc_comm_prov/common/se051h_nfc_comm_prov.h`

```c:matter/third_party/simw-top-mini/repo/demos/se051h_nfc_comm_prov/common/se051h_nfc_comm_prov.h
#define SE051H_PBKDF_PARAMS_ID 0x7FFF3002
#define PBKDF_PARAMS                                                           \
  0x24, 0x00, 0x02, 0x25, 0x01, 0x01, 0x01, 0x25, 0x02, 0x00, 0x00, 0x35,      \
      0x03, 0x26, 0x01, 0xF4, 0x01, 0x00, 0x00, 0x26, 0x02, 0x2C, 0x01, 0x00,  \
      0x00, 0x25, 0x03, 0xA0, 0x0F, 0x25, 0x04, 0x13, 0x00, 0x25, 0x05, 0x0C,  \
      0x00, 0x26, 0x06, 0x00, 0x00, 0x05, 0x01, 0x25, 0x07, 0x01, 0x00, 0x25,  \
      0x08, 0x00, 0x00, 0x26, 0x09, 0x00, 0xFA, 0x00, 0x00, 0x18,
```

**Modify byte 3 to configure notifications:**

-   `0x00` - No notifications
-   `0x01` - Notification on Add NOC command only
-   `0x02` - Notification on Connect Network command only (default)
-   `0x03` - Notifications on both Add NOC and Connect Network commands

### Platform Support

GPIO notification is currently implemented on:

-   FRDM-i.MX93
-   RW61x
-   RT1060 EVKC
-   W72

**Enable GPIO notification:**

-   GN Build: `chip_se05x_host_gpio=frdm_imx93`
-   CMake Build: `-DCONFIG_SE05X_HOST_GPIO=y`

> **Important Notes:**
>
> 1. GPIO notification monitoring is active only when SE051H is in NFC
>    commissioning mode
> 2. The monitoring thread/task stops if non-NFC commissioning is performed

### Platform-Specific GPIO Connections

To enable GPIO notification functionality, hardware connections between the host
MCU and the SE05x board are required. The setup differs based on which SE05x
board variant you are using.

### Option A: Standard SE051ARD Board

To enable GPIO notification functionality, additional hardware modifications are required on the OM-SE051ARD board.

#### Hardware Setup Requirements

**OM-SE051ARD Board Modifications:**

1. **Connect SE051ARD enable pin** (J13-2) to **SE051ARD VDD** (J11-1)
2. **Remove the jumper** on SE051ARD J14
3. **Connect Host Enable Pin** (to control secure element power) to **SE051ARD SE_VDD** (J14-2)
4. **Connect Host GPIO Notification Pin** to **SE051ARD IO2** (J11)
5. **Set J10 on SE051ARD board**. This connects the pull-up resistor to IO2 which let IO2 be always high be default.

**Software Configuration:**

Ensure `CONFIG_SE05X_BOARD_H2` is set to `0` in `third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio.h`:

```c:third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio.h
/* Make this to 1 in case SE05x ARD H2 board is used */
#define CONFIG_SE05X_BOARD_H2 0
```

> **Note:** If the host GPIO cannot drive the SE05x directly, use an additional OM-SE051ARD board as a switch/driver circuit.
The connection of the 3 platforms tested are given below.

---

### Option B: SE05x ARD H2 Board

**Hardware Requirements:**

1. SE05x ARD H2 board
3. Set jumper **J10** on SE051ARDH2 board — this connects the pull-up resistor
   to IO2, keeping IO2 high by default.
4. Connect jumper JP4 on the SE051ARDH2 board to pins 2-3

**Software Configuration:**

In `third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio.h` `CONFIG_SE05X_BOARD_H2` is set to `0` by default. Update it to `1`:

```c:third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio.h
/* Make this to 1 in case SE05x ARD H2 board is used */
#define CONFIG_SE05X_BOARD_H2 1
```

#### FRDM-i.MX93 GPIO Configuration

**Pin Assignments:**

| Function                  | FRDM-i.MX93 Pin         |
| ------------------------- | ----------------------- |
| Enable Pin                | Pin 29 on P11 Connector |
| GPIO Notification Pin     | Pin 31 on P11 Connector |

**Hardware Connections (with SE051ARD switch and DUT board):**

| Source                                       | Destination                     |
| -------------------------------------------- | ------------------------------- |
| FRDM-i.MX93 3.3V Pin 1 on P12 Connector      | SE051ARD (DUT) 3v3 (J8-4)       |
| FRDM-i.MX93 GND Pin 6 on P12 Connector       | SE051ARD (DUT) GND (J8-7)       |
| FRDM-i.MX93 I2C_SCL Pin 7 on P12 Connector   | SE051ARD (DUT) SCL (J2-10)      |
| FRDM-i.MX93 I2C_SDA Pin 9 on P12 Connector   | SE051ARD (DUT) SDA (J2-9)       |
| FRDM-i.MX93 3.3V Pin 1 on P11 Connector      | SE051ARD (switch) Vin (J16-2)   |
| FRDM-i.MX93 Pin 31 on P11 Connector          | SE051ARD (DUT) IO2 (J11-8)      |
| FRDM-i.MX93 Pin 29 on P11 Connector          | SE051ARD (switch) Enable (J13-2)|
| SE051ARD (switch) Vout (J11-5)               | SE051ARD (DUT) SE_VDD (J14-2)   |
| SE051ARD (switch) GND (J2-7)                 | SE051ARD (DUT) GND (J2-7)       |
| SE051ARD (DUT) ENA (J13-2)                   | SE051ARD (DUT) VDD (J11-1)      |


**Hardware Connections (with SE051H2 DUT board):**

| Source                                         | Destination                     |
| ---------------------------------------------- | ------------------------------- |
| FRDM-i.MX93 3.3V Pin 1 on P12 Connector        | SE051H2 Vin (J8-4)              |
| FRDM-i.MX93 GND Pin 6 on P12 Connector         | SE051H2 GND (J8-7)              |
| FRDM-i.MX93 I2C_SCL Pin 7 on P12 Connector     | SE051H2 SCL (J2-10)             |
| FRDM-i.MX93 I2C_SDA Pin 9 on P12 Connector     | SE051H2 SDA (J2-9)              |
| FRDM-i.MX93 Pin 31 on P11 Connector            | SE051H2 IO2 (J11-5)             |
| FRDM-i.MX93 Pin 29 on P11 Connector            | SE051H2 IO/MOSI (J2-4)          |
| FRDM-i.MX93 3.3V Pin 1 on P11 Connector (HIGH) | SE051H2 Enable(J1-6)            |


**Software Implementation:**

A dedicated thread monitors the GPIO pin state and triggers the commissioning flow upon notification.

**Implementation File:**
```
third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio.c
```

**Default Behavior:** The example application restarts upon receiving a GPIO notification.

> **For Production:** Customize the GPIO notification behavior by modifying the `se05x_host_gpio_notification_monitor_init()` function in the implementation file above.

---

#### RW612 GPIO Configuration

**Pin Assignments:**

| Function                  | RW612 Pin |
| ------------------------- | --------- |
| Enable Pin                | J1_12     |
| GPIO Notification Pin     | J1_10     |

**Hardware Connections (with SE051ARD switch and DUT board):**

| Source                               | Destination                     |
| -------------------------------------| ------------------------------- |
| RW612 3.3V (J3-8)                    | SE051ARD (DUT) 3v3 (J8-4)       |
| RW612 GND  (J3-14)                   | SE051ARD (DUT) GND (J8-7)       |
| RW612 I2C_SCL (J1-4)                 | SE051ARD (DUT) SCL (J2-10)      |
| RW612 I2C_SDA (J1-2)                 | SE051ARD (DUT) SDA (J2-9)       |
| RW612 3.3V (J2-16)                   | SE051ARD (switch) Vin (J16-2)   |
| RW612 J1-10                          | SE051ARD (DUT) IO2 (J11-8)      |
| RW612 J1-12                          | SE051ARD (switch) Enable (J13-2)|
| SE051ARD (switch) Vout (J11-5)       | SE051ARD (DUT) SE_VDD (J14-2)   |
| SE051ARD (switch) GND (J2-7)         | SE051ARD (DUT) GND (J2-7)       |
| SE051ARD (DUT) ENA (J13-2)           | SE051ARD (DUT) VDD (J11-1)      |

**Hardware Connections (with SE051H2 DUT board):**

| Source                                  | Destination                     |
| --------------------------------------- | ------------------------------- |
| RW612 3.3V (J3-8)                       | SE051H2 3v3 (J8-4)              |
| RW612 GND  (J3-14)                      | SE051H2 GND (J8-7)              |
| RW612 I2C_SCL (J1-4)                    | SE051H2 SCL (J2-10)             |
| RW612 I2C_SDA (J1-2)                    | SE051H2 SDA (J2-9)              |
| RW612 J1-10                             | SE051H2 IO2 (J11-5)             |
| RW612 J1-12                             | SE051H2 IO/MOSI (J2-4)          |
| RW612 3.3V (J2-16) (HIGH)               | SE051H2 Enable(J1-6)            |

**Software Implementation:**

A dedicated thread monitors the GPIO pin state and triggers the commissioning flow upon notification.

**Implementation File:**
```
third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio_rw61x.c
```

**Default Behavior:** The example application restarts upon receiving a GPIO notification.

> **For Production:** Customize the GPIO notification behavior by modifying the `se05x_host_gpio_notification_monitor_init()` function in the implementation file above.

---

#### RT1060 EVKC GPIO Configuration

**Pin Assignments:**

| Function                  | RT1060 EVKB Pin |
| ------------------------- | --------------- |
| Enable Pin                | J17_1           |
| GPIO Notification Pin     | J17_2           |

**Hardware Connections (with SE051ARD switch and DUT board):**

| Source                                  | Destination                     |
| --------------------------------------- | ------------------------------- |
| EVKCMIMXRT1060 3.3V (J32-4)             | SE051ARD (DUT) 3v3 (J8-4)       |
| EVKCMIMXRT1060 GND (J17_7)              | SE051ARD (DUT) GND (J8-7)       |
| EVKCMIMXRT1060 I2C_SCL (J17-10)         | SE051ARD (DUT) SCL (J2-10)      |
| EVKCMIMXRT1060 I2C_SDA (J17-9)          | SE051ARD (DUT) SDA (J2-9)       |
| EVKCMIMXRT1060 3.3V (J17-8)             | SE051ARD (switch) Vin (J16-2)   |
| EVKCMIMXRT1060 J17-2                    | SE051ARD (DUT) IO2 (J11-8)      |
| EVKCMIMXRT1060 J17-1                    | SE051ARD (switch) Enable (J13-2)|
| SE051ARD (switch) Vout (J11-5)          | SE051ARD (DUT) SE_VDD (J14-2)   |
| SE051ARD (switch) GND (J2-7)            | SE051ARD (DUT) GND (J2-7)       |
| SE051ARD (DUT) ENA (J13-2)              | SE051ARD (DUT) VDD (J11-1)      |

**Hardware Connections (with SE051H2 DUT board):**

| Source                                  | Destination                     |
| --------------------------------------- | ------------------------------- |
| EVKCMIMXRT1060 3.3V (J32-4)             | SE051H2 Vin (J8-4)              |
| EVKCMIMXRT1060 GND (J17-7)              | SE051H2 (DUT) GND (J8-7)        |
| EVKCMIMXRT1060 I2C_SCL (J17-10)         | SE051H2 (DUT) SCL (J2-10)       |
| EVKCMIMXRT1060 I2C_SDA (J17-9)          | SE051H2 (DUT) SDA (J2-9)        |
| EVKCMIMXRT1060 J17-2                    | SE051H2 IO2 (J11-5)             |
| EVKCMIMXRT1060 J17-1                    | SE051H2 IO/MOSI (J2-4)          |
| EVKCMIMXRT1060 3.3V (J17-8) (HIGH)      | SE051H2 Enable(J1-6)            |

**Software Implementation:**

A dedicated thread monitors the GPIO pin state and triggers the commissioning flow upon notification.

**Implementation File:**
```
third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio_rt1060.c
```

**Default Behavior:** The example application restarts upon receiving a GPIO notification.

> **For Production:** Customize the GPIO notification behavior by modifying the `se05x_host_gpio_notification_monitor_init()` function in the implementation file above.

---

#### FRDM W72 GPIO Configuration

**Pin Assignments:**

**Pin Assignments:**

| Function                  | FRDM W72 Pin |
| ------------------------- | ------------ |
| Enable Pin                | J2_10        |
| GPIO Notification Pin     | J1_3         |

**Hardware Connections (with SE051ARD switch and DUT board):**

| Source                                  | Destination                     |
| --------------------------------------- | ------------------------------- |
| FRDM W72 3.3V (J3-2)                    | SE051ARD (DUT) 3v3 (J8-4)       |
| FRDM W72 GND (J3-7)                     | SE051ARD (DUT) GND (J8-7)       |
| FRDM W72 I2C_SCL (J2-1)                 | SE051ARD (DUT) SCL (J2-10)      |
| FRDM W72 I2C_SDA (J2-2)                 | SE051ARD (DUT) SDA (J2-9)       |
| FRDM W72 3.3V (J3-4)                    | SE051ARD (switch) Vin (J16-2)   |
| FRDM W72 J1-3                           | SE051ARD (DUT) IO2 (J11-8)      |
| FRDM W72 J2-10                          | SE051ARD (switch) Enable (J13-2)|
| SE051ARD (switch) Vout (J11-5)          | SE051ARD (DUT) SE_VDD (J14-2)   |
| SE051ARD (switch) GND (J2-7)            | SE051ARD (DUT) GND (J2-7)       |
| SE051ARD (DUT) ENA (J13-2)              | SE051ARD (DUT) VDD (J11-1)      |

**Hardware Connections (with SE051H2 DUT board):**

| Source                                  | Destination                     |
| --------------------------------------- | ------------------------------- |
| FRDM W72 3.3V (J3-4)                    | SE051H2 Vin (J8-4)              |
| FRDM W72 GND (J3-7)                     | SE051H2 (DUT) GND (J8-7)        |
| FRDM W72 I2C_SCL (J2-1)                 | SE051H2 (DUT) SCL (J2-10)       |
| FRDM W72 I2C_SDA (J2-2)                 | SE051H2 (DUT) SDA (J2-9)        |
| FRDM W72 J1-3                           | SE051H2 IO2 (J11-5)             |
| FRDM W72 J2-10                          | SE051H2 IO/MOSI (J2-4)          |
| FRDM W72 3.3V (J3-2) (HIGH)             | SE051H2 Enable(J1-6)            |

**Software Implementation:**

A dedicated thread monitors the GPIO pin state and triggers the commissioning flow upon notification.

**Implementation File:**
```
third_party/simw-top-mini/repo/demos/se05x_host_gpio/se05x_host_gpio_w72.c
```

**Default Behavior:** The example application restarts upon receiving a GPIO notification.

> **For Production:** Customize the GPIO notification behavior by modifying the `se05x_host_gpio_notification_monitor_init()` function in the implementation file above.

---