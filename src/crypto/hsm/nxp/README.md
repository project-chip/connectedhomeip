# Using SE05X for crypto operations

## Introduction

CHIPCryptoPALHsm*SE05X*\*.cpp file provides the integration of secure element
(SE05x) in crypto layer of CHIP stack. By enabling the required directives in
CHIPCryptoPALHsm_config.h header file, required crypto operations can be
performed using SE05x. By default, the secure element is enabled for PAKE
(Verifier), HKDF, HMAC (SHA256).

Following are the list operations supported by secure element.

    1. PAKE
    2. ECC KEY - P256 (Disabled by default)
    3. PBKDF (Disabled by default)
    4. HKDF
    5. HMAC (SHA256)

## Build

-   Use the following gn build arguments to enable the secure element in crypto
    layer,

```
chip_with_se05x=1     ==> To enable Se05x
host = "host_k32w"    ==> To set host
chip_with_se05x_da=1  ==> To enable device attestation using se05x. Provision key at key id = '0xDADADADA'. Also enable `ENABLE_HSM_GENERATE_EC_KEY` in CHIPCryptoPALHsm_config.h file.
```

## Known Limitations:

-   None
