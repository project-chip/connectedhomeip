# Installing Tizen on Raspberry Pi 4

This guide covers how to install Tizen OS on a Raspberry Pi 4 and configure
Wi-Fi connectivity.

## Download Tizen Images

Download images from the Tizen release page:

<https://download.tizen.org/releases/milestone/TIZEN/Tizen-10.0/Tizen-10.0-Unified/>

Select the latest version, then navigate to `images/standard/` and download:

- `tizen-10.0-boot-image.tar.gz` – Bootloader image
- `tizen-10.0-headed-image.tar.gz` – Headed (GUI) OS image

:::{note}
Always check for the latest version. The URLs above are examples and may not
reflect the most recent release.
:::

## Part 1: Preparing the MicroSD Card via WSL 2

Since Windows often locks raw storage drives, we use `usbipd` to pass the USB
card reader directly into the Linux kernel inside WSL.

### 1. Identify and Attach the USB Device (Windows PowerShell)

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

### 2. Format and Flash the Card (WSL Terminal)

Switch to your **WSL (Ubuntu)** terminal to prepare the storage.

**Verify the drive letter:**

Run `lsblk` to find your card (look for the one showing ~29G). Let's assume it
is `/dev/sde`.

**Wipe existing partitions:**

```bash
sudo wipefs -a /dev/sde
```

**Run the Tizen flashing script:**

Navigate to your workspace where the Samsung scripts and downloaded Tizen images
(`boot` and `headed` `.tar.gz` files) are located, then run:

```bash
# 1. Format the card layout for Raspberry Pi 4
sudo ./sd_fusing.py -d /dev/sde -t rpi4 --format

# 2. Flash the bootloader and OS images
sudo ./sd_fusing.py -d /dev/sde -b tizen-10.0-boot-image.tar.gz tizen-10.0-headed-image.tar.gz -t rpi4
```

**Safe Eject:**

Run `sync` in WSL, then go back to PowerShell and run
`usbipd detach --busid 3-21`. You can now insert the card into your Raspberry Pi
4 and boot it up.

## Part 2: Accessing the Tizen Console & Fixing Wi-Fi

The standard Tizen image lacks the Broadcom Wi-Fi firmware for the Raspberry Pi
4. To fix this, you must access the system console using **SDB (Smart
Development Bridge)**.

### 1. Connect via SDB

The `sdb` tool is available inside the Tizen Docker container, or can be
installed locally via the
[Tizen VS Code Extension](./vscode_setup.md#tizen-extension-installation).

Find your Raspberry Pi's IP address from your home router (e.g.,
`192.168.0.117`).

Connect to the device:

```bash
cd ~/.tizen-extension-platform/server/sdktools/data/tools/
./sdb connect 192.168.0.117
./sdb shell
```

### 2. Upload and Install the Wi-Fi Firmware

The Tizen root file system is mounted as **Read-Only** by default. We must
temporarily change it to **Read-Write** to add the missing driver files.

**On the Raspberry Pi console (`sdb shell`):**

Switch to the root administrator and unlock the system:

```bash
su root
# Enter password 'tizen' if prompted
mount -o remount,rw /
mkdir -p /lib/firmware/brcm
```

**On your local WSL terminal (second window):**

Since `sdb` struggles with copying Linux symbolic links directly, copy the
actual physical firmware files from your local git repository
(`firmware-nonfree`) into the temporary RAM directory (`/tmp`) of the Raspberry
Pi:

```bash
cd ~/workspace/matter-workspace/firmware-nonfree/debian/config/brcm80211/brcm/

# Push the physical binary, clm_blob, and txt configuration files
~/.tizen-extension-platform/server/sdktools/data/tools/sdb push ../cypress/cyfmac43455-sdio-standard.bin /tmp/cyfmac43455-sdio.bin
~/.tizen-extension-platform/server/sdktools/data/tools/sdb push ../cypress/cyfmac43455-sdio.clm_blob /tmp/cyfmac43455-sdio.clm_blob
~/.tizen-extension-platform/server/sdktools/data/tools/sdb push brcmfmac43455-sdio.txt /tmp/brcmfmac43455-sdio.txt
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

## Alternative: Native Linux Host

If you are working on a native Linux host (not WSL), the process is simpler:

1. Insert the MicroSD card into your card reader.
2. Find the device with `lsblk` (e.g., `/dev/sde`).
3. Flash using the same `sd_fusing.py` commands as above.
4. Eject the card, insert into Raspberry Pi, and boot.

For SDB access from a native Linux host, ensure the `sdb` tool is in your
`PATH` (installed via the Tizen VS Code extension or Tizen Studio).

## Additional Resources

- [Samsung Tizen OS device configuration guide](https://samsungtizenos.com/docs/application/flutter/guides/flutter-tizen/doc/configure-device)
