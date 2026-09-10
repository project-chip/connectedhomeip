# Telink Matter Getting Started Guide

This guide walks you through setting up the build environment, building,
flashing, and commissioning a Matter-over-Thread device on Telink RISC-V SoC
platforms.

## Prerequisites

### Supported Chips and Boards

| Board target     | Chip Family    | Flash (default) | Notes                                 |
| ---------------- | -------------- | :-------------: | ------------------------------------- |
| `tlsr9518adk80d` | TLSR951X / B91 |      2 MB       | Legacy; broad sample coverage         |
| `tlsr9528a`      | TLSR952X / B92 |      2 MB       | `_retention` variant for low-power    |
| `tlsr9118bdk40d` | TLSR911X / W91 |      2 MB       | Legacy; broad sample coverage         |
| `tl3218x`        | TL321X         |      2 MB       | `_retention` variant for low-power    |
| `tl3238x`        | TL323X         |      2 MB       | Dual-mode (Matter + Zigbee) supported |
| `tl7218x`        | TL721X         |      2 MB       | `_retention` variant for low-power    |

> See the [Release Notes](./releases/telink_release_notes.md) for the exact chip
> versions, EVK versions, and per-example support matrix validated in each
> release.

### Telink Matter ↔ Telink Zephyr Dependency

The Telink Matter SDK is built **on top of** the Telink Zephyr SDK. The two are
tightly coupled and must be used as a matched pair:

-   The **Telink Zephyr SDK** provides Zephyr RTOS, the Telink HAL, the BLE
    stack, MCUBoot, OpenThread, and the WEST build toolchain.
-   The **Telink Matter SDK** (this repository) builds upon that foundation to
    deliver the Matter protocol stack and Telink example applications.

Both are required — the Matter SDK cannot be built without the Zephyr SDK, and
the Zephyr SDK alone does not provide Matter support.

> ⚠️ **Version pairing:** Each Telink Matter release is validated against a
> specific Telink Zephyr SDK tag. Using a different Zephyr revision may cause
> build failures or runtime issues. See the
> [Release Notes](./releases/telink_release_notes.md) for the matched pair.

## Step 1: Set up the Telink Zephyr SDK

The Telink Matter examples use `west build` and require the Telink Zephyr SDK to
be installed and exported via `TELINK_ZEPHYR_BASE`.

### 1.1 Install host dependencies

```bash
sudo apt update && sudo apt upgrade
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev
```

Minimum versions: CMake 3.20.0, Python 3.6, devicetree compiler 1.4.6.

### 1.2 Install west

```bash
pip3 install --user -U west
echo 'export PATH=~/.local/bin:"$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### 1.3 Initialize the Zephyr workspace and switch to the Telink fork

```bash
west init ~/zephyrproject
cd ~/zephyrproject
west update
west blobs fetch hal_telink
west zephyr-export

cd zephyr
git remote add telink https://github.com/telink-semi/zephyr
git fetch telink
git checkout develop     # or the branch matched to your Matter release
cd ..
west update
west blobs fetch hal_telink
```

### 1.4 Install the Zephyr SDK toolchain (riscv64)

Download **Zephyr SDK v0.17.0** and install the riscv64 toolchain:

```bash
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.0/zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz
tar xvf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz -C ~
cd ~/zephyr-sdk-0.17.0
./setup.sh -t riscv64-zephyr-elf -h -c
```

### 1.5 Source the Zephyr environment

```bash
echo "source ~/zephyrproject/zephyr/zephyr-env.sh" >> ~/.bashrc
source ~/.bashrc
```

### 1.6 (hal_v2 chips only) Fetch the TL323X BLE SDK

For **TL322X / TL323X** (hal_v2) chips, additionally fetch the BLE SDK:

```bash
cd ~/zephyrproject/modules/hal/telink/hal_v2
chmod +x fetch_sdk.sh
./fetch_sdk.sh
```

### 1.7 Verify the Zephyr installation

Build the Hello World sample to confirm the toolchain works:

```bash
cd ~/zephyrproject/zephyr
west build -p auto -b tlsr9518adk80d zephyr/samples/hello_world -d build_helloWorld
```

For more details, see the
[Telink Zephyr Getting Started guide](https://github.com/telink-semi/zephyr/blob/develop/doc/telink/getting_started/index.md).

## Step 2: Get the Matter source code

### 2.1 Install Matter host dependencies

```bash
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
  libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
  python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

