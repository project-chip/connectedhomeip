# CHIP Certificate Tool

## Introduction

CHIP Certificate Tool (chip-cert) provides command line interface (CLI) utility
used for generating and manipulating CHIP certificates and CHIP private keys
material.

## Directory Structure

### <code>/src/tools/chip-cert</code>

This directory contains various command handler for the 'chip-cert' tool that:

-   generate CHIP certificate
-   convert CHIP certificate format
-   convert CHIP private key format
-   validate CHIP certificate chain
-   resign CHIP certificate
-   print CHIP certificate
-   generate CHIP attestation certificates

## Usage Examples

Specify 'help' option for the detailed 'chip-cert' tool usage instructions:

```
./chip-cert help
```

Specify '--help' option for detail instructions on usage of each command:

```
./chip-cert gen-cert --help
```

## Operational Certificates Usage Examples

Example command that can be used to generate CHIP root certificate and private
key:

```
./chip-cert gen-cert --type r --subject-chip-id CACACACA00000001 --valid-from "2020-10-15 14:23:43" --lifetime 7305 --out-key Chip-Root-Key.pem --out Chip-Root-Cert.pem --out-format x509-pem
```

The root certificate/key output of last command can then be used to generate
CHIP Intermediate CA (ICA) certificate and private key:

```
./chip-cert gen-cert --type c --subject-chip-id CACACACA00000002 --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key Chip-Root-Key.pem --ca-cert Chip-Root-Cert.pem --out-key Chip-ICA-Key.pem --out Chip-ICA-Cert.pem --out-format x509-pem
```

The generated ICA certificate/key can then be used to sign multiple CHIP Node
certificates:

```
./chip-cert gen-cert --type n --subject-chip-id DEDEDEDE0000001D --subject-fab-id FAB000000000001D --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key Chip-ICA-Key.pem --ca-cert Chip-ICA-Cert.pem --out-key Chip-Node-Key.chip-b64 --out Chip-Node-Cert.chip-b64 --out-format chip-b64
```

Note that in the last example the generated Node certificate and private key are
stored in base-64 encoded CHIP native format and not in PEM format as in the
previous examples.

Now the 'chip-cert' tool can be used to validate generated Node certificate:

```
./chip-cert validate-cert Chip-Node-Cert.chip-b64 -c Chip-ICA-Cert.pem -t Chip-Root-Cert.pem
```

Typically, CA services generate certificates in a standard X.509 PEM format.
They can then use this 'chip-cert' tool to convert certificate into raw CHIP TLV
format before provisioning device with operational credentials:

```
./chip-cert convert-cert Chip-ICA-Cert.pem  Chip-ICA-Cert.chip --chip
```

Developers can use this tool to print the content of a CHIP certificate in a
human friendly/readable form:

```
./chip-cert print-cert Chip-ICA-Cert.chip
```

### Attestation Certificates Usage Examples

Example command that can be used to generate Product Attestation Authority (PAA)
certificate and private key:

```
./chip-cert gen-att-cert --type a --subject-cn "Matter Development PAA 01" --valid-from "2020-10-15 14:23:43" --lifetime 7305 --out-key Chip-PAA-Key.pem --out Chip-PAA-Cert.pem
```

The PAA certificate/key output of last command can then be used to generate the
Product Attestation Intermediate (PAI) certificate and private key:

```
./chip-cert gen-att-cert --type i --subject-cn "Matter Development PAI 01" --subject-vid FFF1 --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key Chip-PAA-Key.pem --ca-cert Chip-PAA-Cert.pem --out-key Chip-PAI-Key.pem --out Chip-PAI-Cert.pem
```

The generated PAI certificate/key can then be used to sign multiple Device
Attestation Certificates (DAC):

```
./chip-cert gen-att-cert --type d --subject-cn "Matter Development DAC 01" --subject-vid FFF1 --subject-pid 0123 --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key Chip-PAI-Key.pem --ca-cert Chip-PAI-Cert.pem --out-key Chip-DAC-Key.pem --out Chip-DAC-Cert.pem
```

The standard openssl command line tool can be used to verify the attestation
certificate chain that was just created:

```
openssl verify -CAfile Chip-PAA-Cert.pem -untrusted Chip-PAI-Cert.pem Chip-DAC-Cert.pem
```
