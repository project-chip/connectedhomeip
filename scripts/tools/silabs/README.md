# `FactoryDataProvider` for EFR32 Matter device

## Introduction

The python `FactoryDataProvider.py` script allows a user to store a set of
commissionable data in the matter nvm3 section of an EFR32 device. This script
can handle writing to any MCU EFR32MG12 or EFR32MG24 family.

# Prerequisites

The users need to have in their hands a valid and matching set of spake2p+
credentials:

-   Setup Discriminator
-   Setup Passcode
-   Spake2p Iteration count
-   Spake2p Salt

and one of the following:

-   Spake2p Verifier

or

-   path to spake2p generator binary

You can use this same spake2p generator to randomly generate a set of
credentials or generate your a verifier based on the given Passcode, Iteration
count and Salt.

More info on the Spake2p generator and how to build it can be found here
[Spake2p](https://github.com/project-chip/connectedhomeip/blob/master/src/tools/spake2p/README.md)

# How to use `FactoryDataProvider.py`

Using Python 3 call the `FactoryDataProvider` script and provide the needed
arguments. The script will validate the received data and if the device is
connected to the host, write the data in the right nvm3 location.

If the device is not connected, The script will generate an image that can be
flashed to your device at a later time but you must provide the MCU family name.
(EFR32MG12 or EFR32MG24).

If you have more than one device connected to the host. Provide the J-tag serial
number to ensure you are writing to the right device.

| Arguments            | Conformance          | Type               | Description                                                                             |
| -------------------- | -------------------- | ------------------ | --------------------------------------------------------------------------------------- |
| --discriminator      | Required             | dec/hex            | Provide BLE pairing discriminator. e.g: 3840 or 0xF00.                                  |
| --passcode           | Required             | dec/hex            | Provide the default PASE session passcode used to generate the spake2p verifier.        |
| --spake2_iteration   | Required             | dec/hex            | Provide Spake2+ iteration count used to generate the spake2p verifier.                  |
| --spake2_salt        | Required             | string<sup>1</sup> | Provide Spake2+ salt used to generate the spake2p verifier.                             |
| --spake2_verifier    | optional<sup>2</sup> | string<sup>1</sup> | Provide The pre-generated Spake2+ verifier.                                             |
| --gen_spake2p_path   | optional<sup>2</sup> | string             | Provide the absolute path to spake2p generator.                                         |
| --mcu_family         | optional             | string             | MCU Family target. Only needed if your board isn't plugged in (EFR32MG12 or EFR32MG24). |
| --jtag_serial        | optional             | string             | Provide the serial number of the jtag if you have more than one board connected.        |
| --product_id         | optional             | dec/hex            | Provide the product ID. e.g: 32773 or 0x8005 (Max 2 bytes).                             |
| --vendor_id          | optional             | dec/hex            | Provide the vendor ID. e.g: 65521 or 0xFFF1 (Max 2 bytes).                              |
| --product_name       | optional             | string             | Provide the product name (Max 32 char).                                                 |
| --vendor_name        | optional             | string             | Provide the vendor name (Max 32 char).                                                  |
| --hw_version         | optional             | dec/hex            | Provide the hardware version value (Max 2 bytes).                                       |
| --hw_version_str     | optional             | string             | Provide the hardware version string (Max 64 char).                                      |
| --unique_id          | optional             | Hex string         | A 128 bits hex string unique id (without 0x).                                           |
| --serial_number      | optional             | string             | Provide serial number of the device (Max 32 char).                                      |
| --commissioning_flow | optional             | dec/hex            | Provide Commissioning Flow 0=Standard, 1=User Action, 2=Custom.                         |
| --rendezvous_flag    | optional             | dec/hex            | Provide Rendez-vous flag: 1=SoftAP, 2=BLE 4=OnNetwork (Can be combined).                |

<sup>1</sup> Salt and verifier must be provided as base64 string

<sup>2</sup> You need to provide either a pre-generated spake2_verifier or path
to the spake2p generator to generate the verifier

For the hex type, provide the value with the `0x` prefix. For hex string type,
do not add the `0x` prefix.

e.g:

```
 # With spake2_verifier
 python3 FactoryDataProvider.py --passcode 20202021 --discriminator 0xf00 --spake2_iteration 1000 --spake2_salt U1BBS0UyUCBLZXkgU2FsdA== --spake2_verifier uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/SRPFNRODm3wH/MBiehuFc6FJ/NH6Rmzw==
 # With gen_spake2p_path
 python3 FactoryDataProvider.py --passcode 62034001 --discriminator 0xf01 --spake2_iteration 15000 --spake2_salt 95834coRGvFhCB69IdmJyr5qYIzFgSirw6Ja7g5ySYA= --gen_spake2p_path /Users/GitHub/connectedhomeip/src/tools/spake2p/out/spake2p
```
