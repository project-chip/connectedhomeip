# Software Setup and Preliminaries

## Using the Matter Accessory Device (MAD) Pre-Built Binaries

If you are just running the Matter demo, and are not interested in building the
Matter Accessory Device images from scratch, you can download the MAD images for
Wi-Fi from this software release on the
[Matter Artifacts Page](../general/ARTIFACTS.md)

Once you have downloaded the image you require for your device, you can skip
forward to the [Running the Matter Demo over Wi-Fi page](./RUN_DEMO.md)

If you are planning to build the Wi-Fi images from scratch, 
continue with this documentation.

## Software Setup

If you have not downloaded or cloned this repository, you can run the following
commands on a Linux terminal running on either Linux machine, WSL or Virtual
Machine to clone the repository and run bootstrap to prepare to build the sample
application images.

1. To download the
   [SiliconLabs Matter codebase](https://github.com/SiliconLabs/matter.git) run
   the following commands.

    ```shell
     $ git clone https://github.com/SiliconLabs/matter.git
    ```

2. Bootstrapping:

    ```shell
    $ cd matter
    $ ./scripts/checkout_submodules.py --shallow --recursive --platform efr32
    $ . scripts/bootstrap.sh
    # Create a directory where binaries will be updated/compiled called `out`
    $ mkdir out
    ```

## Compiling the chip-tool

In order to control the Wi-Fi Matter Accessory Device you will have to compile
and run the chip-tool on either a Linux, Mac or Raspberry Pi. The chip-tool builds
faster on the Mac and Linux machines so that is recommended, but if you have
access to a Raspberry Pi that will work as well.

1. Build the chip-tool

    ```shell
    $ ./scripts/examples/gn_build_example.sh examples/chip-tool out/standalone
    ```

    This will build chip-tool in `out/standalone`.

## Building the Matter Application

The following commands are for building the Matter application. Depending on which device
you are using, select the appropriate command to build.

> **Note:** 
> The following build commands are for the `lighting-app` application. In order to build different applications, for example `lock-app`, `window-app` or `thermostat`, substitute the appropriate application name.
> Additional examples are provided in the [/examples](../../../examples/) directory,
or [/silabs_examples](../../../silabs_examples/) (such as `onoff-plug-app`).

Run the following:

```shell
$ cd matter
$ <run_appropriate_build_command_from_below>
```

Build command for EFR32MG12 + RS9116:

```shell
$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx --wifi rs911x |& tee out/rs911x_lighting.log
```

Build command for EFR32MG12 + WF200:

```shell
$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32 out/wf200_lighting_app BRD41xxx is_debug=false chip_logging=false --wifi wf200 |& tee out/wf200_lighting.log
```

> **Note:** The image size currently exceeds the available flash with CHIP logging enabled.

Build command for EFR32MG24 + RS9116:

```shell
$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx disable_lcd=true use_external_flash=false --wifi rs911x |& tee out/rs911x_lighting.log
```

Build command for EFR32MG24 + SiWx917:

```shell
$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/siwx917_lighting BRD41xxx disable_lcd=true use_external_flash=false --wifi rs911x |& tee out/siwx917_lighting.log
```

Build command for EFR32MG24 + WF200:

```shell
$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/wf200_lighting BRD41xxx disable_lcd=true use_external_flash=false chip_build_libshell=false --wifi wf200 |& tee out/wf200_lighting.log
```

A complete list of hardware supported is included on the [Hardware Requirements page](../general/HARDWARE_REQUIREMENTS.md).

Enable or disable the lighting application's features using the following flags.

1.  `rs91x_wpa3_only` : Use this flag while building to enable wpa3 mode in rs91x wifi chip.

    ```shell
    $ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx rs91x_wpa3_only=true --wifi rs911x |& tee out/rs911x_lighting.log
    ```

> **Note:** 
> 1. WPA/WPA2 is enabled by default for the rs911x
> 2. Enabling WPA3 will disable WPA and WPA2 support

2.  `segger_rtt_buffer_size_up` : Flag to get the complete logs without truncation.

    ```shell
    $ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx segger_rtt_buffer_size_up=2068 --wifi rs911x |& tee out/rs911x_lighting.log
    ```
3.  `show_qr_code=false` : Use this flag while building to disable QR code.

    ```shell
    $ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx show_qr_code=false --wifi rs911x |& tee out/rs911x_lighting.log
    ```
>    **Note:**
>    1. QR code is enabled by default for all except MG24
>    2. QR code is disabled for MG24 because of lcd disable. It cannot be enabled using the flag.

4. `chip_enable_wifi_ipv4` : Use this flag while building to enable IPV4 (disabled by default).

    ```shell
    ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx chip_enable_wifi_ipv4=true --wifi rs911x |& tee out/rs911x_lighting.log
    ```

The generated software can be found in
`out/rs911x_xxx/BRD41xxx/*.out` for the RS9116, in `out/siwx917_xxx/BRD41xxx/*.out`  for the
SiWx917 and in `out/wf200_xxx/BRD41xxx/*.out` for the WF200.

This is what you will flash onto the EFR32. For more information on how to flash
the EFR32 see
[Flashing a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)

**[Optional:** Increasing stack size **]** 

Navigate to
`matter` and open the file in the path
`examples/platform/efr32/FreeRTOSConfig.h`. Find the macro:
\``configMINIMAL_STACK_SIZE`\`, and change the macro value from `140` to
 **`320`**.

<br>

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
