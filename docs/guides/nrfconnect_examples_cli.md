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

When you reboot the board, you will see the boot logs in the console, similar to
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
