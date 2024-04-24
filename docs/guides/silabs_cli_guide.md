# Silabs CLI Guide

## Introduction

The guide discusses the different CLIs that can be enabled with the Silabs
sample apps. The CLIs expose configuration and management APIs via a command
line interface (CLI). For OpenThread devices, the OpenThread CLI can be used
with or without the Matter CLI. For Wi-Fi devices, only the Matter CLI is
available.

-   [Introduction](#introduction)
-   [Enable the CLI interfaces](#enable-the-cli-interfaces)
    -   [Matter CLI](#matter-cli)
    -   [OpenThread CLI](#openthread-cli)
-   [Connecting to the device](#connecting-to-the-device)
    -   [Screen Utility](#screen-utility)
    -   [Tera Term](#tera-term)
-   [Command List](#command-list)
-   [Application Specific Commands](#application-specific-commands)

## Enable the CLI Interfaces

### Matter CLI

To enable the Matter CLI, the `chip_build_libshell=true` argument can be added.
This build argument can be added to OpenThread and Wi-Fi build commands. Here is
an example of the build command with the Matter CLI enabled.

```sh
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs/ ./out/lighting-app BRD4187C chip_build_libshell=true
```

### OpenThread CLI

The OpenThread CLI is enabled by default on all OpenThread builds. To disable
the OpenThread CLI, the `enable_openthread_cli=false` argument can be added.
Here is an example of the build command to disable the OpenThread CLI.

```sh
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs/ ./out/lighting-app BRD4187C enable_openthread_cli=false
```

## Connecting to the Device

The different CLIs are provided through the UART interface. The following table
exposes the UART configurations to connect to the different CLIs.

|  Configuration   | Value  |
| :--------------: | :----- |
| Baudrate (speed) | 115200 |
|       Data       | 8 bit  |
|      Parity      | None   |
|     Stop Bit     | 1 bit  |
|   Flow Control   | None   |

Any serial terminal emulator will permit to connect to the device. For MacOS and
Linux, the screen utility can be used. For Windows, Tera Term can be used.

### Screen Utility

To use the Screen utility, we first need to find the vcom port. For MacOS, it
will be formatted as `/dev/cu.usbmodem...` and for Linux it will be formatted as
`/dev/ttyACM...`.<br/> Here is an example command connecting to the device with
the screen utility.

```sh
screen /dev/cu.usbmodem0004403048491 115200 8-N-1
```

### Tera Term

See the
[Tera Term guide](https://siliconlabs.github.io/matter/latest/wifi/MATTER_SHELL.html)
on how to connect to the device on Windows.

## Command List

When the prompt `matterCli>` is printed, the device is ready for a command.

> **Note**: When the OpenThread CLI is used without the Matter CLI, the prompt
> is `>`.

-   [help](#help)
-   [base64](#base64)
-   [exit](#exit)
-   [version](#version)
-   [ble](#ble)
-   [config](#config)
-   [device](#device)
-   [onboardingcodes](#onboardingcodes)
-   [dns](#dns)
-   [dns](#dns)
-   [ota](#ota)
-   [stat](#stat)
-   [echo](#echo)
-   [log](#log)
-   [rand](#rand)
-   [otcli](#otcli)

### help

Prints the list of commands and their description.

```bash
matterCli> help
  base64          Base64 encode / decode utilities
  exit            Exit the shell application
  help            List out all top level commands
  version         Output the software version
  ble             BLE transport commands
  config          Manage device configuration. Usage to dump value: config [param_name] and to set some values (discriminator): config [param_name] [param_value].
  device          Device management commands
  onboardingcodes Dump device onboarding codes. Usage: onboardingcodes none|softap|ble|onnetwork [qrcode|qrcodeurl|manualpairingcode]
  dns             Dns client commands
  ota             OTA commands
  stat            Statistics commands
  echo            Echo back provided inputs
  log             Logging utilities
  rand            Random number utilities
  otcli           Dispatch OpenThread CLI command
```

### base64

Base64 encode / decode utilities

```bash
matterCli>base64 help
  help            Usage: base64 <subcommand>
  encode          Encode a hex sting as base64. Usage: base64 encode <hex_string>
  decode          Decode a base64 sting as hex. Usage: base64 decode <base64_string>
```

### exit

Exit the application

> **Note**: Application will not respond until reset.

### version

Output the software version

### ble

BLE transport commands

```bash
matterCli> ble help
  help            Usage: ble <subcommand>
  adv             Enable or disable advertisement. Usage: ble adv <start|stop|state>
```

### config

Manage device configuration. Usage to dump value: config [param_name] and to set
some values (discriminator): config [param_name][param_value].

```bash
matterCli> config help
  help            Usage: config <subcommand>
  vendorid        Get VendorId. Usage: config vendorid
  productid       Get ProductId. Usage: config productid
  hardwarever     Get HardwareVersion. Usage: config hardwarever
  pincode         Get commissioning pincode. Usage: config pincode
  discriminator   Get/Set commissioning discriminator. Usage: config discriminator [value]
```

### device

Device management commands

```bash
matterCli> device
  factoryreset    Performs device factory reset
```

### onboardingcodes

Dump device onboarding codes. Usage: onboardingcodes none|softap|ble|onnetwork
[qrcode|qrcodeurl|manualpairingcode]

### dns

Dns client commands

```bash
matterCli> dns
  resolve         Resolve the DNS service. Usage: dns resolve <fabric-id> <node-id> (e.g. dns resolve 5544332211 1)
  browse          Browse DNS services published by Matter nodes. Usage: dns browse <commissionable|commissioner>
```

### ota

OTA commands

```bash
matterCli> ota
  query           Query for a new image. Usage: ota query
  state           Gets state of a current image update process. Usage: ota state
  progress        Gets progress of a current image update process. Usage: ota progress
```

### stat

Statistics commands

### echo

Echo back provided inputs

### log

Logging utilities

### rand

Random number utilities

### otcli

See the official OpenThread CLI
[documentation](https://github.com/openthread/openthread/blob/main/src/cli/README.md)
for the list of commands.

## Application Specific Commands

Samples apps may adds example specific commands to enhance the testable feature
set of the sample. See the sample app documentation for more information on
application specific commands.
