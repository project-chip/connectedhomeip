# Device Attestation Revocation Testing Guide

## Overview

The device attestation revocation tests help identify the devices with revoked
DACs (Device Attestation Certificates) and PAIs (Product Attestation
Intermediates) during commissioning.

This guide demonstrates how to use a sample application and chip-tool to test
the device attestation revocation functionality.

The sample application is injected with revoked DAC and/or PAI certificates.

During commissioning, chip-tool is provided with a revocation set that is
pre-generated using the `generate_revocation_set.py` script.

## Prerequisites

-   Matter application for Linux platform (e.g., examples/lighting-app/linux)
-   DAC provider JSON file containing revoked DAC and/or PAI certificates
-   chip-tool
-   Device attestation revocation set for the respective DAC and/or PAI

## Test Setup

-   Build the lighting-app/linux and chip-tool:

```
./scripts/examples/gn_build_example.sh examples/lighting-app/linux out/host
./scripts/examples/gn_build_example.sh examples/chip-tool out/host
```

-   Run the lighting-app/linux:

```
./out/host/chip-lighting-app --dac_provider <json-file-with-attestation-information>
```

-   Run the chip-tool with the revocation set:

```
./out/host/chip-tool pairing onnetwork 11 20202021 --dac-revocation-set-path <revocation-set-file>
```

### Test Vectors

| Description           | DAC Provider                                                                                                                 | Revocation Set                                                                                                                     | Expected Result                              |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| PAI revoked by PAA    | [revoked-pai.json](../../credentials/test/revoked-attestation-certificates/dac-provider-test-vectors/revoked-pai.json)       | [revocation-set-for-paa.json](../../credentials/test/revoked-attestation-certificates/revocation-sets/revocation-set-for-paa.json) | Commissioning fails with `kPaiRevoked` (202) |
| DAC-01 revoked by PAI | [revoked-dac-01.json](../../credentials/test/revoked-attestation-certificates/dac-provider-test-vectors/revoked-dac-01.json) | [revocation-set-for-pai.json](../../credentials/test/revoked-attestation-certificates/revocation-sets/revocation-set-for-pai.json) | Commissioning fails with `kDacRevoked` (302) |
| DAC-02 revoked by PAI | [revoked-dac-02.json](../../credentials/test/revoked-attestation-certificates/dac-provider-test-vectors/revoked-dac-02.json) | [revocation-set-for-pai.json](../../credentials/test/revoked-attestation-certificates/revocation-sets/revocation-set-for-pai.json) | Commissioning fails with `kDacRevoked` (302) |
| DAC-03 revoked by PAI | [revoked-dac-03.json](../../credentials/test/revoked-attestation-certificates/dac-provider-test-vectors/revoked-dac-03.json) | [revocation-set-for-pai.json](../../credentials/test/revoked-attestation-certificates/revocation-sets/revocation-set-for-pai.json) | Commissioning fails with `kDacRevoked` (302) |
