# Telink Matter Build and Release Notes Update Script

## Overview

The script `docs/platforms/telink/releases/build_and_update_matter_notes.py`
can:

1. Build all Telink board Matter sample applications (using `build_examples.py`)
2. Extract memory usage information (RAM/ROM) from build logs
3. Update the release note file
   `docs/platforms/telink/releases/telink_release_notes.md` (the Resource Usage
   section uses a table format)
4. Collect all sample firmware files for every board and create a separate zip
   archive per board
5. Organize all output under the `build_for_release/` directory

## Files

-   `docs/platforms/telink/releases/build_and_update_matter_notes.py` - main
    build and update script (alongside the release note working copy)
-   `docs/platforms/telink/releases/telink_release_notes.md` - release note
    working copy (in the same directory as the script; the script reads and
    updates it in-place, only replacing the Resource Usage section while
    preserving the remaining static content). At release time,
    `scripts/tools/telink/package_telink_firmware.sh` snapshots it into a
    versioned `telink_release_notes_<version>.md`.
-   `README.md` - Telink Matter SDK documentation
-   `build_for_release/` - root directory for all output
    -   `build_logs/` - directory containing all build logs
    -   `firmware/` - directory containing all board firmware files and zip
        archives

## Usage

### 0. Prerequisites

Before running the script, set up the Telink Zephyr SDK path and activate the
build environment:

```bash
cd /home/ubuntu/zephyrproject/connectedhomeip

# Set the Telink Zephyr SDK path
export TELINK_ZEPHYR_BASE=/home/ubuntu/zephyrproject/zephyr

# Activate the Matter build environment
source ./scripts/activate.sh -p all,telink
```

### 1. Full run (build + update + firmware packaging)

```bash
cd /home/ubuntu/zephyrproject/connectedhomeip
python3 docs/platforms/telink/releases/build_and_update_matter_notes.py
```

This will:

-   Build all Telink board Matter samples using `build_examples.py`
-   Save logs to the `build_for_release/build_logs/` directory
-   Extract memory usage information from the logs
-   Update the `docs/platforms/telink/releases/telink_release_notes.md` file
    (only the Resource Usage section is replaced; the remaining static content
    is preserved)
-   Collect all firmware files into the `build_for_release/firmware/` directory
-   Create a separate firmware zip archive per board

### 2. Update the release notes only (skip building)

```bash
python3 docs/platforms/telink/releases/build_and_update_matter_notes.py --skip-build
```

Use this command when build logs already exist and you only need to update the
release notes and collect firmware.

### 3. Skip firmware collection and packaging

```bash
python3 docs/platforms/telink/releases/build_and_update_matter_notes.py --skip-firmware
```

Use this option when firmware collection and packaging are not needed.

## Firmware zip archive contents

Each board's zip archive contains:

-   `board_name/sample_name/zephyr.bin` - binary firmware file
-   `board_name/sample_name/zephyr.elf` - ELF firmware file
-   `board_name/sample_name/zephyr.hex` - Intel HEX firmware file (if present)
-   `board_name/sample_name/zephyr.dts` - devicetree file
-   `board_name/sample_name/.config` - configuration file
-   `board_name/sample_name/zephyr.signed.bin` - signed OTA firmware (if OTA is
    enabled)
-   `board_name/sample_name/mcuboot.bin` - MCUboot firmware (if present)

## Supported boards

1. tlsr9518adk80d (TLSR951X/B91)
2. tlsr9528a (TLSR952X/B92)
3. tlsr9118bdk40d (TLSR911X/W91)
4. tl3218x (TL321X)
5. tl3238x (TL323X)
6. tl7218x (TL721X)

## Supported sample applications

| Application              | Description                                                    |
| ------------------------ | -------------------------------------------------------------- |
| lighting-app             | Lighting application (supports OTA, Factory Data, LZMA, Shell) |
| light-switch-app         | Light switch application (supports OTA, LZMA, Factory Data)    |
| bridge-app               | Bridge application                                             |
| window-app               | Window covering application                                    |
| air-quality-sensor-app   | Air quality sensor application                                 |
| all-clusters-app         | All-clusters application                                       |
| all-clusters-minimal-app | Minimal all-clusters application (supports NFC Payload)        |
| contact-sensor-app       | Contact sensor application                                     |
| lock-app                 | Door lock application (supports DFU over SMP)                  |
| smoke-co-alarm-app       | Smoke/CO alarm application                                     |
| pump-controller-app      | Pump controller application                                    |
| shell                    | Shell application                                              |
| thermostat               | Thermostat application                                         |
| ota-requestor-app        | OTA requestor application                                      |

## Script features

-   Structured class design, easy to maintain and extend
-   Built on top of `build_examples.py`, consistent with Matter CI
-   Updates the release note file in-place, keeping all static content
    consistent
-   Resource Usage section uses a table format, clearly showing memory usage of
    each sample on each board
-   Only the Resource Usage section is updated; other sections are left
    unchanged
-   Supports adding new boards and sample applications
-   Detailed log output
-   Build step can be skipped to update directly from existing logs
-   Automatically collects firmware files and creates a separate zip archive per
    board
-   Option to skip firmware collection and packaging

## Adding a new board or sample

In the `TelinkMatterBuildManager` class:

1. Add a new build configuration (board, app_name, target_suffix) in the
   `_get_build_targets()` method
2. Add the new board family mapping in the `board_family` dictionary
3. Add the new board display order in the `board_order` list

For example, to add a new lighting-app build:

```python
targets.append(("tl3238x", "lighting-app", "tl3238x-light-ota-factory-data"))
```

## Updating the release note file

If you need to update the static content of the release note file (such as
Introduction, Version Information, Additional Notes, etc.), simply edit the
`docs/platforms/telink/releases/telink_release_notes.md` working copy directly.
The script only replaces the Resource Usage section; the remaining static
sections are preserved. At release time, `package_telink_firmware.sh` snapshots
it into a versioned archive file based on the git tag.

## Build target format

The build target format is `telink-<board>-<app>[-<options>]`, for example:

-   `telink-tl3238x-light-ota-factory-data-4mb` - TL3238X lighting application
    with OTA, Factory Data, 4MB Flash
-   `telink-tl7218x_retention-light-switch-ota-compress-lzma-factory-data` -
    TL7218X Retention light switch application
-   `telink-tlsr9518adk80d-light-ota-rpc-factory-data-4mb` - B91 lighting
    application with RPC, OTA, Factory Data

Available options:

-   `ota` - enable OTA support
-   `factory-data` - enable Factory Data support
-   `compress-lzma` - enable LZMA compression
-   `shell` - enable Shell
-   `rpc` - enable Pigweed RPC
-   `dfu-smp` - enable DFU over SMP
-   `4mb` - 4MB Flash
-   `log-progress` - reduce log output (progress and errors only)
-   `nfc-payload` - NFC Payload support
