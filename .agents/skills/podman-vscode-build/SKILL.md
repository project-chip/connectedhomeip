---
name: podman-vscode-build
description:
    Guidelines for building Matter examples and running tests using Podman with
    the vscode image in a non-interactive environment.
---

# Building and Testing Matter with Podman (VSCode Build Image)

This skill provides guidelines and instructions for configuring, launching, and
executing build/test commands within a Podman container using the official
`ghcr.io/project-chip/chip-build-vscode` image.

Podman runs rootless, which avoids file ownership complications (where files
generated in the container are owned by root on the host) commonly encountered
with Docker.

---

## 1. Prerequisites & Host Configuration

Before launching Podman, you must configure storage to avoid namespace and user
limit errors (e.g., insufficient UIDs or GIDs in the user namespace).

### Edit storage.conf

Edit or create `$HOME/.config/containers/storage.conf` to include:

```toml
[storage]
driver = "overlay"

[storage.options]
ignore_chown_errors = "true"
```

### System Reset & Verification

If you previously attempted to use Podman without this setting, you may need to
reset the storage.

> [!CAUTION] Running `podman system reset` will delete all existing Podman
> containers, images, and volumes.

1. Reset Podman:
    ```bash
    podman system reset
    ```
2. Verify that `graphDriverName` is `overlay` (instead of `vfs`):
    ```bash
    podman info | grep graphDriverName
    ```

---

## 2. Locating the VSCode Image Tag

The correct tag for `ghcr.io/project-chip/chip-build-vscode` changes as
dependencies are updated. To find the tag currently used in CI:

-   Inspect the workflow files under `.github/workflows/` (such as `tests.yaml`
    or `build.yaml`) or the cloudbuild configurations under
    `integrations/cloudbuild/` (such as `chef.yaml`).
-   Search for the image reference
    `ghcr.io/project-chip/chip-build-vscode:<TAG>` to identify the correct
    version tag (e.g., `200`).

---

## 3. Starting the Container

To start a persistent vscode container in the background:

```bash
podman run -dt --cap-add=SYS_PTRACE --name bld_vscode \
    --volume /path/to/connectedhomeip:/workspace \
    ghcr.io/project-chip/chip-build-vscode:<TAG> \
    /bin/sh
```

_Replace `/path/to/connectedhomeip` with the absolute path to your local
repository checkout, and `<TAG>` with the version tag found in step 2._

---

## 4. Non-Interactive Command Execution

Since agents run in a non-interactive environment, do **NOT** use interactive
flags like `-it` (e.g. `podman exec -it ...`). Instead, execute commands
directly or script them.

### Activation is Mandatory

Because `podman exec` does not load login shell initialization scripts, the
Pigweed build environment is **not activated by default**. Sourcing the
environment activation script is required prior to compiling.

-   **Bootstrap**: Use `scripts/bootstrap.sh` if this is the first time setup
    inside the container, or if packages or submodules have changed.
-   **Activate**: Use `scripts/activate.sh` for subsequent runs.

Run the commands in a single subshell using `bash -c`:

```bash
podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && ./scripts/build/build_examples.py --target <target> build"
```

### Sourcing Bootstrap for Specific Platforms

By default, running `bootstrap.sh` inside the container installs python
requirements for host and Zephyr builds. To enable building for other specific
embedded platforms (which downloads/configures additional pip dependencies),
pass the platform names to the `-p` parameter:

```bash
podman exec -w /workspace bld_vscode bash -c "source scripts/bootstrap.sh -p esp32,nrfconnect,silabs,telink"
```

Supported platform identifiers include: `esp32`, `nrfconnect`, `silabs`,
`telink`, `bouffalolab`, `mbed`, `ti`, `zephyr`.

### Additional ESP32 Setup

If building for the ESP32 platform, the ESP-IDF toolchain Python requirements
inside the container must also be explicitly set up. Run the following command
inside the container after bootstrapping:

