# nRF Connect development environment

Matter nRF Connect examples in this repository are built with Nordic
Semiconductor's
[nRF Connect SDK](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/index.html)
(NCS). The SDK version is pinned by the
[ncs-matter add-on](https://nrfconnectdocs.nordicsemi.com/addons/ncs-matter/latest/index.html)
(`third_party/nrfconnect/ncs-matter`).

Use the automated setup script to install dependencies, download the pinned NCS
release and toolchain, bootstrap the Matter build environment, and open an
activated shell ready to build.

<hr>

## Quick start

From the connectedhomeip repository root, run:

```shell
python3 scripts/setup/nrfconnect/setup.py
```

Pass `-y` to automatically download and install `nrfutil` when it is not found
on your PATH. Use `--sdk-path` to install the SDK and toolchain into a custom
directory instead of the default `.environment/nrfconnect/sdk` location. Use
`--skip-bootstrap` to skip running `scripts/bootstrap.sh` if the Matter build
environment is already set up.

When setup completes, the script launches a shell with both the nRF Connect SDK
and Matter environments active. You can build immediately, for example:

```shell
./scripts/build/build_examples.py --target nrf-nrf54lm20dk-light build
```

<hr>

## Setup script overview

`scripts/setup/nrfconnect/setup.py` is a phased installer. Each phase prints a
header, runs one logical step, and stops on the first failure. The script
refuses to start if an nRF Connect environment is already active in the current
shell; deactivate it first with `scripts/setup/nrfconnect/deactivate.sh` (or
`deactivate.bat` on Windows).

The NCS revision is read from `third_party/nrfconnect/ncs-matter/west.yml`, so
the installed SDK always matches the Matter tree you checked out.

### Setup phases

| Phase | Title                                        | What it does                                                                                                                                                                                                                                        |
| ----- | -------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1     | Checking out submodules                      | Runs `scripts/checkout_submodules.py --platform nrfconnect` to fetch Matter Git submodules required for the nRF Connect platform, including the ncs-matter add-on (`third_party/nrfconnect/ncs-matter`).                                            |
| 2     | Checking nrfutil installation                | Looks for the Nordic `nrfutil` CLI on `PATH`. If it is missing, offers to download the correct binary for your OS and CPU architecture, or exits with manual install instructions.                                                                  |
| 3     | Installing nrfutil modules                   | Installs the `device` and `sdk-manager` nrfutil extensions used to download the SDK and toolchain and to flash firmware.                                                                                                                            |
| 4     | Downloading nRF Connect SDK                  | Uses `nrfutil sdk-manager` to install the NCS release pinned by ncs-matter and the matching compiler toolchain under the SDK install directory. Skips download when both are already present.                                                       |
| 5     | Finalizing environment                       | Sets `ZEPHYR_BASE`, writes `.zephyrrc` (or `.zephyrrc.cmd` on Windows) for the NCS toolchain, updates `PATH`, and runs `scripts/setup/nrfconnect/update_ncs.py --update` inside the NCS toolchain to synchronize the workspace with Matter sources. |
| 6     | Checking ZEPHYR_BASE                         | Verifies that `ZEPHYR_BASE` points to a valid Zephyr tree after the environment has been configured.                                                                                                                                                |
| 7     | Running Matter bootstrap                     | Runs `scripts/bootstrap.sh -p nrfconnect` to create the Matter Pigweed virtual environment and install nRF Connect platform build dependencies. Skipped when `--skip-bootstrap` is passed.                                                          |
| 8     | Installing west in the activated environment | Activates the nRF Connect and Matter environments and installs `west` into the Matter Python virtual environment with pip (`west>=1.4.0`).                                                                                                          |
| 9     | Launching activated shell                    | Sources `scripts/setup/nrfconnect/activate.sh` and opens a new shell where you can build immediately.                                                                                                                                               |

### Command-line options

| Option             | Description                                                                                                                                                                |
| ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-y`, `--yes`      | Automatically download `nrfutil` when it is not found on `PATH`.                                                                                                           |
| `--sdk-path PATH`  | Install the SDK and toolchain into `PATH` instead of `.environment/nrfconnect/sdk`. Validates that the required SDK and toolchain are present and installs missing pieces. |
| `--skip-bootstrap` | Skip running `scripts/bootstrap.sh` after setup completes. Use when the Matter build environment is already bootstrapped.                                                  |
| `--cleanup`        | Remove the `.environment` directory with downloaded SDK and toolchain files. Matter bootstrap must be run again afterward.                                                 |

### First-time setup

The first run may take a significant amount of time and download several
gigabytes of data:

-   Matter Git submodules for the nRF Connect platform (including the ncs-matter
    add-on and its dependencies)
-   the `nrfutil` command-line tool (if not already installed)
-   nrfutil `device` and `sdk-manager` modules
-   the nRF Connect SDK release pinned by the ncs-matter add-on
-   the compiler toolchain matching that SDK version

All downloaded SDK and toolchain files are stored locally under
`.environment/nrfconnect/sdk` in the Matter repository root (unless `--sdk-path`
is used).

### Subsequent runs

If you keep the `.environment` directory, later runs of the setup script are
much faster. Already installed SDK and toolchain components are detected and
skipped, and only missing or outdated pieces are updated.

<hr>

## Activating the environment manually

Setup ends in an activated shell, but new terminals need activation again.

**Linux / macOS / Git Bash:**

```shell
source scripts/setup/nrfconnect/activate.sh
```

**Windows Command Prompt:**

```batch
call scripts\setup\nrfconnect\activate.bat
```

`activate.sh` restores the NCS toolchain via `.zephyrrc`, sets `ZEPHYR_BASE`,
then sources the Matter `scripts/activate.sh` so both environments are active
together. It also reconciles Python settings so the Matter Pigweed virtual
environment takes precedence over the NCS toolchain Python.

To restore the previous shell environment:

**Linux / macOS / Git Bash:**

```shell
source scripts/setup/nrfconnect/deactivate.sh
```

**Windows Command Prompt:**

```batch
call scripts\setup\nrfconnect\deactivate.bat
```

<hr>

## Building nRF Connect samples

With the environment active, build from the connectedhomeip repository root
using `scripts/build/build_examples.py`:

```shell
./scripts/build/build_examples.py --target <target-name> build
```

List available targets with:

```shell
./scripts/build/build_examples.py targets | grep nrf-
```

Alternatively, build a specific example with `west` from its `nrfconnect`
directory:

```shell
cd examples/lighting-app/nrfconnect
west build -b nrf52840dk/nrf52840 --sysbuild
```

After a successful `build_examples.py` build, the nRF builder prints
`nrfutil device program` commands for the generated merged HEX file(s).

<hr>

## Requirements summary

The table below lists what you need on the host and on the bench to build,
flash, and run Matter nRF Connect samples from this repository.

| Requirement                               | Required                | Purpose                                                | How to obtain                                                                                        |
| ----------------------------------------- | ----------------------- | ------------------------------------------------------ | ---------------------------------------------------------------------------------------------------- |
| Linux, macOS, or Windows host             | Yes                     | Run setup, build, and flash tools                      | Your development machine                                                                             |
| Python 3                                  | Yes                     | Setup script, west, and build helpers                  | OS package manager or python.org                                                                     |
| Git                                       | Yes                     | Submodules and NCS west workspace                      | OS package manager                                                                                   |
| Internet access                           | Yes (first setup)       | Download submodules, SDK, and toolchain                | Network connection during setup                                                                      |
| `nrfutil` CLI                             | Yes                     | SDK/toolchain management and device flashing           | Installed by setup phase 2; use `-y` for automatic download                                          |
| nrfutil `device` module                   | Yes                     | Program firmware onto a development kit                | Installed by setup phase 3                                                                           |
| nrfutil `sdk-manager` module              | Yes                     | Download pinned NCS release and toolchain              | Installed by setup phase 3                                                                           |
| ncs-matter add-on submodule               | Yes                     | Pins compatible NCS revision and Matter integration    | Checked out by setup phase 1                                                                         |
| nRF Connect SDK (pinned revision)         | Yes                     | Zephyr RTOS, OpenThread, SoftDevice, board support     | Downloaded by setup phase 4 into `.environment/nrfconnect/sdk`                                       |
| Matching Zephyr toolchain                 | Yes                     | Cross-compile firmware for the target SoC              | Downloaded by setup phase 4                                                                          |
| Matter bootstrap (`scripts/bootstrap.sh`) | Yes                     | Pigweed venv, GN/Ninja build deps for Matter           | Run by setup phase 7 (unless `--skip-bootstrap`)                                                     |
| Activated environment                     | Yes (each shell)        | Exposes `ZEPHYR_BASE`, west, and Matter tools together | `scripts/setup/nrfconnect/activate.sh` or setup phase 8                                              |
| SEGGER J-Link software                    | Yes on Linux            | USB debug probe support used when flashing             | [SEGGER J-Link download](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) |
| nRF development kit or dongle             | Yes (on-target testing) | Hardware that runs the sample firmware                 | Nordic development kit matching the build target                                                     |
| USB connection to the kit                 | Yes (on-target testing) | Power and programming interface                        | Connect DK/dongle to the host                                                                        |

### Optional but useful

| Item                                          | Purpose                                                                                           |
| --------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| `--sdk-path`                                  | Keep SDK/toolchain outside the Matter tree                                                        |
| `--skip-bootstrap`                            | Re-run setup without repeating Matter bootstrap                                                   |
| `--serial-number` on `nrfutil device program` | Select a specific kit when multiple devices are connected (`nrfutil device list`)                 |
| nRF Connect for Desktop Toolchain Manager     | Manual alternative to `setup.py` for installing NCS; still run `update_ncs.py --update` afterward |
