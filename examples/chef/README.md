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
2. Update your SoC SDK paths on `chef_config.py` and flip the `configured`
   variable to True
3. Run `$ chef.py -u` to update zap and the toolchain (on selected platforms)
4. Run `$ chef.py -gzbf -t <platform> -d lighting`. This command will run the
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
