# Using CLI in nRF Connect SDK examples

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

The nRF Connect SDK examples let you use several CHIP-specific CLI commands.

These commands are not available in the standard Zephyr shell.

They are currently used for testing purposes and allow only to get some
information about CHIP stack state, but not to modify it.

To enable using CHIP commands, set the `CONFIG_CHIP_ZEPHYR_SHELL=y` Kconfig
option in the `prj.conf` file of the given example.

### Listing CHIP-specific commands

To list all available CHIP-specific commands, enter `chip` in the command line
and press the Tab key. This will list the available commands:

```shell
uart:~$ chip
  qrcode             qrcodeurl          setuppincode       discriminator
  vendorid           productid          manualpairingcode  bleadvertising
  nfcemulation
```

See the following subsections for the description of each CHIP-specific command.

#### `qrcode`

Prints the device
[onboarding QR code payload](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device).
Takes no arguments.

```shell
uart:~$ chip qrcode
MT:W0GU2OTB00KA0648G00
```

#### `qrcodeurl`

Prints the URL to view the
[device onboarding QR code](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/nrfconnect_android_commissioning.md#preparing-accessory-device)
in a web browser. Takes no arguments.

```shell
uart:~$ chip qrcodeurl
https://dhrishi.github.io/connectedhomeip/qrcode.html?data=CH%3AH34.GHY00%200C9SS0
```

#### `setuppincode`

Prints the PIN code for device setup. Takes no arguments.

```shell
uart:~$ chip setuppincode
12345678
```

#### `discriminator`

Prints the device setup discriminator. Takes no arguments.

```shell
uart:~$ chip discriminator
3840
```

#### `vendorid`

Prints the vendor ID of the device. Takes no arguments.

```shell
uart:~$ chip vendorid
9050
```

#### `productid`

Prints the product ID of the device. Takes no arguments.

```shell
uart:~$ chip productid
20043
```

#### `manualpairingcode`

Prints the pairing code for the manual onboarding of a device. Takes no
arguments.

```shell
uart:~$ chip manualpairingcode
35767807533
```

#### `bleadvertising`

Prints the information about the Bluetooth LE advertising status, either `0` if
the advertising is disabled on the device or `1` if it is enabled. Takes no
arguments.

```shell
uart:~$ chip bleadvertising
0
```

#### `nfcemulation`

Prints the information about the NFC tag emulation status, either `0` if the
emulation is disabled on the device or `1` if it is enabled (1). Takes no
arguments.

```shell
uart:~$ chip nfcemulation
0
```
