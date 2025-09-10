# NXP SE05x Secure Element Integration in Matter

-   [Introduction](#introduction)
-   [Supported Platforms](#supported_platforms)
-   [SE05x Crypto Configurations](#se05x_crypto_configurations)
-   [SE05x Type Configurations](#se05x_type_configurations)
-   [Device attestation](#device_attestation)
-   [SCP03](#scp03)

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

### NXP i.MX 8M Mini EVK

Integration of SE05x with i.MX 8M Mini EVK is demonstrated using the thermostat
example. Refer [i.MX 8M Mini EVK](nxp_imx8m_linux_examples.md) to set up the
build environment.

#### Hardware connections

-   Connections using OM-SE051ARD board :

    | Signal  | i.MX 8M Pin | OM-SE051ARD Pin |
    | ------- | ----------- | --------------- |
    | I2C SDA | J801_SDA    | J2_9            |
    | I2C SCL | J801_SCL    | J2_10           |
    | 3V3     | J801_VCC    | J8_4            |
    | GND     | J801_GND    | J8_7            |

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

    | Kconfig Options                      | Description              | Default setting |
    | ------------------------------------ | ------------------------ | --------------- |
    | CONFIG_CHIP_SE05X_SPAKE_VERIFIER     | Spake2P Verifier on SE   | Disabled        |
    | CONFIG_CHIP_SE05X_SPAKE_PROVER       | Spake2P Prover on SE     | Disabled        |
    | CONFIG_CHIP_SE05X_RND_GEN            | Random number generation | Disabled        |
    | CONFIG_CHIP_SE05X_GENERATE_EC_KEY    | Generate EC key in SE    | Enabled         |
    | CONFIG_CHIP_SE05X_ECDSA_VERIFY       | ECDSA Verify             | Enabled         |
    | CONFIG_CHIP_SE05X_PBKDF2_SHA256      | PBKDF2-SHA256            | Disabled        |
    | CONFIG_CHIP_SE05X_HKDF_SHA256        | HKDF-SHA256              | Disabled        |
    | CONFIG_CHIP_SE05X_HMAC_SHA256        | HMAC-SHA256              | Disabled        |
    | CONFIG_CHIP_SE05X_DEVICE_ATTESTATION | Device attestation       | Disabled        |

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
   provision the device attestation key at id - 0x7D300000 and device
   attestation certificate at id - 0x7D300001.

```
cd third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/linux
gn gen out
ninja -C out se05x_dev_attest_key_prov
./out/se05x_dev_attest_key_prov
```

> [!IMPORTANT] The example is currently supported for i.MX 8M Mini EVK, FRDM
> i.MX93 and RW61x. Adapt the above commands to the i.MX and RW612 build
> commands accordingly.

<a name="scp03"></a>

## SCP03

To enable SCP03 authentication with SE05x, build the example with option

For GN Build system -

```
gn gen out --args="chip_se05x_auth=\"scp03\""
```

Ensure to enable CMAC (MBEDTLS_CMAC_C) in mbedtls config file used.