### 2.2 Clone and set up the Matter repository

```bash
git clone https://github.com/telink-semi/connectedhomeip.git
cd connectedhomeip
git checkout <telink_matter_branch>     # e.g. dev-tlk_v1.5
./scripts/checkout_submodules.py --platform telink,linux
```

### 2.3 Bootstrap the Matter build environment

The first run takes a while as it downloads all pip/gn dependencies:

```bash
source scripts/bootstrap.sh
```

> If you switch commits or branches later, re-run bootstrap after cleaning:
>
> ```bash
> rm -rf .environment
> source scripts/bootstrap.sh
> ```

## Step 3: Build a Matter example

Telink Matter examples live under `examples/<app-name>/telink/`. Each example is
built with `west build` (which invokes the Zephyr build system under the hood).

### 3.1 Activate the environment

```bash
cd connectedhomeip
source scripts/activate.sh
```

### 3.2 Build with west

```bash
cd examples/lighting-app/telink
west build -b <build_target>
```

Replace `<build_target>` with your board, e.g. `tlsr9518adk80d`, `tlsr9528a`,
`tl3218x`, `tl3238x`, `tl7218x`, or `tlsr9118bdk40d`.

**Example -- A simple build with 2 MB Flash**

```bash
west build -b tl3238x
```

If your board has a flash size other than the default 2 MB, specify it:

```bash
west build -b tl3238x -- -DFLASH_SIZE=4m
```

The built firmware is at `build/zephyr/zephyr.bin`. When MCUBoot + OTA is
enabled, a merged `build/zephyr/merged.bin` (MCUBoot + app) is also generated.

### 3.3 Build with build_examples.py (CI-style)

Alternatively, use the Matter build system to build via GN:

```bash
./scripts/build/build_examples.py --target telink-tl7218x-lighting-compress-lzma build
```

List all available Telink targets:

```bash
./scripts/build/build_examples.py targets | grep telink
```

Common target options include `-compress-lzma`, `-ota`, `-dfu-smp`,
`-factory-data`, `-shell`, `-rpcs`, and `-4mb-flash`.

### 3.4 TL3238X build configurations

TL3238X supports several configurations depending on flash size and dual-mode
(Matter + Zigbee) needs. Key combinations:

| Config                            | Flash | OTA | LZMA | Dual-mode | Conf file                                    |
| --------------------------------- | :---: | :-: | :--: | :-------: | -------------------------------------------- |
| Default (no OTA, no MCUBoot)      | 2 MB  | No  |  No  |    No     | `boards/tl3238x.conf`                        |
| OTA + BT DFU + LZMA               | 2 MB  | Yes | Yes  |    No     | `boards/tl3238x_2m_flash_ota_lzma.conf`      |
| Dual-mode (Matter + Zigbee) + OTA | 4 MB  | Yes |  No  |    Yes    | `boards/tl3238x_4m_flash_dual_mode_ota.conf` |

> ⚠️ For 2 MB Flash + OTA, LZMA compression is **required** — a non-LZMA build
> will not fit.

Example — 2 MB Flash with OTA + LZMA, software version 2:

```bash
west build -p -b tl3238x -d build_tl3238x_lzma_v1 -- \
  -DCONF_FILE="prj.conf boards/tl3238x_2m_flash_ota_lzma.conf" \
  -DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2
```

## Step 4: Flash the firmware

### 4.1 Option A: west flash (recommended for development)

If your board is connected via a supported debug probe:

```bash
west flash --erase
```

### 4.2 Option B: BDT (Telink flashing tool)

