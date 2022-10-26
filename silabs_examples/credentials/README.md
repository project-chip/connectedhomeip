# Matter Attestation Credentials for EFR32

Attestation credentials are composed of:

-   Firmware information
-   Product Attestation Intermediate (PAI) Certificate
-   Device Attestation Certificate (DAC)
-   Certification Declaration (CD)

The firmware information, the Intermediate Authority Certificate, known in
Matter as Product Attestation Intermediate (PAI), and the Device Attestation
Certificate(DAC) are immutable. These shall be installed in factory, and shall
never be modified. The Certification Declaration (CD) is provided by the CSA to
assert protocol compliance of the firmware, and may apply to multiple devices. A
new CD may be installed for each firmware version.

DAC, PAI, and CD are public, and may be stored as plain-text in non-volatile
memory, protected only against modification, however the private-key associated
with the DAC shall remain secret, and its value should never be exposed. In
order to generate the DAC without exposing its private-key, the following
procedure may be used:

1. In factory, the private key may be generate on-device, directly in the
   secured zone, and used to generate a Certificate Signing Request (CSR)
1. The CSR is sent to the Certificate Authority (CA), which generates and
   returns the Device Attestation Certificate (DAC).
1. The CA's certificate (PAI), and the DAC are stored in non-volatile memory.
1. The private-key used to generate (and sign) the CSR now may be used to sign
   outgoing messages.

## EFR32 Example

