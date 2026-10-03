# CC35XX Matter Getting Started Guide

## Prerequisites

**Hardware:**
- LP-EM-CC35X1 Rev A LaunchPad

**Software:**
- Ubuntu 22.04 (host machine)
- SimpleLink Wi-Fi SDK `10.10.00.18`
- SysConfig `1.26.3`
- SimpleLink Wi-Fi Toolbox `4.1.16`
- cmake `3.21.3`
- arm gnu toolchain `12.3.rel1`

> **Note:** SysConfig and SimpleLink Wi-Fi Toolbox are installed by the SimpleLink Wi-Fi SDK installer and do not need to be installed separately.

---

## Overview

This guide covers the environment setup required to build and flash a Matter example to the TI CC35XX LaunchPad.

The TI CC35XX currently supports the **lighting-app** example demonstrating the use of the OnOff cluster. The On/Off operations are emulated through the status of the red LED on the LaunchPad.

**Key constraints:**
- The CC35XX must be on the same target network as the Matter Controller.
- The SSID, Password, and Security Type of the Access Point must be **hardcoded** into the example application (BLE-based commissioning is not yet supported).

For build, flash, and commissioning instructions, refer to the [lighting-app README](../../../examples/lighting-app/ti/cc35xx/README.md).

For detailed instructions on environment setup for building matter, refer to [Building guide](https://project-chip.github.io/connectedhomeip-doc/guides/BUILDING.html) from matter documentation. 

---

## 1. Install Build Dependencies

> **Note:** The following steps assume a Linux host running **Ubuntu 22.04**.

```bash
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev \
  libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip \
  libgirepository1.0-dev libcairo2-dev libreadline-dev
```

## 2. Clone the Matter Repository

```bash
# Clone the Matter repository with CC35XX support
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Pull platform submodules
# The linux platform is included to support building the chip-tool and ota-provider examples on the host machine
./scripts/checkout_submodules.py --shallow --platform cc35xx linux --recursive

# Bootstrap the build environment (one-time setup, may take several minutes)
source ./scripts/bootstrap.sh
```

> **Note:** After the one-time bootstrap, activate the environment in every new shell before building:
> ```bash
> source ./scripts/activate.sh
> ```

## 3. Setup the SimpleLink Wi-Fi SDK

1. Install cmake, arm gnu toolchain if not present.

2. Set appropriate paths to SYSCONFIG_TOOL, SIMPLELINK_WIFI_TOOLBOX_INSTALL_DIR, CMAKE, GCC_ARMCOMPILER in imports.mak file in SDK.

3. Run below command to build library archives needed for building matter application.

```bash
make GENERATOR=Ninja build-gcc
```

---

## Next Steps

Refer to the [lighting-app README](../../../examples/lighting-app/ti/cc35xx/README.md) for:
- Hardcoding Wi-Fi credentials
- Building the example
- Flashing the device
- Commissioning and controlling the light with CHIP Tool

---

## Appendix: Version Reference

| Component | Version |
|---|---|
| SimpleLink Wi-Fi SDK | `10.10.00.18` |
| SysConfig | `1.26.3` |
| SimpleLink Wi-Fi Toolbox | `4.1.16` |
| JTAG interface | XDS110 |
