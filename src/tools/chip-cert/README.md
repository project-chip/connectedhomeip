# CHIP Certificate Tool

## Table of Contents

-   [CHIP Certificate Tool](#chip-certificate-tool)
    -   [Table of Contents](#table-of-contents)
    -   [Introduction](#introduction)
    -   [Directory Structure](#directory-structure)
        -   [<code>/src/tools/chip-cert</code>](#srctoolschip-cert)
    -   [Usage Examples](#usage-examples)
        -   [Building](#building)
    -   [Operational Certificates Usage Examples](#operational-certificates-usage-examples)
        -   [Attestation Certificates Usage Examples](#attestation-certificates-usage-examples)
    -   [Command Reference](#command-reference)
        -   [help](#help)
        -   [gen-cert](#gen-cert)
        -   [convert-cert](#convert-cert)
        -   [convert-key](#convert-key)
        -   [resign-cert](#resign-cert)
        -   [validate-cert](#validate-cert)
        -   [print-cert](#print-cert)
        -   [gen-att-cert](#gen-att-cert)
        -   [validate-att-cert](#validate-att-cert)
        -   [gen-cd](#gen-cd)
            -   [gen-cd example](#gen-cd-example)
        -   [print-cd](#print-cd)
            -   [print-cd example](#print-cd-example)
        -   [version](#version)

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

### Building

The `chip-cert` tool will be built when `gn_build.sh` is run. To build just the
`chip-cert` tool locally:

```
. ./scripts/activate.sh
gn gen out/host
ninja -C out/host chip-cert
./out/host/chip-cert help
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

The following example generates Node certificate, where the CA cert/key and the
Node key are provided as a command line arguments:

```
./chip-cert gen-cert --type n --subject-chip-id DEDEDEDE0000001E --subject-fab-id FAB000000000001D --valid-from "2020-10-15 14:23:43" --lifetime 7305 --ca-key 30770201010420C31A9FD24F91B28F3553C6DD0BC05DFB264FB19DE4A293457FF61CF08656F795A00A06082A8648CE3D030107A144034200046909160652E60035DEAFF5EE4DCED6E451BB171D39972874193CBDEA79E2C81198A8CA5151F0FC086556B8D63610E9DDB237DA1AFAC7378838897FA46A776BE5 --ca-cert FTABCEV4XDq64xZcJAIBNwMnFAEAAADKysrKGCYE7xcbJyYFbrW5TDcGJxMEAAAAysrKyicVHQAAAAAAsPoYJAcBJAgBMAlBBGkJFgZS5gA13q/17k3O1uRRuxcdOZcodBk8vep54sgRmKjKUVHw/AhlVrjWNhDp3bI32hr6xzeIOIl/pGp3a+U3CjUBKQEkAgAYJAJgMAQUTMntCbE2MN9jRhRZ0bmiX4LtcIYwBRTwPNuYHS2KwOmYp5Apx6b9P/ztyBgwC0D8Ieqk5XNVp4h3De3CAlndmNqPzT/yGQFkgjozuBz41efPVctoPODsGq6zKv/0RIO45obJNN8X1pGQrtv/9JVSGA== --key 04F1C53AFB1761A75FF07437018E5B76BC75F852904DC7C4607839A5D953140FFE253626FB737647F1043F61D91B5EC0D3B42A7A25FA209CAB7ACD1A76CA46ECD2 --out Chip-Node02-Cert.chip-b64 --out-format chip-b64
```

Note that in the last example, to illustrate the fact that multiple key/cert
formats are supported, the CA private key is in the X509 Hex format, the CA
certificate is in the CHIP TLV base64 format and the Node public key is in the
CHIP TLV Hex format.

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

Now the 'chip-cert' tool can be used to validate generated Node certificate:

```
./chip-cert validate-att-cert --dac Chip-DAC-Cert.pem --pai Chip-PAI-Cert.pem --paa Chip-PAA-Cert.pem
```

The equivalent openssl command line tool can also be used to verify the
attestation certificate chain that was just created:

```
openssl verify -CAfile Chip-PAA-Cert.pem -untrusted Chip-PAI-Cert.pem Chip-DAC-Cert.pem
```

## Command Reference

This section provides details on the various command line parameters that can be
passed to the `chip-cert` tool.

### help

```
$ ./out/debug/standalone/chip-cert help
Usage: chip <command> [ <args...> ]

Commands:

    gen-cert -- Generate a CHIP certificate.

    convert-cert -- Convert a certificate between CHIP and X509 form.

    convert-key -- Convert a private key between CHIP and PEM/DER form.

    resign-cert -- Resign a CHIP certificate using a new CA key.

    validate-cert -- Validate a CHIP certificate chain.

    print-cert -- Print a CHIP certificate.

    gen-att-cert -- Generate a CHIP attestation certificate.

    validate-att-cert -- Validate a CHIP attestation certificate chain.

    gen-cd -- Generate a CHIP certification declaration signed message.

    version -- Print the program version and exit.
```

### gen-cert

```
$ ./out/debug/standalone/chip-cert gen-cert -h
Usage: chip-cert gen-cert [ <options...> ]

Generate a CHIP certificate

COMMAND OPTIONS

   -t, --type <cert-type>

       Certificate type to be generated. Valid certificate type values are:
           r - root certificate
           c - CA certificate
           n - node certificate
           f - firmware signing certificate

   -i, --subject-chip-id <hex-digits>

       Subject DN CHIP Id attribute in hexadecimal format with upto 8 octets with or without '0x' prefix.
          - for Root certificate it is ChipRootId
          - for intermediate CA certificate it is ChipICAId
          - for Node certificate it is ChipNodeId. The value should be in a range [1, 0xFFFFFFEFFFFFFFFF]
          - for Firmware Signing certificate it is ChipFirmwareSigningId

   -f, --subject-fab-id <hex-digits>

       Subject DN Fabric Id attribute in hexadecimal format with upto 8 octets with or without '0x' prefix.
       The value should be different from 0.

   -a, --subject-cat <hex-digits>

       Subject DN CHIP CASE Authentication Tag in hexadecimal format with upto 4 octets with or without '0x' prefix.
       The version subfield (lower 16 bits) should be different from 0.

   -c, --subject-cn-u <string>

       Subject DN Common Name attribute encoded as UTF8String.

   -p, --path-len-constraint <int>

       Path length constraint to be included in the basic constraint extension.
       If not specified, the path length constraint is not included in the extension.

   -x, --future-ext-sub <string>

       NID_subject_alt_name extension to be added to the list of certificate extensions.

   -2, --future-ext-info <string>

       NID_info_access extension to be added to the list of certificate extensions.

   -C, --ca-cert <file/str>

       File or string containing CA certificate to be used to sign the new certificate.

   -K, --ca-key <file/str>

       File or string containing CA private key to be used to sign the new certificate.

   -k, --key <file/str>

       File or string containing the public and private keys for the new certificate.
       If not specified, a new key pair will be generated.

   -o, --out <file/stdout>

       File to contain the new certificate.
       If specified '-' then output is written to stdout.

   -O, --out-key <file/stdout>

       File to contain the public/private key for the new certificate.
       This option must be specified if the --key option is not.
       If specified '-' then output is written to stdout.

  -F, --out-format <format>

       Specifies format of the output certificate and private key.
       If not specified, the default base-64 encoded CHIP format is used.
       Supported format parametes are:
           x509-pem  - X.509 PEM format
           x509-der  - X.509 DER raw format
           x509-hex  - X.509 DER hex encoded format
           chip      - raw CHIP TLV format
           chip-b64  - base-64 encoded CHIP TLV format (default)
           chip-hex  - hex encoded CHIP TLV format

   -V, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]

       The start date for the certificate's validity period. If not specified,
       the validity period starts on the current day.

   -l, --lifetime <days>

       The lifetime for the new certificate, in whole days. Use special value
       4294967295 to indicate that certificate doesn't have well defined
       expiration date

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### convert-cert

```
$ ./out/debug/standalone/chip-cert convert-cert -h
Usage: chip-cert convert-cert [ <options...> ] <in-file> <out-file>

Convert operational certificate between CHIP and X.509 formats.

ARGUMENTS

  <in-file/str>

       File or string containing certificate to be converted.
       The format of the input certificate is auto-detected and can be any of:
       X.509 PEM, X.509 DER, X.509 HEX, CHIP base-64, CHIP raw TLV or CHIP HEX.

  <out-file/stdout>

       The output certificate file name, or '-' to write to stdout.

COMMAND OPTIONS

  -p, --x509-pem

       Output certificate in X.509 PEM format.

  -d, --x509-der

       Output certificate in X.509 DER format.

  -X, --x509-hex

       Output certificate in X.509 DER hex encoded format.

  -c, --chip

       Output certificate in raw CHIP TLV format.

  -x, --chip-hex

       Output certificate in CHIP TLV hexadecimal format.

  -b --chip-b64

       Output certificate in CHIP TLV base-64 encoded format.
       This is the default.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### convert-key

```
$ ./out/debug/standalone/chip-cert convert-key -h
Usage: chip-cert convert-key [ <options...> ] <in-file> <out-file>

Convert private/public key between CHIP and X.509 formats.

ARGUMENTS

   <in-file/str>

       File or string containing private/public key to be converted.
       The format of the input key is auto-detected and can be any of:
       X.509 PEM, X.509 DER, X.509 HEX, CHIP base-64, CHIP raw TLV or CHIP HEX.

       Note: the private key formats include both private and public keys, while
       the public key formats include only public keys. Therefore, conversion from any
       private key format to public key is supported but conversion from public key
       to private CANNOT be done.

   <out-file/stdout>

       The output private key file name, or '-' to write to stdout.

COMMAND OPTIONS

   -p, --x509-pem

       Output the private key in SEC1/RFC-5915 PEM format.

   -d, --x509-der

       Output the private key in SEC1/RFC-5915 DER format.

   -x, --x509-hex

       Output the private key in SEC1/RFC-5915 DER hex encoded format.

   -P, --x509-pubkey-pem

       Output the public key in SEC1/RFC-5915 PEM format.

   -c, --chip

       Output the private key in raw CHIP serialized format.
   -x, --chip-hex

       Output the private key in hex encoded CHIP serialized format.

   -b, --chip-b64

       Output the private key in base-64 encoded CHIP serialized format.
       This is the default.

   -e, --chip-hex

       Output the private key in hex encoded CHIP serialized format.

   -C, --chip-pubkey

       Output the raw public key.

   -B, --chip-pubkey-b64

       Output the public key in base-64 encoded format.

   -E, --chip-pubkey-hex

       Output the public key in hex encoded format.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### resign-cert

```
$ ./out/debug/standalone/chip-cert resign-cert -h
Usage: chip-cert resign-cert [ <options...> ]

Resign a CHIP certificate using a new CA certificate/key.

COMMAND OPTIONS

  -c, --cert <file/str>

       File or string containing the certificate to be re-signed.

  -o, --out <file/stdout>

       File to contain the re-signed certificate.
       If specified '-' then output is written to stdout.

  -C, --ca-cert <file/str>

       File or string containing CA certificate to be used to re-sign the certificate.

  -K, --ca-key <file/str>

       File or string containing CA private key to be used to re-sign the certificate.

  -s, --self

       Generate a self-signed certificate.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### validate-cert

```
$ ./out/debug/standalone/chip-cert validate-cert -h
Usage: chip-cert validate-cert [ <options...> ] <target-cert-file>

Validate a chain of CHIP certificates.

ARGUMENTS

  <file/str>

      File or string containing the certificate to be validated.
      The formats of all input certificates are auto-detected and can be any of:
      X.509 PEM, X.509 DER, X.509 HEX, CHIP base-64, CHIP raw TLV or CHIP HEX.

COMMAND OPTIONS

  -c, --cert <file/str>

       File or string containing an untrusted CHIP certificate to be used during
       validation. Usually, it is Intermediate CA certificate (ICAC).

  -t, --trusted-cert <file/str>

       File or string containing a trusted CHIP certificate to be used during
       validation. Usually, it is trust anchor root certificate (RCAC).

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### print-cert

```
$ ./out/debug/standalone/chip-cert print-cert -h
Usage: chip-cert print-cert [<options...>] <cert-file>

Print a CHIP operational certificate.

ARGUMENTS

  <file/str>

       File or string containing a CHIP certificate.

COMMAND OPTIONS

   -o, --out <file/stdout>

       The output printed certificate file name. If not specified
       or if specified '-' then output is written to stdout.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### gen-att-cert

```
$ ./out/debug/standalone/chip-cert gen-att-cert -h
Usage: chip-cert gen-att-cert [ <options...> ]

Generate a CHIP Attestation certificate

COMMAND OPTIONS

   -t, --type <att-cert-type>

       Attestation certificate type to be generated. Valid certificate type values are:
           a - product attestation authority certificate
           i - product attestation intermediate certificate
           d - device attestation certificate

   -c, --subject-cn <string>

       Subject DN Common Name attribute encoded as UTF8String.

   -V, --subject-vid <hex-digits>

       Subject DN CHIP VID attribute (in hex).

   -P, --subject-pid <hex-digits>

       Subject DN CHIP PID attribute (in hex).

   -a, --vid-pid-as-cn

       Encode Matter VID and PID parameters as Common Name attributes in the Subject DN.
       If not specified then by default the VID and PID fields are encoded using
       Matter specific OIDs.

   -C, --ca-cert <file/str>

       File or string containing CA certificate to be used to sign the new certificate.

   -K, --ca-key <file/str>

       File or string containing CA private key to be used to sign the new certificate.

   -k, --key <file/str>

       File or string containing the public and private keys for the new certificate (in an X.509 PEM format).
       If not specified, a new key pair will be generated.

   -o, --out <file/stdout>

       File to contain the new certificate (in an X.509 PEM format).
       If specified '-' then output is written to stdout.

   -O, --out-key <file/stdout>

       File to contain the public/private key for the new certificate (in an X.509 PEM format).
       This option must be specified if the --key option is not.
       If specified '-' then output is written to stdout.

   -f, --valid-from <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]

       The start date for the certificate's validity period. If not specified,
       the validity period starts on the current day.

   -l, --lifetime <days>

       The lifetime for the new certificate, in whole days. Use special value
       4294967295 to indicate that certificate doesn't have well defined
       expiration date

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### validate-att-cert

```
$ ./out/debug/standalone/chip-cert validate-att-cert -h
Usage: chip-cert validate-att-cert [ <options...> ]

Validate a chain of CHIP attestation certificates

COMMAND OPTIONS

  -d, --dac <file/str>

       File or string containing Device Attestation Certificate (DAC) to be validated.
       The DAC format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.

  -i, --pai <file/str>

       File or string containing Product Attestation Intermediate (PAI) Certificate.
       The PAI format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.

  -a, --paa <file/str>

       File or string containing trusted Product Attestation Authority (PAA) Certificate.
       The PAA format is auto-detected and can be any of: X.509 PEM, DER or HEX formats.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

### gen-cd

```
$ ./out/debug/standalone/chip-cert gen-cd -h
Usage: chip-cert gen-cd [ <options...> ]

Generate CD CMS Signed Message

COMMAND OPTIONS

   -K, --key <file/str>

       File or string containing private key to be used to sign the Certification Declaration.

   -C, --cert <file/str>

       File or string containing certificate associated with the private key that is used
       to sign the Certification Declaration. The Subject Key Identifier in the
       certificate will be included in the signed Certification Declaration message.

   -O, --out <file/stdout>

       File to contain the signed Certification Declaration message.
       If specified '-' then output is written to stdout.

   -f, --format-version <int>

       Format Version.

   -V, --vendor-id <hex-digits>

       Vendor Id (VID) in hex.

   -p, --product-id <hex-digits>

       Product Id (PID) in hex. Maximum 100 PID values can be specified.
       Each PID value should have it's own -p or --product-id option selector.

   -d, --device-type-id <hex-digits>

       Device Type Id in hex.

   -c, --certificate-id <string>

       Certificate Id encoded as UTF8 string.

   -l, --security-level <hex-digits>

       Security Level in hex.

   -i, --security-info <hex-digits>

       Security Information in hex.

   -n, --version-number <hex-digits>

       Version Number in hex.

   -t, --certification-type <int>

       Certification Type. Valid values are:
           0 - Development and Test (default)
           1 - Provisional
           2 - Official

   -o, --dac-origin-vendor-id <hex-digits>

       DAC Origin Vendor Id in hex.

   -r, --dac-origin-product-id <hex-digits>

       DAC Origin Product Id in hex.

   -a, --authorized-paa-cert <file>

       File containing PAA certificate authorized to sign PAI which signs the DAC
       for a product carrying this CD. This field is optional and if present, only specified
       PAAs will be authorized to sign device's PAI for the lifetime of the generated CD.
       Maximum 10 authorized PAA certificates can be specified.
       Each PAA should have its own -a (--authorized-paa-cert) option selector.
       The certificate can be in DER or PEM Form.
       Note that only the Subject Key Identifier (SKID) value will be extracted
       from the PAA certificate and put into CD Structure.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

#### gen-cd example

An example of generating a Certificate Declaration (CD) follows:

```
./chip-cert gen-cd -C credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem -K credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem --out cd.bin -f 1 -V FFF1 -p 8000 -d 0016 -c "ZIG0000000000000000" -l 0 -i 0 -n 0001 -t 0
```

The binary output of the CMS signed CD is written to `cd.bin`.

-   Replace -V FFF1 with your VID in uppercase hex with zero padding
-   Replace -p 8000 with your PID in uppercase hex with zero padding
-   Replace -d 0016 with your primary device type in uppercase hex with zero
    padding

NOTE: `dac-origin-vendor-id` and `dac-origin-product-id` are not included in
this example.

### print-cd

```
$ ./out/debug/standalone/chip-cert print-cd -h
Usage: chip-cert print-cd [<options...>] <file/str>

Print a CHIP certification declaration (CD) content.

ARGUMENTS

  <file/str>

       File or string containing a CHIP CMS Signed CD message.

COMMAND OPTIONS

   -o, --out <file/stdout>

       The output printed CD content file name. If not specified
       or if specified '-' then output is written to stdout.

HELP OPTIONS

  -h, --help
       Print this output and then exit.

  -v, --version
       Print the version and then exit.
```

#### print-cd example

An example of printing a Certificate Declaration (CD), which is provided as a
command line argument in a hex format:

```
./chip-cert print-cd 3081f506092a864886f70d010702a081e73081e4020103310d300b0609608648016503040201305006092a864886f70d010701a0430441152400012501f2ff360205018005028018250334122c04135a494732303134325a423333303030322d3234240500240600250794262408002509f1ff250a008018317e307c020103801462fa823359acfaa9963e1cfa140addf504f37160300b0609608648016503040201300a06082a8648ce3d04030204483046022100926296f7578158be7c459388336ca7383766c9eedd9855cbda6f4cf6bdf43211022100e0dbf4a2bcec4ea274baf0dea208b3365c6ed544086d101afdaf079a2c23e0de
```

### version

Displays the version of the tool and copyright information.

```
$ ./out/debug/standalone/chip-cert version
chip 0.0.0
Copyright (c) 2021-2022 Project CHIP Authors
```
