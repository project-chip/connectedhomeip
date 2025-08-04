# Infineon OPTIGA&trade; Trust M Provisioning for Matter

To use Infineon OPTIGA&trade; Trust M for device attestation, Provisioning for
OPTIGA&trade; Trust M with Matter test device Attestation certificate is needed.

## Hardware setup:

[Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)

[OPTIGA™ Trust M MTR](https://www.infineon.com/cms/en/product/evaluation-boards/trust-m-mtr-shield/)

[Shield2Go Adapter for Raspberry Pi](https://www.infineon.com/cms/en/product/evaluation-boards/s2go-adapter-rasp-pi-iot/)
or Jumping Wire

## Provisioning for OPTIGA&trade; Trust M

The
[Linux Tools for OPTIGA&trade; Trust M ](https://github.com/Infineon/linux-optiga-trust-m)
can be used to perform provisioning by following the steps mentioned below.

-   Set up chip-tool on Raspberry Pi 4 by following the instruction listed at
    [Building chip-tool on Raspberry Pi ](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md#installing-prerequisites-on-raspberry-pi-4)
-   Clone the repo from Infineon Public GitHub

```
 $ git clone --recurse-submodules https://github.com/Infineon/linux-optiga-trust-m.git
```

-   Build the Linux tools for OPTIGA&trade; Trust M

```
 $ cd linux-optiga-trust-m/
 $ git checkout provider_dev
 $ git submodule update -f
 $ ./provider_installation_script.sh
```

-   Run the script to generate Matter test DAC for lock-app using the public key
    extracted from the Infineon pre-provisioned Certificate and store it into
    0xE0E0

```
$ cd scripts/matter_provisioning/
$ ./matter_test_provisioning.sh
```

_Note:_

_By running this example `matter_test_provisioning.sh`, the steps shown below
are executed:_

_Step1: Extract the public key from the Infineon pre-provisioned
Certificate(0xE0E0) using openssl command._

_Step2: Generate DAC test certificate using the extracted public key, Signed by
[Matter test PAI](https://github.com/project-chip/connectedhomeip/blob/v1.1-branch/credentials/development/attestation/Matter-Development-PAI-FFF1-noPID-Cert.pem)_.
Please note that production devices cannot re-use these test keys/certificates.

_Step3: Write DAC test certificate into OPTIGA&trade; Trust M certificate slot
0xE0E0._

_Step4: Write Matter test PAI into OPTIGA&trade; Trust M certificate slot 0xE0E8
and test CD into OPTIGA&trade; Trust M Arbitrary OID 0xF1E0._

For certificate claim and OPTIGA&trade; Trust M MTR provisioning, please refer
to our
[README for Late-stage Provisioning](https://github.com/Infineon/linux-optiga-trust-m/blob/provider_dev/scripts/matter_provisioning/README.md#certificate-claiming)
