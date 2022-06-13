# MATTER CHEF APP

The purpose of the chef app is to to increase the coverage of device types in
Matter.

It uses the shell app a starting point, adding the processing of ZAP files and
the support of a few targets under a unified build script: `chef.py`.

As it incorporates the processing of ZAP files as part of the build process, it
does not use `zzz_generated`, but rather places the auto-generated zap artifacts
under its `zap-generated` temporary folder.

All device types available (DM/IM .zap files) are found inside the `devices`
folder.

## Building a Sample Application

Run `chef.py -h` to see the available commands

## Building your first sample

1. Make sure you have the toolchain installed for your desired target
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

4. Run `$ chef.py -u` to update zap and the toolchain (on selected platforms)
5. Run `$ chef.py -gzbf -t <platform> -d lighting`. This command will run the
   ZAP GUI opening the `devices/lighting.zap` file and will allow editing. It
   will then generate the zap artifacts, place them on the `zap-generated`
   folder, run a build and flash the binary in your target

## Creating a new device type in your device library

1. Run `$ chef.py -g -d <device>` to open in the ZAP GUI a device to be used as
   a starting point.
2. Edit your cluster configurations
3. Click on `Save As` and save the file with the name of your new device type
   into the `devices` folder. This device is now available for the script. See
   `chef.py -h` for a list of devices available.

## CI

### zzz_generated

To eliminate a dependency on ZAP in CI jobs, all chef examples found in
`examples/chef/devices` must have their output from the ZAP tool cached in
`examples/chef/zzz_generated`.

To generate the cache, one may execute chef with the option `--generate_zzz`.
This will run ZAP for all devices in `examples/chef/devices` and place the
output into the appropriate directory structure.

Other than the output from the ZAP tool, the cache directory contains two
additional files for each device:

-   `INPUTMD5.txt` contains the md5 hex digest of the ZAP file used to generate
    the directory.
-   `ZAPSHA.txt` contains the commit of ZAP in the user's tree when the
    directory was generated.

```
zzz_generated/
└── lighting-app
    ├── INPUTMD5.txt
    ├── zap-generated
    │   ├── access.h
    │   ├── af-gen-event.h
    │   └── ...
    └── ZAPSHA.txt
```

These additional files will be used by the CI jobs to validate whether the cache
must be regenerated i.e. regeneration is needed when ZAP or the input ZAP files
change.

### Workflow

All CI jobs for chef can be found in `.github/workflows/chef.yaml`.

#### Validate

The workflow begins by calling chef with `--validate_zzz`.

`--validate_zzz` will recalculate the current ZAP commit and the md5 of all
example ZAP files and compare with what is committed to `zzz_generated`.

If the validation job fails, it will provide instructions to repair
`zzz_generated` and no builds will run.

#### Build

Once the validation job is complete, there is a separate job for each platform,
which run in parallel.

These jobs use a platform-specific image with base `chip-build`.

The build jobs call chef with the options `--ci -t <PLATFORM>`. The `--ci`
option will execute builds for all devices specified in `_CI_ALLOW_LIST` defined
in `chef.py` (so long as these devices are also in `/devices`) on the specified
platform.
