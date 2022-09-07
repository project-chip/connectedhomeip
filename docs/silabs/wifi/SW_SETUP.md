# Software Setup and Preliminaries

## Using the Matter Accessory Device (MAD) Pre-Built Binaries

If you are just running the Matter demo, and are not interested in building the
Matter Accessory Device images from scratch, you can download the MAD images for
Wi-Fi from this software release on the
[Matter Artifacts Page](../general/ARTIFACTS.md)

Once you have downloaded the image you require for your device, you can skip
forward to the [Running the Matter Demo over Wi-Fi page](./RUN_DEMO.md)

Otherwise if you are planning to build the Wi-Fi images from scratch please
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

<br>

## Building Software

The following commands are for building the example. Depending on which device
you are using, select the appropriate build command to run.

Build command for EFR32MG12 + RS911x:

`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx --wifi rs911x |& tee out/rs911x_lighting.log`

Build command for EFR32MG12 + WF200:

`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32 out/wf200_lighting_app BRD41xxx is_debug=false --wifi wf200 |& tee out/wf200_lighting.log`

In above command BRD41xxx represent the MG12 boards

> `BRD4161A, BRD4163A, BRD4164A`

Build command for EFR32MG24 + RS911x:

`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD41xxx disable_lcd=true show_qr_code=false use_external_flash=false --wifi rs911x`

Build command for EFR32MG24 + WF200:

`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/wf200_lighting BRD41xxx disable_lcd=true show_qr_code=false use_external_flash=false chip_build_libshell=false --wifi wf200`

In above command BRD41xxx represent the MG24 boards

> `BRD4186C, BRD4187C`

Run the following:

```shell
$ cd matter
$ <appropriate_build_command_from_above>
```

The generated software can be found in `out/rs911x_xxx/BRD4161A/*.out` for the
RS9116 and in `out/wf200_xxx/BRD4161A/*.out` for the WF200.

This is what you will flash onto the EFR32. For more information on how to flash
the EFR32 please check out the page on
[Flashing a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)

## Compiling the ChipTool

In order to control the Wi-Fi Matter Accessory Device you will have to compile
and run the ChipTool on either a Linux, Mac or Raspberry Pi. The ChipTool builds
faster on the Mac and Linux machines so that is recommended, but if you have
access to a Raspberry Pi that will work as well.

1. Build the Chip-Tool

    `$ ./scripts/examples/gn_build_example.sh examples/chip-tool out/standalone`

    This will build chiptool in `out/standalone`.

    After this, follow the steps on the page
    '[Running the Matter Demo over Wi-Fi](RUN_DEMO.md)' to flash the binaries
    and execute the demo.

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
