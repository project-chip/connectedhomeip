# Using CLI in nRF Connect examples

The following examples for the development kits from Nordic Semiconductor
include a command-line interface that allows access to application logs and
[Zephyr shell](https://docs.zephyrproject.org/1.13.0/subsystems/shell.html):

-   [CHIP nRF Connect Lock Example Application](../../examples/lock-app/nrfconnect/README.md)
-   [CHIP nRF Connect Lighting Example Application](../../examples/lighting-app/nrfconnect/README.md)

The
[CHIP nRF Connect Pigweed Example Application](../../examples/pigweed-app/nrfconnect/README.md)
does not support CLI.

## Accessing the CLI console

To access the CLI console, use a serial terminal emulator of your choice, like
Minicom or GNU Screen. Use the baud rate set to `115200`.

For example, to start using the CLI console with Minicom, run the following
command with _/dev/ttyACM0_ replaced with the device node name of your
development kit:

    $ minicom -D /dev/ttyACM0 -b 115200

When you reboot the kit, you will see the boot logs in the console, similar to
the following messages:

```shell
uart:~$
I: nRF5 802154 radio initialized
I: 8 Sectors of 4096 bytes
I: alloc wra: 0, ff0
I: data wra: 0, 0
*** Booting Zephyr OS build v2.4.0-ncs1  ***
I: Init CHIP stack
...
```

This means that the console is working correctly and you can start using shell
commands. For example, issuing the `kernel threads` command will print
information about all running threads:

```shell
uart:~$ kernel threads
Scheduler: 277 since last call
Threads:
 0x20006518 CHIP
        options: 0x0, priority: -1 timeout: 536896912
        state: pending
        stack size 8192, unused 7256, usage 936 / 8192 (11 %)

 0x20004ab0 SDC RX
        options: 0x0, priority: -10 timeout: 536890152
        state: pending
        stack size 1024, unused 848, usage 176 / 1024 (17 %)
...
```

## Listing all commands

To list all available commands, use the Tab key, which is normally used for the
command completion feature.

Pressing the Tab key in an empty command line prints the list of available
commands:

```shell
uart:~$
  clear              date               device             flash
  help               history            kernel             nrf_clock_control
  ot                 resize             shell
```

Pressing the Tab key with a command entered in the command line cycles through
available options for the given command.

## Using OpenThread commands

[OpenThread commands](https://github.com/openthread/openthread/blob/master/src/cli/README.md)
are also accessible from the shell. However, they must preceded by `ot`. For
example:

```shell
uart:~$ ot masterkey
00112233445566778899aabbccddeeff
Done
```

## Using CHIP-specific commands

The nRF Connect examples let you use several CHIP-specific CLI commands.

These commands are not available by default and to enable using them, set the
`CONFIG_CHIP_LIB_SHELL=y` Kconfig option in the `prj.conf` file of the given
example.

Every invoked command must be preceded by the `matter` prefix.

See the following subsections for the description of each CHIP-specific command.

### device

Handles a group of commands that are used to manage the device. You must use
this command together with one of the additional subcommands listed below.

#### factoryreset

Performs device factory reset that is hardware reset preceded by erasing of the
whole CHIP settings stored in a non-volatile memory.

```shell
uart:~$ matter factoryreset
Performing factory reset ...
```

### onboardingcodes

Handles a group of commands that are used to view information about device
onboarding codes. You can use this command without any subcommand to print all
available onboarding codes or to add a specific subcommand.

```shell
uart:~$ matter onboardingcodes
QRCode:             MT:W0GU2OTB00KA0648G00
QRCodeUrl:          https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3AW0GU2OTB00KA0648G00
ManualPairingCode:  34970112332
```

The `onboardingcodes` command can also take the subcommands listed below.

#### qrcode

Prints the device
[onboarding QR code payload](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device).
Takes no arguments.

```shell
uart:~$ matter onboardingcodes qrcode
MT:W0GU2OTB00KA0648G00
```

#### qrcodeurl

Prints the URL to view the
[device onboarding QR code](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device)
in a web browser. Takes no arguments.

```shell
uart:~$ matter onboardingcodes qrcodeurl
https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3AW0GU2OTB00KA0648G00
```

#### manualpairingcode

Prints the pairing code for the manual onboarding of a device. Takes no
arguments.

```shell
uart:~$ matter onboardingcodes manualpairingcode
34970112332
```

### config

Handles a group of commands that are used to view device configuration
information. You can use this command without any subcommand to print all
available configuration data or to add a specific subcommand.

```shell
VendorId:        9050 (0x235A)
ProductId:       20043 (0x4E4B)
ProductRevision: 1 (0x1)
FabricId:
PinCode:         020202021
Discriminator:   f00
DeviceId:
```

The `config` command can also take the subcommands listed below.

#### pincode

Prints the PIN code for device setup. Takes no arguments.

```shell
uart:~$ matter config pincode
020202021
```

#### discriminator

Prints the device setup discriminator. Takes no arguments.

```shell
uart:~$ matter config discriminator
f00
```

#### vendorid

Prints the vendor ID of the device. Takes no arguments.

```shell
uart:~$ matter config vendorid
9050 (0x235A)
```

#### productid

Prints the product ID of the device. Takes no arguments.

```shell
uart:~$ matter config productid
20043 (0x4E4B)
```

#### productrev

Prints the product revision of the device. Takes no arguments.

```shell
uart:~$ matter config productrev
1 (0x1)
```

#### deviceid

Prints the device identifier. Takes no arguments.

#### fabricid

Prints the fabric identifier. Takes no arguments.

### ble

Handles a group of commands that are used to control the device Bluetooth LE
transport state. You must use this command together with one of the additional
subcommands listed below.

#### help

Prints help information about `ble` commands group.

```shell
uart:~$ matter ble help
  help            Usage: ble <subcommand>
  adv             Enable or disable advertisement. Usage: ble adv <start|stop|state>
```

#### adv start

Enables Bluetooth LE advertising.

```shell
uart:~$ matter ble adv start
Starting BLE advertising
```

#### adv stop

Disables Bluetooth LE advertising.

```shell
uart:~$ matter ble adv stop
Stopping BLE advertising
```

#### adv status

Prints the information about the current Bluetooth LE advertising status.

```shell
uart:~$ matter ble adv state
BLE advertising is disabled

```

### nfc

Handles a group of commands that are used to control the device NFC tag
emulation state. You must use this command together with one of the additional
subcommands listed below.

#### start

Starts the NFC tag emulation.

```shell
uart:~$ matter nfc start
NFC tag emulation started
```

#### stop

Stops the NFC tag emulation.

```shell
uart:~$ matter nfc stop
NFC tag emulation stopped
```

#### state

Prints the information about the NFC tag emulation status.

```shell
uart:~$ matter nfc state
NFC tag emulation is disabled
```

### dns

Handles a group of commands that are used to trigger performing DNS queries. You
must use this command together with one of the additional subcommands listed
below.

#### browse

Browses for DNS services of `_matterc_udp` type and prints the received
response. Takes no argument.

```shell
uart:~$ matter dns browse
Browsing ...
DNS browse succeeded:
   Hostname: 0E824F0CA6DE309C
   Vendor ID: 9050
   Product ID: 20043
   Long discriminator: 3840
   Device type: 0
   Device name:
   Commissioning mode: 0
   IP addresses:
      fd08:b65e:db8e:f9c7:2cc2:2043:1366:3b31
```

#### resolve

Resolves the specified Matter node service given by the <fabric-id> and
<node-id>.

```shell
uart:~$ matter dns resolve <fabric-id> <node-id>
Resolving ...
DNS resolve for 000000014A77CBB3-0000000000BC5C01 succeeded:
   IP address: fd08:b65e:db8e:f9c7:8052:1a8e:4dd4:e1f3
   Port: 11097
```