```bash
podman exec -w /workspace bld_vscode bash -c "/opt/espressif/esp-idf/install.sh"
```

### Additional Nordic nRF Connect Setup

If compiling for Nordic nRF Connect targets, the pre-installed SDK files must be
synchronized and updated before building. Discard local template modifications
first, then run the update script inside the container:

```bash
podman exec -w /workspace bld_vscode git -C /opt/NordicSemiconductor/nrfconnect/zephyr checkout -- zephyr-env.sh
podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow"
```

---

## 5. Platform Build Examples & Output Directories

Using Podman with the official vscode image is particularly valuable when
compiling for embedded or cross-compiled targets, as the image pre-packages all
required toolchains and compilers.

Here are examples of compiling Matter applications (e.g., `all-devices-app` or
`all-clusters-app`) for various platforms and where the executables/artifacts
are generated:

### A. Linux ARM64 (e.g. Raspberry Pi)

-   **Target**: `linux-arm64-all-devices-boringssl-no-ble-clang`
-   **Build Command**:
    ```bash
    podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && ./scripts/build/build_examples.py --target linux-arm64-all-devices-boringssl-no-ble-clang build"
    ```
-   **Output Artifact**:
    `<checkout_root>/out/linux-arm64-all-devices-boringssl-no-ble-clang/all-devices-app`

### B. ESP32 (devkitc)

-   **Target**: `esp32-devkitc-all-devices`
-   **Build Command**:
    ```bash
    podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && ./scripts/build/build_examples.py --target esp32-devkitc-all-devices build"
    ```
-   **Output Artifacts**:
    `<checkout_root>/out/esp32-devkitc-all-devices/all-devices-app.elf` and
    `all-devices-app.bin`

### C. Silicon Labs EFR32 (brd4187c)

-   **Target**: `efr32-brd4187c-all-devices`
-   **Build Command**:
    ```bash
    podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && ./scripts/build/build_examples.py --target efr32-brd4187c-all-devices build"
    ```
-   **Output Artifacts**:
    `<checkout_root>/out/efr32-brd4187c-all-devices/matter-silabs-all-devices-example.out`
    (and `.hex` / `.s37`)

### D. Nordic nRF Connect (nrf52840dk)

_Note: Building with the Zephyr SDK toolchain requires setting
`ZEPHYR_TOOLCHAIN_VARIANT=zephyr` explicitly._

-   **Target**: `nrf-nrf52840dk-all-clusters`
-   **Build Command**:
    ```bash
    podman exec -w /workspace bld_vscode bash -c "export ZEPHYR_TOOLCHAIN_VARIANT=zephyr && source scripts/activate.sh && ./scripts/build/build_examples.py --target nrf-nrf52840dk-all-clusters build"
    ```
-   **Output Artifacts**:
    `<checkout_root>/out/nrf-nrf52840dk-all-clusters/merged.hex` (contains both
    bootloader and app) and `zephyr/zephyr.elf`

### E. Telink (tlsr9518adk80d)

-   **Target**: `telink-tlsr9518adk80d-all-devices`
-   **Build Command**:
    ```bash
    podman exec -w /workspace bld_vscode bash -c "source scripts/activate.sh && ./scripts/build/build_examples.py --target telink-tlsr9518adk80d-all-devices build"
    ```
-   **Output Artifacts**:
    `<checkout_root>/out/telink-tlsr9518adk80d-all-devices/zephyr/zephyr.bin`

---

## 6. Accessing Build Artifacts

Because the repository root is mounted as a volume
(`--volume /path/to/connectedhomeip:/workspace`), any build outputs written to
`/workspace/out/` inside the container are **immediately accessible** on the
host filesystem at `<checkout_root>/out/`.

There is **no need** to copy binaries using `podman cp`. You can access or run
the built binary directly from your host repository checkout:

```bash
cp out/<target>/<binary_name> /path/to/destination
```
