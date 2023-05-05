# Matter Software Requirements

## **System Agnostic Software Requirements (Windows/Mac/Linux):**

1. SSH Client ([PuTTY](https://www.putty.org/), Terminal, or similar):
    
    SSH client is used to communicate with the Raspberry Pi over a secure
    shell.

2. [Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/)
    
    Raspberry Pi Disk Imager is used to flash the SD Card that contains the
    operating system for the Raspberry Pi. Note that, for the Thread demo, the
    operating system is the demo image.

3. Flash Tools

    - [Flash tool for EFR32 Device](../general/FLASH_SILABS_DEVICE.md)

    Simplicity Commander standalone or Simplicity Studio is used to flash
    Silicon Labs hardware with firmware images for the RCP and the Matter
    Accessory Device.

    - [Flash tool for SiWx917 SoC Device](../general/FLASH_SILABS_SiWx917_SOC_DEVICE.md)

    The Ozone Debugger can be configured to flash the application for the SiWx917 SoC device.
	
    > **Note:** 
    >    1. Simplicity Studio and Commander tools support is not yet added for the SiWx917 SoC device.
    >    2. The SiWx917 SoC device is not yet officially added in Ozone, but it can be configured in Ozone.

4. [Git Version Control ](https://git-scm.com/downloads)[only required for
   building images and development]

    Clone the Silicon Labs Matter repo, then enter and sync all the necessary
    submodules with the following commands:

    ```shell
    $ git clone https://github.com/SiliconLabs/matter.git
    ```

    ```shell
    $ cd matter
    ```

    ```shell
    $ ./scripts/checkout_submodules.py --shallow --recursive --platform efr32
    ```

5. If you are using an EFR32MG2x device you will require a bootloader to run the
   demo applications. When you flash your application image be sure to include a
   bootloader as well (if one is not already present). Bootloader images are provided on the [Matter Artifacts page](../general/ARTIFACTS.md).

## **Windows-Specific Software Requirements:**

&emsp; These requirements are in addition to those mentioned above, for Windows
only.

&emsp; A Unix-like command line:

-   [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/about)
    **OR**
-   [VirtualBox](https://www.virtualbox.org/) with
    [Ubuntu 20.04.x LTS](https://ubuntu.com/download/desktop)

    > WSL or Virtual Box are used to emulate a virtual Linux machine, which is
    > useful for accessing the Linux command line tools

## **Mac-Specific Requirements (building own images):**

&emsp; These requirements are in addition to those mentioned above for Mac OS
users building their own images.

1. Install Homebrew

    ```shell
    $ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

2. Install pkg-config, openssl, git-lfs

    ```shell
    $ brew install pkg-config openssl git-lfs
    ```

3. Install ARM GNU Toolchain (https://developer.arm.com/downloads/-/gnu-rm)
4. Add ARM GCC toolchain to the search path within `bootstrap.sh` by adding this
   line of code:

    ```shell
    $ export PATH="/Applications/ARM/bin:$PATH"
    ```
    
    Depending on your Mac processor, you will have to complete the following additional steps.

### **Apple Silicon (M1) Software Requirements:**

Users may also have to specify which `pkg-config`/`openssl` to use by adding
these lines of code to `bootstrap.sh`:

```shell
$ export PATH="/opt/homebrew/opt/openssl@3/bin:$PATH"
```

```shell
$ export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig"
```

### **Intel Software Requirements:**

```shell
$ cd /usr/local/lib/pkgconfig
```

```shell
$ ln -s ../../Cellar/openssl@1.1/1.1.1g/lib/pkgconfig/* .
```

`openssl@1.1/1.1.1g` may need to be replaced with the actual version of OpenSSL
installed by Brew.

If using MacPorts, the command 

```shell
$ port install openssl
```

is sufficient to satisfy this
dependency.

## **Linux Specific Requirements:**

On Debian-based Linux distributions such as Ubuntu, these dependencies can be
satisfied with the following:

```shell
$ sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

## **Wi-Fi RS9116 Specific Requirements:**

Before you run the demo or development on the RS9116 please be sure that you
update the RS9116 firmware.

Pre-Built RS9116 firmware is available in the github repository under `third_party/silabs/wiseconnect-wifi-bt-sdk/firmware`

1. [Setting up TeraTerm](https://docs.silabs.com/rs9116/wiseconnect/2.0/tera-term-setup)
2. [Updating the RS9116 Firmware](https://docs.silabs.com/rs9116/wiseconnect/2.0/update-evk-firmware)


## **Wi-Fi SiWx917 Specific Requirements:**

Before you run the demo or development on the SiWx917 please be sure that you
update the SiWx917 firmware.

The WiseMCU Combo SDK package is only available to Alpha customers. Please contact Silicon Labs support.

The WiseMCU Combo SDK package version used in this release is **A.1.0.0.0.23**.

On obtaining the package, extract it, rename the folder as `wisemcu-wifi-bt-sdk` and copy it in the github repository under `third_party/silabs`.

Pre-Built SiWx917 firmware is available under `third_party/silabs/wisemcu-wifi-bt-sdk/connectivity_firmware`

1. [Setting up TeraTerm](https://docs.silabs.com/SiWx917/wiseconnect/2.0/tera-term-setup)
2. For updating the SiWx917 NCP Firmware, refer [Updating the RS9116 Firmware](https://docs.silabs.com/rs9116/wiseconnect/2.0/update-evk-firmware). Instructions are the same for both SiWx917 and RS9116.
3. For updating the SiWx917 SoC Firmware, refer [Updating the SiWx917 SoC Firmware](../wifi/SiWx917_SoC_FWUpdate.md)


---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)
