# Flashing Scripts

## Using

### \${TARGET}.flash.py

When a build target has a corresponding `.flash.py` script, that script can be
run to flash the target to an attached device.

This is really just a wrapper around `${PLATFORM}_firmware_utils.py` with
built-in argument defaults, so that it can typically be run with no arguments.
You can however supply additional command line arguments to override or
supplement the defaults, for example to select a specific connected device. In
particular, `${TARGET}.flash.py --help` lists the available arguments.

### \${PLATFORM}\_firmware_utils.py

These scripts invoking flashing tools for the particular platform with a
more-or-less uniform interface. The core set of command line options is

    --erase               Erase device
    --application FILE    Flash an image
    --verify_application  Verify the image after flashing
    --reset               Reset device after flashing

Running `${PLATFORM}_firmware_utils.py --help` will show the complete list of
available command line options for that platform.

## Generating wrappers

Normally this is done automatically for a suitable GN build target.

The script `gen_flashing_script.py` builds a `${TARGET}.flash.py` wrapper script
to invoke the firmware utils with a particular set of argument defaults. Run
this as

> `gen_flashing_script.py ${PLATFORM} --output` _filename_ [*argument* ...]

The _arguments_ are the same as those of `${PLATFORM}_firmware_utils.py`. Some
platforms may have additional options, e.g. to obtain additional values from a
configuration file. Use `gen_flashing_script.py ${PLATFORM} --help` to list all
options for a particular platform.
