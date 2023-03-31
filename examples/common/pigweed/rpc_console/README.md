---
orphan: true
---

# CHIP RPC CONSOLE

This python application provides a console for interacting with rpc-enabled chip
devices.

The console uses the [pigweed pw_console](https://pigweed.dev/pw_console/), but
with customizations to work better with CHIP, including containing all rpc proto
files required for CHIP.

-   [CHIP RPC CONSOLE](#chip-rpc-console)
    -   [Building](#building)
    -   [Running](#running)

---

## Building

If this is the first time using the checkout the environment must first be
bootstrapped to install all dependencies.

```
$ source <project_root>/scripts/bootstrap.sh
```

If bootstrap has previously been run, then simply activate.

```
$ source <project_root>/scripts/activate.sh
```

The python console is built and installed in the venv using gn:

```
$ cd <project_root>/examples/common/pigweed/rpc_console
$ gn gen out/debug
$ ninja -C out/debug
```

After building the output directory also contains a folder
(chip_rpc_console_wheels), with all the wheels required for the tool. These can
be used to install the console without needing the sdk. Simply install all the
wheels in the folder:

```
$ cd <project_root>/examples/common/pigweed/rpc_console/out/debug
$ pip install chip_rpc_console_wheels/*.whl
```

## Running

To start the console provide the path to the device, for example:

```
$ chip-console --device /dev/ttyUSB0
```

Note that `chip-console` is an entry point for chip_rpc.console and could also
be run with `python -m chip_rpc.console`.

An example RPC command:

```python
rpcs.chip.rpc.Device.GetDeviceInfo()
```
