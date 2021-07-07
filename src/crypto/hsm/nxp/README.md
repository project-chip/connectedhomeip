# Using SE05X for crypto operations

## Introduction

CHIPCryptoPALHsm*SE05X*\*.cpp file provides the integration of secure element
(SE05x) in crypto layer of CHIP stack. By enabling the required directives in
CHIPCryptoPALHsm_config.h header file, required crypto operations can be
performed using SE05x. By default, the secure element is enabled only for PAKE
(Verifier/Prover role) operations and HKDF.

Following are the list operations supported by secure element.

    1. PAKE
    2. ECC KEY - P256 (Disabled by default)
    3. PBKDF (Disabled by default)
    4. HKDF

## Build

-   Use the following gn build arguments to enable the secure element in crypto
    layer,

```
chip_with_se05x=1 host = "host_k32w"
```

## Known Limitations:

-   None
