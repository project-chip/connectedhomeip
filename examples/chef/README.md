# MATTER CHEF APP

The purpose of the chef app is to to:

1. Increase the coverage of device types in Matter
2. Provide a sample application that may have its data model easily configured.

Chef uses the shell app a starting point, but processes the data model defined
on ZAP files during build time. This procedure is handled by its unified build
script: `chef.py`.

As it incorporates the processing of ZAP files as part of the build process, it
does not use `zzz_generated`, but rather places the auto-generated zap artifacts
under its `out` temporary folder.

All device types available (.zap files) are found inside the `devices` folder.

## Building your first sample

1. Make sure you have the toolchain installed for your desired target.
2. Run `chef.py` the first time to create a `config.yaml` configuration file. If
   you already have SDK environment variables such as IDF_PATH (esp32) and
   ZEPHYR_BASE (nrfconnect) it will use those values as default.
3. Update your the SDK paths on `config.yaml`. TTY is the path used by the
   platform to enumerate its device as a serial port. Typical values are:

```
    # ESP32 macOS

    TTY: /dev/tty.usbmodemXXXXXXX

    # ESP32 Linux

    TTY: /dev/ttyACM0

    # NRFCONNECT macOS

    TTY: /dev/tty.usbserial-XXXXX

    # NRFCONNECT Linux

    TTY: /dev/ttyUSB0
```

4. Run `$ chef.py -u` to update zap and the toolchain (on selected platforms).
5. Run `$ chef.py -gzbf -t <platform> -d lighting`. This command will run the
   ZAP GUI opening the `devices/lighting.zap` file and will allow editing. It
   will then generate the zap artifacts, place them on the `zap-generated`
   folder, run a build and flash the binary in your target.
6. Run `chef.py -h` to see all available commands.

## Creating a new device type in your device library

1. Run `$ chef.py -g -d <device>` to open in the ZAP GUI a device to be used as
   a starting point.
2. Edit your cluster configurations
3. Click on `Save As` and save the file with the name of your new device type
   into the `devices` folder. This device is now available for the script. See
   `chef.py -h` for a list of devices available.

## Folder Structure and Guidelines

-   `<platform>`: build system and `main.cpp` file for every supported platform.
    When porting a new platform, please minimize the source code in this folder,
    favoring the `common` folder for code that is not platform related.
-   `common`: contains code shared between different platforms. It may contain
    source code that enables specific features such as `LightingManager` class
    or `LockManager`, as long as the application dynamically identify the
    presence of the relevant cluster configurations and it doesn't break the use
    cases where chef is built without these clusters.
-   `devices`: contains the data models that may be used with chef. As of Matter
    1.0 the data models are defined using .zap files.
-   `out`: temporary folder used for placing ZAP generated artifacts.
-   `sample_app_util`: guidelines and scripts for generating file names for new
    device types committed to the `devices` folder.
-   `config.yaml`: contains general configuration for the `chef.py` script. As
    of Matter 1.0 this is used exclusively for toolchain and TTY interface
    paths.
-   `chef.py`: main script for generating samples. More info on its help
    `chef.py -h`.

## CI

All CI jobs for chef can be found in `.github/workflows/chef.yaml`.

These jobs use a platform-specific image with base `chip-build`.

CI jobs call chef with the options `--ci -t <PLATFORM>`. The `--ci` option will
execute builds for all devices specified in `cicd_config["ci_allow_list"]`
defined in `chef.py` (so long as these devices are also in `/devices`) on the
specified platform.
