# Building for Tizen

## Prerequisites

### Docker Compose Environment

We use a combined Docker Compose configuration with three services for different
Tizen development tasks:

```yaml
version: '3.8'
name: matter
services:
  tizen:
    image: ghcr.io/project-chip/chip-build-tizen:latest
    network_mode: "host"
    privileged: true
    user: ubuntu
    working_dir: /workspace
    volumes:
      - .:/workspace
      - /var/run/docker.sock:/var/run/docker.sock
    environment:
      PW_ENVIRONMENT_ROOT: /workspace/.environment-tizen
    tty: true
    stdin_open: true
  tizen-qemu:
    image: ghcr.io/project-chip/chip-build-tizen-qemu:latest
    network_mode: "host"
    privileged: true
    user: ubuntu
    working_dir: /workspace
    volumes:
      - .:/workspace
      - /var/run/docker.sock:/var/run/docker.sock
    environment:
      PW_ENVIRONMENT_ROOT: /workspace/.environment-tizen-qemu
    tty: true
    stdin_open: true
  crosscompile:
    image: ghcr.io/project-chip/chip-build-crosscompile:latest
    network_mode: "host"
    privileged: true
    user: ubuntu
    working_dir: /workspace
    volumes:
      - .:/workspace
      - /var/run/docker.sock:/var/run/docker.sock
    environment:
      PW_ENVIRONMENT_ROOT: /workspace/.environment-crosscompile
    tty: true
    stdin_open: true
```

| Service | Purpose |
|---------|---------|
| `tizen` | Build Tizen TPK packages with the Tizen SDK toolchain |
| `tizen-qemu` | Run tests in the QEMU emulator |
| `crosscompile` | Build native Linux ARM64 binaries |

### Tizen Studio and VS Code Extension Setup

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

## Building with build_examples.py

### Building the Tizen TPK Package

Start and enter the Tizen compilation environment:

```bash
docker compose run --rm tizen bash
source scripts/activate.sh
```

Compile the application using the `build_examples.py` script. Supply the
`--enable-flashbundle` flag to pack the compiled binary, assets, and manifest
files into a deployable native Tizen package (`.tpk` bundle):

```bash
./scripts/build/build_examples.py --target tizen-arm64-light-no-thread-no-ble --enable-flashbundle build
```

The target artifacts and the generated installation bundle will be placed inside
the `out/tizen-arm64-light-no-thread-no-ble/` directory.

### Available Tizen Targets

The following Tizen target patterns are available:

```text
tizen-{arm,arm64}-{all-clusters,chip-tool,light,tests}[-asan][-coverage][-no-ble][-no-thread][-no-wifi][-ubsan][-with-ui]
```

For example:

- `tizen-arm64-light-no-thread-no-ble` – Lighting app for ARM64 without Thread
  and BLE
- `tizen-arm64-all-clusters-no-thread-no-ble` – All-clusters app
- `tizen-arm64-tests` – Unit tests

### Building the Linux-ARM64 Variant (Native Binary)

To cross-compile for a generic Linux ARM64 target (e.g., Raspberry Pi 4 running
Tizen), use the `crosscompile` Docker service:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-clang build
```

The output binary will be in
`out/linux-arm64-light-no-thread-no-ble-clang/`.

## Alternative: Building with GN and Ninja

For advanced use cases that require direct control over build arguments, you can
build manually using `gn gen` and `ninja`. See the
[lighting-app Tizen README](https://github.com/project-chip/connectedhomeip/blob/master/examples/lighting-app/tizen/README.md)
for detailed instructions.