In Matter, device attestation credentials are obtained through the
[DeviceAttestationCredsProvider](https://github.com/project-chip/connectedhomeip/blob/master/src/credentials/DeviceAttestationCredsProvider.h)
interface. The
[EFR32DeviceAttestationCreds](https://github.com/project-chip/connectedhomeip/blob/master/examples/platform/efr32/EFR32DeviceAttestationCreds.h)
is an example implementation for the EFR32 platform. In this implementation,
PAI, DAC, and CD are all stored in the last page of main flash, which has been
reserved for this purpose. The exact offsets, and file sizes are defined in the
[efr32_creds.h](https://github.com/project-chip/connectedhomeip/blob/master/examples/platform/efr32/efr32_creds.h)
header.

> WARNING: Applications are expected to overwrite the sizes and offsets defined
> by efr32_creds.h. These definitions must match exactly the actual size and
> offsets of the PAI, DAC, and CD files, which may be different from the default
> values found in efr32_creds.h

## Certificate Injection

The following examples illustrate the process of creating the CSR, generating
the DAC, and storing the files in flash, so they may be used by the example
[EFR32DeviceAttestationCreds](https://github.com/project-chip/connectedhomeip/blob/master/examples/platform/efr32/EFR32DeviceAttestationCreds.h)
provider. The source code for these examples can be found in this repository
under `./silabs_examples/credentials`.

The directory structure is as follows:

-   credentials
    -   common
    -   device
        -   brd4164a
        -   brd4166a
        -   brd4186a
    -   host
        -   app
    -   creds.py

The app folder (under host), contains the source code used to read the CSR from
the device, while the folders named brdXXXXy contains the source code of the
device application that generates the CSR, and returned to the host. The
creds.py script completes the process, generating the DAC, and storing the
credential files (PAI, DAC, and CD) into flash. The actual certificate
generation is done using OpenSSL, and the flash writing is performed using
Simplicity Commander.

## Device Setup

1. Enter the creds.py directory:

> `cd silabs_examples/credentials`

2. Set the environment:

> `export BASE_SDK_PATH=../../third_party/silabs/gecko_sdk`<br> 
> `export ARM_GCC_DIR=<ARM GNU toolchain path>`<br>

Example:
> `export BASE_SDK_PATH=/home/myUserName/matter/third_party/silabs/gecko_sdk/`<br>
> `export ARM_GCC_DIR=/usr`<br>

3. Execute the setup script

On Linux, the serial port should have the form `/dev/tty*`, for instance:
> `python3 ./creds.py -p /dev/ttyACM0 -S 440266330 -B brd4186c -C pai_cert.pem -K pai_priv.pem -D cd.bin`

On macOS, use `/dev/cu.*` instead of `/dev/tty.*`, for instance:
> `python3 ./creds.py -p /dev/cu.usbmodem0004402663301 -S 440266330 -B brd4186c -C pai_cert.pem -K pai_priv.pem -D cd.bin`


This script:

-   Compiles both host, and device setup applications
-   Flash the device setup app into the target device
-   Format the input PEM files into DER format
-   Get the CSR file from the target device
-   Generates the DAC using OpenSSL
-   Calculate offsets for DAC, PAI, and DC files
-   Writes DAC, PAI, and DC into the target device's flash using Simplicity
    Commander
-   Generates a efr32_creds.h  header file with the correct offsets and sizes

The parameters are as follows:

> -p, --port Serial port used to connect to the target device<br> -S, --serial
> Serial number of the connected device<br> -B, --board Target board, currently
> brd4164a, brd4166a, brd4186a, or brd4186c<br> -C, --pai_cert PAI certificate
> file in PEM format<br> -K, --pai_key PAI private key file in PEM format<br>
> -D, --cd CD file (binary)<br>

4. Copy the generated header into the Matter code:

> `cp ./temp/efr32_creds.h ${MATTER_HOME}/examples/platform/efr32/`

5. Rebuild the example app with the new credentials enabled, for instance:

> `./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32/ ./out/lighting-app BRD4186C chip_build_platform_attestation_credentials_provider=true`

## Validation

If the certificate injection is successful, the commissioning process should
complete normally. In order to verify that the new certificates are actually
being used, first check the last page of the flash using Commander. The content
of the flash must then be compared with the credentials received by the
commissioner, which may be done using a debugger.

### Flash Dump

On MG12, the last page starts at address 0x000FF800, in MG24, the last page is
located at 0x0817E000. These addresses can be found in the memory map of the
board's datasheet. For instance, for a MG24 board:

> `commander readmem --range 0x0817E000:+2048 --serialno 440266330`

The output should look something like:

    Reading 2048 bytes from 0x0817e000...
    {address:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F}
    0817e000: 30 82 01 CB 30 82 01 71 A0 03 02 01 02 02 08 56
    0817e010: AD 82 22 AD 94 5B 64 30 0A 06 08 2A 86 48 CE 3D
    0817e020: 04 03 02 30 30 31 18 30 16 06 03 55 04 03 0C 0F
    ...
    0817e1a0: 9C 6C B8 8F AC E5 29 36 8D 12 05 4C 0C 02 20 65
    0817e1b0: 5D C9 2B 86 BD 90 98 82 A6 C6 21 77 B8 25 D7 D0
    0817e1c0: 5E DB E7 C2 2F 9F EA 71 22 0E 7E A7 03 F8 91 FF
    0817e1d0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    0817e1e0: 30 82 01 E4 30 82 01 8A A0 03 02 01 02 02 04 1A
    0817e1f0: 3D EE 5A 30 0A 06 08 2A 86 48 CE 3D 04 03 02 30
    0817e200: 3D 31 25 30 23 06 03 55 04 03 0C 1C 4D 61 74 74
    ...
    0817e3a0: 17 32 7A 56 CB 20 02 20 59 7B 5C 8D DB 89 5D FC
    0817e3b0: B6 4B 6A 12 29 27 84 3D 2C 17 69 33 3E 6F BA 14
    0817e3c0: FA 3E 07 CF 0C 25 92 D1 FF FF FF FF FF FF FF FF
    0817e3d0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    0817e3e0: 30 82 02 19 06 09 2A 86 48 86 F7 0D 01 07 02 A0
    0817e3f0: 82 02 0A 30 82 02 06 02 01 03 31 0D 30 0B 06 09
    0817e400: 60 86 48 01 65 03 04 02 01 30 82 01 71 06 09 2A
    ...
    0817e5d0: DE 31 92 E6 78 C7 45 E7 F6 0C 02 21 00 F8 AA 2F
    0817e5e0: A7 11 FC B7 9B 97 E3 97 CE DA 66 7B AE 46 4E 2B
    0817e5f0: D3 FF DF C3 CC ED 7A A8 CA 5F 4C 1A 7C FF FF FF
    0817e600: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF`

Compare this output with the generated efr32_creds.h header:

    #define EFR32_CREDENTIALS_PAI_OFFSET    0x0
    #define EFR32_CREDENTIALS_PAI_SIZE      463

    #define EFR32_CREDENTIALS_DAC_OFFSET    0x1e0
    #define EFR32_CREDENTIALS_DAC_SIZE      488

    #define EFR32_CREDENTIALS_CD_OFFSET     0x3e0
    #define EFR32_CREDENTIALS_CD_SIZE       541

The PAI certificate is located at offset 0 (address 0x0817e000), and has a size
of 463 octets:

    0817e000: 30 82 01 CB 30 82 01 71 A0 03 02 01 02 02 08 56
    0817e010: AD 82 22 AD 94 5B 64 30 0A 06 08 2A 86 48 CE 3D
    ...
    0817e1b0: 5D C9 2B 86 BD 90 98 82 A6 C6 21 77 B8 25 D7 D0
    0817e1c0: 5E DB E7 C2 2F 9F EA 71 22 0E 7E A7 03 F8 91

This should match the contents of the DER-formatted PAI certificate, which is
stored by the setup script as ./temp/pai_cert.der :

> `$ xxd ./temp/pai_cert.der`

    00000000: 3082 01cb 3082 0171 a003 0201 0202 0856  0...0..q.......V
    00000010: ad82 22ad 945b 6430 0a06 082a 8648 ce3d  .."..[d0...*.H.=
    ...
    000001b0: 5dc9 2b86 bd90 9882 a6c6 2177 b825 d7d0  ].+.......!w.%..
    000001c0: 5edb e7c2 2f9f ea71 220e 7ea7 03f8 91    ^.../..q".~....

On this example, the DAC is located at offset 0x1e0 (address 0817e1e0), and has
a size of 488 octets:

    0817e1e0: 30 82 01 E4 30 82 01 8A A0 03 02 01 02 02 04 1A
    0817e1f0: 3D EE 5A 30 0A 06 08 2A 86 48 CE 3D 04 03 02 30
    ...
    0817e3b0: B6 4B 6A 12 29 27 84 3D 2C 17 69 33 3E 6F BA 14
    0817e3c0: FA 3E 07 CF 0C 25 92 D1

This should match the contents of the DER-formatted DAC certificate, which is
stored by the setup script as ./temp/dac_cert.der :

> `$ xxd ./temp/dac_cert.der`

    00000000: 3082 01e4 3082 018a a003 0201 0202 041a  0...0...........
    00000010: 3dee 5a30 0a06 082a 8648 ce3d 0403 0230  =.Z0...*.H.=...0
    ...
    000001d0: b64b 6a12 2927 843d 2c17 6933 3e6f ba14  .Kj.)'.=,.i3>o..
    000001e0: fa3e 07cf 0c25 92d1                      .>...%..

Finally, on this example the CD is located at offset 0x3e0 (address 0x0817e3e0),
with a size of 541 octets.

    0817e3e0: 30 82 02 19 06 09 2A 86 48 86 F7 0D 01 07 02 A0
    0817e3f0: 82 02 0A 30 82 02 06 02 01 03 31 0D 30 0B 06 09
    ...
    0817e5f0: D3 FF DF C3 CC ED 7A A8 CA 5F 4C 1A 7C FF FF FF
    0817e600: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

The CD is a binary file, and is neither modified, nor validated by the setup
script, it is simply stored in flash after the DAC:

> `$ xxd cd.bin`

    00000000: 3082 0219 0609 2a86 4886 f70d 0107 02a0  0.....*.H.......
    00000010: 8202 0a30 8202 0602 0103 310d 300b 0609  ...0......1.0...
    ...
    00000210: d3ff dfc3 cced 7aa8 ca5f 4c1a 7c         ......z.._L.|

The 0xff octets between the files, and at the end of the flash are unmodified
sections of the flash storage.
