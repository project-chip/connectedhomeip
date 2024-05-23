# Using CLI in nRF Connect examples

Some Matter examples for the development kits from Nordic Semiconductor include
a command-line interface that allows access to application logs and
[Zephyr shell](https://docs.zephyrproject.org/1.13.0/subsystems/shell.html).

<hr>

## Accessing the CLI console

To access the CLI console, use a serial terminal emulator of your choice, like
Minicom or GNU Screen. Use the baud rate set to `115200`.

### Example: Starting the CLI console with Minicom

For example, to start using the CLI console with Minicom, run the following
command with `/dev/ttyACM0` replaced with the device node name of your
development kit:

```
minicom -D /dev/ttyACM0 -b 115200
```

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

<hr>

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

<hr>

## Using OpenThread commands

[OpenThread commands](https://github.com/openthread/openthread/blob/master/src/cli/README.md)
are accessible from the shell, but they must preceded by `ot`. For example:

```shell
uart:~$ ot networkkey
00112233445566778899aabbccddeeff
Done
```

<hr>

## Using Matter-specific commands

The nRF Connect examples let you use several Matter-specific CLI commands.

These commands are not available by default and to enable using them, set the
`CONFIG_CHIP_LIB_SHELL=y` Kconfig option in the `prj.conf` file of the given
example.

Every invoked command must be preceded by the `matter` prefix.

See the following subsections for the description of each Matter-specific
command.

### `device` command group

Handles a group of commands that are used to manage the device. You must use
this command together with one of the additional subcommands listed below.

#### `factoryreset` subcommand

Performs device factory reset that is hardware reset preceded by erasing of the
whole Matter settings stored in a non-volatile memory.

```shell
uart:~$ matter device factoryreset
Performing factory reset ...
```

### `onboardingcodes` command group

Handles a group of commands that are used to view information about device
onboarding codes. The `onboardingcodes` command takes one required parameter for
the rendezvous type, then an optional parameter for printing a specific type of
onboarding code.

The full format of the command is as follows:

```
onboardingcodes none|softap|ble|onnetwork [qrcode|qrcodeurl|manualpairingcode]
```

#### `none` subcommand

Prints all onboarding codes. For example:

```shell
uart:~$ matter onboardingcodes none
QRCode:             MT:W0GU2OTB00KA0648G00
QRCodeUrl:          https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3AW0GU2OTB00KA0648G00
ManualPairingCode:  34970112332
```

#### `none qrcode` subcommand

Prints the device
[onboarding QR code payload](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device).
Takes no arguments.

```shell
uart:~$ matter onboardingcodes none qrcode
MT:W0GU2OTB00KA0648G00
```

#### `none qrcodeurl` subcommand

Prints the URL to view the
[device onboarding QR code](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device)
in a web browser. Takes no arguments.

```shell
uart:~$ matter onboardingcodes none qrcodeurl
https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3AW0GU2OTB00KA0648G00
```

#### `none manualpairingcode` subcommand

Prints the pairing code for the manual onboarding of a device. Takes no
arguments.

```shell
uart:~$ matter onboardingcodes none manualpairingcode
34970112332
```

### `config` command group

Handles a group of commands that are used to view device configuration
information. You can use this command without any subcommand to print all
available configuration data or to add a specific subcommand.

```shell
uart:~$ matter config
VendorId:        65521 (0xFFF1)
ProductId:       32768 (0x8000)
HardwareVersion: 1 (0x1)
FabricId:
PinCode:         020202021
Discriminator:   f00
DeviceId:
```

The `config` command can also take the subcommands listed below.

#### `pincode` subcommand

Prints the PIN code for device setup. Takes no arguments.

```shell
uart:~$ matter config pincode
020202021
```

#### `discriminator` subcommand

Prints the device setup discriminator. Takes no arguments.

```shell
uart:~$ matter config discriminator
f00
```

#### `vendorid` subcommand

Prints the vendor ID of the device. Takes no arguments.

```shell
uart:~$ matter config vendorid
65521 (0xFFFF1)
```

#### `productid` subcommand

Prints the product ID of the device. Takes no arguments.

```shell
uart:~$ matter config productid
32768 (0x8000)
```

#### `hardwarever` subcommand

Prints the hardware version of the device. Takes no arguments.

```shell
uart:~$ matter config hardwarever
1 (0x1)
```

#### `deviceid` subcommand

Prints the device identifier. Takes no arguments.

#### `fabricid` subcommand

Prints the fabric identifier. Takes no arguments.

### `ble` command group

Handles a group of commands that are used to control the device Bluetooth LE
transport state. You must use this command together with one of the additional
subcommands listed below.

#### `help` subcommand

Prints help information about the `ble` command group.

```shell
uart:~$ matter ble help
  help            Usage: ble <subcommand>
  adv             Enable or disable advertisement. Usage: ble adv <start|stop|state>
```

#### `adv start` subcommand

Enables Bluetooth LE advertising.

```shell
uart:~$ matter ble adv start
Starting BLE advertising
```

#### `adv stop` subcommand

Disables Bluetooth LE advertising.

```shell
uart:~$ matter ble adv stop
Stopping BLE advertising
```

#### `adv status` subcommand

Prints the information about the current Bluetooth LE advertising status.

```shell
uart:~$ matter ble adv state
BLE advertising is disabled

```

### `nfc` command group

Handles a group of commands that are used to control the device NFC tag
emulation state. You must use this command together with one of the additional
subcommands listed below.

#### `start` subcommand

Starts the NFC tag emulation.

```shell
uart:~$ matter nfc start
NFC tag emulation started
```

#### `stop` subcommand

Stops the NFC tag emulation.

```shell
uart:~$ matter nfc stop
NFC tag emulation stopped
```

#### `state` subcommand

Prints the information about the NFC tag emulation status.

```shell
uart:~$ matter nfc state
NFC tag emulation is disabled
```

### `dns` command group

Handles a group of commands that are used to trigger performing DNS queries. You
must use this command together with one of the additional subcommands listed
below.

#### `browse` subcommand

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

#### `resolve` subcommand

Resolves the specified Matter node service given by the _fabric-id_ and
_node-id_.

```shell
uart:~$ matter dns resolve fabric-id node-id
Resolving ...
DNS resolve for 000000014A77CBB3-0000000000BC5C01 succeeded:
   IP address: fd08:b65e:db8e:f9c7:8052:1a8e:4dd4:e1f3
   Port: 5540
```

### `stat` command group

Handles a group of commands that are used to get and reset the peak usage of
critical system resources used by Matter. These commands are only available when
the `CONFIG_CHIP_STATISTICS=y` Kconfig option is set.

#### `peak` subcommand

Prints the peak usage of system resources.

```shell
uart:~$ matter stat peak
Packet Buffers: 1
Timers: 2
TCP endpoints: 0
UDP endpoints: 1
Exchange contexts: 0
Unsolicited message handlers: 5
Platform events: 2
```

#### `reset` subcommand

Resets the peak usage of system resources.

```shell
uart:~$ matter stat reset
```
