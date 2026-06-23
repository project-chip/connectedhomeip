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
| `tizen` | Build Tizen TPK packages with the Tizen SDK toolchain; includes `sdb` |
| `tizen-qemu` | Extends the `tizen` image with QEMU support; used for running tests in the QEMU emulator; includes `sdb` |
| `crosscompile` | Build native Linux ARM64 binaries that can run on Tizen (e.g., on Raspberry Pi) |

The `tizen` and `tizen-qemu` containers include the `sdb` tool. You can use
`sdb` directly from inside the container, or install the
[Tizen VS Code Extension](./vscode_setup.md#tizen-extension-installation) to
use `sdb` from your host machine.

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
./scripts/build/build_examples.py --target tizen-arm64-light-no-thread --enable-flashbundle build
```

The target artifacts and the generated installation bundle will be placed inside
the `out/tizen-arm64-light-no-thread/` directory.

### Available Tizen Targets

The following Tizen target patterns are available:

```text
tizen-{arm,arm64}-{all-clusters,chip-tool,light,tests}[-asan][-coverage][-no-ble][-no-thread][-no-wifi][-ubsan][-with-ui]
```

For example:

- `tizen-arm64-light-no-thread` – Lighting app for ARM64 without Thread
  (BLE enabled)
- `tizen-arm64-light-no-thread-no-ble` – Lighting app without Thread and BLE
- `tizen-arm64-all-clusters-no-thread` – All-clusters app
- `tizen-arm64-tests` – Unit tests

### Building the Linux-ARM64 Variant (Native Binary)

To cross-compile a native Linux ARM64 binary that can run on Tizen (e.g., on
Raspberry Pi 4), use the `crosscompile` Docker service:

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
[lighting-app Tizen README](../../../examples/lighting-app/tizen/README.md)
for detailed instructions.
