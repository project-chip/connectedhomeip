# Using SE05X for crypto operations

## Introduction

CHIPCryptoPALHsm_SE05X.cpp file provides the integration of secure element
(SE05x) in crypto layer of CHIP stack. By enabling the required directives in
CHIPCryptoPALHsm_config.h header file, required crypto operations can be
performed using SE05x. By default, the secure element is enabled only for PAKE
(Verifier role) operations.

Following table shows the operations supported by secure element.

+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| Operation | Software | Default config | Note | Re-entrant | Directive to
enabled in | | | fallback | value | | | CHIPCryptoPALHsm_config.h |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| SPAKE Verifier | No | Enabled | - | No | ENABLE_HSM_SPAKE_VERIFIER |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| SPAKE Prover | No | Enabled | - | No | ENABLE_HSM_SPAKE_PROVER |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| AES CCM Encrypt | - | - | Not supported | - | Not supported |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| AES CCM Decrypt | - | - | Not supported | - | Not supported |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| SHA256 | No | Disabled | - | No | ENABLE_HSM_HASH_SHA256 |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| SHA256 Multistep | - | - | Not supported | - | Not supported |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| HKDF SHA256 | Yes | Enabled | SW rollback if - | Yes | ENABLE_HSM_HKDF_SHA256
| | | | | salt length is > 64 | | | | | | | info length > 80 | | | | | | |
secret_length > 256 | | | | | | | out_length > 768 | | |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| RAND Generator | No | Enabled | - | Yes | ENABLE_HSM_RAND_GEN |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| ECDSA Sign Message | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY | |
(Nist256) | | | | | |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| ECDSA Sign Hash | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY | |
(Nist256) | | | | | |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| ECDSA Validate Message | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY
| | (Nist256) | | | | | |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| ECDSA Validate Hash | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY | |
(Nist256) | | | | | |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| ECDH derive secret | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| Generate Nist256 key | No | Disabled | - | Yes | ENABLE_HSM_GENERATE_EC_KEY |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| HMAC | Yes | Disabled | - | No | ENABLE_HSM_MAC |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+
| MAC - verify | Yes | Disabled | - | No | ENABLE_HSM_MAC |
+-------------------------+-----------+----------------+---------------------------------+------------+----------------------------+

## Build

-   Use the following gn build arguments to enable the secure element in crypto
    layer,

```
smcom=\"t1oi2c\" with_se05x=1
```

## Note

-   For AES operations the secure element can act as crypto accelerator. The
    secret key is passed by value to crypto layer. When encrypt /decrypt is
    invoked, a key object is created with secret value and used for enc / dec.
    At the end of this operation, key object is deleted.

## Known Limitations:

-       AES CCM currently not supported in SE05X integration layer.

-   The SE05X integration layer is not thread safe.

-   For symmetric operations - key object is created runtime. Secret key is
    stored and used for encryption. This might slow down the operations.
