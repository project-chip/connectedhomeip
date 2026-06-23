# Testing Native Apps on Raspberry Pi

This section documents how to cross-compile the Matter lighting application as a
native Linux ARM64 binary, deploy it to a Raspberry Pi 4 running Tizen OS, and
test it using `chip-tool`.

Unlike the TPK approach, this method runs the application as a standalone
executable without the Tizen package manager. This is useful for rapid
development iteration.

## Prerequisites

- Raspberry Pi 4 running Tizen OS with Wi-Fi configured (see
  [Installing Tizen on Raspberry Pi](./raspberry_pi_install.md))
- `sdb` tool available in your `PATH`
- `chip-tool` built for your host PC

## Building

### Using build_examples.py (Recommended)

Use the `crosscompile` Docker service to build the Linux ARM64 variant:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-clang build
exit
```

The output binary will be in `out/linux-arm64-light-no-thread-no-ble-clang/`.

### Using GN and Ninja (Alternative)

```bash
source scripts/activate.sh
gn gen --check \
    --root=$PW_PROJECT_ROOT/examples/lighting-app/linux \
    "--args=target_os=\"linux\" target_cpu=\"arm64\" chip_config_network_layer_ble=false chip_enable_thread=false" \
    $PW_PROJECT_ROOT/out/linux-arm64-light-no-thread-no-ble-clang
ninja -C $PW_PROJECT_ROOT/out/linux-arm64-light-no-thread-no-ble-clang
```

## Installing the Binary via SDB

Deploy the compiled native executable from your development PC to the target
Raspberry Pi over the network using SDB:

```bash
# Ensure the target directory exists on the Raspberry Pi
sdb shell mkdir -p /opt/matter/

# Push the native executable binary to the target deployment path
sdb push out/linux-arm64-light-no-thread-no-ble-clang/chip-lighting-app /opt/matter/

# Mark the binary as an executable on the device
sdb shell chmod +x /opt/matter/chip-lighting-app
```

## Running the Application

Run the application on the Raspberry Pi with target network parameters:

```bash
# Execute on the Raspberry Pi terminal shell
sdb shell /opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

Or if you already have an `sdb shell` session:

```bash
/opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

## Commissioning and Control via chip-tool

Once the application is running on the Raspberry Pi, use `chip-tool` on your
host PC to pair and control the device.

### Network Commissioning

Pair the target device over the network. We assign `Node ID = 1` using the
configured passcode:

```bash
./out/linux-x64-chip-tool/chip-tool pairing onnetwork 1 11223344
```

Verify that the log output displays successful CASE session establishment.

### Controlling the Cluster State (Toggle)

Send an On/Off toggle action command to the commissioned lighting application
at `Node ID = 1` on endpoint `1`:

```bash
./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
```

## Verification

- **Raspberry Pi Logs:** The terminal should register an incoming ZCL
  interaction model action frame and transition the light attribute state
  (`OFF_ACTION` / `ON_ACTION`).
- **PC Terminal Status:** The `chip-tool` output must return a successful
  command transmission status.

## Cleaning Up

To stop the application on the Raspberry Pi:

```bash
sdb shell killall chip-lighting-app
```

To remove the deployed binary:

```bash
sdb shell rm -rf /opt/matter/
```
