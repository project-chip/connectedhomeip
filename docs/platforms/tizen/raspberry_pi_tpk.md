# Testing TPK Apps on Raspberry Pi

This section documents how to compile the Matter lighting application as a
native Tizen package (`.tpk`) for an ARM64 target (Raspberry Pi 4 running Tizen
OS), install it, and test it using `chip-tool`.

The TPK approach uses the Tizen package manager for proper application lifecycle
management, which is the recommended way for production deployments.

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

## Managing the Application via SDB

Use `sdb` from the Tizen Docker container. Connect to the device and switch to
root:

```bash
docker compose run --rm tizen bash
sdb connect 192.168.0.118
sdb root on
```

### Verifying and Querying Installed Applications

To check if a previous build exists or to fetch the exact Package ID:

```bash
sdb shell pkgcmd -l | grep -i matter
```

Expected identifier:

- **Package ID / App ID:** `org.tizen.matter.example.lighting`

### Installing the TPK

```bash
sdb install out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

Or using `pkgcmd` directly on the device:

```bash
sdb push out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk /tmp/
sdb shell pkgcmd -i -t tpk -p /tmp/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

### Reinstalling the Package

To uninstall the existing package and install a fresh build:

```bash
# 1. Uninstall the existing native package from the target device
sdb uninstall org.tizen.matter.example.lighting

# 2. Install the freshly packed .tpk bundle file
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

:::{note}
When passing user arguments via `app_launcher`, strings cannot start with `-`
(minus) character and all arguments must consist of a name and value. Boolean
options should have value equal to `"true"`.
:::

## Monitoring Application Logs

Tizen channels application standard outputs and errors through the `dlog`
daemon. To trace execution on the target device, filter by the application PID:

```bash
# Launch the app and note the PID from the output
sdb shell app_launcher --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344
# Output: successfully launched pid = 473505

# Capture and filter logs in real time using the PID
sdb shell dlogutil 2>&1 | grep 473505
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

## Uninstalling

```bash
sdb uninstall org.tizen.matter.example.lighting
```

Or on the device directly:

```bash
sdb shell pkgcmd -u -n org.tizen.matter.example.lighting
```
