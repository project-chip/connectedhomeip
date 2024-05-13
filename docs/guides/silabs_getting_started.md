# Silicon Labs Matter Solution Guide

## Introduction

The Silabs Matter Solution Guide explains how to use the Silabs Matter offering.
It explains how to set up the development environment, build and flash a Silabs
sample app.

![Silabs logo](./images/silabs_logo.png)

> **NOTE:** Silicon Laboratories now maintains a public Matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs Matter Github](https://github.com/SiliconLabs/matter/releases)
>
> Developers can find more resources on the
> [Silicon Labs Matter Community Page](https://community.silabs.com/s/article/connected-home-over-ip-chip-faq?language=en_US).

-   [Introduction](#introduction)
-   [Requirements](#requirements)
    -   [Hardware Requirements](#hardware-requirements)
    -   [Software Requirements](#software-requirements)
    -   [Software Artifacts](#software-artifacts)
-   [Building](#building)
    -   [Build Script](#build-script)
    -   [Build Arguments](#build-arguments)
-   [Flashing](#flashing)
    -   [Flasher Arguments](#flasher-arguments)
-   [Standard Application Behavior](#standard-application-behavior)

## Requirements

### Hardware Requirements

For the list of hardware requirements, see the official
[Silicon Labs Matter HW requirements](https://siliconlabs.github.io/matter/latest/general/HARDWARE_REQUIREMENTS.html)
documentation.

### Software Requirements

For the list of software requirements, see the official
[Silicon Labs Matter Software requirements](https://siliconlabs.github.io/matter/latest/general/SOFTWARE_REQUIREMENTS.html)
documentation.

#### Software Artifacts

For pre-built binaries for the latest Silicon Labs Matter release, see the
official
[Silicon Labs Matter Software Artifacts](https://siliconlabs.github.io/matter/latest/general/ARTIFACTS.html#matter-software-artifacts).
This includes all necessary binaries to run a Silicon Labs sample app.

## Building

Silicon Labs currently supports the following list of sample apps in the main
Matter SDK. Every sample has its own documentation explaining its unique
features and functionalities. The examples in the `CSA Matter Repository` column
are supported in the main Matter SDK. Additionally, the
[Silabs Matter Repository](https://github.com/SiliconLabs/matter) offers extra
sample applications for different device-types

<table>
    <tbody>
        <tr>
            <th>CSA Matter Repository</th>
            <th> <a href="https://github.com/SiliconLabs/matter">Silabs Matter Repository</a></th>
        </tr>
        <tr>
          <td>
            <ul>
                <li> <a href="../../lighting-app/silabs/README.md">Lighting App</a></li>
                <li> <a href="../../light-switch-app/silabs/README.md)">Light-Switch App</a></li>
                <li> <a href="../../chef/README.md">Chef App</a></li>
                <li> <a href="../../lock-app/silabs/README.md">Lock App</a></li>
                <li> <a href="../../pump-app/silabs/README.md">Pump App</a></li>
                <li> <a href="../../smoke-co-alarm-app/silabs/README.md">Smoke & CO Alarm App</a></li>
                <li> <a href="../../thermostat/silabs/README.md">Thermostat App</a></li>
            </ul>
          </td>
          <td>
            <ul>
                <li><a href="https://github.com/SiliconLabs/matter/blob/release_2.2.0-1.2/silabs_examples/dishwasher-app/silabs/README.md">Dishwasher App</a></li>
                <li><a href="https://github.com/SiliconLabs/matter/blob/release_2.2.0-1.2/silabs_examples/onoff-plug-app/README.md">On-Off Plug App</a></li>
                <li><a href="https://github.com/SiliconLabs/matter/blob/release_2.2.0-1.2/silabs_examples/silabs-sensors/README.md">Multi Sensor App</a></li>
            </ul>
          </td>
        </tr>
    </tbody>
</table>

### Build Script

To build a Silicon Labs sample apps, we provide the `gn_silabs_examples.sh`
scripts that can be found in the `./scripts/examples` directory. The build
script can be used to build all of the Silabs supported examples. The command
structure is as follows when called from the root of the repository:

```shell
./scripts/examples/gn_silabs_example.sh <path_to_sample_app> <output_directory> <board> <args>
```

To build the lighting app as an OpenThread SoC, the default build command for
the BRD4187C DK is

```shell
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs/ ./out/lighting-app BRD4187C
```

To build the lighting app as an Wi-Fi MG24 + RS9116 NCP, the default build
command for the BRD4187C is

```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/lighting-app_rs9116 BRD4187C use_external_flash=false chip_enable_ble_rs911x=true --wifi rs9116
```

> **Note**: The build argument `--wifi rs9116` is necessary to build the
> BRD4187C image with the necessary code for the NCP combo.
> `chip_enable_ble_rs911x=true` enables the RS9116 NCP bluetooth. The MG24 +
> RS9116 NCP combo does not yet support external flash.

To build the lighting app as an Wi-Fi MG24 + SiWx917 NCP, the default build
command for the BRD4187C is

```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/lighting-app_siwx917 BRD4187C use_external_flash=false chip_enable_ble_rs911x=true --wifi SiWx917
```

> **Note**: The build argument `--wifi SiWx917` is necessary to build BRD4187C
> image with the necessary code for the NCP combo. `chip_enable_ble_rs911x=true`
> enables the RS9116 NCP bluetooth. The MG24 + SiWx917 NCP combo does not yet
> support external flash.

To build the lighting app as an Wi-Fi MG24 + wf200 NCP, the default build
command for the BRD4187C is

```shell
$ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/lighting-app_wf200 BRD4187C --wifi wf200
```

> **Note**: The build argument `--wifi wf200` is necessary to build the BRD4187C
> image with the necessary code for the NCP combo.

#### Build Arguments

The ``gn_silabs_examples.sh` script takes two types of build arguments. The
first type are macros processed within the script itself and the second are GN
arguments. The Macros encapsulate multiple GN arguments to simplify enabling or
disabled specific features.

> **Note**: Executing the build script without any arguments will print a helper
> with the command structure, the list of supported boards, and a list of
> supported macros and arguments
>
> ```sh
> ./scripts/examples/gn_silabs_examples.sh
> ```

Here is a list of some the supported macros and their GN argument equivalent.

|          Macro Name           | Description                                                                                            | GN equivalent                                                                                                                                              |
| :---------------------------: | :----------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------- |
|            --wifi             | Configures an sample app as a Wi-Fi devices.<br /> This macro requires rs9116 or SiWx917 or wf200.     | --wifi rs9116 : use_rs9116=true<br /> --wifi SiWx917 : use_SiWx917=true<br /> --wifi wf200 : use_wf200=true<br />                                          |
|             --icd             | Configures the device as an ICD                                                                        | chip_enable_icd_server=true chip_openthread_ftd=false                                                                                                      |
|          --low-power          | Configures the most power efficient build.<br /> This is used in tandem with the `--icd` macro         | chip_build_libshell=false enable_openthread_cli=false show_qr_code=false disable_lcd=true                                                                  |
|    --chip_enable_wifi_ipv4    | Enables IPv4 support on Wi-fi configured builds                                                        | chip_enable_wifi_ipv4=true chip_inet_config_enable_ipv4=true                                                                                               |
|            --clean            | Cleans the output directory before building                                                            | NA                                                                                                                                                         |
| --additional_data_advertising | Enable additional data advertising and rotating device ID                                              | chip_enable_additional_data_advertising=true chip_enable_rotating_device_id=true                                                                           |
|         --use_ot_lib          | Builds the sample app with the Silabs certified OpenThread libraries                                   | use_silabs_thread_lib=true chip_openthread_target=\$SILABS_THREAD_TARGET openthread_external_platform=\"""\"                                               |
|       --use_ot_coap_lib       | Builds the sample app with the Silabs certified OpenThread COAP libraries                              | use_silabs_thread_lib=true chip_openthread_target=\$SILABS_THREAD_TARGET openthread_external_platform=\"""\" use_thread_coap_lib=true                      |
|           --release           | Remove all logs and debugs features (including the LCD). <br />Yields the smallest image size possible | is_debug=false disable_lcd=true chip_build_libshell=false enable_openthread_cli=false use_external_flash=false chip_logging=false silabs_log_enabled=false |
|         --bootloader          | Adds a bootloader to the built image                                                                   | NA                                                                                                                                                         |
|          --uart_log           | Forwards logs to uart instead of RTT                                                                   | sl_uart_log_output=true                                                                                                                                    |

Here is a list of some of the GN arguments that can be added to the build
command.

> **Note**: All GN arguments can be added to the build.
> `gn args --list <output_directory>` can be used to list all GN arguments.

|      GN argument       | Description                                                                                                                     | Default Value                  |
| :--------------------: | :------------------------------------------------------------------------------------------------------------------------------ | :----------------------------- |
|  chip_build_libshell   | Enables the Matter Shell                                                                                                        | false                          |
|  chip_openthread_ftd   | Defines if the OpenThread device is an FTD (true) or an MTD (false)                                                             | true                           |
|     efr32_sdk_root     | Location for an alternate Gecko SDK                                                                                             | ./third_party/silabs/gecko_sdk |
| enable_heap_monitoring | Monitor & log memory usage at runtime                                                                                           | false                          |
| enable_openthread_cli  | Enables the OpenThread cli                                                                                                      | true                           |
|    kvs_max_entries     | Set the maximum KVS entries that can be stored in NVM <br /> Thresholds: 30 <= kvs_max_entries <= 255                           | 255                            |
| chip_enable_icd_server | Configure device as an intermittently connected device <br /> For Thread builds, chip_openthread_ftd must also be set to false. | false                          |
|      disable_lcd       | Disable the LCD on devices with an LCD                                                                                          | false                          |
|      show_qr_code      | Enables QR code on LCD for devices with an LCD                                                                                  | true                           |

On top of the GN arguments specified here, each sample app will specify, if need
be, the GN arguments specific to it.

## Flashing

The Matter SDK provides a standard way of flashing a sample app binary onto
hardware. After completing a build, a python script is generated that can be
used to flash the binary. The naming structure of the file is

```sh
matter-silabs-<sample_name>-example.flash.py
```

For example, the lighting-app flasher script will be named

```py
matter-silabs-lighting-example.flash.py
```

To execute the script, the following command can be used:

```sh
python3 <path>/matter-silabs-lighting-example.flash.py
```

where `<path>` is the path to the output directory used in the build script.

> **Note**: It is also possible to flash the built binary with commander
> directly. The commander command is
> `commander flash <path>/matter-silabs-lighting-example.<s37/rps>` <br /> The
> `.s37` binaries are used with the MGM24 and EFR32 families while the `.rps`
> binaries are only used for the SiWx917 SoC family.

### Flasher Arguments

The flashing script provides configuration arguments and operation arguments.
The configuration arguments are used to configure the operation arguments.

Here is a list of the configuration arguments that can be added to the flasher
script.

|       Argument        | Description                                                                                                                |
| :-------------------: | :------------------------------------------------------------------------------------------------------------------------- |
|     --verbose, -v     | Report more verbosely                                                                                                      |
|   --commander FILE    | Path to the commander executable                                                                                           |
|  --device, -d DEVICE  | Device family or platform to target (EFR32, MGM24, 917)                                                                    |
| --serialno, -s SERIAL | Serial number of device to flash. <br /> This argument is necessary when multiple boards of the same family are connected. |
|   --ip, -a ADDRESS    | IP Address of the device to flash                                                                                          |

Here is a list of the operations arguments that can be added to the flasher
script.

|       Argument       | Description                                                                                                         | Commander equivalent         |
| :------------------: | :------------------------------------------------------------------------------------------------------------------ | :--------------------------- |
|       --erase        | Erase the devices flash. <br /> This options completely wipes the devices flash including factory data.             | `commander device masserase` |
|  --application FILE  | Specify which binary to flash. <br /> The flasher script provides a default path to the binary that was just built. | `commander flash`            |
|       --reset        | Reset device after flashing                                                                                         | `commander device reset`     |
|     --skip-reset     | Do not reset device after flashing                                                                                  | `commander flash --noreset`  |
| --verify-application | Verify the image after flashing                                                                                     | `commander verify`           |

Executing the flasher scripts with the `--help / -h` arguments will print a help
menu with all the possible arguments.

> **Note**: For a wider range of features, the commander tool can be used
> directly. Running `commander --help / -h` will list all the available options
> of the tool.

## Standard Application Behavior

See the [Standard Application documentation](./silabs_common_app_behavior.md)
for behaviors that are common to all sample apps.

## Silabs CLI

See the [Silabs CLI documentation](./silabs_cli_guide.md) for more information
on the provided cli commands.
