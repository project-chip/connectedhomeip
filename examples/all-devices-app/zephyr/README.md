# Matter All-Devices App for Zephyr

The All-Devices App is a Matter application for Zephyr. It uses the Matter
code-driven data model and can contain one or more device types. The enabled
device types are selected at build time. The default device is an on/off light.

## Setup

Follow the generic Zephyr setup:

https://docs.zephyrproject.org/latest/develop/getting_started/index.html

Then return to the Matter repository and run the Matter bootstrap:

```sh
cd $HOME/src/connectedhomeip
. ./scripts/bootstrap.sh
```

Make sure the Zephyr virtual environment is first in `PATH`:

```sh
export PATH="$HOME/zephyrproject/.venv/bin:$PATH"
```

## Build

Within the `$HOME/zephyrproject/` directory, build the app with West and
sysbuild:

```sh
west build -p always -b xg26_rb4118a --sysbuild \
	$HOME/src/connectedhomeip/examples/all-devices-app/zephyr \
	-d build/all-devices-zephyr-xg26 -- -DUSE_CCACHE=0
```

Flash the resulting images with:

```sh
west flash -d build/all-devices-zephyr-xg26
```

The app supports MCUboot through Zephyr sysbuild. For debugging, use
`west debug`. Use `west attach` when debugging an image that is already signed.

OTA is supported through the Matter OTA Requestor and the Zephyr platform OTA
integration.

## Build Profiles

`prj.conf` is applied by default. Select another profile with `FILE_SUFFIX`:

```sh
# Debug profile
west build ... -- -DFILE_SUFFIX=debug

# Release profile
west build ... -- -DFILE_SUFFIX=release
```

## Device Selection

Change the enabled device types at build time with
`ALL_DEVICES_ENABLED_DEVICES`:

```sh
west build ... -- \
	-DALL_DEVICES_ENABLED_DEVICES="on-off-light;occupancy-sensor"
```

## Factory Reset

Press and hold button 0 for 5 seconds to factory reset the device. Button 0 is
defined as `sw0` in a board overlay.

## Custom AppTask

A product can provide its own `AppTask` subclass and override only the behavior
it needs to change. Set `APP_TASK_INSTANCE_SOURCE` to the source file that
defines `GetAppTask()`:

```sh
west build ... -- \
	-DAPP_TASK_INSTANCE_SOURCE=/path/to/MyAppTaskInstance.cpp
```
