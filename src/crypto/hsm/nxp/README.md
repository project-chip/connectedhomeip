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

## Build

-   Use the following gn build arguments to enable the secure element in crypto
    layer,

```
smcom=\"t1oi2c\" chip_with_se05x=1
```

## Known Limitations:

-   None
