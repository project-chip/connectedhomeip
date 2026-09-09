# Matter All-Devices Application — Silabs (EFR32)

This is the Silabs/EFR32 target for the `all-devices-app`. Refer to the
top-level [all-devices-app README](../README.md) for a description of the
Code-Driven paradigm, supported device types, CLI concepts, and the overall
documentation suite under [`../docs/`](../docs/).

Unlike the Linux/POSIX variant, the Silabs image cannot select device types from
`argv` at boot. Instead the topology is chosen in one of two ways:

1. **Runtime (Matter shell)** — flash the default image and use the `devtype`
   shell command to store a single device type in KVS. The image will
   instantiate that device on the next boot.
2. **Build time (no shell required)** — bake a fixed, ordered list of device
   types into the image via GN args. This is the recommended flow for
   product-like builds and enables running multiple device types on consecutive
   endpoints without any Matter-shell interaction.

The build-time selection is driven by two GN args declared in
[all-devices-common/device-factory/enabled_devices.gni](../all-devices-common/device-factory/enabled_devices.gni):

-   `all_devices_enabled_devices` — subset of device types compiled into the
    `DeviceFactory`. Empty (default) compiles all of them in.
-   `all_devices_default_devices` — ordered list of device types instantiated at
    boot, one per endpoint starting at endpoint 1. Empty (default) means "fall
    back to the runtime KVS override / factory default single device".

Every entry in `all_devices_default_devices` must also be present in
`all_devices_enabled_devices` (unless the enabled list is empty). GN enforces
this at `gen` time.

## Build command (humidity + temperature sensor on BRD4187C)

```bash
./scripts/examples/gn_silabs_example.sh \
    examples/all-devices-app/silabs \
    out/all-devices-app \
    BRD4187C \
    'all_devices_enabled_devices=["humidity-sensor","temperature-sensor"]' \
    'all_devices_default_devices=["humidity-sensor","temperature-sensor"]' \
    'all_devices_app_name="humidity-and-temp-sensor"' \
    'chip_build_libshell=false'
```

Result:

-   Output image:
    `out/all-devices-app/thread/BRD4187C/humidity-and-temp-sensor.out`
-   Endpoint layout on boot:

    ```text
    Endpoint 0 → Root Node
    Endpoint 1 → humidity-sensor
    Endpoint 2 → temperature-sensor
    ```

No Matter shell interaction is needed to select the device types — the mapping
is fixed at build time. Dropping `chip_build_libshell=false` keeps the `devtype`
shell command available but has no effect on the pre-baked topology (the
build-time list takes precedence over any KVS override).

## Building a single device type

Same recipe with a single-element list, e.g. a temperature sensor only:

```bash
./scripts/examples/gn_silabs_example.sh \
    examples/all-devices-app/silabs \
    out/all-devices-app \
    BRD4187C \
    'all_devices_enabled_devices=["temperature-sensor"]' \
    'all_devices_default_devices=["temperature-sensor"]' \
    'all_devices_app_name="temperature-sensor-app"' \
    'chip_build_libshell=false'
```

## Default image (runtime device selection)

Omitting both args produces the full `matter-silabs-all-devices-example.out`
image with every device type registered in the factory and runtime selection via
the shell:

```bash
./scripts/examples/gn_silabs_example.sh \
    examples/all-devices-app/silabs \
    out/all-devices-app \
    BRD4187C
```

Then, from the device shell:

```text
matterCli> devtype list
matterCli> devtype set humidity-sensor
matterCli> reboot
```

## Low-power build (ICD, MTD, no shell/LEDs)

To reproduce a low-power / battery-operated sample-app configuration, add the
`--low-power` flag (which disables LEDs, buttons, LCD and shell), select the
`power-source` device type in addition to the sensor(s) you want on the endpoint
topology, enable the ICD server, and switch the OpenThread stack to MTD:

```bash
./scripts/examples/gn_silabs_example.sh \
    examples/all-devices-app/silabs \
    out/temp_sensor/ \
    BRD2601B \
    --low-power \
    'all_devices_enabled_devices=["humidity-sensor","temperature-sensor","power-source"]' \
    'all_devices_default_devices=["humidity-sensor","temperature-sensor"]' \
    'all_devices_app_name="TempSensor"' \
    'sl_enable_si70xx_sensor=true' \
    'chip_enable_icd_server=true' \
    'chip_openthread_ftd=false'
```

Notes:

-   `all_devices_default_devices` intentionally omits `power-source`; the app
    auto-appends a `power-source` endpoint when `chip_enable_icd_server=true`
    (see `maybeAddPowerSource()` in `src/AppTask.cpp`) so commissioners can
    display a battery level / battery voltage.
-   `sl_enable_si70xx_sensor=true` wires the on-board Si7021 sensor of the
    BRD2601B to feed the humidity- and temperature-sensor endpoints.

### Expected power consumption

Once commissioned, in ICD Idle Mode (i.e. between polls), current draw on the
BRD2601B should easily reach **~4 µA**. If you observe a significantly higher
baseline or short periodic wake-ups on the power scope, double-check that
`--low-power`, `chip_enable_icd_server=true` and `chip_openthread_ftd=false` all
made it into the build.
