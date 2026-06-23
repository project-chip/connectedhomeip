# Debugging with GDB

This section covers how to debug Matter applications running on Tizen from the
command line using GDB.

## Prerequisites

- `sdb` tool available in your `PATH` (inside the Tizen Docker container or
  installed via the
  [VS Code Tizen Extension](./vscode_setup.md#tizen-extension-installation))
- Application built with debug symbols (do not strip symbols)

## Deploying GDBServer to the Target

To debug applications on the target device, `gdbserver` must be available on the
Raspberry Pi. The `tizen_gdbserver_run.sh` helper script handles this
automatically, but you can also deploy it manually:

```bash
# Push the aarch64 gdbserver from the Tizen SDK to the Raspberry Pi
sdb push $TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gdb-15.1/bin/gdbserver /opt/bin/aarch64-linux-gnu-gdbserver

# Grant execution permissions
sdb shell chmod +x /opt/bin/aarch64-linux-gnu-gdbserver
```

## Debugging the Native Linux-ARM64 App via GDB CLI

This approach debugs the standalone `linux-arm64` binary running directly on the
Tizen device. The binary is built using the crosscompile Docker service (see
[Building the Linux-ARM64 Variant](./building.md#building-the-linux-arm64-variant-native-binary)).

### 1. Launch the Application via GDBServer on Target

Run the application binary directly through `gdbserver`, which freezes the
application at the very first instruction entry point. Run this on the target
device:

```bash
sdb shell /opt/bin/aarch64-linux-gnu-gdbserver :9999 /opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

The console will print:
`Process /opt/matter/chip-lighting-app created; pid = <PID>; Listening on port 9999`

### 2. Start the Remote GDB CLI Session on Host PC

Navigate to the build output directory to preserve relative path alignment:

```bash
cd out/linux-arm64-light-no-thread-no-ble-clang
```

Execute the cross-compiled GDB binary with connection and breakpoint parameters:

```bash
$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb \
    -ex "set auto-solib-add off" \
    -ex "target remote RASPBERRY_PI_IP:9999" \
    -ex "break LightingManager::InitiateAction" \
    -ex "continue" \
    chip-lighting-app
```

Replace `RASPBERRY_PI_IP` with your Raspberry Pi's IP address.

### 3. Verify the Breakpoint

Trigger a cluster state change from another terminal:

```bash
./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
```

The GDB console will stop at the breakpoint:

```text
Thread 8 "chip-lighting-a" hit Breakpoint 1, LightingManager::InitiateAction (this=0xaaaad1ed2d30 <LightingManager::sLight>, aAction=LightingManager::OFF_ACTION)
    at ../../examples/lighting-app/lighting-common/src/LightingManager.cpp:46
46          bool action_initiated = false;
```

You can now use standard GDB commands: `print aAction`, `backtrace`,
`continue`, etc.

## Debugging the Tizen TPK App via GDB CLI

Because Tizen apps are managed by the platform runtime lifecycle, we cannot use
direct execution via `gdbserver`. Instead, we use `app_launcher --debug` to
spawn the process in a suspended state, then attach `gdbserver`.

The TPK app is built using the Tizen Docker service (see
[Building the Tizen TPK Package](./building.md#building-the-tizen-tpk-package)).

### 1. Launch the App in Suspended Debug Mode

Run this on the target device:

```bash
sdb shell app_launcher --debug --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344
```

The output will include the assigned PID:
`successfully launched pid = 473505 with debug 1`

### 2. Attach GDBServer to the Suspended Process

Attach `gdbserver` to the frozen PID on port `9999` (replace `473505` with your
active PID):

```bash
sdb shell /opt/bin/aarch64-linux-gnu-gdbserver :9999 --attach 473505
```

The console will report: `Listening on port 9999; Attached; pid = 473505`

### 3. Start the Remote GDB CLI Session on Host PC

Navigate to the Tizen build output directory:

```bash
cd out/tizen-arm64-light-no-thread
```

Run the cross-compiled GDB with sysroot and connection parameters:

```bash
$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb \
    -ex "set sysroot $TIZEN_SDK_SYSROOT_ARM64" \
    -ex "set solib-absolute-prefix $TIZEN_SDK_SYSROOT_ARM64" \
    -ex "target remote RASPBERRY_PI_IP:9999" \
    -ex "break LightingManager::InitiateAction" \
    -ex "continue" \
    chip-lighting-app
```

Replace `RASPBERRY_PI_IP` with your Raspberry Pi's IP address.

### 4. Verify

Trigger a toggle from another terminal:

```bash
./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
```

GDB will trap the execution:

```text
Thread 1 "chip-lighting-a" hit Breakpoint 1, LightingManager::InitiateAction (this=0xaaaad1ed2d30, aAction=LightingManager::OFF_ACTION)
    at ../../examples/lighting-app/tizen/src/LightingManager.cpp:46
46          bool action_initiated = false;
```

## Using the tizen_gdbserver_run.sh Helper Script

The SDK provides a helper script that automates the gdbserver deployment,
application launch in debug mode, and gdbserver attachment:

```bash
./scripts/helpers/tizen_gdbserver_run.sh \
    --target RASPBERRY_PI_IP:26101 \
    --gdbserver-port 9999 \
    --app-name "org.tizen.matter.example.lighting" \
    -- \
    wifi true discriminator 1234 passcode 11223344
```

Options:

| Option | Description |
|--------|-------------|
| `--app-name APP_NAME` | Name of app to debug (required) |
| `--gdbserver-port PORT` | GDB server port (default: 9999) |
| `--target SDB_ID` | SDB identifier (if not specified, first connected device is used) |
| `-- APP_ARGUMENTS` | Arguments to pass to the debugged app |

The script will:

1. Detect or deploy `gdbserver` on the target device
2. Launch the application in suspended debug mode
3. Attach `gdbserver` to the process
4. Set up SDB port forwarding

You can then connect from your host GDB or VS Code.

## Analyzing Crash Dumps

The `tizen-crashlog.sh` helper script processes core dumps from Tizen builds:

```bash
./scripts/helpers/tizen-crashlog.sh
```

It will scan `out/tizen-*/dump/` directories for `.zip` files, extract
core dumps, and generate full thread backtraces using `gdb-multiarch` with the
correct Tizen sysroot.
