# NXP SE05x Secure Element Integration in Matter

-   [Introduction](#introduction)
-   [Supported Platforms](#supported_platforms)
-   [SE05x Crypto Configurations](#se05x_crypto_configurations)
-   [SE05x Type Configurations](#se05x_type_configurations)
-   [Device Attestation](#device_attestation)
-   [SCP03 Authentication](#scp03)
-   [Using Trust Provisioned Verifiers of SE051H for SPAKE2P](#using_tp_verifiers_for_spake)

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

### RT1060 EVKB

Integration of SE05x with RT1060 EVKB board is demonstrated using the thermostat
and all-clusters-app examples.

**Prerequisites:** Refer to [RT1060](nxp_rt1060_guide.md) to set up the build
environment.

Refer to [RW61x and SE05x](./nxp_rt1060_guide.md#se05x_secure_element_with_rt1060)
for detailed hardware connections and build instructions.

Refer to [SE05x Crypto Configurations](#se05x_crypto_configurations) to control
which crypto operations are offloaded to SE05x.

Refer to [SE05x Type Configurations](#se05x_type_configurations) to select the
correct secure element variant.

<a name="se05x_crypto_configurations"></a>

## SE05x Crypto Configurations

The following build options control which cryptographic operations are offloaded
to SE05x:

### GN Build Options

| GN Option                               | Description              | Type    | Default |
| --------------------------------------- | ------------------------ | ------- | ------- |
| chip_se05x_spake_verifier               | SPAKE2+ Verifier on SE   | Boolean | false   |
| chip_se05x_spake_prover                 | SPAKE2+ Prover on SE     | Boolean | false   |
| chip_se05x_rnd_gen                      | Random number generation | Boolean | false   |
| chip_se05x_gen_ec_key                   | Generate EC key in SE    | Boolean | true    |
| chip_se05x_ecdsa_verify                 | ECDSA Verify             | Boolean | true    |
| chip_se05x_pbkdf2_sha256                | PBKDF2-SHA256            | Boolean | false   |
| chip_se05x_hkdf_sha256                  | HKDF-SHA256              | Boolean | false   |
| chip_se05x_hmac_sha256                  | HMAC-SHA256              | Boolean | false   |
| chip_se05x_device_attestation           | Device attestation       | Boolean | false   |
| chip_se05x_spake_verifier_use_tp_values | SPAKE with TP verifiers  | Boolean | false   |
| chip_se05x_spake_verifier_tp_set_no     | TP Passcode set number   | Integer | 1       |
| chip_se05x_spake_verifier_tp_iter_cnt   | SPAKE iteration count    | Integer | 1000    |

**Example:**

```bash
gn gen out --args="chip_se05x_device_attestation=true chip_se05x_hkdf_sha256=true"
```

### CMake/Kconfig Options

| Kconfig Option                                 | Description              | Default |
| ---------------------------------------------- | ------------------------ | ------- |
| CONFIG_CHIP_SE05X_SPAKE_VERIFIER               | SPAKE2+ Verifier on SE   | n       |
| CONFIG_CHIP_SE05X_SPAKE_PROVER                 | SPAKE2+ Prover on SE     | n       |
| CONFIG_CHIP_SE05X_RND_GEN                      | Random number generation | n       |
| CONFIG_CHIP_SE05X_GENERATE_EC_KEY              | Generate EC key in SE    | y       |
| CONFIG_CHIP_SE05X_ECDSA_VERIFY                 | ECDSA Verify             | y       |
| CONFIG_CHIP_SE05X_PBKDF2_SHA256                | PBKDF2-SHA256            | n       |
| CONFIG_CHIP_SE05X_HKDF_SHA256                  | HKDF-SHA256              | n       |
| CONFIG_CHIP_SE05X_HMAC_SHA256                  | HMAC-SHA256              | n       |
| CONFIG_CHIP_SE05X_DEVICE_ATTESTATION           | Device attestation       | n       |
| CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES | SPAKE with TP verifiers  | n       |
| CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO     | TP Passcode set number   | 1       |
| CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITER_CNT   | SPAKE iteration count    | 1000    |

**Example:**

```bash
west build -d <out_dir> -b <board> <example_path> -DCONFIG_CHIP_SE05X_DEVICE_ATTESTATION=y -DCONFIG_CHIP_SE05X_HKDF_SHA256=y
```

<a name="se05x_type_configurations"></a>

## SE05x Type Configuration

Configure the SE05x product variant by updating the feature file at
`third_party/simw-top-mini/repo/fsl_sss_ftr.h`.

**Default Configuration:** SE051H is enabled by default.

```c:third_party/simw-top-mini/repo/fsl_sss_ftr.h
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
[SE05x Device Attestation Example](https://github.com/NXP/plug-and-trust/blob/int/CHIPSE_Release/demos/se05x_dev_attest_key_prov/readme.md)

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

<a name="using_tp_verifiers_for_spake"></a>

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

-   [SE051H Get Passcode Example](https://github.com/NXP/plug-and-trust/blob/int/CHIPSE_Release/demos/se05x_get_passcode/readme.md)
-   [SE051H Provision Example](https://github.com/NXP/plug-and-trust/blob/int/CHIPSE_Release/demos/se051h_nfc_comm_prov/readme.md)

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

**Note:** Factory data must be provisioned first to use Trust provisioned values. Refer to [NXP Manufacturing Flow](nxp_manufacturing_flow.md).

```bash
west build -d <out_dir> -b <board> <example_path> -DCONF_FILE_NAME=prj_wifi_fdata.conf -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER=y -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES=y -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO=1 -DCONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITTER_CNT=1000
```

**Running the Example:**

When running the example, pass the iteration count via command line:

```bash
./thermostat-app --spake2p-iterations 1000
```
