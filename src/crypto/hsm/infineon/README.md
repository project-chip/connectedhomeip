# Using trustm for crypto operations

## Introduction

CHIPCryptoPALHsm*trustm*\*.cpp file provides the integration of secure element
(trustm) in crypto layer of CHIP stack. By enabling the required directives in
CHIPCryptoPALHsm_config.h header file, required crypto operations can be
performed using trustm. By default, the secure element is enabled for HKDF, HMAC
(SHA256).

Following are the list operations supported by secure element.

    1. HKDF
    2. HMAC (SHA256)
    3. P256

## Build

-   Use the following gn build arguments to enable the secure element in crypto
    layer,

```
chip_enable_trustm=true     ==> To enable trustm
chip_enable_trustm_da=true  ==> To enable trustm with device attestation
host = "psoc6"    ==> To set host
```

## Known Limitations:

-   None
