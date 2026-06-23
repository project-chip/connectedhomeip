# Testing TPK Apps on Raspberry Pi

This section documents how to compile the Matter lighting application as a
native Tizen package (`.tpk`) for an ARM64 target (Raspberry Pi 4 running Tizen
OS), install it, and test it using `chip-tool`.

The TPK approach uses the Tizen package manager for proper application lifecycle
management, which is the recommended way for production deployments. This is
different from the
[native Linux ARM64 approach](./raspberry_pi_native_apps.md), which deploys a
standalone binary — that works without TPK packaging but bypasses the Tizen
runtime lifecycle. The `--enable-flashbundle` flag is required to generate the
`.tpk` file; without it, only the raw binary is produced, which cannot be
easily installed and run on Tizen.

## Prerequisites

- Raspberry Pi 4 running Tizen OS with Wi-Fi configured (see
  [Installing Tizen on Raspberry Pi](./raspberry_pi_install.md))
- `sdb` tool — available inside the Tizen Docker container (see
  [Building for Tizen](./building.md#docker-compose-environment))
- `chip-tool` built for your host PC (see
  [Building chip-tool](./building.md#building-chip-tool-for-the-host-pc))

## Building

Start and enter the Tizen compilation environment:

```bash
docker compose run --rm tizen bash
source scripts/activate.sh
```

Compile the application with the `--enable-flashbundle` flag to generate the
`.tpk` bundle:

```bash
./scripts/build/build_examples.py --target tizen-arm64-light-no-thread --enable-flashbundle build
```

The target artifacts and the generated installation bundle will be placed inside
the `out/tizen-arm64-light-no-thread/` directory.

## Connecting via SDB

Use `sdb` from the Tizen Docker container. Connect to the device and switch to
root:

```bash
docker compose run --rm tizen bash
sdb connect RASPBERRY_PI_IP
sdb root on
```

## Installing the TPK

Install the `.tpk` on the device:

```bash
sdb install out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

### Verifying the Installation

To check if the application is installed and fetch the Package ID:

```bash
sdb shell pkgcmd -l | grep -i matter
```

Expected identifier:

- **Package ID / App ID:** `org.tizen.matter.example.lighting`

### Reinstalling

If the application already exists on the device, you must uninstall it first
before installing a new version:

```bash
sdb uninstall org.tizen.matter.example.lighting
sdb install out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

:::{note}
You cannot reinstall an application with a different author certificate. If you
regenerated the certificate, you must uninstall first.
:::

## Launching the Application

Native Tizen applications are managed by the platform runtime. Use `app_launcher`
to start the application:

```bash
sdb shell app_launcher --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344
```

## Monitoring Application Logs

Tizen channels application standard outputs and errors through the `dlog`
daemon. To trace execution on the target device, use `dlogutil` with the
`--pid` option:

```bash
# Launch the app and note the PID from the output
sdb shell app_launcher --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344
# Output: successfully launched pid = 473505

# Capture logs in real time using the PID
sdb shell dlogutil --pid 473505
```

This will stream Matter engine initializations, mDNS advertisements, and
cluster interaction states.

## Commissioning and Control via chip-tool

Open a separate terminal window on your host PC to run the control scripts.

### Commissioning

```bash
./out/linux-x64-chip-tool/chip-tool pairing onnetwork 1 11223344
```

### Attribute Cluster Interaction (Toggle Command)

```bash
./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
```

### Verification

Check your active `dlogutil` terminal window. You should see incoming ZCL
interaction frames and the light attribute status logging transitions between
`OFF_ACTION` and `ON_ACTION`.

## Stopping the Application

```bash
sdb shell app_launcher --stop org.tizen.matter.example.lighting
```
