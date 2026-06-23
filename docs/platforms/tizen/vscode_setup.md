# VS Code Setup for Tizen Development

This section covers how to set up Visual Studio Code for Tizen development,
including extension installation, graphical debugging, and task integration.

## Tizen Extension Installation

Instead of manually installing the heavy standalone Tizen Studio GUI, the
entire ecosystem can be managed via VS Code:

1. Install the **C/C++** extension (by Microsoft) – required for the underlying
   `cppdbg` debugging engine.
2. Install the **Tizen Extension for Visual Studio Code** (by Samsung) from the
   Marketplace.

Once the extension is installed, it handles the download of the required
lightweight CLI package tools:

1. Open the VS Code Command Palette (`Ctrl+Shift+P`).
2. Search for and select: `Tizen: Package Manager`.
3. Follow the UI prompts to install the **Extension Tools** and the
   **10.0 Platform target rootstraps** (specifically ensuring the 64-bit device
   profile is downloaded).

The extension installs the SDK layout into a hidden system extension directory
(typically under `~/.tizen-extension-platform/`). Verify that your shell profile
exposes these automated tools:

```bash
export | grep TIZEN
```

Expected output:

```text
declare -x TIZEN_CLI_PATH="~/.tizen-extension-platform/server/sdktools/data/tools/ide/bin"
declare -x TIZEN_DOTNET_ROOT="~/.tizen-extension-platform/server/sdktools/dotnet"
declare -x TIZEN_SDK="~/.tizen-extension-platform/server/sdktools/data"
declare -x TIZEN_SDK_ROOT="~/.tizen-extension-platform/server/sdktools/data"
declare -x TIZEN_TOOLS_PATH="~/.tizen-extension-platform/server/sdktools/data/tools"
```

The `sdb` tool is installed as part of the Tizen extension and is available at
`~/.tizen-extension-platform/server/sdktools/data/tools/`.

## Graphical Remote Debugging via VS Code

This section automates the CLI attach steps into a native VS Code debugging
configuration, allowing you to hit breakpoints, inspect variables, and view
stack traces visually.

### 1. Launch and Prepare the Process on Target

Use the `tizen_gdbserver_run.sh` helper script to deploy gdbserver, launch the
app in debug mode, attach gdbserver, and set up port forwarding:

```bash
./scripts/helpers/tizen_gdbserver_run.sh \
    --app-name "org.tizen.matter.example.lighting" \
    -- \
    wifi true discriminator 1234 passcode 11223344
```

Alternatively, perform the steps manually:

```bash
# 1. Spawn the application context in frozen debug mode
sdb shell app_launcher --debug --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344

# Note the returned PID from the output, e.g., 473505

# 2. Attach gdbserver to the live PID
sdb shell /opt/bin/aarch64-linux-gnu-gdbserver :9999 --attach 473505

# 3. Set up port forwarding
sdb forward tcp:9999 tcp:9999
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
            "cwd": "${workspaceFolder}/out/tizen-arm64-light-no-thread",
            "program": "${workspaceFolder}/out/tizen-arm64-light-no-thread/chip-lighting-app",
            "targetArchitecture": "arm64",
            "miDebuggerPath": "${env:TIZEN_SDK_ROOT}/tools/aarch64-linux-gnu-gdb-15.1/bin/aarch64-linux-gnu-gdb",
            "miDebuggerServerAddress": "localhost:9999",
            "additionalSOLibSearchPath": "${workspaceFolder}/out/tizen-arm64-light-no-thread;${env:TIZEN_SDK_SYSROOT_ARM64}/lib64;${env:TIZEN_SDK_SYSROOT_ARM64}/usr/lib64",
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
                    "text": "set sysroot ${env:TIZEN_SDK_SYSROOT_ARM64}",
                    "ignoreFailures": false
                },
                {
                    "description": "Set absolute prefix to align with local SDK shared object layout",
                    "text": "set solib-absolute-prefix ${env:TIZEN_SDK_SYSROOT_ARM64}",
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

The configuration uses `localhost:9999` as the debugger server address because
the `tizen_gdbserver_run.sh` helper script (or `sdb forward`) sets up port
forwarding from the host to the target device. The target build is
`tizen-arm64-light-no-thread` — adjust the `cwd`, `program`, and
`additionalSOLibSearchPath` paths if using a different target.

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

> **TODO:** The VS Code tasks described below assume a local Tizen build
> environment is configured on the host machine (with `TIZEN_SDK_ROOT` and
> related environment variables set). When using Docker-based builds, these
> tasks need to be adapted to run commands inside the Docker container. This
> needs to be resolved.

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

## Dev Container vs WSL

> **TODO:** Document the recommended VS Code workflow for Tizen development.
> There are two main approaches:
>
> - **Dev Container:** VS Code can use the all-in-one Matter dev container.
>   However, this container does not include the Tizen SDK, and the experience
>   has been problematic for Tizen development.
> - **WSL:** Running VS Code connected to WSL with the Tizen extension installed
>   locally. This is the currently used approach, but requires the Tizen
>   extension and SDK to be set up on the WSL side.
>
> The recommended approach and detailed setup instructions need to be documented
> here.