Telink provides the **BDT** (Burning Debug Tool) for flashing. On Windows use
the BDT GUI (`Telink BDT.exe`); on Linux use the `bdt` CLI. See the
[Telink Zephyr Getting Started — Flash the Firmware](https://github.com/telink-semi/zephyr/blob/develop/doc/telink/getting_started/index.md#flash-the-firmware)
section for the chip-to-BDT name mapping and the unlock/erase/write flow.

A typical Linux BDT session:

```bash
./bdt B92 wf 0 -s 2040k -e      # erase (replace B92 with your chip name)
./bdt B92 wf 0 -i zephyr.bin    # write
./bdt B92 rst                   # reset
```

> For B92 / TL321X / TL721X, run `./bdt <chip> ulf` to unlock the flash before
> erasing. For TL322X / TL323X, use the **TGui-BDT** tool (or `sctool` on Linux)
> with the on-board programmer.

### 4.3 UART console

Connect UART to view device logs:

| Name | Pin                           |
| :--: | :---------------------------- |
|  RX  | PB0 (pin 17 of J34 connector) |
|  TX  | PB2 (pin 16 of J34 connector) |
| GND  | GND                           |

Baud rate: **115200** bits/s.

## Step 5: Commission and control with chip-tool

### 5.1 Build chip-tool

Follow the
[chip-tool guide](../../development_controllers/chip-tool/chip_tool_guide.md) to
build the Matter controller.

### 5.2 Set up a Thread border router

Matter-over-Thread devices require a Thread border router to bridge the Thread
network to Wi-Fi/Ethernet. Follow the
[OpenThread border router guide](../openthread/openthread_border_router_pi.md)
to set up a Raspberry Pi border router.

### 5.3 Pair the device

```bash
./chip-tool pairing ble-thread ${NODE_ID} hex:${DATASET} ${PIN_CODE} ${DISCRIMINATOR}
```

Example:

```bash
./chip-tool pairing ble-thread 1234 hex:0e080000000000010000000300000f35060004001fffe0020811111111222222220708fd61f77bd3df233e051000112233445566778899aabbccddeeff030e4f70656e54687265616444656d6f010212340410445f2b5ca6f2a93a55ce570a70efeecb0c0402a0fff8 20202021 3840
```

### 5.4 Control the lighting

```bash
./chip-tool onoff on 1                              # switch on
./chip-tool onoff off 1                             # switch off
./chip-tool onoff read on-off 1                     # read state
./chip-tool levelcontrol move-to-level 32 0 0 0 1   # set brightness
```

## Buttons and LEDs

The on-board buttons and LEDs provide basic control and status feedback:

| Button   | Function               | Description                                                    |
| :------- | :--------------------- | :------------------------------------------------------------- |
| Button 1 | Factory reset          | Press 3 times to forget the commissioned Thread network        |
| Button 2 | App control            | Manually triggers the application state (e.g. toggle lighting) |
| Button 3 | Thread start           | Commission with static credentials and enable Thread           |
| Button 4 | Open commission window | Opens the BLE commissioning window                             |

**Red LED** — Thread network state:

| State           | Meaning                                  |
| :-------------- | :--------------------------------------- |
| Short pulses    | Not commissioned; Thread disabled        |
| Frequent pulses | Commissioned; joining the Thread network |
| Wide pulses     | Joined to the Thread network as a CHILD  |

**Green LED** — Identify (blinks when the Identify command is received).

## OTA Firmware Update

OTA is enabled by default only for the `ota-requestor-app` example. To enable
OTA on other examples, set `CONFIG_CHIP_OTA_REQUESTOR=y` in the example's
`prj.conf`.

After building with OTA enabled, the following artifacts are generated:

| File                  | Purpose                                               |
| --------------------- | ----------------------------------------------------- |
| `merged.bin`          | Main binary to flash (MCUBoot + app), for first flash |
| `matter.ota`          | OTA image for the OTA Provider                        |
| `merged_dfu.lzma.bin` | LZMA-compressed DFU image for BLE SMP DFU             |

To test OTA with a Linux OTA Provider, refer to the
[OTA section in the lighting-app README](../../../examples/lighting-app/telink/README.md#ota-with-linux-ota-provider).

## Next steps

-   [Telink Release Notes](./releases/telink_release_notes.md) — version info,
    chip/EVK versions, per-example support matrix, and resource usage tables.
-   [Telink Zephyr Getting Started](https://github.com/telink-semi/zephyr/blob/develop/doc/telink/getting_started/index.md)
    — Zephyr SDK setup, BDT flashing details, and board overviews.
-   Per-example `README.md` files under `examples/<app>/telink/` —
    example-specific build commands, button/LED mappings, and chip-tool usage.
