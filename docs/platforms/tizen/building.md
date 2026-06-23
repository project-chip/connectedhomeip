# Building for Tizen

## Prerequisites

### Docker Compose Environment

We use the `tizen` service defined in the shared `docker-compose.yml` which
contains the complete Tizen native SDK toolchain, rootstraps, compiler
configurations, and the `sdb` tool.

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
```

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

## Building with build_examples.py (Recommended)

Start and enter the Tizen compilation environment:

```bash
docker compose run --rm tizen bash
```

Initialize the Pigweed environment inside the container:

```bash
source scripts/activate.sh
```

### Building the Lighting App (TPK)

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
- `tizen-arm64-light-no-thread-no-ble-clang` – Same with Clang compiler
- `tizen-arm64-all-clusters-no-thread-no-ble` – All-clusters app
- `tizen-arm64-tests` – Unit tests

### Building the Linux-ARM64 Variant (Native Binary)

To cross-compile for a generic Linux ARM64 target (e.g., Raspberry Pi 4 running
Tizen), use the `crosscompile` Docker service:

```yaml
version: '3.8'
name: matter
services:
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

Then build:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-clang build
```

The output binary will be in
`out/linux-arm64-light-no-thread-no-ble-clang/`.

## Building with GN and Ninja (Alternative)

You can also build manually using `gn gen` and `ninja` directly. This gives you
more control over build arguments.

### Activating Environment

```sh
source ./scripts/activate.sh
```

### Generating Build Files

```sh
gn gen --check \
    --fail-on-unused-args \
    --add-export-compile-commands=* \
    --root=$PW_PROJECT_ROOT/examples/lighting-app/tizen \
    "--args=target_os=\"tizen\" target_cpu=\"arm\" tizen_sdk_root=\"$TIZEN_SDK_ROOT\" tizen_sdk_sysroot=\"$TIZEN_SDK_SYSROOT\"" \
    $PW_PROJECT_ROOT/out/tizen-arm-light
```

### Building

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light
```

### Packaging TPK

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light chip-lighting-app:tpk
```

## Preparing Tizen SDK Certificate (Optional)

When building a Matter example application, this step is optional. If an author
certificate and security profile are not found, they will be created
automatically. The automatically generated dummy certificate will have the
following options: name="Matter Example" email="matter@tizen.org"
password="0123456789"

To create and use a custom author certificate:

```sh
$TIZEN_SDK_ROOT/tools/ide/bin/tizen certificate \
    --alias=CHIP \
    --name=CHIP \
    --email=chip@tizen.org \
    --password=chiptizen

$TIZEN_SDK_ROOT/tools/ide/bin/tizen security-profiles add \
    --active \
    --name=CHIP \
    --author=$HOME/tizen-sdk-data/keystore/author/author.p12 \
    --password=chiptizen
```

This is a one-time action. To regenerate the author certificate and security
profile, remove files from the `$HOME` directory:

```sh
rm -r \
    $HOME/tizen-sdk-data \
    $HOME/.tizen-cli-config \
    $HOME/.secretsdb
```

After that, call the certificate generation scripts again.

:::{note}
Regenerating the author certificate and security profile makes it necessary to
remove the previously installed Tizen app. You cannot reinstall an application
on the Tizen device with a different certificate.
:::

```sh
pkgcmd -u -n org.tizen.matter.example.lighting
```
