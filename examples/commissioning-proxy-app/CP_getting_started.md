# Commissioning Proxy — Getting Started Guide

This guide walks a first-time user through every step needed to build, deploy,
and use the Matter Commissioning Proxy on Raspberry Pi hardware.

The Commissioning Proxy (CP) acts as a tunnel between a commissioner (chip-tool)
and a commissionee that is only reachable over a transport not directly
available on the commissioner, or out of range. This guide focuses on **Wi-Fi
PAF (Wi-Fi Aware / NAN)** as the transport, but the Commissioning Proxy
architecture also supports other transports such as BLE.

```
chip-tool  ──Matter (TCP/IP)──►  Commissioning Proxy App  ──Wi-Fi PAF (NAN)──►  Commissionee
```

> **What is Wi-Fi PAF?** Wi-Fi PAF (Protocol for Announcement and Finding) is a
> Matter transport built on Wi-Fi Aware (NAN — Neighbor Awareness Networking).
> It enables device discovery and commissioning without joining a Wi-Fi network
> first, using short-range off-AP Wi-Fi signaling. See **Section 8 of the
> Matter Test-Harness User Guide** for full background on Wi-Fi PAF and its
> hardware/software requirements.

<hr>

-   [1. Components Needed](#1-components-needed)
    -   [1.1 Hardware](#11-hardware)
    -   [1.2 Software Prerequisites](#12-software-prerequisites)
        -   [1.2.1 USB dongle software](#121-usb-dongle-software)
        -   [1.2.2 Build machine](#122-on-the-build-machine-ubuntu-2204-2404-x86-64)
-   [2. Build wpa_supplicant with the Matter NAN patch (Commissioning Proxy RPi only)](#2-build-wpa_supplicant-with-the-matter-nan-patch-commissioning-proxy-rpi-only)
    -   [2.1 What the patch does](#21-what-the-patch-does)
        -   [2.1.1 Create the patch file](#211-create-the-wpa-supplicant-matter-patch-patch-file)
    -   [2.2 Build steps](#22-build-steps)
    -   [2.3 Install on the Commissioning Proxy Raspberry Pi](#23-install-on-the-commissioning-proxy-raspberry-pi)
-   [3. Clone the connectedhomeip repository on the build machine](#3-clone-the-connectedhomeip-repository-on-the-build-machine)
-   [4. Build the Matter SDK (Raspberry Pi)](#4-build-the-matter-sdk-raspberry-pi)
    -   [4.1 One-time Docker image setup](#41-one-time-docker-image-setup)
    -   [4.2 Build the Commissioning Proxy app (RPi arm64)](#42-build-the-commissioning-proxy-app-rpi-arm64)
    -   [4.3 Build chip-tool (RPi arm64)](#43-build-chip-tool-rpi-arm64)
    -   [4.4 Build the End Device app (RPi arm64)](#44-build-the-end-device-app-rpi-arm64)
-   [5. Deploy to Raspberry Pi](#5-deploy-to-raspberry-pi)
-   [6. Configuring the Commissioning Proxy](#6-configuring-the-commissioning-proxy)
    -   [6.1 Start the Commissioning Proxy](#61-start-the-commissioning-proxy)
    -   [6.2 Add the Commissioning Proxy onto the Fabric](#62-add-the-commissioning-proxy-onto-the-fabric)
-   [7. Configuring the End Device (ED)](#7-configuring-the-end-device-ed)
-   [8. Scanning](#8-scanning)
    -   [8.1 Run a foreground scan](#81-run-a-foreground-scan)
    -   [8.2 Start a background scan](#82-start-a-background-scan)
    -   [8.3 Read cached results](#83-read-cached-results)
    -   [8.4 Stop a background scan](#84-stop-a-background-scan)
-   [9. Commissioning the End Device Through the Commissioning Proxy](#9-commissioning-the-end-device-through-the-commissioning-proxy)
-   [10. Troubleshooting](#10-troubleshooting)

<hr>

## 1. Components Needed

### 1.1 Hardware

| Component                    | Role                             | Notes                                                                                                                                                                                                                              |
| ---------------------------- | -------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Raspberry Pi 4 or 5**      | DUT — Commissioning Proxy        | Runs `chip-commissioning-proxy-app`. Requires a USB Wi-Fi dongle with NAN USD support (see note below — the on-board RPi Wi-Fi does **not** support Wi-Fi PAF).                                                            |
| **Raspberry Pi 4 or 5**      | TH — Test Harness / Commissioner | Runs chip-tool and (optionally) Python certification tests. Connected to the same IP network as the proxy RPi. Installation instructions are in section 4.1. TH Installation on Raspberry Pi of the Matter Test-Harness User Guide |
| **Raspberry Pi 4 or 5**      | ED — End Device / Commissionee   | Runs `chip-lighting-app`. Also requires a USB Wi-Fi dongle with NAN USD support.                                                                                                                                                   |
| **WLAN USB dongle** (×2)     | DUT + ED                         | A USB Wi-Fi adapter that supports Wi-Fi Aware (NAN USD) with the latest hostapd/wpa_supplicant. Compatible models: See Section 8 of the Matter Test-Harness User Guide                                                             |
| **Build machine**            | —                                | Ubuntu 22.04 or 24.04 x86-64 with Docker installed. Not needed at runtime — used only to cross-compile the binaries.                                                                                                               |
| **Ethernet switch / router** | —                                | Connects all RPis and the build machine on the same subnet for chip-tool to reach the Commissioning Proxy over IP. The ED does not need to be on the network                                                                       |

> **Why a USB dongle?** The on-board Wi-Fi chipset on Raspberry Pi 4
> and 5 does not support the Wi-Fi Aware NAN USD interface required by Wi-Fi
> PAF. A supported external USB dongle must be plugged into both the Proxy RPi
> and the End Device RPi before starting either app.

### 1.2 Software Prerequisites

### 1.2.1 USB dongle software

Instructions on how to configure the PAF devices can be found here
https://groups.csa-iot.org/wg/members-all/document/43356

The instructions above miss the requirement that the following need installing,
otherwise this error will be seen in the Commissioning Proxy log on start up.
`[PAF] Failed to start Wi-Fi PAF publish: src/platform/Linux/ConnectivityManagerImpl_WiFiPafWpaSupplicant.cpp:455: CHIP Error 0x00000003: Incorrect state`

```bash
# Install these on RPi with PAF USB dongles to allow iwconfig
sudo apt install net-tools
sudo apt install wireless-tools
```

### 1.2.2 On the **build machine** (Ubuntu 22.04/24.04 x86-64):

```bash
sudo apt-get update
sudo apt-get install -y git docker.io python3 python3-pip \
    libnl-3-dev libnl-genl-3-dev pkg-config libssl-dev \
    libdbus-1-dev build-essential
sudo usermod -aG docker $USER   # log out and back in after this
```

<hr>

## 2. Build wpa_supplicant with the Matter NAN patch (Commissioning Proxy RPi only)

The Proxy RPi requires a custom build of wpa_supplicant. The standard Ubuntu
package does not enable `CONFIG_NAN_USD` (Wi-Fi Aware Un Synchronized Service
Discovery), and is missing a small extension needed for Commissioning Proxy
scanning.

> **End Device (ED) wpa_supplicant**: The End Device RPi uses the standard Wi-Fi
> PAF wpa_supplicant build described in **Section 8 of the Matter Test-Harness
> User Guide**. The patch below is only required on the Proxy RPi.

### 2.1 What the patch does

The patch file `wpa-supplicant-matter.patch` (in the same directory as this
guide) is primarily about enabling a **NAN scan mechanism** for the
Commissioning Proxy.

When the proxy performs a NAN scan to discover nearby commissionees, it
subscribes to the Matter NAN service as a passive listener. Without the patch,
wpa_supplicant automatically sends a NAN Follow-up frame back to every publisher
it discovers — this would accidentally initiate a PAF session (i.e. a connection
attempt) for every device found during a scan. The patch adds a `discovery_only`
flag that tells wpa_supplicant to suppress those automatic Follow-up replies for
scan-only subscriptions, so the proxy can observe which devices are present
without triggering any connections.

The four changes are:

| File                                      | Change                                                                                                           |
| ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `wpa_supplicant/defconfig`                | Enables `CONFIG_NAN_USD=y` so NAN USD support is compiled in                                                     |
| `src/common/nan_de.h`                     | Adds `discovery_only` boolean to `nan_subscribe_params`                                                          |
| `src/common/nan_de.c`                     | Skips the automatic Follow-up reply to the publisher when `discovery_only` is set                                |
| `wpa_supplicant/dbus/dbus_new_handlers.c` | Exposes `discovery_only` as a D-Bus parameter so the Matter stack can pass it from the background-scan call path |

### 2.1.1 Create the `wpa-supplicant-matter.patch` patch file.

Save the following content as `/home/ubuntu/wpa-supplicant-matter.patch`:

```bash
diff --git a/src/common/nan_de.c b/src/common/nan_de.c
index 2af1afd73..1196ec617 100644
--- a/src/common/nan_de.c
+++ b/src/common/nan_de.c
@@ -910,7 +910,8 @@ static void nan_de_rx_publish(struct nan_de *de, struct nan_de_service *srv,
 		nan_de_tx_multicast(de, srv, instance_id);
 	}

-	if (!de->offload && !srv->subscribe.active && req_instance_id == 0) {
+	if (!de->offload && !srv->subscribe.active && req_instance_id == 0 &&
+	    !srv->subscribe.discovery_only) {
 		/* Passive subscriber replies with a Follow-up message without
 		 * Service Specific Info field if it received a matching
 		 * unsolicited Publish message. */
diff --git a/src/common/nan_de.h b/src/common/nan_de.h
index b2688a83c..6b9e2ad1f 100644
--- a/src/common/nan_de.h
+++ b/src/common/nan_de.h
@@ -154,6 +154,11 @@ struct nan_subscribe_params {

 	/* Proximity ranging flag */
 	bool proximity_ranging;
+
+	/* If true, suppress the automatic Follow-up sent to the publisher when
+	 * a passive subscriber matches an unsolicited Publish frame.  Use for
+	 * discovery-only scans where no session is to be established. */
+	bool discovery_only;
 };

 /* Returns -1 on failure or >0 subscribe_id */
diff --git a/wpa_supplicant/dbus/dbus_new_handlers.c b/wpa_supplicant/dbus/dbus_new_handlers.c
index 76a9297eb..8a9cec6a3 100644
--- a/wpa_supplicant/dbus/dbus_new_handlers.c
+++ b/wpa_supplicant/dbus/dbus_new_handlers.c
@@ -6855,6 +6855,10 @@ DBusMessage * wpas_dbus_handler_nan_subscribe(DBusMessage *message,
 			for (i = 0; i < entry.array_len; i++)
 				int_array_add_unique(
 					&freq_list, entry.uint16array_value[i]);
+		} else if (os_strcmp(entry.key, "discovery_only") == 0 &&
+			   entry.type == DBUS_TYPE_BOOLEAN) {
+			params.discovery_only = entry.bool_value;
+			wpa_dbus_dict_entry_clear(&entry);
 		} else {
 			wpa_printf(MSG_DEBUG,
 				   "dbus: NANSubscribe - unsupported dict entry '%s'",
diff --git a/wpa_supplicant/defconfig b/wpa_supplicant/defconfig
index 84ac8ba12..2a8f3e9f3 100644
--- a/wpa_supplicant/defconfig
+++ b/wpa_supplicant/defconfig
@@ -681,4 +681,4 @@ CONFIG_DPP2=y
 #CONFIG_NO_WMM_AC=y

 # Wi-Fi Aware unsynchronized service discovery (NAN USD)
-#CONFIG_NAN_USD=y
+CONFIG_NAN_USD=y
```

### 2.2 Build steps

Run these steps on the **RPi build machine** (cross-compiling for arm64 is
possible but building natively on the RPi itself is simplest for
wpa_supplicant):

```bash
# 1. Clone upstream hostap
git clone https://w1.fi/hostap.git
cd hostap

# 2. Apply the Matter NAN patch
#    Adjust the path to match where you have cloned this repository.
#    Use the Hash below for the exact wpa_supplicant used
git checkout e17107912
git apply /home/ubuntu/wpa-supplicant-matter.patch

# 3. Copy the patched defconfig as the active build config
cd wpa_supplicant
cp defconfig .config

# 4. Install build dependencies
#    The Matter TH image installs some runtime libraries (libbz2, libnl3,
#    libdbus) with a custom 'build0.1' suffix that is not in any Ubuntu repo.
#    The -dev packages pin the exact repo version, so apt refuses to install
#    them until you downgrade those orphaned packages back to repo versions.
sudo apt update
sudo apt install libnl-3-dev libnl-genl-3-dev libnl-route-3-dev

# 5. Build
make all
```

After a successful build the binary is at `wpa_supplicant/wpa_supplicant`.
Verify `CONFIG_NAN_USD` was compiled in:

```bash
strings wpa_supplicant | grep -c nan_usd
# Should print a non-zero number
```

### 2.3 Install on the Commissioning Proxy Raspberry Pi

Copy the binary to the **Commissioning Proxy RPi only** and replace the system
binary:

```bash
# From the build machine
scp wpa_supplicant ubuntu@<proxy-rpi-ip>:/tmp/wpa_supplicant

# On the Commissioning Proxy RPi
ssh ubuntu@<proxy-rpi-ip>
sudo systemctl stop wpa_supplicant
sudo cp /tmp/wpa_supplicant /usr/sbin/wpa_supplicant
sudo systemctl start wpa_supplicant
```

After restart, confirm the service is running:

```bash
systemctl status wpa_supplicant
```

<hr>

## 3. Clone the connectedhomeip repository on the build machine

If you have not already cloned the `commissioning-proxy` forked branch, do so
now on the **build machine**.

Note: If this is a new build machine you will need to set up appropriate SSH
keys and upload to github

```bash
ssh-keygen -t ed25519 -C "build machine"
cat ~/.ssh/id_ed25519.pub
# Copy and Paste the certificate into github
```

Clone connectedhomeip repository

```bash
git clone git@github.com:simonhmorris1/connectedhomeip.git

cd connectedhomeip

./scripts/checkout_submodules.py --shallow --platform  linux

git checkout commissioning-proxy

# Need to do this first to get the subsequent apt-get to work
sudo apt-get update

sudo apt-get install git gcc g++ pkg-config cmake curl libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev \
     libevent-dev default-jre
```

<hr>

## 4. Build the Matter SDK (Raspberry Pi)

All Matter binaries for the RPi are cross-compiled on the build machine inside a
Docker container that provides the arm64 toolchain and sysroot. Or they can be
built natively on a RaspberryPi, but that requires at least 8GB of RAM.

### 4.1 One-time Docker image setup

Create a `Dockerfile.chip-cross-aarch64` in `~/scripts/`:

```bash
FROM ghcr.io/project-chip/chip-build-crosscompile:latest

# Enable arm64 packages
# archive.ubuntu.com only serves amd64; arm64 lives on ports.ubuntu.com.
# Pin existing sources to amd64, then add ports.ubuntu.com for arm64.
RUN dpkg --add-architecture arm64 \
 && if [ -f /etc/apt/sources.list.d/ubuntu.sources ]; then \
        sed -i '/^Types:/a Architectures: amd64' /etc/apt/sources.list.d/ubuntu.sources; \
    else \
        sed -i 's/^deb http/deb [arch=amd64] http/' /etc/apt/sources.list; \
    fi \
 && printf 'Types: deb\nURIs: http://ports.ubuntu.com/ubuntu-ports\nSuites: noble noble-updates noble-backports noble-security\nComponents: main restricted universe multiverse\nArchitectures: arm64\n' \
      > /etc/apt/sources.list.d/ubuntu-arm64.sources \
 && apt-get update

# Tooling (native amd64)
RUN apt-get install -y --no-install-recommends \
    git \
    cmake \
    ninja-build \
    pkg-config \
    curl \
    unzip \
    python3 \
    python3-venv \
    python3-dev \
    python3-pip \
    default-jre

# Cross toolchain + arm64 sysroot
RUN apt-get install -y --no-install-recommends \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    libc6-dev-arm64-cross \
    linux-libc-dev-arm64-cross

# Runtime / system libs for ARM64
RUN apt-get install -y --no-install-recommends \
    libssl-dev:arm64 \
    libdbus-1-dev:arm64 \
    libglib2.0-dev:arm64 \
    libavahi-client-dev:arm64 \
    libgirepository1.0-dev:arm64 \
    libcairo2-dev:arm64 \
    libreadline-dev:arm64

# Clean up
RUN rm -rf /var/lib/apt/lists/*
```

Build the image

```bash
docker build -t chip-cross-aarch64 \
    -f ~/scripts/Dockerfile.chip-cross-aarch64 \
    ~/scripts
```

This only needs to be run once (or after a Docker image update). The build can
take 10–20 minutes the first time.

> The base image `chip-build-crosscompile` already contains Clang, GN, ninja,
> and an Ubuntu 24.04 arm64 sysroot at `/opt/ubuntu-24.04-aarch64-sysroot`.

### 4.2 Build the Commissioning Proxy app (RPi arm64)

Ensure other submodules are updated

```
git submodule update --init --depth 1 third_party/uriparser/repo
```

Create and run this bash script, ~/scripts/build-commissioning-proxy-rpi.sh

```bash

#!/usr/bin/env bash
set -euo pipefail

REPO="$HOME/connectedhomeip"
IMAGE="chip-cross-aarch64"
ENV_VOL="chip-pw-env-$(id -u)"
UID_GID="$(id -u):$(id -g)"

# Ensure the env volume exists and is writable by your UID/GID
docker volume inspect "$ENV_VOL" >/dev/null 2>&1 || docker volume create "$ENV_VOL" >/dev/null

# Fix ownership of the env volume (runs as root inside container)
docker run --rm \
  -v "$ENV_VOL:/pw_env" \
  "$IMAGE" \
  bash -lc "mkdir -p /pw_env && chown -R $UID_GID /pw_env"

# Build as your user so outputs on the bind mount are owned by you
docker run --rm -t \
  --user "$UID_GID" \
  -v "$REPO:/workdir/connectedhomeip" \
  -v "$ENV_VOL:/pw_env" \
  -w /workdir/connectedhomeip \
  "$IMAGE" \
  bash -lc 'set -eo pipefail
set +u

git config --global --add safe.directory /workdir/connectedhomeip
git config --global --add safe.directory /workdir/connectedhomeip/third_party/pigweed/repo

export PW_PROJECT_ROOT=/workdir/connectedhomeip
export PW_ENVIRONMENT_ROOT=/pw_env

source scripts/bootstrap.sh

cd examples/commissioning-proxy-app/linux

# Keep GN args as a single line so quoting is unambiguous
gn gen out/rpi-arm64 --args='\''import("//with_pw_rpc.gni") target_os="linux" target_cpu="arm64" is_debug=true is_clang=true sysroot="/opt/ubuntu-24.04-aarch64-sysroot" system_libdir="lib/aarch64-linux-gnu"'\''


ninja -C out/rpi-arm64'

```

Output binary:

```
examples/commissioning-proxy-app/linux/out/rpi-arm64/chip-commissioning-proxy-app
```

### 4.3 Build chip-tool (RPi arm64)

chip-tool runs on the **Test Harness RPi** (not the build machine). Create and
run this bash script, ~/scripts/build-chip-tool-rpi.sh

```bash
#!/usr/bin/env bash
set -euo pipefail

REPO="$HOME/connectedhomeip"
IMAGE="chip-cross-aarch64"
ENV_VOL="chip-pw-env-$(id -u)"
UID_GID="$(id -u):$(id -g)"

# Ensure the env volume exists and is writable by your UID/GID
docker volume inspect "$ENV_VOL" >/dev/null 2>&1 || docker volume create "$ENV_VOL" >/dev/null

# Fix ownership of the env volume (runs as root inside container)
docker run --rm \
  -v "$ENV_VOL:/pw_env" \
  "$IMAGE" \
  bash -lc "mkdir -p /pw_env && chown -R $UID_GID /pw_env"

# Build as your user so outputs on the bind mount are owned by you
docker run --rm -t \
  --user "$UID_GID" \
  -v "$REPO:/workdir/connectedhomeip" \
  -v "$ENV_VOL:/pw_env" \
  -w /workdir/connectedhomeip \
  "$IMAGE" \
  bash -lc 'set -eo pipefail
set +u

git config --global --add safe.directory /workdir/connectedhomeip
git config --global --add safe.directory /workdir/connectedhomeip/third_party/pigweed/repo

export PW_PROJECT_ROOT=/workdir/connectedhomeip
export PW_ENVIRONMENT_ROOT=/pw_env

source scripts/bootstrap.sh

cd examples/chip-tool

gn gen out/rpi-arm64 --args='\''target_os="linux" target_cpu="arm64" is_debug=true is_clang=true sysroot="/opt/ubuntu-24.04-aarch64-sysroot" system_libdir="lib/aarch64-linux-gnu"'\''

ninja -C out/rpi-arm64'
```

Output binary:

```
examples/chip-tool/out/rpi-arm64/chip-tool
```

### 4.4 Build the End Device app (RPi arm64)

It is not necessary to rebuild the ED, this is here for completeness and to pull
in some of the optimisations made to PAF on this branch.

Create and run this bash script, ~/scripts/build-lighting-app-rpi.sh

```bash
#!/usr/bin/env bash
set -euo pipefail

REPO="$HOME/connectedhomeip"
IMAGE="chip-cross-aarch64"
ENV_VOL="chip-pw-env-$(id -u)"
UID_GID="$(id -u):$(id -g)"

# Ensure the env volume exists and is writable by your UID/GID
docker volume inspect "$ENV_VOL" >/dev/null 2>&1 || docker volume create "$ENV_VOL" >/dev/null

# Fix ownership of the env volume (runs as root inside container)
docker run --rm \
  -v "$ENV_VOL:/pw_env" \
  "$IMAGE" \
  bash -lc "mkdir -p /pw_env && chown -R $UID_GID /pw_env"

# Build as your user so outputs on the bind mount are owned by you
docker run --rm -t \
  --user "$UID_GID" \
  -v "$REPO:/workdir/connectedhomeip" \
  -v "$ENV_VOL:/pw_env" \
  -w /workdir/connectedhomeip \
  "$IMAGE" \
  bash -lc 'set -eo pipefail
set +u

git config --global --add safe.directory /workdir/connectedhomeip
git config --global --add safe.directory /workdir/connectedhomeip/third_party/pigweed/repo

export PW_PROJECT_ROOT=/workdir/connectedhomeip
export PW_ENVIRONMENT_ROOT=/pw_env

source scripts/bootstrap.sh

cd examples/lighting-app/linux

gn gen out/rpi-arm64 --args='\''target_os="linux" target_cpu="arm64" is_debug=true is_clang=true sysroot="/opt/ubuntu-24.04-aarch64-sysroot" system_libdir="lib/aarch64-linux-gnu"'\''

ninja -C out/rpi-arm64'
```

Output binary:

```
examples/lighting-app/linux/out/rpi-arm64/chip-lighting-app
```

<hr>

## 5. Deploy to Raspberry Pi

### Create the apps directory (if not present)

```bash
ssh ubuntu@<proxy-rpi-ip> 'mkdir -p ~/apps'
ssh ubuntu@<ed-rpi-ip>    'mkdir -p ~/apps'
ssh ubuntu@<th-rpi-ip>    'mkdir -p ~/apps'
```

### Copy the binaries

> **Note**: `scp` will fail with "dest open: Failure" if the target binary is
> currently running on the RPi. Stop the app before copying.

```bash
# Commissioning Proxy → DUT RPi
scp examples/commissioning-proxy-app/linux/out/rpi-arm64/chip-commissioning-proxy-app \
    ubuntu@<proxy-rpi-ip>:/home/ubuntu/apps/

# Lighting App → ED RPi
scp examples/lighting-app/linux/out/rpi-arm64/chip-lighting-app \
    ubuntu@<ed-rpi-ip>:/home/ubuntu/apps/

# chip-tool → TH RPi
scp examples/chip-tool/out/rpi-arm64/chip-tool \
    ubuntu@<th-rpi-ip>:/home/ubuntu/apps/
```

### Clear persisted state

```bash
ssh ubuntu@<proxy-rpi-ip> 'rm -f /tmp/chip_*'
ssh ubuntu@<ed-rpi-ip>    'rm -f /tmp/chip_*'
ssh ubuntu@<th-rpi-ip>    'rm -f /tmp/chip_*'
```

<hr>

## 6. Configuring the Commissioning Proxy

### 6.1 Start the Commissioning Proxy

On the **Proxy (DUT) RPi**, start the Commissioning Proxy. `freq_list=2437,5220`
selects channel 6 (2.4 GHz) — the default Matter PAF channel and channel 44
(5GHz):

```bash
/home/ubuntu/apps/chip-commissioning-proxy-app \
    --wifi \
    --wifipaf "freq_list=2437,5220" \
    --discriminator 3947
```

**Argument reference**

| Argument                      | Description                                                                                |
| ----------------------------- | ------------------------------------------------------------------------------------------ |
| `--wifi`                      | Enables Wi-Fi management via wpa_supplicant (required for PAF)                             |
| `--wifipaf "freq_list=<MHz>"` | Sets the NAN channel(s). Use `2437` for CH6 (2.4 GHz). For 5 GHz add e.g. `2437,5745,5220` |
| `--discriminator <value>`     | 12-bit value used to identify the proxy during its own commissioning                       |

### 6.2 Add the Commissioning Proxy onto the Fabric

Before the proxy can tunnel commissioning messages it must itself join the
Matter fabric. Run this from the **TH RPi**:

```bash
/home/ubuntu/apps/chip-tool pairing onnetwork 1998 20202021
```

`1998` is the node ID assigned to the proxy; `20202021` is the default PASE
passcode. You may use any non-zero node ID.

After commissioning completes chip-tool prints:

```
CHIP:TOO: Device commissioning completed with success
```

> **Tip**: If you restart the proxy app it re-reads the stored fabric credential
> automatically. You do not need to run `pairing onnetwork` again unless you
> have cleared `/tmp/chip_*`.

<hr>

## 7. Configuring the End Device (ED)

On the **End Device RPi**, start the lighting app. The `freq_list` should match
(or overlap) the proxy's. The ED should not be on any network as it is being
Commissioned

```bash
/home/ubuntu/apps/chip-lighting-app \
    --wifi \
    --wifipaf "freq_list=2437" \
    --discriminator 3840
```

The end device advertises itself as a NAN publisher. Check the log for:

```
WiFi-PAF: Starting NAN publish
```

> The ED discriminator (`3840` above) must match the value you pass to
> chip-tool's `pairing proxy` command in the next step.

<hr>

## 8. Scanning

The proxy supports scan and background NAN scanning so that commissioners can
discover nearby devices. Background scan results are cached and can be
subscribed to as needed.

For a concise command reference (syntax, argument tables, bitmaps) see
`linux/README.md §Foreground Scanning` and `§Background Scanning`.

### 8.1 Run a foreground scan

Annotated form (for reference):

```bash
/home/ubuntu/apps/chip-tool commissioningproxy \
    proxy-scan-request \
    8 \                           # Transport: 2 = BLE, 8=WiFiPAF
    1998 1 \                      # <proxy-node-id> <endpoint>
    --allow-large-payload true \  # Use TCP for the CASE session
    --WiFiBands 5 \               # WiFiBands: 1 = 2.4 GHz, 4 = 5 GHz, 5 = both
    --timeout 20                  # chip-tool response timeout in seconds
```

Copy-paste form:

```bash
/home/ubuntu/apps/chip-tool commissioningproxy proxy-scan-request 8 1998 1 --allow-large-payload true --WiFiBands 5 --timeout 20
```

### 8.2 Start a background scan

Annotated form (for reference):

```bash
/home/ubuntu/apps/chip-tool commissioningproxy \
    proxy-back-ground-scan-start-request \
    8   \    # Transport: 2 = BLE, 8=WiFiPAF
    0   \    # Background scan Timeout in seconds (0 = no expiry)
    5   \    # WiFiBands: 1 = 2.4 GHz, 4 = 5 GHz, 5 = both
    1998 1   # <proxy-node-id> <endpoint>
```

Copy-paste form:

```bash
/home/ubuntu/apps/chip-tool commissioningproxy proxy-back-ground-scan-start-request 8 0 5 1998 1
```

### 8.3 Read cached results

```bash
/home/ubuntu/apps/chip-tool commissioningproxy read cached-results 1998 1
```

The output lists discovered commissionees with their peer descriptor (address,
band, discriminator).

### 8.4 Stop a background scan

Annotated form (for reference):

```bash
/home/ubuntu/apps/chip-tool commissioningproxy \
    proxy-back-ground-scan-stop-request \
    8 \    # Transport: 2 = BLE, 8=WiFiPAF
    5 \    # WiFiBands: 1 = 2.4 GHz, 4 = 5 GHz, 5 = both
    1998 1 # <proxy-node-id> <endpoint>
```

Copy-paste form:

```bash
/home/ubuntu/apps/chip-tool commissioningproxy proxy-back-ground-scan-stop-request 8 5 1998 1
```

<hr>

## 9. Commissioning the End Device Through the Commissioning Proxy

For a concise command reference see
`linux/README.md §Commissioning a Device via the Proxy`.

With the Commissioning Proxy on the Fabric and the end device running, issue the
`pairing proxy` command from the **TH RPi**:

| Argument                | Description                                                          |
| ----------------------- | -------------------------------------------------------------------- |
| `node-id`               | Node ID to assign to the newly commissioned device                   |
| `wifi-ssid`             | SSID of the Wi-Fi network to provision on the device                 |
| `wifi-password`         | Password for that Wi-Fi network                                      |
| `ed-passcode`           | PASE passcode of the end device (default: `20202021`)                |
| `ed-discriminator`      | Discriminator of the end device (e.g. `3840`)                        |
| `proxy-node-id`         | Node ID assigned to the proxy at commissioning time (e.g. `1998`)    |
| `proxy-connect-timeout` | How long (seconds) to wait for the NAN session to form (default: 30) |

This commissions the device as node 1999, provisioning it onto `MyHomeWiFi`
through the proxy at node 1998.

Annotated form (for reference):

```bash
/home/ubuntu/apps/chip-tool pairing proxy \
    1999                \ # <node-id>
    "MyHomeWiFi"        \ # "<wifi-ssid>"
    "MyPassword123"     \ # "<wifi-password>"
    20202021            \ # <ed-passcode>
    3840                \ # <ed-discriminator>
    1998                \ # <proxy-node-id>
    5                     # <proxy-connect-timeout>
```

Copy-paste form:

```bash
/home/ubuntu/apps/chip-tool pairing proxy 1999 "MyHomeWiFi" "MyPassword123" 20202021 3840 1998 5
```

### What happens under the hood

1. chip-tool establishes a CASE session to the proxy over Matter (TCP/UDP).
2. chip-tool sends `ProxyConnectRequest` — the proxy starts a NAN subscribe for
   the end device's discriminator.
3. The end device's NAN publisher responds; the proxy completes the PAF
   handshake and returns `ProxyConnectResponse` with a session ID.
4. chip-tool runs PASE and commissioning, tunneling every packet through
   `ProxyMessageRequest` / `ProxyMessageResponse`.
5. The end device receives `ConnectNetwork`. With
   `CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE=1` (the default
   for the Linux lighting-app and commissioning-proxy-app — see
   `IMPLEMENTATION_README.md §Network Commissioning`), the ED sends
   `ConnectNetworkResponse(Success)` over NAN **before** starting Wi-Fi
   association, then defers the blocking D-Bus join.
6. The end device associates with Wi-Fi. The ED then closes its NAN publisher
   at PAFTP tx-idle and posts `kOperationalNetworkEnabled`, which triggers
   mDNS / operational advertisement; tearing down publish at the same time
   closes the PAFTP session per Matter spec §4.20.3.10 [4.780].
7. chip-tool sends `ProxyDisconnectRequest` to clean up the PAF session.

**Expected total time**: 30–60 seconds depending on Wi-Fi join speed.

<hr>

## 10. Troubleshooting

### Commissioning Proxy app fails to start with NAN errors

```
Error: WiFiPAF: NAN start failed
```

-   Confirm the USB Wi-Fi dongle is plugged in and visible: `lsusb` should list
    the dongle; `ip link` should show a second `wlan1` interface.
-   Confirm the patched wpa_supplicant is installed and running:
    `systemctl status wpa_supplicant` must show `active (running)`.
-   Check that `CONFIG_NAN_USD` was compiled in:
    `strings /usr/sbin/wpa_supplicant | grep -c nan_usd` should return > 0.
-   Do not use the on-board RPi Wi-Fi for PAF — it does not support NAN
    USD. The USB dongle must be the interface wpa_supplicant manages.
-   ensure iwconfig works, See Section 1.2.1 USB dongle software
-   Ensure `~/script/config_paf_env.sh comee` succeeds

### Proxy Connect times out (commissioner gets `Status::Timeout`)

The proxy waited for the connect timeout for the NAN session and gave up. Common
causes:

-   End device is not running or is on a different NAN channel. Confirm both
    sides use the same `freq_list` value.
-   End device RPi also needs a USB Wi-Fi dongle with NAN support (same hardware
    requirement as the proxy).
-   NAN frequency not allowed by regulatory domain. `freq_list=2437` (channel 6,
    2.4 GHz) is permitted in all regions.

### PAF session closes mid-commissioning

Look for `WiFiPAFCloseSession` in the proxy log. The PAFTP ack-receive timer (30
s) fired. The most common cause is the end device's radio going off-channel
while it scans for and joins the Wi-Fi network — this can sometimes happen and
the 30 s timeout is designed to survive it. If it fires faster than ~15 s, RF
range or wpa_supplicant configuration is the likely cause. Try again.

### `ConnectNetwork` takes a long time (step 6 slow)

`ConnectNetwork` can take 10–15 s while the end device scans for and joins the
Wi-Fi AP. This is normal. chip-tool's exchange timeout is automatically extended
by the proxy for this step. On Linux builds with
`CHIP_DEVICE_CONFIG_WIFIPAF_EARLY_CONNECT_NETWORK_RESPONSE=1`, the ED returns
`ConnectNetworkResponse(Success)` over PAF before association starts and
recovers from association failure via fail-safe expiry.

### `ProxyMessageRequest` returns `Failure` ~35 s in

The proxy enforces a hard per-message deadline equal to the spec-defined
`responseTimeout` (`HandleProxyMessageRequest` →
`ProxyMessageResponseTimeoutCallback`). If the ED never replies, the proxy
returns `Status::Failure` to chip-tool instead of waiting for the longer
PAFTP ack-receive timeout. Check the ED log: if the ED is busy with WiFi
association and not draining its PAFTP receive queue, the request will not
complete in time.

### chip-tool `pairing proxy` command not found

You are running an upstream chip-tool build that does not include the
`commissioning-proxy` branch. Build chip-tool from this repository
(`commissioning-proxy` branch) using `~/scripts/build-chip-tool-rpi.sh`.

### Key log strings:

| String                           | Where | Meaning                                                   |
| -------------------------------- | ----- | --------------------------------------------------------- |
| `WiFiPAFMessageReceived`         | Proxy | Received a PAF frame from the end device                  |
| `OnProxyWiFiPAFMessageReceived`  | Proxy | Routing PAF frame to `ProxyMessageResponse`               |
| `WiFiPAFCloseSession`            | Proxy | PAF session closed — look for `ack-received timer` nearby |
| `InvokeCommandResponse`          | Proxy | Sent `ProxyMessageResponse` to commissioner               |
| `WiFi-PAF: Starting NAN publish` | ED    | End device is advertising via NAN                         |
| `Outbound message (N) done`      | ED    | wpa_supplicant accepted a NAN frame for TX                |
| `Cluster=0x0031 Command=0x0006`  | ED    | `ConnectNetwork` command received                         |
