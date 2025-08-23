# NXP SE05x Secure Element Integration in Matter

-   [Introduction](#introduction)
-   [Supported Platforms](#supported_platforms)
-   [SE05x Crypto Configurations](#se05x_crypto_configurations)
-   [SE05x Type Configurations](#se05x_type_configurations)
-   [Device attestation](#device_attestation)
-   [SCP03](#scp03)
-   [SE051H NFC / Unpowered Commissioning](#se051h_nfc_unpowered_commissioning)

<hr>

<a name="introduction"></a>

## Introduction

The integration of SE05x Secure Element in Matter stack can be used to offload
required crypto operations of matter to SE05x.

The following crypto operations are supported using SE05x,

-   Random number generation
-   EC Key Generate
-   ECDSA Sign
-   ECDSA Verify
-   HKDF
-   HMAC
-   Spake2P

<a name="supported_platforms"></a>

## Supported Platforms

The integration of secure element is done with the following platforms

### NXP i.MX 8M Mini EVK and FRDM-i.MX93

Integration of SE05x with i.MX 8M Mini EVK and FRDM-i.MX93 is demonstrated using
the thermostat example. Refer [i.MX 8M Mini EVK](nxp_imx8m_linux_examples.md) to
set up the build environment.

#### Hardware connections

-   Connections using OM-SE051ARD board on i.MX 8M Mini EVK :

    | Signal  | i.MX 8M Pin | OM-SE051ARD Pin |
    | ------- | ----------- | --------------- |
    | I2C SDA | J801_SDA    | J2_9            |
    | I2C SCL | J801_SCL    | J2_10           |
    | 3V3     | J801_VCC    | J8_4            |
    | GND     | J801_GND    | J8_7            |

-   Connections using OM-SE051ARD board on FRDM i.MX93 :

    | Signal            | FRDM i.MX 93 Pin        | OM-SE051ARD Pin |
    | ----------------- | ----------------------- | --------------- |
    | I2C SDA           | Pin 9 on P12 Connector  | J2_9            |
    | I2C SCL           | Pin 7 on P12 Connector  | J2_10           |
    | 3V3               | Pin 1 on P12 Connector  | J8_4            |
    | GND               | Pin 6 on P12 Connector  | J8_7            |
    | Enable (optional) | Pin 29 on P11 Connector | J1_6            |

---

**NOTE** To control secure element using enable pin, ensure to connect jumper
J14 on OM-SE051ARD board to 3-4 pins. Also build the example with
--args="chip_se05x_host_gpio=frdm_imx93" option to allow frdm imx93 gpio to
control the enable pin as required.

---

#### Build the example as

```
    cd connectedhomeip
    ./scripts/examples/imxlinux_example.sh -s examples/thermostat/nxp/linux-se05x/ -o out/thermostat -d
```

Refer [SE05x Crypto Configurations](#se05x_crypto_configurations) sections to
control what crypto operations to be offloaded to SE05x.

Refer [SE05x Type Configurations](#se05x_type_configurations) sections to select
the correct variant of secure element connected.

### RW61x

Integration of SE05x with RW61x is demonstrated using the thermostat and all
cluster app. Refer [RW61x](nxp_rw61x_guide.md) to set up the build environment.

#### Hardware connections and Building the example with SE05x

Refer [RW61x and SE05x](./nxp_rw61x_guide.md#se05x_secure_element_with_rw61x)
for hardware connections / Building.

Refer [SE05x Crypto Configurations](#se05x_crypto_configurations) sections to
control what crypto operations to be offloaded to SE05x.

Refer [SE05x Type Configurations](#se05x_type_configurations) sections to select
the correct variant of secure element connected.

<a name="se05x_crypto_configurations"></a>

## SE05x Crypto Configurations

Following GN / cmake options can be used to enable / disable the crypto
operations to be offloaded to SE05x

-   GN Options :

    | GN Options                    | Description              | Type    | Default setting |
    | ----------------------------- | ------------------------ | ------- | --------------- |
    | chip_se05x_spake_verifier     | Spake2P Verifier on SE   | Boolean | Disabled        |
    | chip_se05x_spake_prover       | Spake2P Prover on SE     | Boolean | Disabled        |
    | chip_se05x_rnd_gen            | Random number generation | Boolean | Disabled        |
    | chip_se05x_gen_ec_key         | Generate EC key in SE    | Boolean | Enabled         |
    | chip_se05x_ecdsa_verify       | ECDSA Verify             | Boolean | Enabled         |
    | chip_se05x_pbkdf2_sha256      | PBKDF2-SHA256            | Boolean | Disabled        |
    | chip_se05x_hkdf_sha256        | HKDF-SHA256              | Boolean | Disabled        |
    | chip_se05x_hmac_sha256        | HMAC-SHA256              | Boolean | Disabled        |
    | chip_se05x_device_attestation | Device attestation       | Boolean | Disabled        |

*   CMAKE Options :

    | GN Options                    | Description              | Default setting |
    | ----------------------------- | ------------------------ | --------------- |
    | CHIP_SE05X_SPAKE_VERIFIER     | Spake2P Verifier on SE   | Disabled        |
    | CHIP_SE05X_SPAKE_PROVER       | Spake2P Prover on SE     | Disabled        |
    | CHIP_SE05X_RND_GEN            | Random number generation | Disabled        |
    | CHIP_SE05X_GENERATE_EC_KEY    | Generate EC key in SE    | Enabled         |
    | CHIP_SE05X_ECDSA_VERIFY       | ECDSA Verify             | Enabled         |
    | CHIP_SE05X_PBKDF2_SHA256      | PBKDF2-SHA256            | Disabled        |
    | CHIP_SE05X_HKDF_SHA256        | HKDF-SHA256              | Disabled        |
    | CHIP_SE05X_HMAC_SHA256        | HMAC-SHA256              | Disabled        |
    | CHIP_SE05X_DEVICE_ATTESTATION | Device attestation       | Disabled        |

<a name="se05x_type_configurations"></a>

## SE05x Type Configuration

Ensure to update the SE05x feature file
`third_party/simw-top-mini/repo/fsl_sss_ftr.h` with correct product variant.

SE050E is enabled by default.

```
/** Compiling without any Applet Support */
#define SSS_HAVE_APPLET_NONE 0

/** SE050 Type A (ECC) */
#define SSS_HAVE_APPLET_SE05X_A 0

/** SE050 Type B (RSA) */
#define SSS_HAVE_APPLET_SE05X_B 0

/** SE050 (Super set of A + B), SE051, SE052 */
#define SSS_HAVE_APPLET_SE05X_C 0

/** SE051 with SPAKE Support */
#define SSS_HAVE_APPLET_SE051_H 0

/** AUTH */
#define SSS_HAVE_APPLET_AUTH 0

/** SE050E */
#define SSS_HAVE_APPLET_SE050_E 1
```

<a name="device_attestation"></a>

## Device attestation

To use SE05x for device attestation,

1. Enable device attestation when building the example using
   `"chip_se05x_device_attestation=true"` for GN build OR
   `"-DCHIP_SE05X_DEVICE_ATTESTATION`" for cmake Build.

2. Run the provision example (one time)
   `third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/` to
   provision the device attestation key at id - 0x7FFF3007 and device
   attestation certificate at id - 0x7FFF3003.

```
cd third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/linux
gn gen out
ninja -C out se05x_dev_attest_key_prov
./out/se05x_dev_attest_key_prov
```

> [!IMPORTANT] The example is currently supported for i.MX 8M Mini EVK, FRDM
> i.MX93 and RW61x. Adapt the above commands to the i.MX and RW612 build
> commands accordingly.

Refer
[SE05x Device attestation Example](https://bitbucket.sw.nxp.com/projects/SIMW/repos/simw-github-mini-pub/browse/demos/se05x_dev_attest_key_prov/readme.md?at=refs%2Fheads%2Fint%2FCHIPSE_Release_NFC_Comm)
for more details.

<a name="scp03"></a>

## SCP03

To enable SCP03 authentication with SE05x, build the example with option

For GN Build system -

```
gn gen out --args="chip_se05x_auth=\"scp03\""
```

> [!IMPORTANT] Ensure to enable CMAC (MBEDTLS_CMAC_C) in mbedtls config file
> used.

<a name="se051h_nfc_unpowered_commissioning"></a>

## SE051H NFC / Unpowered Commissioning

SE051H secure element can be used for NFC / Unpowered Commissioning of Matter
devices. Refer the below steps to to set up the NFC commissioning demo.

### Step 1: SE051H Provisioning (One Time)

For the commissioning to work, SE051H needs to be provisioned to necessary
crypto objects. Use the SE051H provision example to do this. Refer
[SE051H Provision Example](https://bitbucket.sw.nxp.com/projects/SIMW/repos/simw-github-mini-pub/browse/demos/se051h_nfc_comm_prov/readme.md?at=refs%2Fheads%2Fint%2FCHIPSE_Release_NFC_Comm)
for more details.

### Step 2: SE051H Read Pass-code

SE051H has a binary file containing pass-code and salt which is used for NFC
commissioning. The pass-code is required to be provided on the commissionee
side. Use the SE051H get pass-code example to do this. Refer
[SE051H Get Pass-code Example](https://bitbucket.sw.nxp.com/projects/SIMW/repos/simw-github-mini-pub/browse/demos/se05x_get_passcode/readme.md?at=refs%2Fheads%2Fint%2FCHIPSE_Release_NFC_Comm)
for more details.

### Step 3: NFC Commissioning

Build the chip tool on Ubuntu Machine as

```
./scripts/build/build_examples.py --target linux-x64-chip-tool-nfc-commission build
```

Connect the PCSC reader on Ubuntu Machine and perform the NFC commissioning
using chip tool. Ensure to connect Jumper J6 to 2-3 pins to use onboard antenna
of OM-SE051ARD board

```
./out/linux-x64-chip-tool-nfc-commission/chip-tool pairing nfc-wifi 1 "ssid-name" "password" <passcode (Noted in previous step)> 3840
```

### Step 4: Run Thermostat example

Build the thermostat example with SE05x support enabled. Refer
[SE05x Supported Platforms](#supported_platforms) sections. The example will be
built with the SE05x KVS implementation.
(`src\platform\nxp\crypto\se05x\linux_kvs\KeyValueStoreManagerImpl.h`). During
KVS initialization, the required contents from SE05x is read and is put in to
the KVS file / file system for CASE operation to work.

-   As of current implementation, only the following details are read (fabric id
    = 1 is assumed in the below table) :

    | Content                            | Key id                                  | Key Name   |
    | ---------------------------------- | --------------------------------------- | ---------- |
    | Node Operational Certificate       | Part operational credential cluster (1) | f/1/n      |
    | Root certificate                   | Part operational credential cluster (2) | f/1/r      |
    | Intermediate certificate Authority | Part operational credential cluster (3) | f/1/i      |
    | IPK Epoch key                      | 0x7FFF3601                              | f/1/k/0    |
    | Node Operational Key pair          | 0x7FFF3101 (4)                          | f/1/o      |
    | ACL                                | 0x7FFF3501                              | f/1/ac/0/0 |
    | Fabric Group Info                  | -                                       | f/1/g      |
    | Meta Data                          | Vendor id read from 0x7FFE0028          | f/1/m      |
    | Fabric index info                  | -                                       | g/fidx     |

*   (1) : Id of the binary file containing the node operational certificate is
    embedded in the TLV of operational credential cluster file.

*   (2) : Id of the binary file containing the root certificate is embedded in
    the TLV of operational credential cluster file.

*   (3) : Id of the binary file containing the Intermediate certificate
    authority is embedded in the TLV of operational credential cluster file.

*   (4) : The Node operational key pair cannot be read from SE05x. Only the
    public key is read out. The private key bytes are filled with the
    information of the Node operational key id. This is called reference key.
    Reference key is used to refer SE05x for ECDSA Sign operation.
