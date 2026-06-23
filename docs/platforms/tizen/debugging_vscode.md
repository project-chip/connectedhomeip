# Debugging with VS Code

This section covers how to debug Matter applications running on Tizen, both from
the command line using GDB and graphically using Visual Studio Code.

## Prerequisites

- Tizen SDK installed with VS Code extension (see
  [Building for Tizen](./building.md#tizen-studio-and-vs-code-extension-setup))
- `sdb` tool available in your `PATH`
- `gdbserver` deployed on the target device
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
Tizen device.

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

Execute the cross-compiled GDB binary:

```bash
$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb chip-lighting-app
```

### 3. Establish Connection and Set Breakpoints

Inside the GDB session (`(gdb)` prompt), execute:

```text
(gdb) set auto-solib-add off
(gdb) target remote ${TARGET_IP}:9999
(gdb) break LightingManager::InitiateAction
(gdb) continue
```

Replace `${TARGET_IP}` with your Raspberry Pi's IP address.

### 4. Verify the Breakpoint

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
cd out/tizen-arm64-light-no-thread-no-ble
```

Run the cross-compiled GDB:

```bash
$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb chip-lighting-app
```

### 4. Establish Connection and Environment Realignment

Inside the GDB session, map the system library roots (`sysroot`) so GDB can
align the device's shared object memory with your local SDK copies:

```text
(gdb) set sysroot /home/ubuntu/.tizen-extension-platform/server/sdktools/data/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core
(gdb) set solib-absolute-prefix /home/ubuntu/.tizen-extension-platform/server/sdktools/data/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core
(gdb) target remote ${TARGET_IP}:9999
(gdb) break LightingManager::InitiateAction
(gdb) continue
```

### 5. Verify

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
    --target 192.168.0.118:26101 \
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

## Graphical Remote Debugging via VS Code

This section automates the CLI attach steps into a native VS Code debugging
configuration, allowing you to hit breakpoints, inspect variables, and view
stack traces visually.

### 1. Launch and Prepare the Process on Target

Before starting the debug session in VS Code, the target process must be
instantiated and waiting:

```bash
# 1. Spawn the application context in frozen debug mode
sdb shell app_launcher --debug --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344

# Note the returned PID from the output, e.g., 473505

# 2. Attach gdbserver to the live PID
sdb shell /opt/bin/aarch64-linux-gnu-gdbserver :9999 --attach 473505
```

Alternatively, use the helper script:

```bash
./scripts/helpers/tizen_gdbserver_run.sh \
    --app-name "org.tizen.matter.example.lighting" \
    -- \
    wifi true discriminator 1234 passcode 11223344
```

### 2. VS Code Workspace Configuration (launch.json)

Create or update the `.vscode/launch.json` file in your workspace root:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "LightingApp (Tizen aarch64) - Remote Debug",
            "type": "cppdbg",
            "request": "launch",
            "cwd": "${workspaceFolder}/out/tizen-arm64-light-no-thread-no-ble",
            "program": "${workspaceFolder}/out/tizen-arm64-light-no-thread-no-ble/chip-lighting-app",
            "targetArchitecture": "arm64",
            "miDebuggerPath": "${env:TIZEN_SDK_ROOT}/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb",
            "miDebuggerServerAddress": "${TARGET_IP}:9999",
            "additionalSOLibSearchPath": "${workspaceFolder}/out/tizen-arm64-light-no-thread-no-ble;${env:TIZEN_SDK_ROOT}/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core/lib64;${env:TIZEN_SDK_ROOT}/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core/usr/lib64",
            "linux": {
                "MIMode": "gdb"
            },
            "setupCommands": [
                {
                    "description": "Tell GDB not to stop the app on VS Code internal SIGINT signals",
                    "text": "handle SIGINT nostop noprint pass",
                    "ignoreFailures": true
                },
                {
                    "description": "Tell GDB not to stop on standard terminate signals",
                    "text": "handle SIGTERM nostop noprint pass",
                    "ignoreFailures": true
                },
                {
                    "description": "Lock down architecture layout to aarch64 inside the GDB engine",
                    "text": "set architecture aarch64",
                    "ignoreFailures": false
                },
                {
                    "description": "Redirect sysroot to the local 64-bit Tizen SDK rootstrap directory",
                    "text": "set sysroot ${env:TIZEN_SDK_ROOT}/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core",
                    "ignoreFailures": false
                },
                {
                    "description": "Set absolute prefix to align with local SDK shared object layout",
                    "text": "set solib-absolute-prefix ${env:TIZEN_SDK_ROOT}/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core",
                    "ignoreFailures": true
                },
                {
                    "description": "Enable pretty-printing for complex C++ structures",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "stopAtEntry": false
        }
    ]
}
```

Replace `${TARGET_IP}` with your Raspberry Pi's IP address.

### 3. Starting the Debugging Session

1. Open your source codebase folder in VS Code.
2. Open the file containing your target logic (e.g.,
   `examples/lighting-app/tizen/src/LightingManager.cpp`).
3. Click to the left of a line number to set a **visual red breakpoint** inside
   `LightingManager::InitiateAction`.
4. Navigate to the **Run & Debug** view (`Ctrl+Shift+D`), select
   **"LightingApp (Tizen aarch64) - Remote Debug"**, and press **F5**.
5. The VS Code status bar will turn orange, signaling that the graphical wrapper
   has successfully established a link with the target `gdbserver`.

### 4. Verification and Interactive Use

1. Issue a toggle command from your host PC terminal:

   ```bash
   ./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
   ```

2. VS Code will flash, highlighting your line of code in yellow.
3. You can now use the graphic debug interface:
   - Track live stack frames in the **Call Stack** panel
   - Mouse over variables (like `aAction`) to see data tooltips
   - View live component variable fields in the **Variables** pane
   - Step through code using the graphical controls (Step Over, Step Into)

## VS Code Tasks for Tizen

The `augustocdias.tasks-shell-input` VS Code extension provides task
integration for common Tizen development operations.

### Available Tasks

1. **Build app:**

   Open the Command Palette (`Ctrl+Shift+P`) → `Tasks: Run Task` →
   `Build LightingApp (Tizen)`

2. **SDB connect to device:** Required to run Tizen commands below if the device
   is debugged over network.

   Open the Command Palette → `Tasks: Run Task` → `Connect to device (Tizen)` →
   insert IP address and port

3. **Install app:** Separated from the build step.

   Open the Command Palette → `Tasks: Run Task` →
   `Install LightingApp (Tizen)`

4. **Launch with gdbserver attached:** Requires the app to be installed
   previously.

   Open the Command Palette → `Tasks: Run Task` →
   `Launch LightingApp with gdbserver attached (Tizen)`

## Analyzing Crash Dumps

The `tizen-crashlog.sh` helper script processes core dumps from Tizen builds:

```bash
./scripts/helpers/tizen-crashlog.sh
```

It will scan `out/tizen-*/dump/` directories for `.zip` files, extract
core dumps, and generate full thread backtraces using `gdb-multiarch` with the
correct Tizen sysroot.
