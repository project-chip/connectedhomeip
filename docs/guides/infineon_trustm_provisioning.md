# Infineon OPTIGA&trade; Trust M Provisioning for Matter

To use Infineon OPTIGA&trade; Trust M for device attestation, Provisioning for
OPTIGA&trade; Trust M with Matter test device Attestation certificate is needed.

## Hardware setup:

[Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)

[OPTIGA™ Trust M S2GO](https://www.infineon.com/cms/en/product/evaluation-boards/s2go-security-optiga-m/)

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
 $ ./trustm_installation_aarch64_script.sh
```

-   Run the script to generate Matter test DAC for lock-app using the public key
    extracted from the Infineon pre-provisioned Certificate and store it into
    0xe0e3

```
$ cd scripts/matter_provisioning/
$ ./matter_dac_provisioning.sh
```

_Note:_

_By running this example matter_dac_provisioning.sh, the steps shown below are
executed:_

_Step1: Extract the public key from the Infineon pre-provisioned
Certificate(0xe0e0) using openssl command._

_Step2: Generate DAC test certificate using the extracted public key, Signed by
[Matter test PAI](https://github.com/project-chip/connectedhomeip/blob/v1.1-branch/credentials/development/attestation/Matter-Development-PAI-FFF1-noPID-Cert.pem)_.
Please note that production devices cannot re-use these test keys/certificates.

_Step3: Write DAC test certificate into OPTIGA&trade; Trust M certificate slot
0xe0e3_

_Step4: Write Matter test PAI into OPTIGA&trade; Trust M certificate slot 0xe0e8
and test CD into OPTIGA&trade; Trust M Arbitrary OID 0xf1e0._
