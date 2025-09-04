# Building

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../../docs/platforms/nxp/nxp_imx8m_linux_examples.md)
for details.

# Connecting SE05x to i.MX 8M

Refer the below image to connect SE05x to i.MX 8M using I2C pins.

![SE05x-i.MX8M-Mini](../../../platform/nxp/se05x/linux/doc/images/imx8evk_se05x.jpg)

# SE05x Integration Configuration

Following crypto operations can be offloaded to SE05x secure element,

    1. Random number generation
    2. EC Key Generate
    3. ECDSA Sign
    4. ECDSA Verify
    5. HKDF
    6. HMAC
    7. Spake

Other crypto operations are done on host (using mbed-TLS)

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

# SE05x Type Configuration

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

# Device attestation

To use SE05x for device attestation,

1. Enable device attestation option when building the example -

```
gn gen out --args="chip_se05x_device_attestation=true"
```

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

# SCP03

To enable SCP03 authentication with SE05x, build the example with option

```
gn gen out --args="chip_se05x_auth=\"scp03\""
```

Ensure to enable CMAC (MBEDTLS_CMAC_C) in mbedtls config file used.
