# Installing Tizen on Raspberry Pi 4

This guide covers how to install Tizen OS on a Raspberry Pi 4 and configure
Wi-Fi connectivity.

## Prerequisites

-   Linux host, or Windows with WSL Ubuntu
-   MicroSD card reader
-   Docker Compose environment configured (see
    [Building for Tizen](./building.md#docker-compose-environment))

## Download Tizen Images

Download images from the Tizen release page:

<https://download.tizen.org/releases/milestone/TIZEN/Tizen-10.0/Tizen-10.0-Unified/>

Select the latest version, then navigate to `images/standard/` and download:

-   `tizen-10.0-boot-image.tar.gz` – Bootloader image
-   `tizen-10.0-headed-image.tar.gz` – Headed (GUI) OS image

:::{note} Always check for the latest version. The URLs above are examples and
may not reflect the most recent release. :::

## Flashing the MicroSD Card

The flashing process uses the `sd_fusing.py` script. Clone it from
<https://git.tizen.org/cgit/platform/kernel/tizen-fusing-scripts>:

```bash
git clone https://git.tizen.org/cgit/platform/kernel/tizen-fusing-scripts
cd tizen-fusing-scripts
# Script is at: scripts/sd_fusing.py
```

For reference, see the official Tizen documentation at
<https://docs.stg.tizen.org/platform/developing/flashing-rpi/>.

### On Ubuntu / Linux

1. Insert the MicroSD card into your card reader.
2. Find the device with `lsblk` (look for the one matching your card size):

    ```bash
    lsblk
    ```

3. Wipe existing partitions (replace `/dev/sdYourCard` with your actual device,
   e.g., `/dev/sde`):

    ```bash
    sudo wipefs -a /dev/sdYourCard
    ```

4. Flash the card:

    ```bash
    # Format the card layout for Raspberry Pi 4
    sudo ./sd_fusing.py -d /dev/sdYourCard -t rpi4 --format

    # Flash the bootloader and OS images
    sudo ./sd_fusing.py -d /dev/sdYourCard -b tizen-10.0-boot-image.tar.gz tizen-10.0-headed-image.tar.gz -t rpi4
    ```

5. Safe eject:

    ```bash
    sync
    ```

    You can now insert the card into your Raspberry Pi 4 and boot it up.

### On Windows + WSL 2 (Optional)

Since Windows often locks raw storage drives, we use `usbipd` to pass the USB
card reader directly into the Linux kernel inside WSL.

#### 1. Identify and Attach the USB Device (Windows PowerShell)

Open **PowerShell as Administrator** to find and bind your USB card reader.

**List all USB devices:**

```powershell
usbipd list
```

**Force Windows to release the card reader** (using your device's specific
BUSID, for example, `3-21`):

```powershell
usbipd bind --busid 3-21 --force
```

**Attach the device to WSL:**

```powershell
usbipd attach --busid 3-21 --wsl
```

#### 2. Flash the Card (WSL Ubuntu Terminal)

Switch to your **WSL (Ubuntu)** terminal and follow the same flashing steps as
in the Ubuntu section above. After flashing, run `sync` in WSL, then go back to
PowerShell and run `usbipd detach --busid 3-21`.

## Configuring Wi-Fi

The standard Tizen image lacks the Broadcom Wi-Fi firmware for the Raspberry
Pi 4. To install the missing firmware, you need access to the device console via
**SDB (Smart Development Bridge)**.

### Connect via SDB

The `sdb` tool is available inside the Tizen Docker container, or you can
install the
[Tizen VS Code Extension](./vscode_setup.md#tizen-extension-installation) to use
`sdb` from your host machine.

To use `sdb` from Docker:

```bash
docker compose run --rm tizen bash
```

Find your Raspberry Pi's IP address from your home router, then connect and
switch to root:

```bash
sdb connect RASPBERRY_PI_IP
sdb root on
sdb shell
```

### Upload and Install the Wi-Fi Firmware

The Wi-Fi firmware files are available from the
[RPi-Distro/firmware-nonfree](https://github.com/RPi-Distro/firmware-nonfree.git)
repository:

```bash
git clone https://github.com/RPi-Distro/firmware-nonfree.git
```

The Tizen root file system is mounted as **Read-Only** by default. We must
temporarily change it to **Read-Write** to add the missing driver files.

**On the Raspberry Pi console (`sdb shell`):**

Unlock the file system and create the firmware directory:

```bash
mount -o remount,rw /
mkdir -p /lib/firmware/brcm
```

**On your local terminal (second window):**

Copy the physical firmware files from the cloned `firmware-nonfree` repository
into the temporary RAM directory (`/tmp`) of the Raspberry Pi:

```bash
cd firmware-nonfree/debian/config/brcm80211/brcm/

# Push the physical binary, clm_blob, and txt configuration files
sdb push ../cypress/cyfmac43455-sdio-standard.bin /tmp/cyfmac43455-sdio.bin
sdb push ../cypress/cyfmac43455-sdio.clm_blob /tmp/cyfmac43455-sdio.clm_blob
sdb push brcmfmac43455-sdio.txt /tmp/brcmfmac43455-sdio.txt
```

**Back on the Raspberry Pi console (`sdb shell`):**

Move the files from `/tmp` into the correct system paths, renaming them to match
what the Linux kernel expects for the Raspberry Pi 4 model:

```bash
cd /lib/firmware/brcm/

cp /tmp/cyfmac43455-sdio.bin brcmfmac43455-sdio.bin
cp /tmp/cyfmac43455-sdio.bin brcmfmac43455-sdio.raspberrypi,4-model-b.bin

cp /tmp/cyfmac43455-sdio.clm_blob brcmfmac43455-sdio.clm_blob
cp /tmp/cyfmac43455-sdio.clm_blob brcmfmac43455-sdio.raspberrypi,4-model-b.clm_blob

cp /tmp/brcmfmac43455-sdio.txt brcmfmac43455-sdio.txt
cp /tmp/brcmfmac43455-sdio.txt brcmfmac43455-sdio.raspberrypi,4-model-b.txt

# Lock the system back to Read-Only and restart
mount -o remount,ro /
reboot
```

After the reboot, the Raspberry Pi kernel successfully detects the wireless
chip, creating the `wlan0` interface. You can now scan and connect to your local
network directly inside the Tizen graphical user interface (GUI).

## Additional Resources

-   [Tizen flashing documentation](https://docs.stg.tizen.org/platform/developing/flashing-rpi/)
-   [sd_fusing.py script source](https://git.tizen.org/cgit/platform/kernel/tizen-fusing-scripts)
-   [Wi-Fi firmware source](https://github.com/RPi-Distro/firmware-nonfree)
-   [Samsung Tizen OS device configuration guide](https://samsungtizenos.com/docs/application/flutter/guides/flutter-tizen/doc/configure-device)
