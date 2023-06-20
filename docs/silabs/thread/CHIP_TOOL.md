# Using the Mattertool (chip-tool)

The following commands show how to start a new Thread network from the local
OTBR, commission an EFR32 Matter End Device (Matter Accessory Device), and then
send the on/off commands with the `mattertool` automated script. The `mattertool` 
script provides an interface into various chip-tool and otbr commands used to create 
and interact with a Matter network

## Basic Mattertool Commands

| **Command**              | **Usage**                                                                 |
| ------------------------ | ------------------------------------------------------------------------- |
| `mattertool startThread` | Starts the thread network on the OTBR                                     |
| `mattertool bleThread`   | Starts commissioning of a Matter Accessory Device using the chip-tool      |
| `mattertool on`          | Sends the _on_ command to the Matter Accessory Device using the chip-tool  |
| `mattertool off`         | Sends the _off_ command to the Matter Accessory Device using the chip-tool |

You can also use the full chip-tool command set (still using mattertool)

```shell
$ mattertool levelcontrol read current-level 106 1
```

<br><br>

## Advanced Information on the Matter Hub

### Image tree

-   home
    -   ubuntu (you are here)
        -   connectedhomeip (git repo:
            https://github.com/project-chip/connectedhomeip.git)
            -   .
        -   ot-br-posix (git repo:
            https://github.com/openthread/ot-br-posix.git)
            -   .
        -   scripts (in-house scripts)
            -   configurations.sh
            -   matterTool.sh
            -   setupOTBR.sh

<br>

## Open Thread Border Router (OTBR)

For information on what commits to use for the OTBR and RCP, consult the
[Matter Repositories and Commit Hashes page](../general/COMMIT_HASHES.md)

The pre-installed OTBR is configured for the infrastructure interface eth0.

Bash script to modify, reinstall or update the OTBR:

```shell
$ otbrsetup
```

This bash script centralizes and simplifies the local OTBR installation.

Available commands:

| **Command**                    | **Description**                                                                                                                    |
| ------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------- |
| -h, --help                     | Prints help options                                                                                                                |
| -if, --interface <eth0\|wlan0> | Select infrastructure interface. Default eth0                                                                                      |
| -i, --install                  | Bootstrap, set up and install the OTBR. Usually for a new installation                                                             |
| -s, --setup                    | Runs the OTBR setup only, use this to change the configured infrastructure interface (use in combination with -if wlan0 for Wi-Fi) |
| -u, --update                   | Update the OTBR installation after the repo is updated                                                                             |

<br>

### Usage:

<br>

Change infrastructure to wlan0: `$ otbrsetup -if wlan0 -s` <br> Rerun full
install for eth0 interface: `$ otbrsetup -i`

## Upgrading the OpenThread Border Router (OTBR)

Change OTBR commit reference/version

```shell
$ cd /home/ubuntu/ot-br-posix
```

```shell
$ git fetch
```

```shell
$ git checkout <SHA>
```

```shell
$ otbrsetup -u
```

## Upgrading the Matter - Chip-tool

For more information on the commit hashes used for this demo please consult the
following page:
[Matter Repositories and Commit Hashes](../general/COMMIT_HASHES.md)

To change the chip-tool commit reference/version, follow these steps:

```shell
$ cd /home/ubuntu/connectedhomeip
```

```shell
$ git fetch
```

```shell
$ git checkout <SHA>
```

```shell
$ mattertool buildCT
```

The mattertool script centralizes and simplifies the use of chip-tool and
starting a clean thread network.

<br>

Available commands:

| **Command**  | **Description**                                                                                               |
| ------------ | ------------------------------------------------------------------------------------------------------------- |
| help         | Prints help options                                                                                           |
| startThread  | Start a new thread network and store the operational thread dataset for the commissioning purpose (bleThread) |
| bleThread    | For Matter Bluetooth LE thread commissioning with an EFR32 device                                             |
| bleWifi      | For Matter Bluetooth LE Wi-FI commissioning with an EFR32 device                                              |
| buildCT      | Clean build of the chip-tool                                                                                   |
| cleanVars    | Erase every Set variable used in the script. They will be set back to default or randomized value             |
| off          | Turn off the Light on the already-commissioned EFR32 device                                                   |
| on           | Turn on the Light on the already-commissioned EFR32 device                                                    |
| toggle       | Toggle the Light on the already-commissioned EFR32 device                                                     |
| parsePayload | Parse the given Payload (QrCode string)                                                                       |
| rebuildCT    | Rebuild the chip-tool                                                                                          |
| vars         | Print the Variables in use by the script                                                                      |

<br>

Some options/arguments can be added to the command to update the values of the
variables used by the script.

<br>

Available commands:

| **Command**              | **Description**                                       |
| ------------------------ | ----------------------------------------------------- |
| -h, --help               | Prints help options                                   |
| -n, --nodeId DIGIT       | Specify the Nodeid you are trying to reach            |
| -e, --endpoint DIGIT     | Specify an endpoint for the desired cluster           |
| -d, --dataset HEX_STRING | Thread Operation Dataset to be provisioned            |
| -s, --ssid STRING        | Wi-Fi AP SSID that the end devices need to connect to |
| -p, --password STRING    | Wi-Fi AP password                                     |

> These configurations are held until overwritten, cleared with cleanVars or
> when Raspberry Pi reboots.

<br>

Active variables used by mattertool:

| **Variable**    | **Value**                                             |
| --------------- | ----------------------------------------------------- |
| MATTER_ROOT     | /home/ubuntu/connectedhomeip                          |
| CHIPTOOL_PATH   | /home/ubuntu/connectedhomeip/out/standalone/chip-tool |
| NODE_ID         | 31354                                                 |
| THREAD_DATA_SET | \<the_value_you_get>                                  |
| PINCODE         | 20202021                                              |
| DISCRIMINATOR   | 3840                                                  |
| SSID            | \<your_SSID>                                          |
| lastNodeId      | 0                                                     |

You can preset them with export X=Y before running the script or use some
available options to change some of them.

In most cases, MATTER_ROOT, CHIPTOOL_PATH, PINCODE, and DISCRIMINATOR should
remain at the default set value.

For commissioning commands (bleThread, bleWifi) NODE_ID will be randomized if
it is the same as the last pairing

When the startThread command is used, THREAD_DATA_SET will be assigned with
the right operation dataset for the created Thread Network.

<br>

### Scripts Alias

The commands presented above are linked to scripts. You can edit **_.bashrc_**
and rename the following alias to your liking.

```shell
$ alias mattertool=`source $HOME/scripts/matterTool.sh`
```

```shell
$ alias otbrsetup=`source $HOME/scripts/setupOTBR.sh`
```
