# Matter Device Attestation

## Matter Certificate Overview

For Matter devices to be commissioned into a Matter network, a Matter commissioner must verify that the devices are certified by the Connectivity Standards Alliance, this step in the Commissioning process is called Device Attestation. Each certified device must be configured with a unique Device Attestation Certificate (DAC) and its corresponding DAC private key, which will be checked by a commissioner to add this device to its Matter fabric. For a more conceptual overview of the Matter Certificates and Device Attestation Procedure, refer to [Matter Security](../general/SECURITY.md).

## Device Attestation Public Key Infrastructure and Certification Declaration

Device Attestation Certificates or DACs must be included in all commissionable Matter products and must be unique in each product. DACs are immutable, so they must be installed in-factory and must be issued by a Product Attestation Intermediate (PAI) which chains directly to a Product Attestation Authority (PAA), issued by specified root Certification Authorities (CAs). These root CAs are entities that have been approved by the CSA to issue digital Matter Certificates. Therefore, if you decide not to apply to become a Certification Authority, you will need to request the generation of the Matter Certificate Chain which is a Public Key Infrastructure. To request these certificates, you must meet the following requirements:

- Certify your Matter Product. CSA will issue a CD with a corresponding VID and PID.

- Select a Certification Authority where you will request your DACs. At Silicon Labs, we have partnered with Kudelski to offer the [Custom Manufacturing Service](./USING_CPMS.md) to facilitate this process.

The PAA are root certificates (certificates of a Root Certificate Authority) and are used to sign PAIs (intermediate certificates). For the attestation process to succeed, the certificate chain must start from a trusted Root Certificate; for this purpose, Matter has a database, called [Distributed Compliance Ledger (DCL)](https://webui.dcl.csa-iot.org/), where the PAAs will reside.

>Note: PAAs are not stored on the target devices.

The PAIs are intermediate certificates, signed by the PAA's private key, and are used to sign the DACs (device certificates). The PAI is stored on the target device and is sent to the Commissioner during the attestation process.

The DAC is the certificate that uniquely identifies the device itself. It is signed by the PAI, and like the PAI itself, it is sent to the Commissioner during the attestation process. The DAC public-key must match the device's private-key, which should be stored in the most secure location possible and is used to sign outgoing messages during commissioning.

The CD (Certification Declaration) is a file issued by CSA upon the firmware's certification process. It contains the Vendor ID (VID), and a list of Product IDs (PIDs), which should match the VID, and PID stored in the Subject field of both the PID, and DAC certificates. Along with the PID and DAC, the CD is stored on the device and sent to the Commissioner during the commissioning process.

## Certification Authorities and Recommended Certificate Use

The Certification Authority's certificate chain is used to validate any certificate said authority has signed and confirm the Matter Device Identity. A Root Certification Authority is a CA whose certificates are self-signed. They are the root of trust. These are the most trusted and secured CAs, and their private keys are expected to be the most highly secured keys. Root CAs can generate the whole Certificate Chain (PAA > PAI > DAC). Intermediate Certificate Authorities are CAs whose certificates have been signed by a higher-level CA from which you can generate DACs that will be signed by these Intermediate CAs and saved on Matter Devices. These CAs will generate (PAI-DAC). A compromise in the private key of the Root CA would jeopardize not only their issued Root Certificates, but also all the certificates in that chain including PAIs and DACs.

![Certification Authorities](resources/certification-authorities.png)

While the information in the certificate location is public, write access to the certificate location should be restricted. Usually the certificates are installed in-factory, but the exact procedure depends on the CAs involved, and the mechanisms available to secure the DAC private-key, and the certificate files. Ideally:

1. The device must run a small application in privilege mode that generates the key-pair in a secured environment. This procedure should run on-device.

2. The device issues a CSR, signed with its new private key, that is sent to the CA.

3. The CA issues the new certificate from the certificate Chain and signs it using its own PAI private key.

4. The newly created DAC is returned to the device.

5. The device stores the new DAC in a read-only section of non-volatile memory.

6. The device boots into normal operation mode, thus forbidding any further modification of either the DAC key-pair, and the DAC.

7. From this point on, the DAC private-key value should never be exposed, and the DAC should never be modified, unless it is compromised.

However, in some environments, the key-pair is generated outside the device. A CA may generate the DAC private keys on behalf of the device and use the keys to generate the DAC too. In this case, both the DAC key-pair, and the DAC are stored into the device on secured locations, and any external copy of the DAC private-key must be destroyed.

## Device Attestation Procedure

When commissioning a device, the Commissioner must execute this procedure to validate if the device is indeed Matter certified and compliant, and if it has been produced by a certified manufacturer. The procedure is as follows:

1. The commissioner generates a random 32-byte nonce meant for attestation.
2. The commissioner sends the generated nonce to the commissionee (Matter Device being attested) and makes an Attestation Request.
3. The commissionee responds with an Attestation Response including an Attestation Information message signed with the DAC private key. This Attestation Information must be validated by the commissioner to accept the Matter Device into its Matter Fabric (Network).
4. The commissioner will retrieve the Matter Certificate Chain in order to attest the Matter Device.

Attestation Information to validate:

- The commissioner validates the PAA retrieved is trusted. Generally, the trusted PAA certificates are included in the Distributed Compliance Ledger (DCL).
- The device presents a valid CD, PAI, and DAC. This is determined by verifying the Certificate Chain.
- The VIDs and PIDs on the CD, PAI, and DAC must match. If the VID is included in the PAA, it must also match the other certificates.
- DAC must be valid, and signed by the PAI.
- The PAI must be valid, and signed by a trusted PAA.
- The Discriminator stored on the device must match the one searched by the Commissioner.
- The Passcode provided by the Commissioner must match the one stored on the device.
- The Attestation nonce from the device must match the Commissioners provided nonce.
