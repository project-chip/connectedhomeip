# Building a Wi-Fi End Device for Matter in GitHub

## Software Setup

If you have not downloaded or cloned this repository, you can run the following
commands on a Linux terminal running on either Linux machine, WSL or Virtual
Machine to clone the repository and run bootstrap to prepare to build the sample
application images. Users may need to run the various commands as the root user or with certain privilges enabled.

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
3. Troubleshooting the errors:
   
   1. For resolving [Git Submodule Error](./images/git_submodule_error.png), run below command:
      ```shell
      $ git submodule update --init --checkout
      ```
   2. For resolving [Bootstrapping Error](./images/Boostrapping_Error.png), run below command:
      ```shell
      $ pip install --upgrade prompt-toolkit
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

## Using the Matter Accessory Device (MAD) Pre-Built Binaries

If you are just running the Matter demo, and are not interested in building the
Matter Accessory Device images from scratch, you can download the MAD images for
Wi-Fi from this software release on the
[Matter Artifacts Page](../general/ARTIFACTS.md)

Once you have downloaded the image you require for your device, you can skip
forward to the instructions for running the demo.

  - **For EFR32MG24 host processors**

    - [Running the Matter Demo for EFR32 device over Wi-Fi page](./RUN_DEMO.md)

  - **For SiWx917 SoC processor**

    - [Running the Matter Demo for SiWx917 SoC device over Wi-Fi page](./RUN_DEMO_SiWx917_SoC.md)

If you are planning to build the Wi-Fi images from scratch, 
continue with this documentation.

## Building the Matter Application

Depending on the host processdor used and the application required, select the appropriate build command :

```shell
$ cd matter
$ <run_appropriate_build_command_from_below>
```
Syntax for the build command:
```shell
$ ./scripts/examples/gn_silabs_example.sh <path to the application code> <out folder for the generated binary> <Device/HW for which the binary is built> <build parameters if required>
```

>    **Note:**
>    1. Build commands should not be executed under root user.
>    2. The build commands given below are for the `lighting-app` application.
>    3. In order to build applications other than the `lighting-app`, (such as `thermostat-app`, `lock-app`, `light-switch-app`, `window-app`, `onoff-plug-app`), substitute the appropriate application path in the build command.
>    4. Path to the sample application examples are provided in the [/examples](https://github.com/SiliconLabs/matter/blob/latest/examples/) or [/silabs_examples](https://github.com/SiliconLabs/matter/blob/latest/silabs_examples/) directory.
>    5. To build for EFR32MG24 / 917 SoC host processors, substitute `BRD41xxx` in the build command with the appropriate board number from the [Hardware Requirements Page](../general/HARDWARE_REQUIREMENTS.md).

Build command for EFR32MG24 + RS9116:

```shell
$ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting BRD41xxx --wifi rs9116
```

Build command for EFR32MG24 + SiWx917:

```shell
$ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_lighting BRD41xxx disable_lcd=true use_external_flash=false --wifi SiWx917
```

Build command for EFR32MG24 + WF200:

```shell
$ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/wf200_lighting BRD41xxx chip_build_libshell=false --wifi wf200
```

Build command for SiWx917 SoC processor(common flash):

```shell
$ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/SiWx917_lighting BRD4338A
```

>    **Note:**
>    1. If the build fails during the creation of the .rps file in WSL, then sym link commander with the proper path to the executable using below command and rebuild the application.
>    ```shell
>    cd .local/bin
>    ln -s <commander_path>/commander.exe commander
>    ```

Enable or disable the application's features using the following build parameters.

1.  `segger_rtt_buffer_size_up` : Flag to get the complete logs without truncation.

    ```shell
    $ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting BRD41xxx segger_rtt_buffer_size_up=2068 --wifi rs9116
    ```
    
2.  `disable_lcd=true` : Use this flag while building to disable the LCD.
    
     ```shell
    $ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting BRD41xxx disable_lcd=true --wifi rs9116
    ```
    
3.  `show_qr_code=false` : Use this flag while building to disable the QR code.

    ```shell
    $ ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting BRD41xxx show_qr_code=false --wifi rs9116
    ```
>    **Note:**
>    1. QR code is enabled by default for all except MG24
>    2. QR code is disabled for MG24 because of lcd disable. It cannot be enabled using the flag.

4. `chip_enable_wifi_ipv4` : Use this flag while building to enable IPV4 (disabled by default).

    ```shell
    ./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting BRD41xxx chip_enable_wifi_ipv4=true --wifi rs9116 
    ```

The generated software binaries can be found in the `out/` folder created during the build.
Once you have downloaded the binaries for your device, you can follow the instructions for running the demo.

  - **For EFR32MG24 host processors**

    - [Running the Matter Demo for EFR32 device over Wi-Fi page](./RUN_DEMO.md)

  - **For SiWx917 SoC processor**

    - [Running the Matter Demo for SiWx917 SoC device over Wi-Fi page](./RUN_DEMO_SiWx917_SoC.md)

**[Optional:** Increasing stack size **]** 

Navigate to
`matter` and open the file in the path
`examples/platform/silabs/efr32/FreeRTOSConfig.h`. Find the macro:
\``configMINIMAL_STACK_SIZE`\`, and change the macro value from `140` to
 **`320`**.
